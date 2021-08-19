###############################################################################
# Copyright (C) 2018 - 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################

CC ?= gcc

CFLAGS += -Wall -Wextra
INCLUDEDIR = ../include
LIBDIR = ../src

SRCS = $(wildcard *.c)
APPS = $(patsubst %.c, %, $(SRCS))
APPSTMPS = $(patsubst %.c, %.out, $(SRCS))

all: build

build: $(APPSTMPS)

%.out: %.o
	$(CC) -o $(patsubst %.out, %, $@) $< -L$(LIBDIR) -lxaiengine

%.o: %.c
	$(CC) -I$(INCLUDEDIR) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(APPS) *.o
