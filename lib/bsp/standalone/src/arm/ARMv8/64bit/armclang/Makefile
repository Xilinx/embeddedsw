###############################################################################
#
# Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
# XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Except as contained in this notice, the name of the Xilinx shall not be used
# in advertising or otherwise to promote the sale, use or other dealings in
# this Software without prior written authorization from Xilinx.
#
###############################################################################

include config.make

AS=armasm
COMPILER=armclang
ARCHIVER=armar
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=

LIB=libxil.a

CC_FLAGS = $(COMPILER_FLAGS)
ECC_FLAGS = $(EXTRA_COMPILER_FLAGS)

RELEASEDIR=../../../lib
INCLUDEDIR=../../../include

INCLUDES=-I./. -I${INCLUDEDIR}

INCLUDEFILES=*.h

COBJS	=	$(addsuffix .o, $(basename $(wildcard *.c)))
AOBJS1	=	asm_vectors.o
AOBJS2	=	translation_table.o
AOBJS3	=	boot.o
OBJS    =	$(COBJS) $(AOBJS1) $(AOBJS2) $(AOBJS3)

ASOURCES1 = asm_vectors.S
ASOURCES2 = translation_table.S
ASOURCES3 = boot.S

OBJECTS = $(OBJS)

libs: banner standalone_libs clean

$(AOBJS1): $(ASOURCES1)
ifeq ($(findstring asm_vectors.S,$(wildcard *.S)),asm_vectors.S)
	${COMPILER} $(INCLUDES) --target=aarch64-arm-none-eabi -E -o asm_vectors_post.s asm_vectors.S
endif
	${AS} --cpu=8-A.64 --fpu=fp-armv8 -o asm_vectors.o asm_vectors_post.s

$(AOBJS2): $(ASOURCES2)
ifeq ($(findstring translation_table.S,$(wildcard *.S)),translation_table.S)
	${COMPILER} $(INCLUDES) --target=aarch64-arm-none-eabi -E -o translation_table_post.s translation_table.S
endif
	${AS} --cpu=8-A.64 --fpu=fp-armv8 -o translation_table.o translation_table_post.s

$(AOBJS3): $(ASOURCES3)
ifeq ($(findstring boot.S,$(wildcard *.S)),boot.S)
	${COMPILER} $(INCLUDES) --target=aarch64-arm-none-eabi -E -o boot_post.s boot.S
endif
	${AS} --cpu=8-A.64 --fpu=fp-armv8 -o boot.o boot_post.s

%.o: %.c
	${COMPILER} -c $(CC_FLAGS) $(ECC_FLAGS) $(INCLUDES) -o $@ $<

banner:
	echo "Compiling standalone"

standalone_libs: ${OBJECTS}
	$(ARCHIVER) -r ${RELEASEDIR}/${LIB} ${OBJECTS}

.PHONY: include
include: standalone_includes

standalone_includes:
	${CP} ${INCLUDEFILES} ${INCLUDEDIR}

clean:
	rm -rf ${OBJECTS}
