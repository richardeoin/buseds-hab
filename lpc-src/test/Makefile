# Compiles various unit tests
# Copyright (C) 2013  Richard Meadows
# 
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
# 
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

-include ../makefile.conf

# Compilation Flags
#
# Display all warnings. Compile functions and data into their own sections so
# they can be discarded if unused.  The linker performs garbage collection of
# unused input sections.
#
CFLAGS	= $(FLAGS) -Wall -Wextra -std=gnu99 -ffunction-sections -fdata-sections

all: square-test rtty-test tmp102-test

square-test: ../src/square.c
	$(CC) $(CFLAGS) -D SQUARE_TEST $(addprefix -I ../,$(INCLUDES)) -o $@ $<

rtty-test: ../src/rtty.c
	$(CC) $(CFLAGS) -D RTTY_TEST $(addprefix -I ../,$(INCLUDES)) -o $@ $<

tmp102-test: ../src/tmp102.c ../src/i2c.c
	$(CC) $(CFLAGS) -D TMP102_TEST $(addprefix -I ../,$(INCLUDES)) -o $@ $< ../src/i2c.c