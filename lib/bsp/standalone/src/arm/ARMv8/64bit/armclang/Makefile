###############################################################################
# Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################

include config.make

AS=$(ASSEMBLER)
COMPILER=$(COMPILER)
ARCHIVER=$(ARCHIVER)
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
