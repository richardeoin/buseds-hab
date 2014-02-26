/*
 * Receives NMEA GPS strings over UART
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

#include "LPC11xx.h"
#include <string.h>
#include "uart.h"

#define START_CODE	$

int in_index;

/**
 * Called when a character can be read from the Rx FIFO.
 */
void rx_read(uint8_t number) {
  uint8_t i, data;

  for (i = 0; i < number; i++) {
    data = LPC_UART->RBR;

    if (data == START_CODE) {
      in_index = 0;
    } else if (in_index < MAX_NMEA_FRAME_SIZE && in_index >= 0) { /* Just data */
      nmea_frame[in_index] = LPC_UART->RBR;
      in_index++;
    }
  }
}

/**
 * UART interrupt.
 */
extern void UART_IRQHandler(void) {
  uint8_t i, Dummy=Dummy, iir;

  /* While interrupt pending */
  while (!((iir = LPC_UART->IIR) & 1)) {
    /* Switch by interrupt source */
    switch((iir & (7 << 1)) >> 1) {
      case 0x3:/* Rx Line / Status Error*/
	Dummy = LPC_UART->LSR;
	break;
      case 0x2:/* Rx Data Available*/
	/* We can empty RX_FIFO_TRIGGER_LEVEL bytes from the FIFO*/
	rx_read(RX_FIFO_TRIGGER_LEVEL);
	break;
      case 0x6: /* Rx Data Character Timeout*/
	/* Empty the Rx FIFO */
	Dummy = LPC_UART->RBR;
	rx_read(1);
	break;
    }
  }
}

/**
 * Initialises the UART at 9600 baud. The system core clock must be a
 * multiple of 250kHz.
 */
void uart_init(int SystemCoreClock) {
  LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 12);
  LPC_SYSCON->UARTCLKDIV = 1; /* Use the main clock for the UART */

  /* Configure Pins */
  LPC_IOCON->PIO1_6 |= 0x1;
  LPC_IOCON->PIO1_7 |= 0x1;

  /* Configure UART */
  LPC_UART->LCR |= (1 << 7) | (3 << 0);	/* 8-bit words, 1 stop bit, DL access */
  LPC_UART->FCR |= (3 << 6) | (7 << 0);	/* FIFO Enable+Reset, RX Trig 3 */

  /* Baud Rate = (SysCClk) / (16*PCLK_DIV*1.625) = 250000 / 16*1.625*/
  /* 1.625 (DivAddVal = 5, MulVal = 8) */
  int PLCK_DIV = SystemCoreClock / 250000;
  /* Load the Divisor Latches */
  LPC_UART->DLL = PCLK_DIV & 0xFF;
  LPC_UART->DLM = PCLK_DIV >> 8;
  /* Load the fractional divider */
  LPC_UART->FDR = (8 << 4) | (5 << 0);

  /* Configure Interrupts */
  LPC_UART->IER |= (1 << 2) | (1 << 0); /* Enable the RBR and RXL interrupts */

  /* Enable the interrupt in the NVIC */
  NVIC_SetPriority(UART_IRQn, 1);/* 2nd highest priority*/
  NVIC_EnableIRQ(UART_IRQn);

  /* Start the receiver waiting for the start of a packet */
  in_index = -1;
}
