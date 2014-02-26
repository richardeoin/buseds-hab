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

#ifndef SPI_H
#define SPI_H

#define FIFOSIZE 8

/* SSEL: P0[2], Active Low */
#define SD_SPI_ENABLE()    do { LPC_GPIO0->DIR |= (1<<2); \
    LPC_GPIO0->MASKED_ACCESS[1<<2] = 0; } while (0)
#define SD_SPI_DISABLE()   do { LPC_GPIO0->DIR |= (1<<2); \
    LPC_GPIO0->MASKED_ACCESS[1<<2] = (1<<2); } while (0)

/**
 * SSP Status register
 */
#define SSPSR_TFE       (0x1<<0)
#define SSPSR_TNF       (0x1<<1)
#define SSPSR_RNE       (0x1<<2)
#define SSPSR_RFF       (0x1<<3)
#define SSPSR_BSY       (0x1<<4)
/**
 * SSP CR0 register
 */
#define SSPCR0_DSS      (0x1<<0)
#define SSPCR0_FRF      (0x1<<4)
#define SSPCR0_SPO      (0x1<<6)
#define SSPCR0_SPH      (0x1<<7)
#define SSPCR0_SCR      (0x1<<8)
/**
 * SSP CR1 register
 */
#define SSPCR1_LBM      (0x1<<0)
#define SSPCR1_SSE      (0x1<<1)
#define SSPCR1_MASTER   (0x0<<2)
#define SSPCR1_SLAVE    (0x1<<2)
#define SSPCR1_SOD      (0x1<<3)
/**
 * SSP Interrupt Mask Set/Clear register
 */
#define SSPIMSC_RORIM   (0x1<<0)
#define SSPIMSC_RTIM    (0x1<<1)
#define SSPIMSC_RXIM    (0x1<<2)
#define SSPIMSC_TXIM    (0x1<<3)
/**
 * SSP0 Interrupt Status register
 */
#define SSPRIS_RORRIS   (0x1<<0)
#define SSPRIS_RTRIS    (0x1<<1)
#define SSPRIS_RXRIS    (0x1<<2)
#define SSPRIS_TXRIS    (0x1<<3)
/**
 * SSP0 Masked Interrupt register
 */
#define SSPMIS_RORMIS   (0x1<<0)
#define SSPMIS_RTMIS    (0x1<<1)
#define SSPMIS_RXMIS    (0x1<<2)
#define SSPMIS_TXMIS    (0x1<<3)
/**
 * SSP0 Interrupt clear register
 */
#define SSPICR_RORIC    (0x1<<0)
#define SSPICR_RTIC     (0x1<<1)

uint8_t spi_xfer(uint8_t data);
void spi_init(void);
void spi_frequency(uint32_t frequency);

#endif /* SPI_H */
