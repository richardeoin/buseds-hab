/*
 * Implements the UKHAS Communication Protocol
 * Copyright (C) 2014  richard
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
#include <stdio.h>
#include "protocol.h"
#include "bmp085.h"
#include "gps.h"

int sentence_id = 0;

/**
 * CRC Function for the XMODEM protocol.
 * http://www.nongnu.org/avr-libc/user-manual/group__util__crc.html#gaca726c22a1900f9bad52594c8846115f
 */
uint16_t crc_xmodem_update(uint16_t crc, uint8_t data)
{
  int i;

  crc = crc ^ ((uint16_t)data << 8);
  for (i = 0; i < 8; i++) {
    if (crc & 0x8000) {
      crc = (crc << 1) ^ 0x1021;
    } else {
      crc <<= 1;
    }
  }

  return crc;
}

/**
 * Calcuates the CRC checksum for a communications string
 * See http://ukhas.org.uk/communication:protocol
 */
uint16_t crc_checksum(char *string)
{
  size_t i;
  uint16_t crc;
  uint8_t c;

  crc = 0xFFFF;

  // Calculate checksum ignoring the first two $s
  for (i = 2; i < strlen(string); i++) {
    c = string[i];
    crc = crc_xmodem_update(crc, c);
  }

  return crc;
}

/**
 * Builds a communctions frame compliant with the protocol described
 * at http://ukhas.org.uk/communication:protocol
 */
void build_communctions_frame(char* string, int string_size, struct barometer* b)
{
  int print_size;

  print_size = snprintf(string, string_size, "$$%s,%d", CALLSIGN, sentence_id++
			/* Time */
			/* GPS (Lat, Long, Alt, Satillies in View) */
			/* Barometric Alt */
			/* Internal Temperature */
			/* External Temperature */
			/* Acceleration */
/* Cutdown State */
    );

  /* If the above print plus checksum will be truncated */
  if (print_size >= (string_size - 7)) {
#ifdef DEBUG
    while(1); // Assert
#endif
  } else {                      /* Add checksum */
    /* Star + 4 Hex + \n + \0 */
    sprintf(string + print_size, "*%04X\n", crc_checksum(string));
  }
}
