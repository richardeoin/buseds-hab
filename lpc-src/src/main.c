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

#define RTTY_BAUD	50

int main (void) {
  SystemInit();

  /* Update the value of SystemCoreClock */
  SystemCoreClockUpdate();

  GREEN_ON();

  /* Configure the SysTick */
  SysTick_Config(SystemCoreClock / RTTY_BAUD);

  while (1) {
    rtty_set_string("M0SBU TEST TEST", 15);
    for (int i = 0; i < 1000*100; i++);
    GREEN_TOGGLE();
  }
}

double temp;

extern void SysTick_Handler(void) {
  /* Push RTTY bits */
  rtty_tick();
}
