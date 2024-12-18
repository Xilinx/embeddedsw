#/******************************************************************************
#* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
#* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
#* SPDX-License-Identifier: MIT
#******************************************************************************/

CC := mb-gcc
CC_FLAGS := -MMD -MP -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mno-xl-soft-div -mcpu=v11.0 -mno-xl-soft-mul -mxl-multiply-high
CFLAGS := -Os -flto=16 -ffat-lto-objects
_LN_XILPM_FLAGS:=--whole-archive

XILPM_BOOT_LIB ?= 1
XILPM_RUNTIME_LIB ?= 0
ifeq ($(XILPM_RUNTIME_LIB),1)
	XILPM_BOOT_LIB = 1
	# For now, we are using only xilpm_runtime (eemi) library by default if XILPM_RUNTIME_LIB is enabled
	XILPM_RUNTIME_FLAGS := -DXILPM_RUNTIME -DXILPM_RUNTIME_EEMI
	# To use xilpm_runtime (subsys) library, uncomment below line
	# XILPM_RUNTIME_FLAGS := "-DXILPM_RUNTIME -DXILPM_RUNTIME_SUBSYS"
else ifeq ($(XILPM_RUNTIME_LIB),EEMI)
	XILPM_BOOT_LIB = 1
	XILPM_RUNTIME_LIB = 1
	# Enable xilpm_runtime (eemi) library
	XILPM_RUNTIME_FLAGS := -DXILPM_RUNTIME -DXILPM_RUNTIME_EEMI
else ifeq ($(XILPM_RUNTIME_LIB),SUBSYS)
	XILPM_BOOT_LIB = 1
	XILPM_RUNTIME_LIB = 1
	# Enable xilpm_runtime (subsys) library
	XILPM_RUNTIME_FLAGS := -DXILPM_RUNTIME -DXILPM_RUNTIME_SUBSYS
endif
ifeq ($(XILPM_BOOT_LIB), 1)
	XILPM_BOOT_FLAGS := -DXILPM_BOOT
endif
ifeq ($(XILPM_BOOT_LIB), 1)
	_LN_XILPM_FLAGS:=$(strip $(_LN_XILPM_FLAGS),-lxilpm)
endif
ifneq ($(filter-out 0,$(XILPM_RUNTIME_LIB)),)
	_LN_XILPM_FLAGS:=$(strip $(_LN_XILPM_FLAGS),-lxilpm_runtime)
endif
_LN_XILPM_FLAGS:=$(strip $(_LN_XILPM_FLAGS),--no-whole-archive)
_CC_XILPM_FLAGS += $(XILPM_BOOT_FLAGS) $(XILPM_RUNTIME_FLAGS)
LN_FLAGS := -fdiagnostics-color=always -Wl,--start-group,-lxil,-lgcc,-lc,--end-group -Wl,--start-group,-lxilffs,-lxilpdi,-lxilplmi,-lxilpuf,-lxilloader,$(_LN_XILPM_FLAGS),-lxilsecure,-lxilnvm,-lxilocp,-lxilcert,-lxil,-lgcc,-lc,--end-group -Wl,--gc-sections
c_SOURCES := $(wildcard *.c)
c_SOURCES += $(wildcard ../common/*.c)
INCLUDES := $(wildcard *.h)
INCLUDES += $(wildcard ../common/*.h)
OBJS := $(patsubst %.c, %.o, $(c_SOURCES))
OBJS += $(patsubst %.S, %.o, $(S_SOURCES))
OBJS += $(patsubst %.s, %.o, $(s_SOURCES))
LSCRIPT := -Tlscript.ld

CURRENT_DIR = $(shell pwd)
DEPFILES := $(patsubst %.o, %.d, $(OBJS))
LIBS := ../../misc/versal_aiepg2/versal_plm_bsp/psx_pmc_0/lib/libxil.a
EXEC := plm.elf

INCLUDEPATH := -I../../misc/versal_aiepg2/versal_plm_bsp/psx_pmc_0/include -I. -I../common/
LIBPATH := -L../../misc/versal_aiepg2/versal_plm_bsp/psx_pmc_0/lib
CFLAGS += $(_CC_XILPM_FLAGS)
all: $(EXEC)

$(EXEC): $(LIBS) $(OBJS) $(INCLUDES)
	$(CC) -o $@ $(OBJS) $(CC_FLAGS) $(CFLAGS) $(LN_FLAGS) $(LIBPATH) $(LSCRIPT)

$(OBJS): $(LIBS)

$(LIBS):
	echo "Copying BSP files"
	../../misc/versal_aiepg2/copy_bsp.sh
	echo "Compiling bsp"
	$(MAKE) _CC_XILPM_FLAGS="$(_CC_XILPM_FLAGS)" -C ../../misc/versal_aiepg2/versal_plm_bsp

%.o:%.c
	$(CC) $(CC_FLAGS) $(CFLAGS) -c $< -o $@ $(INCLUDEPATH)

clean:
	rm -rf $(OBJS) $(LIBS) $(EXEC) *.o *.d ../common/*.o ../common/*.d
	rm -rf ../../misc/versal_aiepg2/versal_plm_bsp
