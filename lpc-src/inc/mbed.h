/*
 * Interface and Power Control for the mbed
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

#ifndef MBED_H
#define MBED_H

#include "LPC11xx.h"

/**
 * Power Control on P1[9]. Active Low
 */

#define MBED_ON()	do { LPC_GPIO1->DIR |= (1<<9); \
    LPC_GPIO1->MASKED_ACCESS[1<<9] = (0<<9); } while (0);
#define HEATER_OFF()	do { LPC_GPIO1->DIR |= (1<<9); \
    LPC_GPIO1->MASKED_ACCESS[1<<9] = (1<<9); } while (0);

#endif /* MBED_H */
