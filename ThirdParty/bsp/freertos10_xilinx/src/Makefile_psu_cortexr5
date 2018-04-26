#
# Copyright (C) 2012-2020 Xilinx, Inc.
#
# This file is part of the port for FreeRTOS made by Xilinx to allow FreeRTOS
# to operate with Xilinx Zynq devices.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software. If you wish to use our Amazon
# FreeRTOS name, please do so in a fair use way that does not cause confusion.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# http://www.FreeRTOS.org
# http://aws.amazon.com/freertos
#
# 1 tab == 4 spaces!
#

#
# Processor architecture
# ps7_cortexa9
#

DRIVER_LIB_VERSION = v1.0

ARCH = psu_cortexr5

SYSTEMDIR = ../../..

TOPDIR = .

ARCH_PREFIX = armr5-none-eabi

#
# gnu tools for Makefile
#
CC = $(ARCH_PREFIX)-gcc
AS = armr5-none-eabi-as
AR = $(ARCH_PREFIX)-ar
CP = cp

ifeq ($(COMPILER) , armr5-none-eabi-gcc)
 EXTRA_COMPILER_FLAGS	=	+= -nostartfiles
endif

EXTRA_COMPILER_FLAGS += -mcpu=cortex-r5 \
		  -mfloat-abi=hard \
                  -mfpu=vfpv3-d16 \

COMPILER_FLAGS += -mcpu=cortex-r5

#
# Compiler, linker and other options.
#
CFLAGS = ${COMPILER_FLAGS} ${EXTRA_COMPILER_FLAGS}

#
# System project directories.
#
LIBDIR = $(SYSTEMDIR)/lib
INCLUDEDIR = $(SYSTEMDIR)/include

# Kernel library.
LIBFREERTOS = ${LIBDIR}/libfreertos.a
LIBXIL = ${LIBDIR}/libxil.a

INCLUDEFILES =	${TOPDIR}/*.h

INCLUDES = -I$(INCLUDEDIR) \
	-I${TOPDIR}

SRCFILES:=$(wildcard *.c)

ASSEMBLY_SRCFILES:=$(wildcard *.S)

OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
ASSEMBLY_OBJECTS  = $(addsuffix .o, $(basename $(wildcard *.S)))

STANDALONE_VERSION = $(wildcard ../../standalone*)

libs: $(LIBFREERTOS) standalone_lib

$(LIBFREERTOS): $(OBJECTS) $(ASSEMBLY_OBJECTS)
	@$(ARCHIVER) -r ${LIBFREERTOS} $^


standalone_lib:
	$(MAKE) -f Makefile_depends -e "COMPILER_FLAGS=$(COMPILER_FLAGS)" "EXTRA_COMPILER_FLAGS=$(EXTRA_COMPILER_FLAGS)" -C ${STANDALONE_VERSION}/src libs

DEPFILES := $(SRCFILES:%.c=%.d)

include $(wildcard $(DEPFILES))

ASSEMBLY_DEPFILES := $(ASSEMBLY_SRCFILES:%.S=%.d)

include $(wildcard $(ASSEMBLY_DEPFILES))

include $(wildcard ../../../../dep.mk)

%.o: %.S
	@$(COMPILER) $(COMPILER_FLAGS) $(CC_FLAG) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) $(DEPENDENCY_FLAGS) -o $@ $<


%.o: %.c
	@$(COMPILER) $(COMPILER_FLAGS) $(CC_FLAG) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) $(DEPENDENCY_FLAGS) -o $@ $<

.PHONY: include
include: freertos_include include_standalone

include_standalone:
	@echo "includes"
	$(MAKE) -f Makefile_depends -e "COMPILER_FLAGS=$(COMPILER_FLAGS)" "EXTRA_COMPILER_FLAGS=$(EXTRA_COMPILER_FLAGS)" -C ${STANDALONE_VERSION}/src include

freertos_include: $(addprefix $(INCLUDEDIR)/,$(wildcard *.h))

$(INCLUDEDIR)/%.h: %.h
	$(CP) $< $@

clean:
	$(MAKE) -f Makefile_depends -C ${STANDALONE_VERSION}/src -s clean
	rm -rf $(LIBFREERTOS)
	rm -rf ${OBJECTS}
	rm -rf $(DEPFILES)
	rm -rf $(ASSEMBLY_DEPFILES)
