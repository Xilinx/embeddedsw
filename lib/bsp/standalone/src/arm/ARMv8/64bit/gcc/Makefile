###############################################################################
#
# Copyright (C) 2014 - 2015 Xilinx, Inc. All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
#
#
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
libs: $(LIBS)

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
