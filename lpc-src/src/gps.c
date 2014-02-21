/*
 * Processes NMEA GPS strings into a useful structure
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
#include "gps.h"

int access_flag = 0;

struct gps_data gps_data;

/**
 * Processes a single NMEA GPS frame.
 */
int process_gps_frame(char* frame) {
  int hours, minutes, seconds, frac_seconds;
  int lat_deg, lat_min, lat_frac_min;
  int long_deg, long_min, long_frac_min;

  if (strncmp(frame, "$GPGGA", 6)) {
    return 1;			/* String starts wrong */
  }

  /* Next field */
  frame = strchr(frame, ','); frame++;
  /* Time of day */
  sscanf(frame, "%2d%2d%2d.%d", &hours, &minutes, &seconds, &frac_seconds);

  /* Next field */
  frame = strchr(frame, ','); frame++;
  /* Latitude */
  sscanf(frame, "%2d%2d.%d", &lat_deg, &lat_min, &lat_frac_min);
  gps_data.lat = lat_deg;
  gps_data.lat += (float)lat_min / 60;
  gps_data.lat += (float)lat_frac_min / (60 * 10000);

  /* Next field */
  frame = strchr(frame, ','); frame++;
  if (frame[0] == 'S') gps_data.lat *= -1;

  /* Next field */
  frame = strchr(frame, ','); frame++;
  /* Longitude */
  sscanf(frame, "%2d%3d.%d", &long_deg, &long_min, &long_frac_min);
  gps_data.lon = long_deg;
  gps_data.lon += (float)long_min / 60;
  gps_data.lon += (float)long_frac_min / (60 * 10000);

  /* Next field */
  frame = strchr(frame, ','); frame++;
  if (frame[0] == 'W') gps_data.lon *= -1;

  /* Next field */
  frame = strchr(frame, ','); frame++;
  /* Fix Indicator: IGNORE */

  /* Next field */
  frame = strchr(frame, ','); frame++;
  /* Satellites */
  sscanf(frame, "%d", &gps_data.satellites);

  /* Next field */
  frame = strchr(frame, ','); frame++;
  /* HDOP: IGNORE */

  /* Next field */
  frame = strchr(frame, ','); frame++;
  /* Altitude */
  sscanf(frame, "%f", &gps_data.altitude);

  return 0;
}

void get_gps_data(struct gps_data* data) {
  access_flag = 1;		/* Raise the flag */
  memcpy(data, (void*)&gps_data, sizeof(struct gps_data));
  access_flag = 0;		/* Clear the flag */
}

#ifdef GPS_TEST

// Test Dependancies
#include <assert.h>
#include <stdio.h>
#include <math.h>

void test_frame(FILE* fp) {
  char frame_string[0x100];

  if (fgets(frame_string, 0x100, fp)) {

    printf("\nTesting: %s", frame_string);
    process_gps_frame(frame_string);
  }
}

/**
 * Asserts if two numbers differ by more than delta
 */
void delta_assert(double x, double y) {
  if (fabs(x - y) > 1e-4) {
    printf("Error %g != %g\n", x, y);
  }
}

int main(void) {
  printf("*** GPS_TEST ***\n");

  FILE* fp;

  fp = fopen("test/gps_sample_data.txt", "r");
  if (fp) {
    ////////////////////////////////////////
    test_frame(fp);

    delta_assert(gps_data.lat, 51.8218);
    delta_assert(gps_data.lon, -0.01268);
    assert(gps_data.altitude == 22074);
    assert(gps_data.satellites == 10);
    ////////////////////////////////////////
    test_frame(fp);

    delta_assert(gps_data.lat, 51.8099433);
    delta_assert(gps_data.lon, -0.00071333);
    assert(gps_data.altitude == 22508);
    assert(gps_data.satellites == 10);
    ////////////////////////////////////////
    test_frame(fp);

    delta_assert(gps_data.lat, 51.80774167);
    delta_assert(gps_data.lon, 0.000653333);
    assert(gps_data.altitude == 22597);
    assert(gps_data.satellites == 10);
    ////////////////////////////////////////
    test_frame(fp);

    delta_assert(gps_data.lat, 51.103595);
    delta_assert(gps_data.lon, 0.958891666);
    assert(gps_data.altitude == 6055);
    assert(gps_data.satellites == 10);
    ////////////////////////////////////////
    test_frame(fp);

    delta_assert(gps_data.lat, 51.05447);
    delta_assert(gps_data.lon, 0.932895);
    assert(gps_data.altitude == -4);
    assert(gps_data.satellites == 10);
    ////////////////////////////////////////

    fclose (fp);
  } else {
    printf("Failed to load sample data\n");
  }

  printf("\n*** DONE ***\n");
}

#endif
