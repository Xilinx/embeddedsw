###############################################################################
# Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################
include config.make

CC=$(COMPILER)
AR=$(ARCHIVER)
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=
LIB=libxil.a
CC_FLAGS = $(subst -pg, -DPROFILING, $(COMPILER_FLAGS))
ECC_FLAGS = $(subst -pg, -DPROFILING, $(EXTRA_COMPILER_FLAGS))

ECC_FLAGS	+= -nostartfiles\
		  -march=armv8-a
ECC_FLAGS_NO_FLTO1 = $(subst -flto,,$(ECC_FLAGS))
ECC_FLAGS_NO_FLTO = $(subst -ffat-lto-objects,,$(ECC_FLAGS_NO_FLTO1))


RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}


OUTS = *.o
OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
ASSEMBLY_OBJECTS  = $(addsuffix .o, $(basename $(wildcard *.S)))
INCLUDEFILES=*.h
INCLUDEFILES+=includes_ps/*.h
libs: $(LIBS) clean

standalone_libs: $(LIBSOURCES)
	echo "Compiling standalone ARMv8 64 bit"
	$(CC) $(CC_FLAGS) $(ECC_FLAGS) $(INCLUDES) $(filter-out _exit.c, $^)
	$(CC) $(CC_FLAGS) $(ECC_FLAGS_NO_FLTO) $(INCLUDES) _exit.c
	$(AR) -r ${RELEASEDIR}/${LIB} ${OBJECTS} ${ASSEMBLY_OBJECTS}

.PHONY: include
include: standalone_includes

standalone_includes:
	${CP} ${INCLUDEFILES} ${INCLUDEDIR}

clean:
	rm -rf ${OBJECTS}
	rm -rf ${ASSEMBLY_OBJECTS}
