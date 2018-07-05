###############################################################################
# Copyright (c) 2011 - 2020 Xilinx, Inc.  All rights reserved.
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

ifeq (($(notdir $(CC))) , arm-xilinx-eabi-gcc)
ECC_FLAGS	+= -nostartfiles\
		  -march=armv7-a \
		  -mfloat-abi=soft \
		  -mfpu=neon
endif

ifeq (($(notdir $(CC))) , arm-none-eabi-gcc)
ECC_FLAGS	+= -nostartfiles
endif

RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

OUTS = *.o
OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
ASSEMBLY_OBJECTS  = $(addsuffix .o, $(basename $(wildcard *.S)))
INCLUDEFILES=*.h

libs: $(LIBS) clean

standalone_libs: $(LIBSOURCES)
	echo "Compiling standalone"
	$(CC) $(CC_FLAGS) $(ECC_FLAGS) $(INCLUDES) $^
	$(AR) -r ${RELEASEDIR}/${LIB} ${OBJECTS} ${ASSEMBLY_OBJECTS}

profile_libs:
	$(MAKE) -C profile COMPILER_FLAGS="$(COMPILER_FLAGS)" EXTRA_COMPILER_FLAGS="$(EXTRA_COMPILER_FLAGS)" COMPILER="$(CC)" ARCHIVER="$(AR)" libs

.PHONY: include
include: standalone_includes profile_includes

standalone_includes:
	${CP} ${INCLUDEFILES} ${INCLUDEDIR}

profile_includes:
	$(MAKE) -C profile COMPILER_FLAGS="$(COMPILER_FLAGS)" EXTRA_COMPILER_FLAGS="$(EXTRA_COMPILER_FLAGS)" COMPILER="$(CC)" ARCHIVER="$(AR)" include

clean:
	rm -rf ${OBJECTS}
	rm -rf ${ASSEMBLY_OBJECTS}
	$(MAKE) -C profile COMPILER_FLAGS="$(COMPILER_FLAGS)" EXTRA_COMPILER_FLAGS="$(EXTRA_COMPILER_FLAGS)" COMPILER="$(CC)" ARCHIVER="$(AR)" clean
