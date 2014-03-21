/*
 * Decodes data from the IMU
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
#include "stdio.h"
#include "imu.h"

int imu_access_flag = 0;

struct imu_angle imu_angle;
struct imu_raw imu_raw;

/**
 * Assembles a float from an integer and fractional part, where the
 * fractional part always has two digits.
 */
float make_float_from_parts(int var_i, int var_f) {
  return var_i +
    ((float)var_f / 100) * ((var_i > 0) ? 1 : -1) ;
}
/**
 * Processes a frame from the IMU. Any fractional number always has
 * two digits.
 */
void process_imu_frame(uint8_t* data, uint16_t len) {
  int count;
  int roll_i, roll_f, pitch_i, pitch_f, yaw_i, yaw_f;

  if (!imu_access_flag) {		/* Check the flag isn't up */

    count = sscanf((char*)data,
		   "!ANG:%d.%d,%d.%d,%d.%d,AN:%d,%d,%d,%d,%d,%d,%d,%d,%d",
		   &roll_i, &roll_f, &pitch_i, &pitch_f, &yaw_i, &yaw_f,// Angle
		   &imu_raw.gyro.x, &imu_raw.gyro.y, &imu_raw.gyro.z, // Gyroscope
		   &imu_raw.accel.x, &imu_raw.accel.y, &imu_raw.accel.z, // Accelerometer
		   &imu_raw.magneto.x, &imu_raw.magneto.y, &imu_raw.magneto.z); // Magneto

    imu_angle.roll = make_float_from_parts(roll_i, roll_f);
    imu_angle.pitch = make_float_from_parts(pitch_i, pitch_f);
    imu_angle.yaw = make_float_from_parts(yaw_i, yaw_f);
  }

  len++; // UNUSED
  count++; // UNUSED
}

void get_imu_raw_data(struct imu_raw* data) {
  imu_access_flag = 1;		/* Raise the flag */
  memcpy(data, (void*)&imu_raw, sizeof(struct imu_raw));
  imu_access_flag = 0;		/* Clear the flag */
}
