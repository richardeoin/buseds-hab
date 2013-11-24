/* 
 * Calcuates altitude in the bottom layer of a 1976 US Standard Atmosphere
 * Copyright (C) 2013  richard
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

#include <math.h>

/**
 * Useful Resources:
 * Wikipedia: http://en.wikipedia.org/wiki/Barometric_formula
 * Online Calculator: http://www.digitaldutch.com/atmoscalc/
 */

/**
 * This calculation is only valid for the bottom layer of a US
 * standard atmosphere, i.e. < 11,000m (The Troposphere)
 */

/**
 * The pressure at sea level, in Pascals
 */
#define P0	101325
/**
 * Equal to (Universal Gas Constant R (Nm) * Temperature Lapse Rate L (K))
 *   /  (Gravity g (ms-2) * Molar Mass of Air M (kgmol-1))
 * = (8.3144 * 0.0065) / (9.80665 * 0.0289644) = 0.19026
 */
#define EXP	0.19026
/**
 * Equal to the Standard Temperature T (K) / Temperature Lapse Rate deltaT (K)
 * = (288.15 / 0.0065) = 44330.8
 */
#define SCALE	44330.8

/**
 * Returns the altitude in meters for a given pressure in Pascals
 */
double pressure_to_altitude(int32_t pressure) {
  return SCALE * (1 - pow((double)pressure / P0, EXP));
}

#ifdef ALTITUDE_TEST

#include <stdio.h>

#include <stdlib.h>

#define MAX_ERROR 20

void altitude_test(double altitude, uint32_t pressure) {
  double test_altitude = pressure_to_altitude(pressure);

  if (test_altitude > altitude - MAX_ERROR &&
      test_altitude < altitude + MAX_ERROR) { // Success
    printf("%dPa = %gm (Expected %gm)\n", pressure, test_altitude, altitude);
  } else { // Fail
    printf("\nERROR:\n");
    printf("%dPa = %gm (Expected %gm)\n", pressure, test_altitude, altitude);
    exit(1);
  }
}

int main(void) {
  printf("*** ALTITUDE_TEST ***\n\n");

  printf("Data from Wolfram Alpha...\n\n");
  altitude_test(-100, 102500);
  altitude_test(0, 101300);
  altitude_test(1000, 89880);
  altitude_test(3000, 70120);
  altitude_test(7000, 41110);
  altitude_test(11000, 22700);

  printf("\n*** DONE ***\n");
}

#endif
