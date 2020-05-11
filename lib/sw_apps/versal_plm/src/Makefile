#/******************************************************************************
#* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
#* SPDX-License-Identifier: MIT
#******************************************************************************/


CC := mb-gcc
CC_FLAGS := -MMD -MP    -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mno-xl-soft-div -mcpu=v10.0 -mno-xl-soft-mul -mxl-multiply-high
CFLAGS := -Os
LN_FLAGS := -Wl,--start-group,-lxil,-lgcc,-lc,--end-group -Wl,--start-group,-lxilffs,-lxilpdi,-lxilplmi,-lxilpuf,-lxilloader,-lxilpm,-lxilsecure,-lxilsem,-lxil,-lgcc,-lc,--end-group -Wl,--gc-sections

c_SOURCES := $(wildcard *.c)
INCLUDES := $(wildcard *.h)
OBJS := $(patsubst %.c, %.o, $(c_SOURCES))
OBJS += $(patsubst %.S, %.o, $(S_SOURCES))
OBJS += $(patsubst %.s, %.o, $(s_SOURCES))
LSCRIPT := -Tlscript.ld

CURRENT_DIR = $(shell pwd)
DEPFILES := $(patsubst %.o, %.d, $(OBJS))
LIBS := ../misc/versal_plm_bsp/psv_pmc_0/lib/libxil.a
EXEC := plm.elf

INCLUDEPATH := -I../misc/versal_plm_bsp/psv_pmc_0/include -I.
LIBPATH := -L../misc/versal_plm_bsp/psv_pmc_0/lib

all: $(EXEC)

$(EXEC): $(LIBS) $(OBJS) $(INCLUDES)
	$(CC) -o $@ $(OBJS) $(CC_FLAGS) $(CFLAGS) $(LN_FLAGS) $(LIBPATH) $(LSCRIPT)

$(LIBS):
	echo "Copying BSP files"
	../misc/copy_bsp.sh
	echo "Compiling bsp"
	$(MAKE) -C ../misc/versal_plm_bsp

%.o:%.c
	$(CC) $(CC_FLAGS) $(CFLAGS) -c $< -o $@ $(INCLUDEPATH)

clean:
	rm -rf $(OBJS) $(LIBS) $(EXEC) *.o *.d
	rm -rf ../misc/versal_plm_bsp
