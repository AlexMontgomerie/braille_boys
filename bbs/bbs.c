#include "bbs.h"
#include "sdk_config.h"
#include "nrf_drv_gpiote.h"
#include "SEGGER_RTT.h"
#include "fifo.h"
#include "ble_nus.h"
#include "char_conversion.h"

ble_nus_t m_nus; 

/**
 * Global fifo structures
 */
buffer_t temp_raw_buffer; 
buffer_t temp_braille_buffer;
buffer_t temp_message_buffer;

buffer_t* 	m_raw_fifo = &temp_raw_buffer;
buffer_t* 	m_braille_fifo = &temp_braille_buffer;
buffer_t*		m_message_fifo = &temp_message_buffer;



/**
 * Static memory initialisation for buffers
 */
uint8_t		raw_buf[BUFF_SIZE];
uint8_t 	braille_buf[BUFF_SIZE];
uint8_t 	message_buf[BUFF_SIZE];

/**
 * Pointers to characters being pulled from the buffers
 */
uint8_t temp_raw_char;
uint8_t temp_braille_char;
uint8_t temp_message_char;

uint8_t* p_raw_char = &temp_raw_char;
uint8_t* p_braille_char = &temp_braille_char;
uint8_t* p_message_char = &temp_message_char;

uint8_t BBS_STATUS = 0;

void log_braille_status(void) {
	SEGGER_RTT_printf(0,"Braille in: ");
	
	SEGGER_RTT_printf(0,"%d",BIT_CHECK(BBS_STATUS,5));
	SEGGER_RTT_printf(0,"%d",BIT_CHECK(BBS_STATUS,4));
	SEGGER_RTT_printf(0,"%d",BIT_CHECK(BBS_STATUS,3));
	SEGGER_RTT_printf(0,"%d",BIT_CHECK(BBS_STATUS,2));
	SEGGER_RTT_printf(0,"%d",BIT_CHECK(BBS_STATUS,1));
	SEGGER_RTT_printf(0,"%d",BIT_CHECK(BBS_STATUS,0));
	
	SEGGER_RTT_printf(0,"\r\n");
	return;
}

void log_string(uint8_t * p_string, uint16_t length) {
	SEGGER_RTT_printf(0,"STRING: ");
	int i = 0;
	for(i=0;i<length;i++) {
		SEGGER_RTT_printf(0,"%c",p_string[i]);
	}
	SEGGER_RTT_printf(0,"\r\n");
	return;
}

uint32_t clear_msg(void){
	BBS_STATUS &= ~MSG_BIT_MASK;
}

void update_msg_buf(void) {

	SEGGER_RTT_printf(0,"Updating message buffer with ");
	log_braille_status();
	
	uint32_t err_code;
	
	err_code = fifo_put(m_message_fifo, braille_2_char(BBS_STATUS & MSG_BIT_MASK,48));
	err_code = clear_msg();
	return;
}

void receive_char(void) {
	uint32_t err_code;
	err_code = fifo_get(m_braille_fifo,p_braille_char);
	switch(err_code) {
		//Buffer is empty (nothing to get)
		case BUFF_EMPTY:
			//probably should do something here
		break;
		
		//have got character out of buffer
		case NRF_SUCCESS:
			err_code = clear_output();
			err_code = set_output(p_braille_char);
		break;
		
		default: break;
		
	}
}

void send_string(void) {
	
SEGGER_RTT_printf(0,"Sending String \r\n");
	
	uint32_t err_code = NRF_SUCCESS;
	uint8_t string_buf[BUFF_SIZE+1];
	uint8_t * p_string = &string_buf[0];
	uint16_t length = 0;
	
	
	while(err_code!=BUFF_EMPTY) {
		err_code = fifo_get(m_message_fifo, p_message_char);
		if(err_code == BUFF_EMPTY) break;
		*(p_string + (uint8_t)length) = *p_message_char;
		length++;
	}
	*(p_string + (uint8_t)length) = '\r';
	length++;
	*(p_string + (uint8_t)length) = '\n';
	SEGGER_RTT_printf(0,"Sending ");
	log_string(p_string, length);
	err_code = ble_nus_string_send(&m_nus, p_string, length);
	SEGGER_RTT_printf(0,"sending error: %d \r\n",err_code);
	return;
}

void pin_control_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
#if	action == NRF_GPIOTE_POLARITY_LOTOHI
	uint32_t err_code;
	switch(pin) {
		/**
		 *	Case for when switching between tx and rx mode
		 */
		case CONTROL_INPUT_0:
			rxtx_toggle();
		break;
		
		/**
		 *	Case to tap through the received characters, or send a character
		 */
		case CONTROL_INPUT_1:
			
			//if in RX mode ...
			if(BIT_CHECK(BBS_STATUS,RXTX_MASK)) {
				receive_char();
			}
			
			//else in TX mode ...
			else {
				update_msg_buf();
			}
		break;
		
		case CONTROL_INPUT_2:
			//if in TX mode ...
			if(!BIT_CHECK(BBS_STATUS,RXTX_MASK)) {
				send_string();
			}			
		break;

		case CONTROL_INPUT_3:
		break;
		
		default: break;
	}
