/*
 * Manages writes to the SD card
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

#include "LPC11xx.h"
#include "sd.h"

/**
 * The index of the next available block is stored in block 0
 */

uint32_t next_block = 0;

uint32_t get_next_block(void) {
  uint32_t block;

  disk_read((uint8_t*)(&block), 4, 0);

  return block;
}
int set_next_block(uint32_t block) {
  return disk_write((uint8_t*)(&block), 4, 0);
}

/**
 * Writes up to 512 octets to a single block. The write is performed
 * against the next available block.
 */
int disk_write_next_block(const uint8_t *buffer, uint32_t length) {
  if (!next_block) { /* We need to grab the next block index */
    next_block = get_next_block();
  }

  if (disk_write(buffer, length, next_block++) == 0) { // Success
    return set_next_block(next_block); /* Write new position to card */
  }

  return 0; // Fail
}
