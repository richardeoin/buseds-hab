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
#include <stdlib.h>
#include <math.h>
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
 * Printing helper functions. From
 * http://ukhas.org.uk/guides:common_coding_errors_payload_testing
 */
int print_six_dp(char* s, size_t n, double val) {
  int i1 = val;
  long i2 = labs(lround((val - i1) * 1000000));

  return snprintf(s, n, "%s%li.%06li,", (val < 0 ? "-" : ""), labs(i1), i2);
}
int print_one_dp(char* s, size_t n, double val) {
  int i1 = val;
  long i2 = labs(lround((val - i1) * 10));

  return snprintf(s, n, "%s%li.%01li,", (val < 0 ? "-" : ""), labs(i1), i2);
}

/**
 * Builds a communctions frame compliant with the protocol described
 * at http://ukhas.org.uk/communication:protocol
 */
int build_communctions_frame(char* string, int string_size, struct gps_time* gt,
			     struct barometer* b, struct gps_data* gd,
			     double b_altitude, double temperature,
			     struct imu_raw* ir,
			     int cutdown_minutes, float cutdown_voltage)
{
  int print_size;

  print_size = snprintf(string, string_size,
			"$$%s,%d,%02d:%02d:%02d,",
			CALLSIGN, sentence_id++,
			gt->hours, gt->minutes, gt->seconds);
  /* GPS */
  print_size += print_six_dp(string + print_size, string_size - print_size,
			     gd->lat);
  print_size += print_six_dp(string + print_size, string_size - print_size,
			     gd->lon);
  print_size += snprintf(string + print_size, string_size - print_size,
			 "%d,%d,", gd->altitude, gd->satellites);

  /* Barometer & Temperature */
  print_size += print_one_dp(string + print_size, string_size - print_size,
			     b_altitude);
  print_size += print_one_dp(string + print_size, string_size - print_size,
			     temperature);
  print_size += print_one_dp(string + print_size, string_size - print_size,
			     b->temperature);

  /* Acceleration */
  print_size += snprintf(string + print_size, string_size - print_size,
			 "%d,%d,%d,",
			 ir->accel.x, ir->accel.y, ir->accel.z);
  /* Cutdown */
  print_size += snprintf(string + print_size, string_size - print_size,
			 "%d,", cutdown_minutes);
  print_size += print_one_dp(string + print_size, string_size - print_size,
			     cutdown_voltage);
  print_size--; // Delete last comma

  /* If the above print plus checksum will be truncated */
  if (print_size >= (string_size - 7)) {
#ifdef DEBUG
    while(1); // Assert
#endif
  } else {                      /* Add checksum */
    /* Star + 4 Hex + \n + \0 */
    print_size += sprintf(string + print_size, "*%04X\n", crc_checksum(string));

    return print_size + 1; // +1 for null terminator
  }

  return 0;
}

#ifdef PROTOCOL_TEST

// Test Dependancies
#include <assert.h>
#include <stdio.h>
#include <time.h>

int main(int argc, char** argv) {
  (void)argc; // UNUSED
  (void)argv;

  printf("*** PROTOCOL_TEST ***\n\n");

  char string[1000];
  struct gps_time gt;
  struct barometer b;
  struct gps_data gd;
  struct imu_raw ir;

  time_t rawtime;
  struct tm *ti;

  time(&rawtime);
  ti = localtime(&rawtime);

  gt.hours = ti->tm_hour; gt.minutes = ti->tm_min; gt.seconds = ti->tm_sec;
  b.temperature = 22.2; b.pressure = 99999;
  gd.lat = 51.23445; gd.lon = -2.23554;
  gd.altitude = 2333; gd.satellites = 9;
  ir.accel.x = 100; ir.accel.y = 100; ir.accel.z = 100;

  build_communctions_frame(string, 1000, &gt, &b, &gd, 145.2, -0.2, &ir, 120, 5.6);

  printf("%s", string);

  printf("\n*** DONE ***\n");
}

#endif
