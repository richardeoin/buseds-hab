/* mbed Microcontroller Library
 * Copyright (c) 2006-2012 ARM Limited
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/* Introduction
 * ------------
 * SD and MMC cards support a number of interfaces, but common to them all
 * is one based on SPI. This is the one I'm implmenting because it means
 * it is much more portable even though not so performant, and we already
 * have the mbed SPI Interface!
 *
 * The main reference I'm using is Chapter 7, "SPI Mode" of:
 *  http://www.sdcard.org/developers/tech/sdcard/pls/Simplified_Physical_Layer_Spec.pdf
 *
 * SPI Startup
 * -----------
 * The SD card powers up in SD mode. The SPI interface mode is selected by
 * asserting CS low and sending the reset command (CMD0). The card will
 * respond with a (R1) response.
 *
 * CMD8 is optionally sent to determine the voltage range supported, and
 * indirectly determine whether it is a version 1.x SD/non-SD card or
 * version 2.x. I'll just ignore this for now.
 *
 * ACMD41 is repeatedly issued to initialise the card, until "in idle"
 * (bit 0) of the R1 response goes to '0', indicating it is initialised.
 *
 * You should also indicate whether the host supports High Capicity cards,
 * and check whether the card is high capacity - i'll also ignore this
 *
 * SPI Protocol
 * ------------
 * The SD SPI protocol is based on transactions made up of 8-bit words, with
 * the host starting every bus transaction by asserting the CS signal low. The
 * card always responds to commands, data blocks and errors.
 *
 * The protocol supports a CRC, but by default it is off (except for the
 * first reset CMD0, where the CRC can just be pre-calculated, and CMD8)
 * I'll leave the CRC off I think!
 *
 * Standard capacity cards have variable data block sizes, whereas High
 * Capacity cards fix the size of data block to 512 bytes. I'll therefore
 * just always use the Standard Capacity cards with a block size of 512 bytes.
 * This is set with CMD16.
 *
 * You can read and write single blocks (CMD17, CMD25) or multiple blocks
 * (CMD18, CMD25). For simplicity, I'll just use single block accesses. When
 * the card gets a read command, it responds with a response token, and then
 * a data token or an error.
 *
 * SPI Command Format
 * ------------------
 * Commands are 6-bytes long, containing the command, 32-bit argument, and CRC.
 *
 * +---------------+------------+------------+-----------+----------+--------------+
 * | 01 | cmd[5:0] | arg[31:24] | arg[23:16] | arg[15:8] | arg[7:0] | crc[6:0] | 1 |
 * +---------------+------------+------------+-----------+----------+--------------+
 *
 * As I'm not using CRC, I can fix that byte to what is needed for CMD0 (0x95)
 *
 * All Application Specific commands shall be preceded with APP_CMD (CMD55).
 *
 * SPI Response Format
 * -------------------
 * The main response format (R1) is a status byte (normally zero). Key flags:
 *  idle - 1 if the card is in an idle state/initialising
 *  cmd  - 1 if an illegal command code was detected
 *
 *    +-------------------------------------------------+
 * R1 | 0 | arg | addr | seq | crc | cmd | erase | idle |
 *    +-------------------------------------------------+
 *
 * R1b is the same, except it is followed by a busy signal (zeros) until
 * the first non-zero byte when it is ready again.
 *
 * Data Response Token
 * -------------------
 * Every data block written to the card is acknowledged by a byte
 * response token
 *
 * +----------------------+
 * | xxx | 0 | status | 1 |
 * +----------------------+
 *              010 - OK!
 *              101 - CRC Error
 *              110 - Write Error
 *
 * Single Block Read and Write
 * ---------------------------
 *
 * Block transfers have a byte header, followed by the data, followed
 * by a 16-bit CRC. In our case, the data will always be 512 bytes.
 *
 * +------+---------+---------+- -  - -+---------+-----------+----------+
 * | 0xFE | data[0] | data[1] |        | data[n] | crc[15:8] | crc[7:0] |
 * +------+---------+---------+- -  - -+---------+-----------+----------+
 */

#include "LPC11xx.h"
#include "sd.h"
#include "sd_spi.h"

#define SD_COMMAND_TIMEOUT 5000

#define SD_DBG             0

