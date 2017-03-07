/*
Module to convert Braille to Characters
-Uses 

notes
-Needs to stay in flash

 to look up a character's corresponding braille value,
 the character can be used as an array index with
 an offset subtracted.
*/

#include "sdk_config.h"
#include "char_conversion.h"
#include <stdint.h>

uint8_t char_2_braille(char character){
	
	static const uint8_t braille_lookup[26] = {
		1, 3, 9, 25, 17, 11, 27, 19, 10, 		// a, b, c, d, e, f, g, h, i
		26,	5, 7, 13, 29, 21, 15, 31, 23, 	// j, k, l, m, n, o, p, q, r
		14,	30, 37, 39, 58, 45, 61, 53			// s, t, u, v, w, x, y, z			
	};

	return braille_lookup[character-96];
}
