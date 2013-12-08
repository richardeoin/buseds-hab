## Firmware for the Sparkfun IMU

I have configured the firmware to output both processed and raw data,
and to do so over the SPI Bus.

### Building

I used the Arduino IDE (Version 1.0.5). You might be able to upload to
the board directly from the IDE, otherwise you need to extract the
intel `.hex` file. In linux it's under
`/tmp/build..../S9DOF_AHRS.cpp.hex`, windows users can find
it in `C:\Users\Owner\AppData\Local\Temp\build...\S9DOF_AHRS.cpp.hex`.

I used
[this](http://mbed.org/users/aberk/notebook/avr910-in-system-programming-using-mbed/)
program for the mBed to actually flash the board. See the
[wiring diagram](SPI_Wire_Connections.pdf).

### Status LED

In SPI mode the Status LED is wired to the SCK signal, so will not work properly!

### Slave Select Pin

The slave select pin is not normally broken out on this board, so this
firmware outputs it on the `TXO` connection. The slave select pin
clocks for each byte, rather than each sentence so that a fully
compliant SPI slave can read every byte.
