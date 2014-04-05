/*
 * Demo C Application: Toggles an output at 20Hz.
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
#include "rtty.h"
#include "i2c.h"
#include "tmp102.h"
#include "imu.h"
#include "spi.h"
#include "leds.h"
#include "bmp085.h"
#include "altitude.h"
#include "cutdown_heat.h"
#include "mbed.h"
#include "sd.h"
#include "sd_spi.h"
#include "wdt.h"
#include "gps.h"
#include "uart.h"
#include "protocol.h"
#include "disk_write.h"
#include "pwrmon.h"

/**
 **************************
 Flight Parameters
 *************************/

/**
 * Watchdog disabled to allow debugging - REMOVE BEFORE FLIGHT
 */
#define WATCHDOG_DISABLED
/**
 * The number of minutes until the cutdown system activates
 * MAX = 2^32/60*RTTY_BAUD ~= 10^6
 */
#define CUTDOWN_TIME		60*2
/**
 * The mBed is powered on below the given barometric altitude in
 * meters
 */
#define GSM_ON_BELOW_ALTITUDE	1000
/**
 * The altitude ceiling at which cutdown will occour
 */
#define CUTDOWN_CEILING		60000
/**
 * The minimum barometric altitude in meters at which the balloon must
 * be for cutdown to occour.
 */
#define MIN_CUTDOWN_ALTITUDE	1000
/**
 * The threshold temperature for the heater to activate in °C
 */
#define HEATER_THRESHOLD	-30



/**
 **************************
 System Parameters
 *************************/

/**
 * RTTY Baud Clock.
 */
#define RTTY_BAUD       	50
/**
 * The maximum length of a single transmitter string
 */
#define TX_STRING_LENGTH	0x200

/**
 **************************
 Globals
 *************************/

int sd_good = 0;
uint32_t ticks_until_cutdown = CUTDOWN_TIME * RTTY_BAUD * 60;
float cutdown_voltage = 0;

/**
 **************************
 System Control Logic
 *************************/

void control_gsm(double altitude) {
  if (altitude < GSM_ON_BELOW_ALTITUDE && altitude != -1) {
    MBED_ON();
  } else {
    MBED_OFF();
  }
}
void control_cutdown(uint32_t ticks, double altitude) {
  if ((ticks == 0 && altitude > MIN_CUTDOWN_ALTITUDE) ||
      (altitude > CUTDOWN_CEILING && altitude != -1)) {

    CUTDOWN_ON(); // Mechanical disconnect
  } else {
    CUTDOWN_OFF();
  }
}
void control_heater(double internal_temperature) {
  if (internal_temperature < HEATER_THRESHOLD && internal_temperature != -1) {
    HEATER_ON();
  } else {
    HEATER_OFF();
  }
}

/**
 * Called at the end of an ADC conversion
 */
void pwrmon_callback(uint16_t adc_value) {
  cutdown_voltage = adc_value * (6.6 / 1024);
}

/**
 * Main system entry point
 */
int main (void) {
  SystemInit();

  /* Initialise Pins */
  CUTDOWN_OFF();
  HEATER_OFF();
  MBED_OFF();
  GREEN_OFF();

  /* Update the value of SystemCoreClock */
  SystemCoreClockUpdate();

  /* Initialise Interfaces */
  i2c_init();
  spi_init(process_imu_frame); // IMU
  sd_spi_init(); // SD
  uart_init(); // GPS
  pwrmon_init(); // ADC

  /* Initialise Sensors */
  init_barometer();

  /* SD Card */
  if (initialise_card()) { // Initialised to something
    if (disk_initialize() == 0) { // Disk initialisation was successful
      sd_good = 1;
    }
  }

  GREEN_ON();

  /* Configure the SysTick */
  NVIC_SetPriority(SysTick_IRQn, 0); // Highest Priority Interrupt
  SysTick_Config(SystemCoreClock / RTTY_BAUD);

  /* Watchdog - Disabled for debugging */
#ifndef WATCHDOG_DISABLED
  init_watchdog();
#endif

  struct barometer* b;
  struct imu_raw ir;
  struct gps_data gd;
  struct gps_time gt;
  double alt, ext_temp;
  int tx_length; // The length of the built tx string

  char tx_string[TX_STRING_LENGTH];

  while (1) {
    /* Grab Data */
    pwrmon_start(pwrmon_callback);
    b = get_barometer();
    get_imu_raw_data(&ir);
    get_gps_data(&gd);
    get_gps_time(&gt);
    ext_temp = get_temperature();

    /* Data Processing */
    if (b->valid) {
      alt = pressure_to_altitude(b->pressure);
    } else {
      alt = -1;
      b->temperature = -1;
    }

    /* Act on the data */
    control_gsm(alt);
    control_cutdown(ticks_until_cutdown, alt);
    control_heater(b->temperature);

    /* Create a protocol string */
    tx_length = build_communications_frame(tx_string, TX_STRING_LENGTH,
					 &gt, b, &gd, alt, ext_temp, &ir,
					 ticks_until_cutdown / (RTTY_BAUD*60),
					 cutdown_voltage);

    /* Transmit - Quietly fails if another transmission is ongoing */
    rtty_set_string(tx_string, tx_length);

    /* Store */
    if (sd_good) {
      tx_length -= 2; // Remove \n\0
      tx_length += communications_frame_add_extra(tx_string + tx_length,
				     TX_STRING_LENGTH - tx_length, &ir);

      disk_write_next_block((uint8_t*)tx_string, tx_length+1); // Include null terminator
    }

    /* Housekeeping */
    GREEN_TOGGLE();
    feed_watchdog();
  }
}

extern void SysTick_Handler(void) {
  /* Push RTTY bits */
  rtty_tick();
  /* Countdown */
  if (ticks_until_cutdown) {
    ticks_until_cutdown--;
  }
}
