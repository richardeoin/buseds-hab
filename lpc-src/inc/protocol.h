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

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "bmp085.h"
#include "gps.h"
#include "imu.h"

#define CALLSIGN        "BUSEDS1"

int build_communications_frame(char* string, int string_size, struct gps_time* gt,
			     struct barometer* b, struct gps_data* gd,
			     double altitude, double temperature,
			     struct imu_raw* ir,
			     int cutdown_minutes, float cutdown_voltage);
int communications_frame_add_extra(char* string, int string_length, struct imu_raw* ir);

#endif /* PROTOCOL_H */
