#/******************************************************************************
#* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
#* Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
#* SPDX-License-Identifier: MIT
#******************************************************************************/


CC := mb-gcc
CC_FLAGS := -MMD -MP    -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mno-xl-soft-div -mcpu=v10.0 -mno-xl-soft-mul -mxl-multiply-high
CFLAGS := -Os -flto -ffat-lto-objects
LN_FLAGS := -Wl,--start-group,-lxil,-lgcc,-lc,--end-group -Wl,--start-group,-lxilffs,-lxilpdi,-lxilplmi,-lxilpuf,-lxilloader,-lxilpm,-lxilsecure,-lxil,-lgcc,-lc,--end-group -Wl,--gc-sections

c_SOURCES := $(wildcard *.c)
INCLUDES := $(wildcard *.h)
OBJS := $(patsubst %.c, %.o, $(c_SOURCES))
OBJS += $(patsubst %.S, %.o, $(S_SOURCES))
OBJS += $(patsubst %.s, %.o, $(s_SOURCES))
LSCRIPT := -Tlscript.ld

CURRENT_DIR = $(shell pwd)
DEPFILES := $(patsubst %.o, %.d, $(OBJS))
LIBS := ../../misc/versal/imgsel_bsp/psv_pmc_0/lib/libxil.a
EXEC := imgsel.elf

INCLUDEPATH := -I../../misc/versal/imgsel_bsp/psv_pmc_0/include -I.
LIBPATH := -L../../misc/versal/imgsel_bsp/psv_pmc_0/lib

all: $(EXEC)

$(EXEC): $(LIBS) $(OBJS) $(INCLUDES)
	$(CC) -o $@ $(OBJS) $(CC_FLAGS) $(CFLAGS) $(LN_FLAGS) $(LIBPATH) $(LSCRIPT)

$(LIBS):
	echo "Copying BSP files"
	../../misc/versal/copy_bsp.sh
	echo "Compiling bsp"
	$(MAKE) -C ../../misc/versal/imgsel_bsp

%.o:%.c
	$(CC) $(CC_FLAGS) $(CFLAGS) -c $< -o $@ $(INCLUDEPATH)

clean:
	rm -rf $(OBJS) $(LIBS) $(EXEC) *.o *.d
	rm -rf ../../misc/versal/imgsel_bsp