#define R1_IDLE_STATE           (1 << 0)
#define R1_ERASE_RESET          (1 << 1)
#define R1_ILLEGAL_COMMAND      (1 << 2)
#define R1_COM_CRC_ERROR        (1 << 3)
#define R1_ERASE_SEQUENCE_ERROR (1 << 4)
#define R1_ADDRESS_ERROR        (1 << 5)
#define R1_PARAMETER_ERROR      (1 << 6)

// Types
//  - v1.x Standard Capacity
//  - v2.x Standard Capacity
//  - v2.x High Capacity
//  - Not recognised as an SD Card
#define SDCARD_FAIL 0
#define SDCARD_V1   1
#define SDCARD_V2   2
#define SDCARD_V2HC 3

/* ======== Private Functions ======== */

int _cmd(int cmd, int arg);
int _cmdx(int cmd, int arg);
int _cmd58();
int _cmd8();
int _block_read(uint8_t *buffer, uint32_t length);
int _block_write(const uint8_t*buffer, uint32_t length);
static uint32_t ext_bits(unsigned char *data, int msb, int lsb);
uint64_t _sd_sectors();

/* ======== Private Variables ======== */

uint64_t _sectors;
int cdv;

/* ======== Public Function ======== */

int initialise_card(void) {
  uint8_t i;

  /* Set to 100kHz for initialisation, and clock card with cs = 1 */
  sd_spi_frequency(100000);
  SD_SPI_DISABLE();

  for (i = 0; i < 16; i++) {
    /* Write times are improved by having this set to 0xFF rather than 0xAA.
     * Maybe this has some kind of effect on the internal SD operating frequency?
     */
    sd_spi_xfer(0xFF);
  }

  /* Send CMD0, should return with all zeros except IDLE STATE set (bit 0) */
  for (i = 0; _cmd(0, 0) != R1_IDLE_STATE && i < 5; i++);

  if(i == 5) { /* If it failed 5 times */
    return SDCARD_FAIL;
  }

  /* Send CMD8 to determine whether it is version 2.x */
  int r = _cmd8();
  if (r == R1_IDLE_STATE) {
    return initialise_card_v2();
  } else if (r == (R1_IDLE_STATE | R1_ILLEGAL_COMMAND)) {
    return initialise_card_v1();
  } else {
    // Not in idle state after sending CMD8 (not an SD card?)
    return SDCARD_FAIL;
  }
}

int initialise_card_v1(void) {
  uint32_t i;

  for (i = 0; i < SD_COMMAND_TIMEOUT; i++) {
    _cmd(55, 0);
    if (_cmd(41, 0) == 0) {
      cdv = 512;
      // Init: SDCARD_V1
      return SDCARD_V1;
    }
  }

  // Timeout waiting for v1.x card
  return SDCARD_FAIL;
}

int initialise_card_v2(void) {
  uint32_t i, t;

  for (i = 0; i < SD_COMMAND_TIMEOUT; i++) {

    _cmd58();
    _cmd(55, 0);
    if (_cmd(41, 0x40000000) == 0) {
      _cmd58();
      cdv = 1;
      return SDCARD_V2;
    }
  }

  // Timeout waiting for v2.x card
  return SDCARD_FAIL;
}

int disk_initialize(void) {
  _sectors = _sd_sectors();

  /* Set block length to 512 (CMD16) */
  if (_cmd(16, 512) != 0) {
    return 1;
  }

  sd_spi_frequency(1000000); /* Set to 1MHz for data transfer. */
  return 0;
}

/**
 * Write up to 512 octets to a single block.
 * The `length` argument specifies the number of octets to write.
 * Returns 0 on success, 1 on failure.
 */
int disk_write(const uint8_t *buffer, uint32_t length, uint64_t block_number) {
  if (length > 512) { return 0; } /* We can only write 512 octets or less */
  if (block_number > 0x007FFFFF) { return 0; } /* We don't support the 64-bit address space yet */

  /* Set write address for single block (CMD24) */
  if (_cmd(24, block_number * 512) != 0) {
    return 1;
  }

  /* Send the data block */
  _block_write(buffer, length);
  return 0;
}
/**
 * Read  up to 512 octets from a single block.
 * The 'length' argument specifies the number of octets to read.
 * Returns 0 on success, 1 on failure.
 */
int disk_read(uint8_t *buffer, uint32_t length, uint64_t block_number) {
  /* We can only read 512 octets or less */
  if (length > 512) { return 0; }
  /* We don't support the 64-bit address space yet */
  if (block_number > 0x007FFFFF) { return 0; }

  /* Set read address for single block (CMD17) */
  if (_cmd(17, block_number * 512) != 0) {
    return 1;
  }

  /* Receive the data */
  _block_read(buffer, length);
  return 0;
}

