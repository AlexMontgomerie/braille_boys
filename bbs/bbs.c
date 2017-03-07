#include "bbs.h"
#include "sdk_config.h"
#include "nrf_drv_gpiote.h"
#include "SEGGER_RTT.h"
#include "fifo.h"

/**
 * Global fifo structures
 */
buffer_t temp_raw_buffer; 
buffer_t temp_braille_buffer;

buffer_t* 	m_raw_fifo = &temp_raw_buffer;
buffer_t* 	m_braille_fifo = &temp_braille_buffer;


/**
 * Static memory initialisation for buffers
 */
uint8_t		raw_buf[BUFF_SIZE];
uint8_t 	braille_buf[BUFF_SIZE];

/**
 * Pointers to characters being pulled from the buffers
 */
uint8_t temp_raw_char;
uint8_t temp_braille_char;

uint8_t* p_fifo_raw_character = &temp_raw_char;
uint8_t* p_fifo_braille_character = &temp_braille_char;

void receive_char(void) {
	uint32_t err_code;
	err_code = fifo_get(m_raw_fifo, p_fifo_raw_character);
	switch(err_code) {
		
		case BUFF_EMPTY:
			
		break;
		
		//have got character out of buffer
		case NRF_SUCCESS:
			//braille_char temp_braille_char = char2braille(p_fifo_raw_character);
			//err_code = fifo_put(m_braille_fifo,temp_braille_char);
			//send_to_motors();
		break;
		
		default: break;
		
	}
}

void pin_control_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
	uint32_t err_code;
	switch(pin) {
		/**
		 *	Case for when switching between tx and rx mode
		 */
		case CONTROL_INPUT_1:
			//toggle_txrx();
		break;
		
		/**
		 *	Case to tap through the received characters, or send a character
		 */
		case CONTROL_INPUT_0:
			// if rx mode, call rx fn
			err_code = fifo_get(m_braille_fifo,p_fifo_braille_character);
			SEGGER_RTT_printf(0,"converted character: %d \r\n",*p_fifo_braille_character);
			//if tx mode, call tx fn
		break;
		
		default: break;
		
		
	}
	return;
}

void pin_message_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
	return;
}



void pin_test_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
	uint32_t err_code = fifo_get(m_raw_fifo,p_fifo_raw_character);
	switch(err_code) {
		case NRF_SUCCESS:
			SEGGER_RTT_printf(0,"Pulled %c from the buffer\r\n",*p_fifo_raw_character);
		break;
		
		case BUFF_EMPTY:
			SEGGER_RTT_printf(0,"Buffer is empty (current pointer value is %c)\r\n",*p_fifo_raw_character);
		break;
		
		default: break;
		
	}
	return;
}




/**
	TODO maybe we just have a main pin event handler and main function call
void main_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	switch pin
	case MOTOR_INPUT_0 //control
	control_fn_call();
	break

	switch pin
	case MOTOR_INPUT_1
	break

and so on

	return;
}

void main_function() {
	heres where everything happens
}
 */
 
 
 
 
 

/** 
 *	Initialising the input pins
 *	FIX ME its a bit messy, can make prettier
 */

uint32_t add_input_pins(nrf_drv_gpiote_in_config_t * config)
{
	
	uint32_t err_code;
	
	/**
	 *	Initialise all the message input pins
	 */
	
/*	
	//for INPUT_0
	err_code = nrf_drv_gpiote_in_init(BRAILLE_INPUT_0, config, pin_message_event_handler);
	if (err_code != NRF_SUCCESS)
	{	
		APP_ERROR_CHECK(err_code);
	}
	nrf_drv_gpiote_in_event_enable(BRAILLE_INPUT_0, true);


	//for INPUT_1
	err_code = nrf_drv_gpiote_in_init(BRAILLE_INPUT_1, config, pin_tx_event_handler);
	if (err_code != NRF_SUCCESS)
	{	
		APP_ERROR_CHECK(err_code);
	}
	nrf_drv_gpiote_in_event_enable(BRAILLE_INPUT_1, true);		
	//for INPUT_2
	err_code = nrf_drv_gpiote_in_init(BRAILLE_INPUT_2, config, pin_tx_event_handler);
	if (err_code != NRF_SUCCESS)
	{	
		APP_ERROR_CHECK(err_code);
	}
	nrf_drv_gpiote_in_event_enable(BRAILLE_INPUT_2, true);		
	//for INPUT_3
	err_code = nrf_drv_gpiote_in_init(BRAILLE_INPUT_3, config, pin_tx_event_handler);
	if (err_code != NRF_SUCCESS)
	{	
		APP_ERROR_CHECK(err_code);
	}
	nrf_drv_gpiote_in_event_enable(BRAILLE_INPUT_3, true);
*/
	
	/*//for INPUT_4
	err_code = nrf_drv_gpiote_in_init(BRAILLE_INPUT_4, config, pin_tx_event_handler);
	if (err_code != NRF_SUCCESS)
	{	
		APP_ERROR_CHECK(err_code);
	}
	nrf_drv_gpiote_in_event_enable(BRAILLE_INPUT_4, true);	
	//for INPUT_5
	err_code = nrf_drv_gpiote_in_init(BRAILLE_INPUT_5, config, pin_tx_event_handler);
	if (err_code != NRF_SUCCESS)
	{	
		APP_ERROR_CHECK(err_code);
	}
	nrf_drv_gpiote_in_event_enable(BRAILLE_INPUT_5, true);	*/
	
	/**
	 *	Initialise all the control input pins
	 */
	//for CONTROL_0
	err_code = nrf_drv_gpiote_in_init(CONTROL_INPUT_0, config, pin_control_event_handler);
	if (err_code != NRF_SUCCESS)
	{	
		APP_ERROR_CHECK(err_code);
	}
	nrf_drv_gpiote_in_event_enable(CONTROL_INPUT_0, true);	
	
	return err_code;
}

/** 
 *	Initialising the output pins
 */

uint32_t add_output_pins(nrf_drv_gpiote_in_config_t * config)
{
	//FIX ME
	uint32_t err_code;
	return err_code;
}



/** Function to initialise IO and braille service related buffers
*/
void bbs_init(void)
{

	//initialise gpio tasks and events
	uint32_t err_code;
	err_code = nrf_drv_gpiote_init();
  APP_ERROR_CHECK(err_code);
	
	/** 
	 *	config for input pins 
	 *		-detect low to high and transition
	 *		-pull up on pin
	 */
	nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
	config.pull = NRF_GPIO_PIN_PULLUP;
	
	//adding input pins SEGGER_RTT_printf(0,"\r\nadding input pins!\r\n");	
	err_code = add_input_pins(&config);
	APP_ERROR_CHECK(err_code);
	
  // adding output pins SEGGER_RTT_printf(0,"\r\nadding output pins!\r\n");	
	
	// adding fifo buffers SEGGER_RTT_printf(0,"\r\nadding fifo buffers!\r\n");	
	
	err_code = add_fifo(m_raw_fifo, &raw_buf[0]);
	APP_ERROR_CHECK(err_code);
	
	err_code = add_fifo(m_braille_fifo, &braille_buf[0]);
	APP_ERROR_CHECK(err_code);	
	return;
}
