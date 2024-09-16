#/*************************************************************************************************
#* Copyright (C) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
#* SPDX-License-Identifier: MIT
#*************************************************************************************************/


CC := riscv64-unknown-elf-gcc
CC_FLAGS := -MMD -MP -mabi=ilp32 -march=rv32imac_zicsr_zifencei
CFLAGS := -O2 -flto -ffat-lto-objects

SYSROOT = $(shell $(CC) -print-sysroot)
MULTILIB = $(SYSROOT)/usr/lib/rv32imc/ilp32

LN_FLAGS = -nostartfiles -L$(MULTILIB) $(MULTILIB)/crt0.o $(MULTILIB)/crtbegin.o $(MULTILIB)/crtend.o $(MULTILIB)/crti.o $(MULTILIB)/crtn.o -Wl,--start-group,-lxil,-lxilasu,-lgcc,-lc,--end-group       -Wl,--no-warn-rwx-segment,--gc-sections

c_SOURCES := $(wildcard *.c) $(wildcard */*.c)
S_SOURCES := $(wildcard *.S) $(wildcard */*.S)
s_SOURCES := $(wildcard *.s) $(wildcard */*.s)
INCLUDES := $(wildcard *.h) $(wildcard */*.h)
OBJS := $(patsubst %.c, %.o, $(c_SOURCES))
OBJS += $(patsubst %.S, %.o, $(S_SOURCES))
OBJS += $(patsubst %.s, %.o, $(s_SOURCES))
LSCRIPT := -Tlscript.ld

CURRENT_DIR = $(shell pwd)
DEPFILES := $(patsubst %.o, %.d, $(OBJS))
LIBS := ../misc/asufw_bsp/asu_0/lib/libxil.a
EXEC := asufw.elf

INCLUDEPATH := -I../misc/asufw_bsp/asu_0/include -I./ -I./task -I./xfih -I./sha -I./trng -I./ecc -I./aes -I./rsa_ecc
LIBPATH := -L../misc/asufw_bsp/asu_0/lib

all: $(EXEC)

$(EXEC): $(LIBS) $(OBJS) $(INCLUDES)
	$(CC) -o $@ $(sort $(OBJS)) $(CC_FLAGS) $(CFLAGS) $(LN_FLAGS) $(LIBPATH) $(LSCRIPT)

$(LIBS):
	echo "Copying BSP files"
	../misc/copy_bsp.sh
	echo "Compiling bsp"
	$(MAKE) -C ../misc/asufw_bsp/

%.o:%.c
	$(CC) $(CFLAGS) $(CC_FLAGS) -c $< -o $@ $(INCLUDEPATH)

%.o:%.S
	$(CC) $(CFLAGS) $(CC_FLAGS) -c $< -o $@ $(INCLUDEPATH)

%.o:%.s
	$(CC) $(CFLAGS) $(CC_FLAGS) -c $< -o $@ $(INCLUDEPATH)

clean:
	rm -rf $(OBJS) $(LIBS) $(EXEC) *.o *.d */*.o */*.d
	rm -rf ../misc/asufw_bsp

-include $(DEPFILES)
