/*
 * Controls the heater and cutdown MOSFETS
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

#ifndef CUTDOWN_HEAT_H
#define CUTDOWN_HEAT_H

#include "LPC11xx.h"

/**
 * Cutdown on P3[5]
 * Heater on P3[4]
 *
 *  *** NOTE WRONG TEXT ON LPCXPRESSO SILK ***
 */

#define CUTDOWN_ON()	do { LPC_GPIO3->DIR |= (1<<5); \
    LPC_GPIO3->MASKED_ACCESS[1<<5] = (1<<5); } while (0);
#define CUTDOWN_OFF()	do { LPC_GPIO3->DIR |= (1<<5); \
    LPC_GPIO3->MASKED_ACCESS[1<<5] = (0<<5); } while (0);

#define HEATER_ON()	do { LPC_GPIO3->DIR |= (1<<4); \
    LPC_GPIO3->MASKED_ACCESS[1<<4] = (1<<4); } while (0);
#define HEATER_OFF()	do { LPC_GPIO3->DIR |= (1<<4); \
    LPC_GPIO3->MASKED_ACCESS[1<<4] = (0<<4); } while (0);

#endif /* CUTDOWN_HEAT_H */
