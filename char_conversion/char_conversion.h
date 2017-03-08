#include "sdk_config.h"
#include <stdint.h>

#define LOWER_CASE_MOD 	48
#define UPPER_CASE_MOD 	10
#define NUMPUNC_MOD			60

#define IS_CAPITAL(c)		((c)>=(65)&&(c)<=(90))
#define IS_NUMPUNC(c)		((c)>=(33)&&(c)<=(64))

uint8_t char_2_braille(char character);

extern void modifier_parser(uint8_t * p_string, uint16_t string_length, uint8_t * p_mod);

uint8_t braille_2_char(uint8_t braille, uint8_t modifier);