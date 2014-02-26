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
#include "imu.h"

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
void build_communctions_frame(char* string, int string_size, struct gps_time* gt,
			      struct barometer* b, struct gps_data* gd,
			      double altitude, double temperature,
			      struct imu_raw* ir, int cutdown)
{
  int print_size;

  print_size = snprintf(string, string_size,
			"$$%s,%d,%02d:%02d:%02d,%.6f,%.6f,%.1f,%d,%.1f,%.1f,%.1f,%d,%d,%d,%d",
			CALLSIGN, sentence_id++,
			gt->hours, gt->minutes, gt->seconds, /* Time */
			gd->lat, gd->lon, gd->altitude, gd->satellites,/* GPS */
			altitude, temperature, b->temperature, /* TMP/BMP */
			ir->accel.x, ir->accel.y, ir->accel.z,/* Acceleration */
			cutdown); /* Cutdown State */

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

#ifdef PROTOCOL_TEST

// Test Dependancies
#include <assert.h>
#include <stdio.h>

int main(int argc, char** argv) {
  printf("*** PROTOCOL_TEST ***\n\n");

  char string[1000];
  struct gps_time gt;
  struct barometer b;
  struct gps_data gd;
  struct imu_raw ir;

  gt.hours = 1; gt.minutes = 1; gt.seconds = 1;
  b.temperature = 22.2; b.pressure = 99999;
  gd.lat = 51.23445; gd.lon = -2.23554;
  gd.altitude = 2333; gd.satellites = 9;
  ir.accel.x = 100; ir.accel.y = 100; ir.accel.z = 100;

  build_communctions_frame(string, 1000, &gt, &b, &gd, 145.2, 20.1, &ir, 0);

  printf("%s", string);

  printf("\n*** DONE ***\n");
}

#endif