int disk_status() { return 0; }
int disk_sync() { return 0; }
uint64_t disk_sectors() { return _sectors; }


/* ======== PRIVATE FUNCTIONS ======== */

int _cmd(int cmd, int arg) {
  uint32_t i;

  SD_SPI_ENABLE();

  /* Send a command */
  sd_spi_xfer(0x40 | cmd);
  sd_spi_xfer(arg >> 24);
  sd_spi_xfer(arg >> 16);
  sd_spi_xfer(arg >> 8);
  sd_spi_xfer(arg >> 0);
  sd_spi_xfer(0x95);

  /* Wait for the response (response[7] == 0) */
  for (i = 0; i < SD_COMMAND_TIMEOUT; i++) {
    int response = sd_spi_xfer(0xFF);
    if (!(response & 0x80)) {
      SD_SPI_DISABLE();
      sd_spi_xfer(0xFF);
      return response;
    }
  }
  SD_SPI_DISABLE();
  sd_spi_xfer(0xFF);
  return -1; /* Timeout */
}
int _cmdx(int cmd, int arg) {
  uint32_t i;

  SD_SPI_ENABLE();

  /* Send a command */
  sd_spi_xfer(0x40 | cmd);
  sd_spi_xfer(arg >> 24);
  sd_spi_xfer(arg >> 16);
  sd_spi_xfer(arg >> 8);
  sd_spi_xfer(arg >> 0);
  sd_spi_xfer(0x95);

  /* Wait for the response (response[7] == 0) */
  for (i = 0; i < SD_COMMAND_TIMEOUT; i++) {
    int response = sd_spi_xfer(0xFF);
    if (!(response & 0x80)) {
      return response;
    }
  }
  SD_SPI_DISABLE();
  sd_spi_xfer(0xFF);
  return -1; /* Timeout */
}


int _cmd58(void) {
  uint32_t i;
  int arg = 0;

  SD_SPI_ENABLE();

  /* Send a command */
  sd_spi_xfer(0x40 | 58);
  sd_spi_xfer(arg >> 24);
  sd_spi_xfer(arg >> 16);
  sd_spi_xfer(arg >> 8);
  sd_spi_xfer(arg >> 0);
  sd_spi_xfer(0x95);

  /* Wait for the response (response[7] == 0) */
  for (i = 0; i < SD_COMMAND_TIMEOUT; i++) {
    int response = sd_spi_xfer(0xFF);
    if (!(response & 0x80)) {
      int ocr = sd_spi_xfer(0xFF) << 24;
      ocr |= sd_spi_xfer(0xFF) << 16;
      ocr |= sd_spi_xfer(0xFF) << 8;
      ocr |= sd_spi_xfer(0xFF) << 0;
      SD_SPI_DISABLE();
      sd_spi_xfer(0xFF);
      return response;
    }
  }
  SD_SPI_DISABLE();
  sd_spi_xfer(0xFF);
  return -1; /* Timeout */
}

int _cmd8(void) {
  uint32_t i;
  uint8_t j;

  SD_SPI_ENABLE();

  /* Send a command */
  sd_spi_xfer(0x40 | 8); /* CMD8 */
  sd_spi_xfer(0x00);     /* Reserved */
  sd_spi_xfer(0x00);     /* Reserved */
  sd_spi_xfer(0x01);     /* 3.3v */
  sd_spi_xfer(0xAA);     /* Check pattern */
  sd_spi_xfer(0x87);     /* CRC */

  /* Wait for the response (response[7] == 0) */
  for (i = 0; i < SD_COMMAND_TIMEOUT * 1000; i++) {
    char response[5];
    response[0] = sd_spi_xfer(0xFF);
    if (!(response[0] & 0x80)) {
      for (j = 1; j < 5; j++) {
	response[i] = sd_spi_xfer(0xFF);
      }
      SD_SPI_DISABLE();
      sd_spi_xfer(0xFF);
      return response[0];
    }
  }
  SD_SPI_DISABLE();
  sd_spi_xfer(0xFF);
  return -1; /* Timeout */
}

