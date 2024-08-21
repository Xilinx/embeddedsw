#/******************************************************************************
#* Copyright (C) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
#* SPDX-License-Identifier: MIT
#******************************************************************************/


CC := riscv64-unknown-elf-gcc
CC_FLAGS := -MMD -MP -mabi=ilp32 -march=rv32imac_zicsr_zifencei
CFLAGS :=
BSP_FLAGS := -Os -flto -ffat-lto-objects -ggdb3 -gdwarf-5 -c

SYSROOT = $(shell $(CC) -print-sysroot)
MULTILIB = $(SYSROOT)/usr/lib/rv32imc/ilp32

LN_FLAGS = -nostartfiles -L$(MULTILIB) $(MULTILIB)/crt0.o $(MULTILIB)/crtbegin.o $(MULTILIB)/crtend.o $(MULTILIB)/crti.o $(MULTILIB)/crtn.o -Wl,--start-group,-lxil,-lgcc,-lc,--end-group       -Wl,--no-warn-rwx-segment,--gc-sections


c_SOURCES := $(wildcard *.c)
S_SOURCES := $(wildcard *.S)
s_SOURCES := $(wildcard *.s)
INCLUDES := $(wildcard *.h)
OBJS := $(patsubst %.c, %.o, $(c_SOURCES))
OBJS += $(patsubst %.S, %.o, $(S_SOURCES))
OBJS += $(patsubst %.s, %.o, $(s_SOURCES))
LSCRIPT := -Tlscript.ld

CURRENT_DIR = $(shell pwd)
DEPFILES := $(patsubst %.o, %.d, $(OBJS))
LIBS := ../misc/spartanup_plm_bsp/psxl_pmcl_0/lib/libxil.a
EXEC := plm.elf

INCLUDEPATH := -I../misc/spartanup_plm_bsp/psxl_pmcl_0/include
LIBPATH := -L../misc/spartanup_plm_bsp/psxl_pmcl_0/lib

all: $(EXEC)
	riscv64-unknown-elf-size plm.elf

$(EXEC): $(LIBS) $(OBJS) $(INCLUDES)
	$(CC) -o $@ $(sort $(OBJS)) $(CC_FLAGS) $(CFLAGS) $(LN_FLAGS) $(LIBPATH) $(LSCRIPT)

$(LIBS):
	echo "Copying BSP files"
	../misc/copy_bsp.sh
	echo "Compiling bsp"
	$(MAKE) -C ../misc/spartanup_plm_bsp

%.o:%.c
	$(CC) $(CFLAGS) $(BSP_FLAGS) $(CC_FLAGS) -c $< -o $@ $(INCLUDEPATH)

%.o:%.S
	$(CC) $(CFLAGS) $(BSP_FLAGS) $(CC_FLAGS) -c $< -o $@ $(INCLUDEPATH)

%.o:%.s
	$(CC) $(CFLAGS) $(BSP_FLAGS) $(CC_FLAGS) -c $< -o $@ $(INCLUDEPATH)

clean:
	rm -rf $(OBJS) $(LIBS) $(EXEC) *.o *.d
	rm -rf ../misc/spartanup_plm_bsp

-include $(DEPFILES)
