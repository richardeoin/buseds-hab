/*
 * Demo C Application: Toggles an output at 20Hz.
 * Copyright (C) 2013  Richard Meadows
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
#include "rtty.h"
#include "i2c.h"
#include "tmp102.h"
#include "imu.h"
#include "spi.h"
#include "leds.h"
#include "bmp085.h"
#include "altitude.h"
#include "cutdown_heat.h"
#include "mbed.h"
#include "sd.h"
#include "sd_spi.h"

#define RTTY_BAUD       50

int main (void) {
  SystemInit();

  /* Initialise Pins */
  CUTDOWN_OFF();
  HEATER_OFF();
  MBED_OFF();
  GREEN_OFF();

  /* Update the value of SystemCoreClock */
  SystemCoreClockUpdate();

  CUTDOWN_ON();

  CUTDOWN_OFF();

  while (1);

  /* Initialise Interfaces */
  i2c_init();
  spi_init(process_imu_frame); // IMU
  sd_spi_init(); // SD

  /* Initialise Sensors */
  init_barometer();

  /* SD Card */
  initialise_card();

  GREEN_ON();

  /* Configure the SysTick */
  SysTick_Config(SystemCoreClock / RTTY_BAUD);

  struct barometer* b;
  struct imu_raw ir;

  while (1) {
    // Wait and set this immediately
    while (rtty_set_string("M0SBU TEST TEST", 15));

    b = get_barometer();
    get_imu_raw_data(&ir);

    double a = pressure_to_altitude(b->pressure);

    (void)a;

    GREEN_TOGGLE();
  }
}

double temp;

extern void SysTick_Handler(void) {
  /* Push RTTY bits */
  rtty_tick();
}
