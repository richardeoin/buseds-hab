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

/* Layer Heights h0 -> h7 (km) */
double h[] = { 0, 11, 20, 32, 47, 51, 71, 84.8520 };
/* Pressures p0 -> p7 (Pa) */
double p[] = { 101325, 22632.0, 5474.89, 868.019,
	       110.906, 66.9388, 3.95642, 0.373384 };
/* Lapse Rate l0 -> l7 (K/km) */
double l[] = { -6.5, 0, 1.0, 2.8, 0, -2.8, -2, 0 };
/* Temperature t0 -> t7 (K) */
double t[] = { 288.15, 216.65, 216.65, 228.65,
	       270.65, 270.65, 214.65, 186.946 };
/* Gas Constant of Air (J/kgK) */
#define R	287.053
/* Gravity at 45° Latitude (m/s2) */
#define G	-9.80665

/* Scaling Function: Height (m) */
double hi(int i) { return h[i] * 1000; }
/* Scaling Function: Lapse Rate (K/m) */
double lr(int i) { return l[i] / 1000; }

/**
 * Returns the altitude within normal layer i at pressure pr.
 */
double normal_layer(double pr, int i) {
  return hi(i) + (t[i] / lr(i)) * (pow(pr / p[i], (lr(i) * R) / G) - 1);
}
/**
 * Returns the altitude within isothermic layer i at pressure pr.
 */
double isothermic_layer(double pr, int i) {
  return hi(i) + ((R * t[i]) / G) * log(pr / p[i]);
}
/**
 * Returns the altitude in meters for a given pressure in Pascals
 */
double pressure_to_altitude(int32_t pressure) {
  double pr = (double)pressure;

  if (pr <= p[6]) { // Layer 6: Normal
    return normal_layer(pr, 6);

  } else if (pr <= p[5]) { // Layer 5: Normal
    return normal_layer(pr, 5);

  } else if (pr <= p[4]) { // Layer 4: Isothermic
    return isothermic_layer(pr, 4);

  } else if (pr <= p[3]) { // Layer 3: Normal
    return normal_layer(pr, 3);

  } else if (pr <= p[2]) { // Layer 2: Normal
    return normal_layer(pr, 2);

  } else if (pr <= p[1]) { // Layer 1: Isothermic
    return isothermic_layer(pr, 1);
  }

  // Layer 0: Normal
  return normal_layer(pr, 0);
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

  printf("Data from Wolfram Alpha/ http://www.digitaldutch.com/atmoscalc/...\n\n");
  altitude_test( -100, 102532 );
  altitude_test(    0, 101325 );
  altitude_test( 1000, 89874.6);
  altitude_test( 3000, 70108.5);
  altitude_test( 7000, 41060.7);
  altitude_test(11000, 22632.1);
  altitude_test(15000, 12044.6);
  altitude_test(20000, 5474.89);
  altitude_test(25000, 2511.02);
  altitude_test(30000, 1171.87);
  altitude_test(35000, 558.924);
  altitude_test(40000, 277.522);

  printf("\n*** DONE ***\n");
}

#endif
