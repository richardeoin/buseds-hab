/*
 * Bit-bangs RTTY
 * Copyright (C) 2013  richard
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "LPC11xx.h"
#include <string.h>

/**
 * Interface to the physical world on P0[7] (Also red LED)
 */
#ifndef RTTY_TEST

#define RTTY_PORT		LPC_GPIO0
#define RTTY_PIN		7

#define RTTY_ACTIVATE()		RTTY_PORT->DIR |=  (1 << RTTY_PIN)
#define RTTY_DEACTIVATE()	RTTY_PORT->DIR &= ~(1 << RTTY_PIN)
#define RTTY_SET(b)		RTTY_PORT->MASKED_ACCESS[1 << RTTY_PIN] = (~b << RTTY_PIN)
#define RTTY_NEXT()

#else

#include <stdio.h>
#define RTTY_ACTIVATE()
#define RTTY_DEACTIVATE()
#define RTTY_SET(b)		printf("%d", b & 1)
#define RTTY_NEXT()		printf("\n")

#endif

/**
 * Formatting 8N2
 */
#define ASCII_BITS	8
#define BITS_PER_CHAR	11

/**
 * Output String
 */
#define RTTY_STRING_MAX	0x200

/**
 * Where we currently are in the rtty output byte
 *
 * 0 = Start Bit
 * 1-8 = Data Bit
 * 10 = Stop Bit
 * 11 = Stop Bit
 */
uint8_t rtty_phase;
/**
 * Where we are in the current output string
 */
uint32_t rtty_index;

/**
 * Details of the string that is currently being output
 */
char rtty_string[RTTY_STRING_MAX];
uint32_t rtty_string_length = 0;

/**
 * Returns 1 if we're currently outputting.
 */
int rtty_active(void) {
  return (rtty_string_length > 0);
}

/**
 * Sets an output string.
 *
 * Returns 0 on success, 1 if a string is already active or 2 if the
 * specified string was too long.
 */
int rtty_set_string(char* string, uint32_t length) {
  if (length > RTTY_STRING_MAX) return 2; // To long

  if (!rtty_active()) {
    // Copy
    memcpy(rtty_string, string, length);
    rtty_string_length = length;
    // Initialise
    rtty_index = 0;
    rtty_phase = 0;

    return 0; // Success
  } else {
    return 1; // Already active
  }
}

/**
 * Called at the baud rate, outputs bits of rtty
 */
void rtty_tick(void) {
  if (rtty_active()) {
    RTTY_ACTIVATE();

    if (rtty_phase == 0) { // Start
      // Low
      RTTY_SET(0);
    } else if (rtty_phase < ASCII_BITS + 1) {
      // Data
      RTTY_SET(rtty_string[rtty_index] >> (rtty_phase - 1));
    } else if (rtty_phase < BITS_PER_CHAR) { // Stop
      // High
      RTTY_SET(1);
    }

    rtty_phase++;

    if (rtty_phase >= BITS_PER_CHAR) { // Next character
      rtty_phase = 0; rtty_index++; RTTY_NEXT();

      if (rtty_index >= rtty_string_length) { // All done, deactivate
	rtty_string_length = 0; // Deactivate
      }
    }
  } else {
    RTTY_DEACTIVATE();
  }
}


#ifdef RTTY_TEST

int main() {
  printf("*** RTTY_TEST ***\n\n");

  rtty_set_string("RTTY", 4);
  while (rtty_active()) {
    rtty_tick();
  }

  printf("\n*** DONE ***\n");
}

#endif
