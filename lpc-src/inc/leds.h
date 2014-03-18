/*
 * Macros for controlling the on-board LEDS
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

#ifndef LEDS_H
#define LEDS_H

#include "LPC11xx.h"

/**
 * Yellow on P1[3]
 * Green on P1[4]
 */

/**
 * We don't use Yellow any more because it interferes with one of the SW lines.
 */
/* #define YELLOW_ON()	do { LPC_IOCON->SWDIO_PIO1_3 |= 0x1;		\
    LPC_GPIO1->DIR |= (1<<3); LPC_GPIO1->MASKED_ACCESS[1<<3] = (1<<3); } while (0);
#define YELLOW_OFF()	LPC_GPIO1->MASKED_ACCESS[1<<3] = 0;
#define YELLOW_TOGGLE()	do { if (LPC_GPIO1->MASKED_ACCESS[1<<3]) { \
      YELLOW_OFF(); } else { YELLOW_ON(); } } while (0);
*/

#define GREEN_ON()	do { LPC_GPIO1->DIR |= (1<<4); \
    LPC_GPIO1->MASKED_ACCESS[1<<4] = (1<<4); } while (0);
#define GREEN_OFF()	LPC_GPIO1->MASKED_ACCESS[1<<4] = 0;
#define GREEN_TOGGLE()	do { if (LPC_GPIO1->MASKED_ACCESS[1<<4]) { \
      GREEN_OFF(); } else { GREEN_ON(); } } while (0);

#endif /* LEDS_H */
