# Makefile for Xilinx MCAP Interface Library
# (c) 2014-2015 Xilinx Inc.

PCIUTILS_PATH =

PCILIB_PATH=$(PCIUTILS_PATH)/lib/

INCLUDES = -I$(PCILIB_PATH) -I$(PCIUTILS_PATH) -I.

OPT=-O2
CFLAGS=$(OPT) -Wall -W -Wno-parentheses -Wstrict-prototypes -Wmissing-prototypes $(INCLUDES)

LDFLAGS = -L$(PCILIB_PATH)
LDLIBS = -lpci

PCILIB = $(PCILIB_PATH)/libpci.a

MCAPLIB = libmcap.a
MCAPINC = mcap_lib.h

OBJ = mcap_lib.o

all: $(MCAPLIB) mcap

$(MCAPLIB): $(OBJ)
	ar rcs $(MCAPLIB) $(OBJ)

$(OBJ):
	gcc $(LDFLAGS) $(CFLAGS) -c mcap_lib.c $< -o $@ $(LDLIBS)

mcap: mcap.o
	gcc $(CFLAGS) mcap.c $(MCAPLIB) $(PCILIB) -lz -o mcap

clean:
	rm -f *.o *.a mcap
