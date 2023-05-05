#/******************************************************************************
#* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
#* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
#* SPDX-License-Identifier: MIT
#******************************************************************************/

CC := mb-gcc
CC_FLAGS := -MMD -MP    -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mno-xl-soft-div -mcpu=v10.0 -mno-xl-soft-mul -mxl-multiply-high
CFLAGS := -Os
LN_FLAGS := -Wl,--start-group,-lxil,-lgcc,-lc,--end-group
ifeq ($(findstring gcc, $(CC)), gcc)
    CUSTOM_COMPILER_FLAGS+=-Wall -Wextra -Warray-bounds=2 -Wformat=2 -Wno-main -Wimplicit-function-declaration -Wmissing-prototypes -Wmissing-braces -Wmultistatement-macros -Wparentheses -Wreturn-type -Wsequence-point -Wbool-compare -Wbool-operation -Wchar-subscripts -Wenum-compare -Winit-self -fno-strict-aliasing -Wtautological-compare -Wuninitialized -Wunused-function -Wunused-label -Wunused-variable -Wcast-function-type -Wenum-conversion -Wmissing-parameter-type -Wtype-limits -Wshift-negative-value -Wshadow -Wdouble-promotion -Wundef -fno-common -Wconversion -Wnull-dereference -Wmissing-include-dirs -Wswitch-default -Wswitch-enum -Wswitch-bool -Wshift-overflow=2 -Wswitch-outside-range -Wswitch-unreachable -Wunused-parameter -Wduplicated-branches -Wduplicated-cond -Wdiv-by-zero -Wshadow=local -Wpointer-arith -Wunused-macros -Wconversion -Wdangling-else -Wlogical-op -Waggressive-loop-optimizations -Wstrict-prototypes -Woverflow -Wredundant-decls -Wvla
endif

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
LIBS := ../../misc/versal_psmfw_bsp/psv_psm_0/lib/libxil.a
EXEC := psmfw.elf

INCLUDEPATH := -I../../misc/versal_psmfw_bsp/psv_psm_0/include -I. -I../common
LIBPATH := -L../../misc/versal_psmfw_bsp/psv_psm_0/lib

all: $(EXEC)

$(EXEC): $(LIBS) $(OBJS) $(INCLUDES)
	$(CC) -o $@ $(OBJS) $(CC_FLAGS) $(CFLAGS) $(LN_FLAGS) $(LIBPATH) $(LSCRIPT)

$(OBJS): $(LIBS)

$(LIBS):
	echo "Copying BSP files"
	../../misc/copy_bsp.sh
	echo "Compiling bsp"
	$(MAKE) -C ../../misc/versal_psmfw_bsp

%.o:%.c
	$(CC) $(CC_FLAGS) $(CFLAGS) $(CUSTOM_COMPILER_FLAGS) -c $< -o $@ $(INCLUDEPATH)

clean:
	rm -rf $(OBJS) $(LIBS) $(EXEC) *.o *.d ../common/*.o ../common/*.d
	rm -rf ../../misc/versal_psmfw_bsp
