##############################################################################
# Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
#
# SPDX-License-Identifier: MIT
#
##############################################################################

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
AOBJS2	=	boot.o
AOBJS3	=	xil-crt0.o
AOBJS4	=	cpu_init.o
OBJS    =	$(COBJS) $(AOBJS1) $(AOBJS2) $(AOBJS3) $(AOBJS4)

ASOURCES1 = asm_vectors.S
ASOURCES2 = boot.S
ASOURCES3 = xil-crt0.S
ASOURCES4 = cpu_init.S

OBJECTS = $(OBJS)

libs: banner standalone_libs clean

$(AOBJS1): $(ASOURCES1)
ifeq ($(findstring asm_vectors.S,$(wildcard *.S)),asm_vectors.S)
	${COMPILER} $(INCLUDES) --target=arm-arm-none-eabi -E -o asm_vectors_post.s asm_vectors.S
endif
	${AS} --cpu=Cortex-R5 -o asm_vectors.o asm_vectors_post.s

$(AOBJS2): $(ASOURCES2)
ifeq ($(findstring boot.S,$(wildcard *.S)),boot.S)
	${COMPILER} $(INCLUDES) --target=arm-arm-none-eabi -E -o boot_post.s boot.S
endif
	${AS} --cpu=Cortex-R5 -o boot.o boot_post.s

$(AOBJS3): $(ASOURCES3)
ifeq ($(findstring xil-crt0.S,$(wildcard *.S)),xil-crt0.S)
	${COMPILER} $(INCLUDES) --target=arm-arm-none-eabi -E -o xil-crt0_post.s xil-crt0.S
endif
	${AS} --cpu=Cortex-R5 -o xil-crt0.o xil-crt0_post.s

$(AOBJS4): $(ASOURCES4)
ifeq ($(findstring cpu_init.S,$(wildcard *.S)),cpu_init.S)
	${COMPILER} $(INCLUDES) --target=arm-arm-none-eabi -E -o cpu_init_post.s cpu_init.S
endif
	${AS} --cpu=Cortex-R5 -o cpu_init.o cpu_init_post.s

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
