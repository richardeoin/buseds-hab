//
// http://www.gammon.com.au/forum/?id=10892
//

void spi_transfer(char c) {
  digitalWrite(SS, LOW);    // SS on pin 10
  digitalWrite(1, LOW);    // SS on pin D1
  SPI.transfer((byte)c);
  digitalWrite(SS, HIGH);    // SS on pin 10
  digitalWrite(1, HIGH);    // SS on pin D1
}

void spi_transfer_str(const char *s) {
  while(*s)
    spi_transfer(*s++);
}

void spi_transfer_int(int number) {
  char buf[6];

  itoa(number, buf, 10);
  spi_transfer_str(buf);
}

void spi_transfer_double(float number) {
  char buf[8];
  // !ANG:-000.00,-000.00,-000.00\r\n
  // !ANG:0.00,0.00,0.00\r\n
  // dtostrf(floatVar, minStringWidthIncDecimalPoint, numVarsAfterDecimal, charBuf);
  dtostrf((double)number, 3, 2, buf);
  spi_transfer_str(buf);
}

void spi_println(void) {
  spi_transfer('\r');
  spi_transfer('\n');
}
