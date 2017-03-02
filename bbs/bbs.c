//

#include "bbs.h"
#include "sdk_config.h"
#include "nrf_drv_gpiote.h"
#include "SEGGER_RTT.h"
#include "app_fifo.h"

#define BUFF_SIZE 255
#define MEM_ERROR 9

static buffer 	m_raw_fifo;
static buffer 	m_braille_fifo;

static uint8_t	raw_buf[BUFF_SIZE];
static uint8_t 	braille_buf[BUFF_SIZE];

static uint8_t *p_fifo_raw_character;
static uint8_t *p_fifo_braille_character;

/** Pin event to read recieved bytes
 */
void pin_read_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	while(raw_fifo_get(p_fifo_raw_character)==NRF_SUCCESS){
		//put the data processing code here!!!!
		braille_fifo_put(*p_fifo_raw_character);
	}
	
	uint32_t err_code = braille_fifo_get(p_fifo_braille_character);
	
	if(err_code==NRF_SUCCESS)
	{
		SEGGER_RTT_printf(0,"\r\ntx Char at output: %d\r\n",*p_fifo_braille_character);
	}
	return;
}

void pin_rx_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	SEGGER_RTT_printf(0,"\r\nPin Event\r\n");
	return;
}

void pin_tx_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	SEGGER_RTT_printf(0,"\r\ntx Pin Event: %d\r\n",pin);
	return;
}

uint32_t add_input_pins(nrf_drv_gpiote_in_config_t * config)
{
	//for INPUT_0
	uint32_t err_code = nrf_drv_gpiote_in_init(BRAILLE_INPUT_0, config, pin_read_event_handler);
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
	
	return err_code;
}

uint32_t add_fifo(bbs_buffers_t * p_buffers)
{
		m_raw_fifo.p_start = &raw_buf[0];
		m_raw_fifo.P_end = &raw_buf[BUFF_SIZE];
		m_raw_fifo.read_pos = 0;
		m_raw_fifo.write_pos = 0;
	
		m_braille_fifo.p_start = &raw_buf[0];
		m_braille_fifo.P_end = &raw_buf[BUFF_SIZE];
		m_braille_fifo.read_pos = 0;
		m_braille_fifo.write_pos = 0;
}

/** Function to put byte into raw character buffer
 *	@input byte of data
 */
uint32_t raw_fifo_put(uint8_t character)
{
	if(m_raw_fifo.write_pos>=BUFF_SIZE)
	{
		return MEM_ERROR;
	}
	
	*(m_raw_fifo.p_start + m_raw_fifo.write_pos) = character;
	m_raw_fifo.write_pos++;
	return NRF_SUCCESS;
}

uint32_t raw_fifo_get(uint8_t * p_character)
{
	if(m_raw_fifo.read_pos>m_raw_fifo.write_pos)
	{
		return MEM_ERROR;
	}
	if(m_raw_fifo.read_pos>=BUFF_SIZE)
	{
		return MEM_ERROR;
	}
	
	*p_character = *(m_raw_fifo.p_start + m_raw_fifo.read_pos);
	m_raw_fifo.read_pos++;
	return NRF_SUCCESS;
}

/** Function to put byte into braille character buffer
 *	@input byte of data
 */
uint32_t braille_fifo_put(uint8_t character)
{
	if(m_braille_fifo.write_pos>=BUFF_SIZE)
	{
		return MEM_ERROR;
	}
	
	*(m_braille_fifo.p_start + m_braille_fifo.write_pos) = character;
	m_braille_fifo.write_pos++;
	return NRF_SUCCESS;
}

/** Function to get 
	@return 
*/
uint32_t braille_fifo_get(uint8_t * p_character)
{
	if(m_braille_fifo.read_pos>m_braille_fifo.write_pos)
	{
		return MEM_ERROR;
	}	
	if(m_braille_fifo.read_pos>=BUFF_SIZE)
	{
		return MEM_ERROR;
	}
	
	*p_character = *(m_braille_fifo.p_start + m_braille_fifo.read_pos);
	m_braille_fifo.read_pos++;
	return NRF_SUCCESS;
}


/** Function to initialise IO and braille service related buffers
*/
void bbs_init(void)
{
	/*
	bbs_buffers_t buffer;
  static uint8_t     raw_buf[BUFF_SIZE];                                                    \
  static uint8_t     braille_buf[BUFF_SIZE]; 
	buffer.raw_buf = raw_buf;
	buffer.raw_buf_size = sizeof(raw_buf);
	buffer.braille_buf 	= braille_buf;
	buffer.braille_buf_size = sizeof(braille_buf);
	*/
	uint32_t err_code;
	err_code = nrf_drv_gpiote_init();
  APP_ERROR_CHECK(err_code);
	
	nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
	config.pull = NRF_GPIO_PIN_PULLUP;
	
	err_code = add_input_pins(&config);
	APP_ERROR_CHECK(err_code);
  SEGGER_RTT_printf(0,"\r\nHERE!\r\n");	
	//err_code = add_fifo(&buffer);
	APP_ERROR_CHECK(err_code);
	
	return;
}