/*
 * Reads data from a BMP085
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
 * DATASHEET: http://dlnmh9ip6v2uc.cloudfront.net/datasheets/Sensors/Pressure/BST-BMP085-DS000-06.pdf
 */

#include "bmp085.h"

/**
 * The address of the BMP085
 */
#define BMP085_ADDRESS		0xEE
/**
 * BMP085 Registers
 */
#define BMP085_CONTROL_REG	0xF4
#define BMP085_DATA_REG		0xF6
/**
 * Control Register Values
 */
typedef enum {
  TEMPERATURE			= 0x2E,
  PRESSURE_ULTRALOW		= 0x34, //  4500µS Delay
  PRESSURE_STANDARD		= 0x74, //  7500µS Delay
  PRESSURE_HIGHRES		= 0xB4, // 13500µS Delay
  PRESSURE_ULTRAHIGHRES		= 0xF4  // 22500µS Delay
} bmp085_command;

/**
 * The mode of pressure measurement
 */
#define PRESSURE_MODE		PRESSURE_STANDARD
#define PRESSURE_DELAY		7500

#define TEMPERATURE_DELAY	4500

/**
 * Barometer data structure
 */
struct barometer barometer;

/**
 * Calibration Values
 */
struct calibration {
  int16_t AC1, AC2, AC3, B1, B2, MB, MC, MD;
  uint16_t AC4, AC5, AC6;
} calibration;

/**
 * Lookup table for oversampling values.
 */
uint8_t oversampling(void) {
  switch(PRESSURE_MODE) {
    case PRESSURE_ULTRALOW:
      return 0;
    case PRESSURE_HIGHRES:
      return 2;
    case PRESSURE_ULTRAHIGHRES:
      return 3;
    default: // PRESSURE_STANDARD
      return 1;
  }
}

/**
 * Implements a microsecond delay for use with the BMP085
 */
void bmp085_delay_us(uint16_t microseconds) {
  int32_t i = microseconds * 12;

  while(i--);
}

/**
 * Utility function to read a 16-bit register.
 */
uint16_t read_16(char address) {
  I2CWriteLength = 2;
  I2CReadLength = 2;
  I2CMasterBuffer[0] = BMP085_ADDRESS;
  I2CMasterBuffer[1] = address;
  I2CMasterBuffer[2] = BMP085_ADDRESS | RD_BIT;

  if (i2c_engine() == I2CSTATE_ACK) { // All is well
    return (I2CSlaveBuffer[0] << 8) | I2CSlaveBuffer[1];
  }

  return -1;
}

/**
 * Writes a command to the BMP085's control register.
 */
uint8_t write_command(bmp085_command command) {
  I2CWriteLength = 3;
  I2CReadLength = 0;
  I2CMasterBuffer[0] = BMP085_ADDRESS;
  I2CMasterBuffer[1] = BMP085_CONTROL_REG;
  I2CMasterBuffer[2] = command;

  if (i2c_engine() == I2CSTATE_ACK) { // All is well
    return 1;
  }

  return -1;
}
/**
 * Reads off the BMP085's calibration values.
 */
void bmp085_get_cal_param(struct calibration *c) {
  c->AC1 = read_16(0xAA);
  c->AC2 = read_16(0xAC);
  c->AC3 = read_16(0xAE);
  c->AC4 = read_16(0xB0);
  c->AC5 = read_16(0xB2);
  c->AC6 = read_16(0xB4);
  c->B1  = read_16(0xB6);
  c->B2  = read_16(0xB8);
  c->MB  = read_16(0xBA);
  c->MC  = read_16(0xBC);
  c->MD  = read_16(0xBE);
}

/**
 * Takes a temperature measurement and returns the uncompensated value.
 */
int32_t bmp085_get_ut(void) {
  if (write_command(TEMPERATURE) == -1) {
    return -1; // Fail
  }

  bmp085_delay_us(TEMPERATURE_DELAY);

  return read_16(BMP085_DATA_REG);
}
/**
 * Takes a pressure measurement and returns the uncompenstated value.
 */
int32_t bmp085_get_up(void) {
  if (write_command(PRESSURE_MODE) == -1) {
    return -1; // Fail
  }

  bmp085_delay_us(PRESSURE_DELAY);

  I2CWriteLength = 2;
  I2CReadLength = 3;
  I2CMasterBuffer[0] = BMP085_ADDRESS;
  I2CMasterBuffer[1] = BMP085_DATA_REG;
  I2CMasterBuffer[2] = BMP085_ADDRESS | RD_BIT;

  if (i2c_engine() == I2CSTATE_ACK) { // All is well
    return ((I2CSlaveBuffer[0] << 16) | (I2CSlaveBuffer[1] << 8) |
	    I2CSlaveBuffer[2]) >> (8 - oversampling());
  }

  return -1;
}
/**
 * Returns the variable B5, which is used for both temperature and
 * pressure calculations.
 */
int32_t get_B5(struct calibration *c, int32_t* b5) {
  int32_t x1, x2;
  int32_t ut = bmp085_get_ut();

  if (ut == -1) {
    return -1;
  }

  x1 = ((ut - c->AC6) * c->AC5) >> 15;
  x2 = (double)(c->MC << 11) / (x1 + c->MD);
  *b5 = x1 + x2; // B5

  return 1;
}
/**
 * Returns the temperature in °C using variable B5
 */
double bmp085_get_temperature(int32_t B5) {
  return (double)((B5 + 8) >> 4) / 10;
}
/**
 * Returns the pressure in pascals using the calibration and variable
 * B5.
 */
int32_t bmp085_get_pressure(struct calibration *c, int32_t B5) {
  int64_t B6, X1, X2, X3, B3, pressure;
  uint64_t B4, B7;

  int32_t up = bmp085_get_up();

  if (up == -1) {
    return -1;
  }

  B6 = B5 - 4000;
  X1 = (c->B2 * ((B6 * B6) >> 12)) >> 11;
  X2 = (c->AC2 * B6) >> 11;
  X3 = X1 + X2;
  B3 = ((((((int64_t)(c->AC1) * 4) + X3) << oversampling()) + 2) >> 2);
  X1 = (c->AC3 * B6) >> 13;
  X2 = (c->B1 * ((B6 * B6) >> 12)) >> 16;
  X3 = ((X1 + X2) + 2) >> 2;
  B4 = (c->AC4 * (uint64_t)(X3 + 32768)) >> 15;
  B7 = ((uint64_t)(up - B3) * (50000 >> oversampling()));

  if (B7 < 0x80000000) {
    pressure = (B7 << 1) / B4;
  } else {
    pressure = (B7 / B4) << 1;
  }

  X1 = (pressure >> 8);
  X1 *= X1;
  X1 = (X1 * 3038) >> 16;
  X2 = (-7357 * pressure) >> 16;
  pressure += (X1 + X2 + 3791) >> 4;

  return pressure;
}

struct barometer* get_barometer(void) {
  int64_t B5;

  if (get_B5(&calibration, &B5) == -1) {
    // Invalid
    barometer.valid = 0;

  } else {
    // Temperature
    barometer.temperature = bmp085_get_temperature(B5);

    // Pressure
    barometer.pressure = bmp085_get_pressure(&calibration, B5);

    // Valid
    if (barometer.pressure != -1) {
      barometer.valid = 1;
    } else {
      barometer.valid = 0;
    }
  }

  return &barometer;
}
void init_barometer(void) {
  bmp085_get_cal_param(&calibration);
}
