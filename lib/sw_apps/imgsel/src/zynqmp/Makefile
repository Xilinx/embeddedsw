#/******************************************************************************
#* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
#* SPDX-License-Identifier: MIT
#******************************************************************************/


BSP_DIR := ../../misc/imgsel_bsp
ASFLAGS = --cpu=8-A.64 --fpu=fp-armv8
LSCRIPT := -Tlscript_a53.ld
BOARD   ?= zcu102
EXEC := imgsel.elf
LIBS = libxil.a
c_SOURCES := $(wildcard *.c)
c_SOURCES += $(wildcard ../common/*.c)
INCLUDES := $(wildcard ../common/*.h)
INCLUDES += $(wildcard *.h)
S_SOURCES := xis_translation_table_a53_64.S
OBJS := $(patsubst %.c, %.o, $(c_SOURCES))
LSCRIPT :=  -Tlscript.ld
INCLUDEPATH := -I$(BSP_DIR)/psu_cortexa53_0/include -I. -I../common -I../../misc -I../../misc/$(BOARD)

CC      :=      aarch64-none-elf-gcc
AS      :=      aarch64-none-elf-gcc
LINKER  :=      aarch64-none-elf-gcc
DUMP    :=      aarch64-none-elf-objdump -xSD
LIBPATH := 		$(BSP_DIR)/psu_cortexa53_0/lib
LDFLAGS := -Wl,--start-group,-lxil,-lgcc,-lc,--end-group -L$(LIBPATH) -L./ -Wl,--build-id=none
OBJS += $(patsubst %.S, %.o, $(S_SOURCES))

ifeq "$(BOARD)" "som"
	ECFLAGS :=-Wall -O0 -g3 -c -fmessage-length=0 -DXIS_UPDATE_A_B_MECHANISM
else
	ECFLAGS :=-Wall -O0 -g3 -c -fmessage-length=0
endif

all: $(EXEC)

$(EXEC): $(LIBS) $(OBJS) $(INCLUDES)
	$(LINKER) -o $@ $(OBJS) $(CC_FLAGS) $(LDFLAGS) $(LSCRIPT)
	$(DUMP) $(EXEC)  > dump
	rm -rf $(OBJS)

$(LIBS):
	echo "Copying BSP files"
	../../misc/copy_bsp.sh $(BOARD)
	echo "Compiling bsp"
	make -C ../../misc BOARD=$(BOARD) PROC=a53 A53_STATE=64

%.o:%.c
	$(CC) $(CC_FLAGS) $(CFLAGS) $(ECFLAGS) -c $< -o $@ $(INCLUDEPATH)
%.o:%.S
	$(AS) $(CC_FLAGS) $(CFLAGS) $(ECFLAGS) -c $< -o $@ $(INCLUDEPATH)
%.o:%.s
	$(AS) $(CC_FLAGS) $(CFLAGS) $(ECFLAGS) -c $< -o $@ $(INCLUDEPATH)
clean:
	rm -rf $(OBJS) *.elf dump
	rm -rf ../../misc/imgsel_bsp
