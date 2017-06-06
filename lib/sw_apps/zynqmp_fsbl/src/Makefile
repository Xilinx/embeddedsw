###############################################################################
#
# Copyright (C) 2012 - 2014 Xilinx, Inc.  All rights reserved.
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
# Use of the Software is limited solely to applications:
# (a) running on a Xilinx device, or
# (b) that interact with a Xilinx device through a bus or interconnect.
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
BOARD := zcu102
PROC ?= a53
A53_STATE ?= 64

c_SOURCES := $(wildcard *.c)
c_SOURCES += psu_init.c
S_SOURCES := $(wildcard *.S)
#remove unnecessary translation table files
ifeq "${PROC}" "a53"
ifeq "${A53_STATE}" "64"
S_SOURCES := $(filter-out xfsbl_translation_table_a53_32.S,$(S_SOURCES))
endif
ifeq "${A53_STATE}" "32"
S_SOURCES := $(filter-out xfsbl_translation_table_a53_64.S,$(S_SOURCES))
endif
endif
ifeq "${PROC}" "r5"
S_SOURCES := $(filter-out xfsbl_translation_table_a53_32.S xfsbl_translation_table_a53_64.S,$(S_SOURCES))
endif

s_SOURCES := $(wildcard *.s)
INCLUDES := $(wildcard *.h)
OBJS := $(patsubst %.c, %.o, $(c_SOURCES))
OBJS += $(patsubst %.S, %.o, $(S_SOURCES))

OBJS += $(patsubst %.s, %.o, $(s_SOURCES))

MISC_DIR := ../misc
BSP_DIR	:= $(MISC_DIR)/zynqmp_fsbl_bsp

ifeq "${PROC}" "a53"
ifeq "${A53_STATE}" "64"
CC := aarch64-none-elf-gcc
CC_FLAGS := -MMD -MP -Wall -fmessage-length=0 -DARMA53_64 -Wall -Wextra -Os -flto -ffat-lto-objects
LSCRIPT := -Tlscript_a53.ld
endif
ifeq "${A53_STATE}" "32"
CC := arm-none-eabi-gcc
CC_FLAGS := -MMD -MP -Wall -fmessage-length=0 -march=armv7-a -DARMA53_32 -Wall -Wextra -Os -flto -ffat-lto-objects
#linker script for a53_32 and r5 is same.
LSCRIPT := -Tlscript.ld
endif
INCLUDEPATH := -I$(BSP_DIR)/psu_cortexa53_0/include -I.
LIBPATH := -L$(BSP_DIR)/psu_cortexa53_0/lib
LIBS := $(BSP_DIR)/psu_cortexa53_0/lib/libxil.a
endif
ifeq "${PROC}" "r5"
A53_STATE :=
CC := armr5-none-eabi-gcc
CC_FLAGS := -MMD -MP -Wall -fmessage-length=0 -mcpu=cortex-r5 -mfloat-abi=hard -DARMR5 -Wall -Wextra -mfpu=vfpv3-d16 -Os -flto -ffat-lto-objects
LSCRIPT := -Tlscript.ld
INCLUDEPATH := -I$(BSP_DIR)/psu_cortexr5_0/include -I. -I$(MISC_DIR)/$(BOARD)
LIBPATH := -L$(BSP_DIR)/psu_cortexr5_0/lib
LIBS := $(BSP_DIR)/psu_cortexr5_0/lib/libxil.a
endif

override CFLAGS := -DXPS_BOARD_ZCU102 $(CFLAGS)
ECFLAGS :=
LDFLAGS := -Wl,--start-group,-lxil,-lgcc,-lc,-lxilffs,-lxilsecure,-lxilpm,--end-group
#	-Wl,--start-group,-lxilffs,-lxil,-lgcc,-lc,--end-group \
#	-Wl,--start-group,-lxilsecure,-lxil,-lgcc,-lc,--end-group  -Wl,--gc-sections

EXEC := fsbl.elf

all: $(EXEC)

$(EXEC): $(LIBS) $(OBJS) $(INCLUDES)
	$(CC) -o $@ $(OBJS) $(CFLAGS) $(CC_FLAGS) $(LDFLAGS) $(LIBPATH) $(LSCRIPT)
	rm -rf $(OBJS) *.d

$(LIBS):
	cp $(MISC_DIR)/$(BOARD)/psu_init.* .
	@echo "Copying BSP files"
	$(MISC_DIR)/copy_bsp.sh "$(BOARD)" "$(PROC)" "$(A53_STATE)"
	@echo "Compiling bsp"
ifeq "${PROC}" "a53"
	make -C $(MISC_DIR) "PROC=$(PROC)" "A53_STATE=$(A53_STATE)"
endif
ifeq "${PROC}" "r5"
	make -C $(MISC_DIR) "PROC=$(PROC)"
endif

$(OBJS): $(LIBS) $(INCLUDES) psu_init.c

%.o:%.c
		$(CC) $(CC_FLAGS) $(CFLAGS) -c $< -o $@ $(INCLUDEPATH)

%.o:%.S
		$(CC) $(CC_FLAGS) $(CFLAGS) -c $< -o $@ $(INCLUDEPATH)

%.o:%.s
		$(CC) $(CC_FLAGS) $(CFLAGS) -c $< -o $@ $(INCLUDEPATH)

clean:
	rm -rf $(OBJS) $(BSP_DIR) $(LIBS) $(EXEC) *.d psu_init*
