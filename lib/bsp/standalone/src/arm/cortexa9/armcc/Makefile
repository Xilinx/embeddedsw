###############################################################################
# Copyright (c) 2011 - 2020 Xilinx, Inc.  All rights reserved.
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

ECC_FLAGS	+= --cpu=Cortex-A9 --fpu=VFPv3_FP16 --vectorize

RELEASEDIR=../../../lib
INCLUDEDIR=../../../include

INCLUDES=-I./. -I${INCLUDEDIR}

INCLUDEFILES=*.h

COBJS	=	$(addsuffix .o, $(basename $(wildcard *.c)))
AOBJS1	=	asm_vectors.o
AOBJS2	=	translation_table.o
OBJS    = $(COBJS) $(AOBJS1) $(AOBJS2) boot_post.o

ASOURCES1 = asm_vectors.s
ASOURCES2 = translation_table.s

# Replace boot.o with boot_post.o
OBJECTS = $(OBJS)

libs: boot_libs banner $(LIBS) clean

boot_libs:
ifeq ($(findstring boot.S,$(wildcard *.S)),boot.S)
	${COMPILER} $(INCLUDES) -E -o boot_post.s boot.S
endif
	${AS} --cpu=Cortex-A9 --fpu=VFPv3 -o boot_post.o boot_post.s
	rm -f boot.S

$(AOBJS1): $(ASOURCES1)
	${AS} --cpu=Cortex-A9 --fpu=VFPv3 -o $@ $<

$(AOBJS2): $(ASOURCES2)
	${AS} --cpu=Cortex-A9 --fpu=VFPv3 -o $@ $<

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