#endif
	return;
}

/**
 *	Event handler to update input pins
 *	@brief will determine input message
 */

void pin_message_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
#if	action == NRF_GPIOTE_POLARITY_LOTOHI
	switch(pin) {
		
		case BRAILLE_INPUT_0:
			SEGGER_RTT_printf(0,"Setting message bit 0 \r\n");
			BIT_SET(BBS_STATUS,MSG_BIT_0);
		break;
		
		case BRAILLE_INPUT_1:
			BIT_SET(BBS_STATUS,MSG_BIT_1);
		break;
		
		case BRAILLE_INPUT_2:
			BIT_SET(BBS_STATUS,MSG_BIT_2);
		break;

		case BRAILLE_INPUT_3:
			BIT_SET(BBS_STATUS,MSG_BIT_3);
		break;
		
		case BRAILLE_INPUT_4:
			BIT_SET(BBS_STATUS,MSG_BIT_4);
		break;
		
		case BRAILLE_INPUT_5:
			BIT_SET(BBS_STATUS,MSG_BIT_5);
		break;		
	}
#endif
	return;
}
 

/** 
 *	Initialising the input pins
 *	FIX ME its a bit messy, can make prettier
 */

uint32_t add_input_pins(void)
{
	nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
	config.pull = NRF_GPIO_PIN_PULLUP;
	
	uint32_t err_code;
	
	/**
	 *	Initialise all the message input pins
	 */
	
	
	//for INPUT_0
	err_code = nrf_drv_gpiote_in_init(BRAILLE_INPUT_0, &config, pin_message_event_handler);
	if (err_code != NRF_SUCCESS)
	{	
		APP_ERROR_CHECK(err_code);
	}
	nrf_drv_gpiote_in_event_enable(BRAILLE_INPUT_0, true);

/*
	//for INPUT_1
	err_code = nrf_drv_gpiote_in_init(BRAILLE_INPUT_1, &config, pin_message_event_handler);
	if (err_code != NRF_SUCCESS)
	{	
		APP_ERROR_CHECK(err_code);
	}
	nrf_drv_gpiote_in_event_enable(BRAILLE_INPUT_1, true);	

	
	//for INPUT_2
	err_code = nrf_drv_gpiote_in_init(BRAILLE_INPUT_2, &config, pin_tx_event_handler);
	if (err_code != NRF_SUCCESS)
	{	
		APP_ERROR_CHECK(err_code);
	}
	nrf_drv_gpiote_in_event_enable(BRAILLE_INPUT_2, true);		
	//for INPUT_3
	err_code = nrf_drv_gpiote_in_init(BRAILLE_INPUT_3, &config, pin_tx_event_handler);
	if (err_code != NRF_SUCCESS)
	{	
		APP_ERROR_CHECK(err_code);
	}
	nrf_drv_gpiote_in_event_enable(BRAILLE_INPUT_3, true);

	
	//for INPUT_4
	err_code = nrf_drv_gpiote_in_init(BRAILLE_INPUT_4, &config, pin_tx_event_handler);
	if (err_code != NRF_SUCCESS)
	{	
		APP_ERROR_CHECK(err_code);
	}
	nrf_drv_gpiote_in_event_enable(BRAILLE_INPUT_4, true);	
	//for INPUT_5
	err_code = nrf_drv_gpiote_in_init(BRAILLE_INPUT_5, &config, pin_tx_event_handler);
	if (err_code != NRF_SUCCESS)
	{	
		APP_ERROR_CHECK(err_code);
	}
	nrf_drv_gpiote_in_event_enable(BRAILLE_INPUT_5, true);	
	*/
	
	/**
	 *	Initialise all the control input pins
	 */
	//for CONTROL_0
	err_code = nrf_drv_gpiote_in_init(CONTROL_INPUT_0, &config, pin_control_event_handler);
	if (err_code != NRF_SUCCESS)
	{	
		APP_ERROR_CHECK(err_code);
	}
	nrf_drv_gpiote_in_event_enable(CONTROL_INPUT_0, true);	
	
	
	//for CONTROL_1
	err_code = nrf_drv_gpiote_in_init(CONTROL_INPUT_1, &config, pin_control_event_handler);
	if (err_code != NRF_SUCCESS)
	{	
		APP_ERROR_CHECK(err_code);
	}
	nrf_drv_gpiote_in_event_enable(CONTROL_INPUT_1, true);	
	
	
	//for CONTROL_2
	err_code = nrf_drv_gpiote_in_init(CONTROL_INPUT_2, &config, pin_control_event_handler);
	if (err_code != NRF_SUCCESS)
	{	
		APP_ERROR_CHECK(err_code);
	}
	nrf_drv_gpiote_in_event_enable(CONTROL_INPUT_2, true);	

/*	
	//for CONTROL_3
	err_code = nrf_drv_gpiote_in_init(CONTROL_INPUT_3, &config, pin_control_event_handler);
	if (err_code != NRF_SUCCESS)
	{	
		APP_ERROR_CHECK(err_code);
	}
	nrf_drv_gpiote_in_event_enable(CONTROL_INPUT_3, true);
	*/
	return err_code;
}

