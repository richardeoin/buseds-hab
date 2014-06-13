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
#include "gps.h"

/**
 * Recevies NMEA frames on P1[6] at 4800 baud.
 */

#define MAX_NMEA_FRAME_SIZE	0x200

char nmea_frame[MAX_NMEA_FRAME_SIZE];

#define START_CODE		'$'
#define CHECKSUM_CODE		'*'
#define CHECKSUM_LENGTH		2
#define RX_FIFO_TRIGGER_LEVEL	14

int in_index, checksum_index;

/**
 * Called when a character can be read from the Rx FIFO.
 */
void rx_read(uint8_t number) {
  uint8_t i, data;

  for (i = 0; i < number; i++) {
    data = LPC_UART->RBR;

    if (data == START_CODE) {


      /* Start a new frame */
      in_index = 0;
      checksum_index = 0;
      nmea_frame[in_index] = data;
      in_index++;

    } else if (in_index < MAX_NMEA_FRAME_SIZE && in_index >= 0) {
      /* Add a character to the current frame */
      nmea_frame[in_index] = data;
      in_index++;

      /* If we're in a checksum sequence */
      if (checksum_index) {
	if (checksum_index++ == CHECKSUM_LENGTH) {
	  process_gps_frame(nmea_frame);
	  in_index = -1;
	}
      } else if (data == CHECKSUM_CODE) {
	/* Otherwise look for the start of the checksum sequence */
	checksum_index = 1;
      }
    }
  }
}

/**
 * UART interrupt.
 */
extern void UART_IRQHandler(void) {
  uint8_t Dummy=Dummy, iir;

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
 * multiple of 125kHz.
 */
void uart_init(void) {

  /* Configure Pins */
  LPC_IOCON->PIO1_6 &= ~0x7;
  LPC_IOCON->PIO1_6 |= 0x1;
  LPC_IOCON->PIO1_7 &= ~0x7;
  LPC_IOCON->PIO1_7 |= 0x1;

  LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 12);
  LPC_SYSCON->UARTCLKDIV = 1; /* Use the main clock for the UART */


  /* Configure UART */
  LPC_UART->LCR = 0x83;	/* 8-bit words, 1 stop bit, DL access */

  /* Baud Rate = (SysCClk) / (16*PCLK_DIV*1.625) = 125000 / 16*1.625 = 4800 */
  /* 1.625 (DivAddVal = 5, MulVal = 8) */
  int pclk_div = SystemCoreClock / 125000;
  /* Load the Divisor Latches */
  LPC_UART->DLL = pclk_div & 0xFF;
  LPC_UART->DLM = pclk_div >> 8;
  /* Load the fractional divider */
  LPC_UART->FDR = (8 << 4) | (5 << 0);

  LPC_UART->LCR = 0x3; /* DLAB = 0 */
  LPC_UART->FCR |= (3 << 6) | (7 << 0);	/* FIFO Enable+Reset, RX Trig 3 */

  /* Configure Interrupts */
  LPC_UART->IER |= (1 << 2) | (1 << 0); /* Enable the RBR and RXL interrupts */

  /* Enable the interrupt in the NVIC */
  NVIC_SetPriority(UART_IRQn, 1); /* 2nd highest priority*/
  NVIC_EnableIRQ(UART_IRQn);

  /* Start the receiver waiting for the start of a packet */
  in_index = -1;
}
