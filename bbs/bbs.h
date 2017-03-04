/** Braille Boys Hardware drivers and general device structures
*/

#include "sdk_config.h"
#include "nrf_drv_gpiote.h"
#include "SEGGER_RTT.h"

#define BRAILLE_MOTOR_0			21	//pin assignment for motor 0
#define BRAILLE_MOTOR_1			22	//pin assignment for motor 1
#define BRAILLE_MOTOR_2			23	//pin assignment for motor 2
#define BRAILLE_MOTOR_3			24	//pin assignment for motor 3
#define BRAILLE_MOTOR_4			4		//pin assignment for motor 4
#define BRAILLE_MOTOR_5			5		//pin assignment for motor 5

#define BRAILLE_INPUT_0 		17	//pin assignment for input of bit 0 for braille character
#define BRAILLE_INPUT_1 		18	//pin assignment for input of bit 1 for braille character
#define BRAILLE_INPUT_2 		19	//pin assignment for input of bit 2 for braille character
#define BRAILLE_INPUT_3 		20	//pin assignment for input of bit 3 for braille character
#define BRAILLE_INPUT_4 		4		//pin assignment for input of bit 4 for braille character
#define BRAILLE_INPUT_5 		5		//pin assignment for input of bit 5 for braille character

#define CONTROL_INPUT_0 		0		//pin assignment for control button 0
#define CONTROL_INPUT_1 		1		//pin assignment for control button 1
#define CONTROL_INPUT_2 		2		//pin assignment for control button 2
#define CONTROL_INPUT_3 		3		//pin assignment for control button 3

/**
	typedef for a braille character
	we'll use 8 bits for now
 */ 
typedef uint8_t braille_char;

/**
	Enumeration to define the current states of the glove
 */
typedef enum
{ 
    RECEIVING,
		WAIT_TO_RECEIVE
} bbs_state_t;

/**********************************************
	Struct for holding current state of inputs
	FIX ME do we need this??
 */
typedef struct {
	braille_char output;
	braille_char input;
	bbs_state_t state;
} bbs_t;
/**********************************************/

/**
 * Structure of the fifo implimentation
 * TODO test the actual implimentation of this
 */
typedef struct
{
	uint8_t * p_start;
	uint8_t * P_end;
	uint32_t 	buf_size;
	uint32_t 	read_pos;
	uint32_t write_pos;
} buffer_t;

/**
 * List of function references
*/

void pin_rx_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

void pin_tx_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

uint32_t raw_fifo_put(uint8_t byte);

uint32_t raw_fifo_get(uint8_t * p_byte);

uint32_t braille_fifo_put(uint8_t byte);

uint32_t braille_fifo_get(uint8_t * p_byte);

void bbs_init(void);

static bbs_state_t	m_bbs_state;