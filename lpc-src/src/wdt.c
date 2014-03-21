/*
 * Interface code for the Watchdog timer
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

/**
 * Watchdog runs from internal oscillator, timeout of 2 - 4 seconds??
 */

#include "LPC11xx.h"

/**
 * Runs the watchdog feed sequence if the wdt ahb bus is active
 */
void feed_watchdog(void) {
  if (LPC_SYSCON->SYSAHBCLKCTRL & (1 << 15)) {
    LPC_WDT->FEED = 0xAA;
    LPC_WDT->FEED = 0x55;
  }
}

/**
 *
 */
void init_watchdog(void) {
  /* Enable interface clock */
  LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 15);

  LPC_SYSCON->WDTCLKSEL = 0x0; // Use IRC oscillator
  LPC_SYSCON->WDTCLKUEN = 0;
  LPC_SYSCON->WDTCLKUEN = 1; // Switch Clock

  /* IRC = 12MHz, TC = 4 * 6*10_6, => Twdt = 2 seconds */
  LPC_WDT->TC = 6*1000*1000;

  /* Lock on the watchdog */
  LPC_WDT->MOD |= 0x3; // WDEN = 1, WDRESET = 1

  /* And feed to enable */
  feed_watchdog();
}