/** 
 *	Initialising the output pins
 */

uint32_t add_output_pins(void)
{
	//FIX ME
	uint32_t err_code;
	
	nrf_drv_gpiote_out_config_t config = GPIOTE_CONFIG_OUT_SIMPLE(true);
	
	//MOTOR 0
	err_code = nrf_drv_gpiote_out_init(BRAILLE_MOTOR_0, &config);
	if (err_code != NRF_SUCCESS)
	{	
		APP_ERROR_CHECK(err_code);
	}
	
	//MOTOR 1
	err_code = nrf_drv_gpiote_out_init(BRAILLE_MOTOR_1, &config);
	if (err_code != NRF_SUCCESS)
	{	
		APP_ERROR_CHECK(err_code);
	}
	
	//MOTOR 2
	err_code = nrf_drv_gpiote_out_init(BRAILLE_MOTOR_2, &config);
	if (err_code != NRF_SUCCESS)
	{	
		APP_ERROR_CHECK(err_code);
	}
	
	//MOTOR 3
	err_code = nrf_drv_gpiote_out_init(BRAILLE_MOTOR_3, &config);
	if (err_code != NRF_SUCCESS)
	{	
		APP_ERROR_CHECK(err_code);
	}
	/*
	//MOTOR 4
	err_code = nrf_drv_gpiote_out_init(BRAILLE_MOTOR_4, &config);
	if (err_code != NRF_SUCCESS)
	{	
		APP_ERROR_CHECK(err_code);
	}
	
	//MOTOR 5
	err_code = nrf_drv_gpiote_out_init(BRAILLE_MOTOR_5, &config);	
	if (err_code != NRF_SUCCESS)
	{	
		APP_ERROR_CHECK(err_code);
	}
	*/
	return err_code;
}

uint32_t clear_output(void) {
	
	//clear all outputs
	nrf_drv_gpiote_out_clear(BRAILLE_MOTOR_0);
	nrf_drv_gpiote_out_clear(BRAILLE_MOTOR_1);
	nrf_drv_gpiote_out_clear(BRAILLE_MOTOR_2);
	nrf_drv_gpiote_out_clear(BRAILLE_MOTOR_3);
	/*
	nrf_drv_gpiote_out_clear(BRAILLE_MOTOR_4);
	nrf_drv_gpiote_out_clear(BRAILLE_MOTOR_5);
	*/
	return NRF_SUCCESS;
}

uint32_t set_output(uint8_t * p_char) {
	uint32_t err_code;
	
	if(BIT_CHECK(*p_char,0)) {
		nrf_drv_gpiote_out_set(BRAILLE_MOTOR_0);
	}
	
	if(BIT_CHECK(*p_char,1)) {
		nrf_drv_gpiote_out_set(BRAILLE_MOTOR_1);
	}
		
	if(BIT_CHECK(*p_char,2)) {
		nrf_drv_gpiote_out_set(BRAILLE_MOTOR_2);
	}
		
	if(BIT_CHECK(*p_char,3)) {
		nrf_drv_gpiote_out_set(BRAILLE_MOTOR_3);
	}
	/*
	if(BIT_CHECK(*p_char,4)) {
		nrf_drv_gpiote_out_set(BRAILLE_MOTOR_0);
	}
		
	if(BIT_CHECK(*p_char,5)) {
		nrf_drv_gpiote_out_set(BRAILLE_MOTOR_0);
	}
	*/
	return NRF_SUCCESS;
}

void rxtx_toggle(void) {
	if(BIT_CHECK(BBS_STATUS,RXTX_MASK)) {
		SEGGER_RTT_printf(0,"Now TX mode \r\n");
		BIT_CLEAR(BBS_STATUS,RXTX_MASK);
		clear_output();
	}
	else {
		SEGGER_RTT_printf(0,"Now RX mode \r\n");
		BIT_SET(BBS_STATUS,RXTX_MASK);		
	}
}


/** Function to initialise IO and braille service related buffers
*/
void bbs_init(void)
{

	//initialise gpio tasks and events
	uint32_t err_code;
	err_code = nrf_drv_gpiote_init();
  APP_ERROR_CHECK(err_code);

	//adding input pins SEGGER_RTT_printf(0,"\r\nadding input pins!\r\n");	
	err_code = add_input_pins();
	APP_ERROR_CHECK(err_code);
	
  // adding output pins SEGGER_RTT_printf(0,"\r\nadding output pins!\r\n");	
	err_code = add_output_pins();
	APP_ERROR_CHECK(err_code);	
	
	// adding fifo buffers SEGGER_RTT_printf(0,"\r\nadding fifo buffers!\r\n");	
	err_code = add_fifo(m_raw_fifo, &raw_buf[0]);
	APP_ERROR_CHECK(err_code);
	
	err_code = add_fifo(m_braille_fifo, &braille_buf[0]);
	APP_ERROR_CHECK(err_code);

	err_code = add_fifo(m_message_fifo, &message_buf[0]);
	APP_ERROR_CHECK(err_code);
	
	return;
}
