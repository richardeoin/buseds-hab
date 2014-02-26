/*
 * Transfers data on the SPI0 port
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
#include "sd_spi.h"

uint8_t sd_spi_xfer(uint8_t data) {
  LPC_SPI0->DR = data;

  /* Wait until the Busy bit is cleared */
  while ((LPC_SPI0->SR & (SSPSR_BSY|SSPSR_RNE)) != SSPSR_RNE);

  return LPC_SPI0->DR;
}
void sd_spi_flush(void) {
  uint8_t i, Dummy=Dummy;
  for (i = 0; i < FIFOSIZE; i++) {
    Dummy = LPC_SPI0->DR;		/* Clear the RxFIFO */
  }
}
/**
 * Sets the SPI frequency.
 */
void sd_spi_frequency(uint32_t frequency) {
  uint32_t div = SystemCoreClock / frequency;

  if (div >= 2) {
    /* SSPCPSR clock prescale register, master mode, minimum divisor is 0x02 */
    LPC_SPI0->CPSR = div & 0xFE;
  }
}
/**
 * Initialisation
 */
void sd_spi_init(void) {
  /* De-assert reset for the SSP1 peripheral */
  LPC_SYSCON->PRESETCTRL |= (1 << 2);

  /* Enable the clock to the module */
  LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 18);
  LPC_SYSCON->SSP1CLKDIV = 0x01; /* Full clock to the module */

  /*  SSP I/O configuration */
  LPC_IOCON->PIO2_2 &= ~0x07;		/* Leave the pull-ups on */
  LPC_IOCON->PIO2_2 |= 0x02;		/* SSP MISO */
  LPC_IOCON->PIO2_3 &= ~0x07;		/* SSP MOSI */
  LPC_IOCON->PIO2_3 |= 0x02;
  LPC_IOCON->PIO2_1 &= ~0x07;		/* SSP CLK */
  LPC_IOCON->PIO2_1 |= 0x02;
  LPC_IOCON->PIO2_0 &= ~0x07;		/* SSP SSEL */
  LPC_IOCON->PIO2_0 |= 0x02;

  /* Set DSS data to 8-bit, Frame format SPI, CPOL = 0, CPHA = 0, and SCR is 2 */
  LPC_SPI0->CR0 = 0x0007;

  /* Initially 100kHz */
  spi_frequency(100*1000);

  /* Master SSP Enabled */
  LPC_SPI0->CR1 = SSPCR1_SSE;

  /* Set SSPINMS registers to enable interrupts */
  /* Interrupt when the receive buffer gets full */
  LPC_SPI0->IMSC = SSPIMSC_RORIM | SSPIMSC_RTIM;

  /* Enable interrupts in the NVIC */
  NVIC_EnableIRQ(SSP0_IRQn);
}

/**
 * Interrupt handler. Doesn't do much
 */
void SSP0_IRQHandler(void) {
  uint8_t data;

  data = LPC_SPI0->MIS;

  // Unused
  (void)data;

  return;
}
