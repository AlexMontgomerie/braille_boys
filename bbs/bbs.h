/** Braille Boys Hardware drivers and general device structures
*/

#include "sdk_config.h"
#include "nrf_drv_gpiote.h"
#include "SEGGER_RTT.h"

#define BRAILLE_MOTOR_0			21	//pin assignment for motor 0
#define BRAILLE_MOTOR_1			22	//pin assignment for motor 1
#define BRAILLE_MOTOR_2			23	//pin assignment for motor 2
#define BRAILLE_MOTOR_3			24	//pin assignment for motor 3
#define BRAILLE_MOTOR_4			4	//pin assignment for motor 4
#define BRAILLE_MOTOR_5			5	//pin assignment for motor 5

#define BRAILLE_INPUT_0 		17	//pin assignment for input of bit 0 for braille character
#define BRAILLE_INPUT_1 		18	//pin assignment for input of bit 1 for braille character
#define BRAILLE_INPUT_2 		19	//pin assignment for input of bit 2 for braille character
#define BRAILLE_INPUT_3 		20	//pin assignment for input of bit 3 for braille character
#define BRAILLE_INPUT_4 		4	//pin assignment for input of bit 4 for braille character
#define BRAILLE_INPUT_5 		5	//pin assignment for input of bit 5 for braille character

typedef uint8_t braille_char;

typedef enum
{ 
    RECEIVING,
		WAIT_TO_RECEIVE
} bbs_state_t;

typedef struct {
	braille_char output;
	braille_char input;
	bbs_state_t state;
} bbs_t;

typedef struct
{
    uint8_t * raw_buf;      		/**< Pointer to the RX buffer. */
    uint32_t  raw_buf_size; 		/**< Size of the RX buffer. */
    uint8_t * braille_buf;      /**< Pointer to the TX buffer. */
    uint32_t  braille_buf_size; /**< Size of the TX buffer. */
} bbs_buffers_t;

typedef struct
{
	uint8_t * p_start;
	uint8_t * P_end;
	uint32_t 	read_pos;
	uint32_t write_pos;
} buffer;

void pin_rx_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

void pin_tx_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

uint32_t raw_fifo_put(uint8_t byte);

uint32_t raw_fifo_get(uint8_t * p_byte);

uint32_t braille_fifo_put(uint8_t byte);

uint32_t braille_fifo_get(uint8_t * p_byte);

void bbs_init(void);

static bbs_state_t	m_bbs_state;