int _block_read(uint8_t *buffer, uint32_t length) {
  uint32_t i, Dummy=Dummy;

  SD_SPI_ENABLE();

  /* Read until start byte (0xFF) */
  while (sd_spi_xfer(0xFF) != 0xFE);

  /* Read a full 512-octet block */
  for (i = 0; i < length; i++) {
    buffer[i] = sd_spi_xfer(0xFF);
  }
  for (i = length; i < 512; i++) {
    Dummy = sd_spi_xfer(0xFF);
  }
  sd_spi_xfer(0xFF); /* checksum */
  sd_spi_xfer(0xFF);

  SD_SPI_DISABLE();
  sd_spi_xfer(0xFF);
  return 0;
}

int _block_write(const uint8_t* buffer, uint32_t length) {
  uint32_t i;

  SD_SPI_ENABLE();

  /* Indicate start of block */
  sd_spi_xfer(0xFE);

  /* Write a full 512-octet block */
  for (i = 0; i < length; i++) {
    sd_spi_xfer(buffer[i]);
  }
  for (i = length; i < 512; i++) {
    sd_spi_xfer(0xFF);
  }

  /* Write the checksum */
  sd_spi_xfer(0xFF);
  sd_spi_xfer(0xFF);

  /* Check the response token */
  if ((sd_spi_xfer(0xFF) & 0x1F) != 0x05) {
    SD_SPI_DISABLE();
    sd_spi_xfer(0xFF);
    return 1;
  }

  /* Wait for write to finish */
  i = 0;
  while (sd_spi_xfer(0xFF) == 0) {
    i++;
  }

  SD_SPI_DISABLE();
  sd_spi_xfer(0xFF);
  return 0;
}

static uint32_t ext_bits(unsigned char *data, int msb, int lsb) {
  uint32_t bits = 0;
  uint32_t size = 1 + msb - lsb;
  uint32_t i;

  for (i = 0; i < size; i++) {
    uint32_t position = lsb + i;
    uint32_t byte = 15 - (position >> 3);
    uint32_t bit = position & 0x7;
    uint32_t value = (data[byte] >> bit) & 1;
    bits |= value << i;
  }
  return bits;
}
/**
 * Prints details about the size of the card.
 */
static void print_card(uint32_t c_size, uint64_t capacity, uint64_t sectors) {
  (void)c_size; // UNUSED
  (void)capacity;
  (void)sectors;

  //uint32_t* capacity_ptr = (uint32_t*)&capacity;
  //uint32_t* sectors_ptr = (uint32_t*)&sectors;

  /* %lld doesn't work, so we have to do this */
  //debug_printf("c_size: %d -- capacity: 0x%04x%08x -- sectors: 0x%04x%08x \n", c_size,
  //	       capacity_ptr[1], capacity_ptr[0], sectors_ptr[1], sectors_ptr[0]);
}

uint64_t _sd_sectors() {
  uint32_t c_size, c_size_mult, read_bl_len;
  uint32_t block_len, mult, blocknr, capacity;
  uint32_t hc_c_size;
  uint64_t blocks;

  /* CMD9, Response R2 (R1 byte + 16-byte block read) */
  if (_cmdx(9, 0) != 0) {
    // Didn't get a response from the disk
    return 0;
  }

  uint8_t csd[16];
  if (_block_read(csd, 16) != 0) {
    // Couldn't read csd response from disk
    return 0;
  }

  // csd_structure : csd[127:126]
  // c_size        : csd[73:62]
  // c_size_mult   : csd[49:47]
  // read_bl_len   : csd[83:80] - the *maximum* read block length

  int csd_structure = ext_bits(csd, 127, 126);

  switch (csd_structure) {
    case 0:
      cdv = 512;
      c_size = ext_bits(csd, 73, 62);
      c_size_mult = ext_bits(csd, 49, 47);
      read_bl_len = ext_bits(csd, 83, 80);

      block_len = 1 << read_bl_len;
      mult = 1 << (c_size_mult + 2);
      blocknr = (c_size + 1) * mult;
      capacity = blocknr * block_len;
      blocks = capacity / 512;
      // debug_puts("SD Card");
      print_card(c_size, capacity, blocks);
      break;

    case 1:
      cdv = 1;
      hc_c_size = ext_bits(csd, 63, 48);
      blocks = (hc_c_size+1)*1024;
      // debug_puts("SDHC Card");
      print_card(hc_c_size, blocks*512, blocks);
      break;

    default:
      // debug_puts("CSD struct unsupported");
      return 0;
  };
  return blocks;
}
