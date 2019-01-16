###############################################################################
#
# Copyright (C) 2011 - 2015 Xilinx, Inc.  All rights reserved.
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

AS=iasmarm
COMPILER=iccarm
ARCHIVER=iarchive
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=

LIB=libxil.a

CC_FLAGS = $(COMPILER_FLAGS)
ECC_FLAGS = $(EXTRA_COMPILER_FLAGS)

ECC_FLAGS	+= --fpu=VFPv3

RELEASEDIR=../../../lib
INCLUDEDIR=../../../include

INCLUDES=-I./. -I${INCLUDEDIR}

INCLUDEFILES=*.h

OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
OBJECTS +=	$(addsuffix .o, $(basename $(wildcard *.s)))
#OBJECTS +=	boot.o

libs: banner $(LIBS) clean

#boot.s: boot.S
#	${COMPILER} $(INCLUDES) -E -o $@ $<
#	rm -f boot.S

%.o: %.s
	${AS} --cpu Cortex-A9 --fpu VFPv3 $(INCLUDES) -o $@ $<

%.o: %.c
	${COMPILER} $(CC_FLAGS) $(ECC_FLAGS) $(INCLUDES) -o $@ $<

banner:
	echo "Compiling standalone"
	echo "${COMPILER} $(CC_FLAGS) $(ECC_FLAGS) $(INCLUDES) -o $@ $<"

standalone_libs: ${OBJECTS}
	$(ARCHIVER) -r ${RELEASEDIR}/${LIB} ${OBJECTS}

.PHONY: include
include: standalone_includes

standalone_includes:
	${CP} ${INCLUDEFILES} ${INCLUDEDIR}

clean:
	rm -rf ${OBJECTS}
