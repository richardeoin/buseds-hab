/* 
 * Reads temperature data from a TMP102, returns as a double
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

#include "i2c.h"

/**
 * DATASHEET: https://www.sparkfun.com/datasheets/Sensors/Temperature/tmp102.pdf
 */

/**
 * The address of the TMP102 with ADDR pulled to V+
 */
#define TMP102_ADDRESS 0x92
/**
 * TMP102 Registers
 */
#define TMP102_TEMPERATURE_REG	0x00
#define TMP102_CONTROL_REG	0x01

/**
 * By default the TMP102 converts readings at 4Hz
 */

/**
 * Processes a temperature value for the TMP102 into a dobule.
 */
double process_temperature(int16_t value) {
 /* Sign extension from 12-bit to 16-bit */
  if (value & 0x0800) {
    value |= 0xF000;
  }

  /* The temperature reading is scaled at 0.0625°C/count */
  return (double)value * 0.0625;
}
/**
 * Gets the temperature from the TMP102
 */
double get_temperature(void) {
  int16_t value;

  I2CWriteLength = 2;
  I2CReadLength = 2;
  I2CMasterBuffer[0] = TMP102_ADDRESS;
  I2CMasterBuffer[1] = TMP102_TEMPERATURE_REG;
  I2CMasterBuffer[2] = TMP102_ADDRESS | RD_BIT;

  if (i2c_engine() == I2CSTATE_ACK) { // All is well
    /* 12 bit data, MSb first */
    value = (I2CSlaveBuffer[0] << 4) | (I2CSlaveBuffer[1] >> 4);

    return process_temperature(value);
  } else { // Fail
    return -1000;
  }
}

#ifdef TMP102_TEST

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Performs a test of the conversion routine.
 */
void process_test(int16_t value, double result) {
  if (process_temperature(value) != result) {
    printf("ERROR: 0x%03x = %g°C, not %g°C as specified!\n",
	   value, process_temperature(value), result);
    exit(1);
  } else {
    printf("0x%03X = %g°C\n", value, process_temperature(value));
  }
}

int main(void) {
  printf("*** TMP102_TEST ***\n\n");

  /* From Table 5. */
  process_test(0x7FF, 127.9375);
  process_test(0x640, 100);
  process_test(0x4B0, 75);
  process_test(0x000, 0);
  process_test(0xFFC, -0.25);
  process_test(0xE70, -25);
  process_test(0xC90, -55);

  printf("\n*** DONE ***\n");
}

#endif
