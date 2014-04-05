/*
 * Reads the ADC pin, used to monitor cutdown voltage
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
#include "pwrmon.h"

/**
 * Cutdown Monitoring: P1[2] / AD3
 */

#define NULL (void*)0

enum {
  ADINT_FLAG =		0x00010000,
};
enum {
  PWRMON_ENABLE =	1,
  PWRMON_DISABLE =	0,
};
enum {
  PWRMON_ADC_CHANNEL =	3,
};

/**
 * The callback we're going to call when the measurement completes.
 */
pwrmon_done_func callback;

/**
 * Starts a measurement.
 */
void pwrmon_start(pwrmon_done_func _callback) {
  callback = _callback;

  /* Disable the power down bit to the ADC block. */
  LPC_SYSCON->PDRUNCFG &= ~(1 << 4);

  /* Enable AHB clock to the ADC. */
  LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 13);

  /* Configure the IO Pins for AD3 */
  LPC_IOCON->R_PIO1_2 &= ~0x9F; /* 0xx00000 - Mode R, No Pull Up/Down, Analogue Mode */
  LPC_IOCON->R_PIO1_2 |= 0x02;  /* xxxxxx1x - Mode AD3 */

  /* Set the clock on the ADC Block  (must be < 4.5MHz) */
  LPC_ADC->CR = ((8-1) << 8);	/* Divide PCLK by 8 */

  /* Enable Interrupts */
  NVIC_SetPriority(ADC_IRQn, 2); // 3rd priority
  NVIC_EnableIRQ(ADC_IRQn);
  LPC_ADC->INTEN = (1 << PWRMON_ADC_CHANNEL);	/* Only interrupt on the selected channel */

  LPC_ADC->CR &= 0xFFFFFF00; /* Clear the channel select bits */
  LPC_ADC->CR |= (1 << 24) | (1 << PWRMON_ADC_CHANNEL); /* Trigger measurement */
}

/**
 * Called when the measurement is done.
 */
void pwrmon_done(void) {
  uint16_t adc_value;

  /* Read the 10-bit value off the ADC */
  adc_value = (LPC_ADC->DR[PWRMON_ADC_CHANNEL] >> 6) & 0x3FF;

  /* Disable the interrupt in the NVIC */
  NVIC_DisableIRQ(ADC_IRQn);

  /* Disable AHB clock to the ADC. */
  LPC_SYSCON->SYSAHBCLKCTRL &= ~(1 << 13);

  /* And power down the ADC. */
  LPC_SYSCON->PDRUNCFG |= (1 << 4);

  /* Make the callback */
  if (callback != NULL) {
    callback(adc_value);
  }
}
/**
 * Interrupt.
 */
void ADC_IRQHandler(void) {
  uint32_t adc_stat;
  adc_stat = LPC_ADC->STAT; /* Reading the STAT register will clear the interrupt */

  if (adc_stat & ADINT_FLAG) { /* A channel is done */
    if ((adc_stat & 0xFF) & (1 << PWRMON_ADC_CHANNEL)) {
      pwrmon_done();
    } else {
      // Unknown ADC Channel Finished..
    }
  } else {
    // Unknown ADC Interrupt
  }
}

/**
 * Initialisation.
 */
void pwrmon_init(void) {
  /* Set the callback to null */
  callback = NULL;
}



