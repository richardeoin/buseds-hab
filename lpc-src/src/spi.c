/* 
 * Reads data frames from the SPI1 module.
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
#include "spi.h"

#define SPI_BUFFER_LEN	0x100

/**
 * Buffer for received data.
 */
uint8_t spi_buffer[SPI_BUFFER_LEN];
uint16_t spi_buffer_index = 0;
/**
 * A function that we call to have data processed.
 */
spi_frame_func frame_pr = 0;


uint16_t spi_xfer_16(uint16_t data) {
  LPC_SPI1->DR = data;

  /* Wait until the Busy bit is cleared */
  while ((LPC_SPI1->SR & (SSPSR_BSY|SSPSR_RNE)) != SSPSR_RNE);

  return LPC_SPI1->DR;
}
uint8_t spi_xfer(uint8_t data) {
  LPC_SPI1->DR = data;

  /* Wait until the Busy bit is cleared */
  while ((LPC_SPI1->SR & (SSPSR_BSY|SSPSR_RNE)) != SSPSR_RNE);

  return LPC_SPI1->DR;
}
void spi_write(uint16_t data) {
  /* Wait until there's space in the TxFIFO.
   * The TxFIFO Not Full (TNF) flag goes high when the buffer is not full. */
  while ((LPC_SPI1->SR & SSPSR_TNF) == 0);

  LPC_SPI1->DR = data;
}
uint16_t spi_read(void) {
  /* Wait until there's something in the RxFIFO.
   * The RxFIFO Not Empty flag goes high when there's something in the buffer. */
  while ((LPC_SPI1->SR & SSPSR_RNE) == 0);

  return LPC_SPI1->DR;
}
void spi_dump_bytes(uint32_t dumpCount) {
  while (dumpCount-- > 0) {
    spi_read();		/* Clear the RxFIFO */
  }
}
void spi_flush(void) {
  uint8_t i, Dummy=Dummy;
  for (i = 0; i < FIFOSIZE; i++) {
    Dummy = LPC_SPI1->DR;		/* Clear the RxFIFO */
  }
}

void spi_init(spi_frame_func frame_processing_function) {
  frame_pr = frame_processing_function;

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

  /* Set DSS data to 8-bit, Frame format SPI, CPOL = 0, CPHA = 0, and SCR is 0 */
  LPC_SPI1->CR0 = 0x0007;

  /* SSPCPSR clock prescale register, master mode, minimum divisor is 0x02 */
  LPC_SPI1->CPSR = 0x2;

  /* Device select as slave, SSP Enabled, we never drive the bus */
  LPC_SPI1->CR1 = SSPCR1_SLAVE | SSPCR1_SSE | SSPCR1_SOD;

  /* Set SSPINMS registers to enable interrupts */
  /* Interrupt when the receive buffer gets full */
  LPC_SPI1->IMSC = SSPIMSC_RORIM | SSPIMSC_RTIM | SSPIMSC_RXIM;

  /* Enable interrupts in the NVIC */
  NVIC_EnableIRQ(SSP1_IRQn);
}

/**
 * Interrupt handler.
 */
void SSP1_IRQHandler(void) {
  uint8_t data;

  /* Clear interrupts */
  LPC_SPI1->ICR |= 0x3;

  /* While there's data to be read */
  while (LPC_SPI1->SR & SSPSR_RNE) {
    /* Read it */
    data = LPC_SPI1->DR;

    if (data == '!') { // If started frame
      spi_buffer_index = 0;
    }

    spi_buffer[spi_buffer_index] = data;

    if (spi_buffer[spi_buffer_index] == '\n') { // End of frame
      /* Get the frame processed */
      if (frame_pr) {
	frame_pr(spi_buffer, spi_buffer_index+1);
      }

      /* Setup for next rx */
      spi_buffer_index = 0;
    } else {
      spi_buffer_index++;
    }

    if (spi_buffer_index >= SPI_BUFFER_LEN) { /* Buffer overflow */
      spi_buffer_index = 0;
    }
  }
}
