## Component Descriptions

| Component | Part Number | Product Page | Schematic / Datasheet | Bus | Weight | Current Draw (Operational @ 3.3V) | Minimum Operating Temperature | Price
| --- | --- | --- | --- | --- | --- | --- | --- | ---
| Temperature Sensor | TMP102 | [Sparkfun](https://www.sparkfun.com/products/9418) | [TI](https://www.sparkfun.com/datasheets/Sensors/Temperature/tmp102.pdf) | I2C 0x90 - 0x96 | | 10µA | -55°C
| Pressure Sensor | BMP085 | [Sparkfun](https://www.sparkfun.com/products/retired/9694) [Replacement, Generic Breakout](http://www.ebay.co.uk/itm/BMP085-Breakout-Barometric-Pressure-Sensor-/321123029556) | [Bosch](http://dlnmh9ip6v2uc.cloudfront.net/datasheets/Sensors/Pressure/BST-BMP085-DS000-06.pdf) | I2C 0xEE || 10µA | -40°C (0°C for full accuracy)
| IMU | SEN-10736 | [Sparkfun](https://www.sparkfun.com/products/10736) | [Sparkfun](http://dlnmh9ip6v2uc.cloudfront.net/datasheets/Sensors/IMU/9DOF-Razor-v22.pdf) |
| Accelerometer | ADXL345 | [Sparkfun] (https://www.sparkfun.com/products/9045) | [Analog Devices] (https://www.sparkfun.com/datasheets/Sensors/Accelerometer/ADXL345.pdf) | I2C 0x1D || 40µA @ 2.5V | -40°C
| Gyroscope | ITG-3200 | [Sparkfun] (https://www.sparkfun.com/products/9793) | [InvenSense] (https://www.sparkfun.com/datasheets/Sensors/Gyro/PS-ITG-3200-00-01.4.pdf) | I2C 0xD0 - 0xD2 || 6.5mA @ 2.5V | -40°C
| Magnetometer | HMC5883L | [Sparkfun] (https://www.sparkfun.com/products/10494) | [Honeywell] (http://dlnmh9ip6v2uc.cloudfront.net/datasheets/Sensors/Magneto/HMC5883L-FDS.pdf) | I2C 0x3C || 100μA @ 2.5V | -30°C
| IMU Processor | ATMega 328P |
| GPS | EM406 | [Sparkfun] (https://www.sparkfun.com/products/465) | [GlobalSat] (https://www.sparkfun.com/datasheets/GPS/EM-406A_User_Manual.PDF) | Serial 8N1 4800 || 70mA @ 4.5-6.5V | -40°C
| Radio | NTX2 | [Radiomatrix] (http://www.radiometrix.com/content/ntx2) | [Radiomatrix] (http://www.radiometrix.com/files/additional/ntx2nrx2.pdf) | Serial RTTY 8N2 50 / 300 || 18mA | -10°C (Will drift anyhow)
| SD Card | | | | SPI 1MHz || 30mA |
| SD Card Holder | | [Proto PIC](http://proto-pic.co.uk/breakout-board-for-microsd-transflash/) | | | | | | £4.59
| mBed || [mBed](mbed.org) | | | | [100mA](http://mbed.org/users/no2chem/notebook/mbed-power-controlconsumption/) |
| LPC1115 | LPC1115 | [Farnell](http://uk.farnell.com/nxp/om13035/lpc1115-lpcxpresso-eval-board/dp/2103787) | [Schematic](http://www.embeddedartists.com/sites/default/files/docs/schematics/LPCXpressoLPC1114revA.pdf) [Datasheet](http://www.nxp.com/documents/data_sheet/LPC111X.pdf) | | | 5mA @ 24MHz | -40°C | £15.15
| Battery Connectors | SPH-004T-P0.5S, PHR-2 | [Contact](http://uk.farnell.com/jsp/search/productdetail.jsp?SKU=1830762) [Housing](http://uk.farnell.com/jsp/search/productdetail.jsp?SKU=3616186) | | | | | | £0.56

## Minor Components

| ID | Part Number | Description | Supplier | Order Code | Quantity
| --- | --- | --- | --- | --- | ---
| U1 | MCP1827 | ADJ VREG 1.5A | Farnell | [1840847](http://uk.farnell.com/jsp/search/productdetail.jsp?id=1840847) | 1
| C1, C2 | | 1µF Capacitor, 16V, 5mm | Farnell | [1236655](http://uk.farnell.com/jsp/search/productdetail.jsp?id=1236655) | 2
| Q1, Q2 | PSMN2R7-30PL | MOSFET,N CH,30V,100A,TO-220 | Farnell | [1845665](http://uk.farnell.com/jsp/search/productdetail.jsp?id=1845665) | 2
| JP5, JP6, JP7 | CTB3051/2BK | TERMINAL BLOCK, PCB, 3.5MM, 2WAY | Farnell | [3882615](http://uk.farnell.com/jsp/search/productdetail.jsp?id=3882615) | 3
| LED1 | | LED, ORANGE, 3MM, 50MCD, 629NM | Farnell | [2112099](http://uk.farnell.com/jsp/search/productdetail.jsp?id=2112099) | 1
| LED2 | | LED, GREEN, 3MM, 50MCD, 572NM | Farnell | [2112096](http://uk.farnell.com/jsp/search/productdetail.jsp?id=2112096) | 1
| JTAG1 | | HEADER, VERTICAL, 2ROW, 10WAY | Farnell | [1099564](http://uk.farnell.com/jsp/search/productdetail.jsp?id=1099564) | 1
| R1, R7, R9 - R12 | | 10kΩ Resistor, 0.25W | Generic | | 6
| R2, R3 | | 15kΩ Resistor, 0.25W | Generic | | 2
| R4, R5 | | 4.7kΩ Resistor, 0.25W | Generic | | 2
| R6 | | 47kΩ Resistor, 0.25W | Generic | | 1
| R8 | | 115kΩ Resistor, 0.25W, 1% | Farnell | [2329996](http://uk.farnell.com/jsp/search/productdetail.jsp?id=2329996) | 1
| R13, R14 | | 470Ω Resistor | Generic | 2
| X1 | 5-1814832-1 | SMA Socket, Straight Vertical, PCB Mount, 50Ω | Farnell | [1248990](http://uk.farnell.com/jsp/search/productdetail.jsp?id=1248990) | 1
| Various | M20-7822046 | CONNECTOR, RECEPTACLE, THT, 2.54MM, 20P | Farnell | [7991967](http://uk.farnell.com/jsp/search/productdetail.jsp?id=7991967) | 4
