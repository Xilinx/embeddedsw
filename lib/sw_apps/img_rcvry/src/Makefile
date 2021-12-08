#/******************************************************************************
#* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
#* SPDX-License-Identifier: MIT
#******************************************************************************/


BSP_DIR	:= ../misc/img_rcvry_bsp
LSCRIPT := -Wl,-T -Wl,lscript.ld
EXEC := ImgRecovery.elf
LIBS = libxil.a
c_SOURCES := $(wildcard *.c)
INCLUDES := $(wildcard *.h)
OBJS := $(patsubst %.c, %.o, $(c_SOURCES))
CFLAGS =  -Wall -O0 -g3 -fmessage-length=0
INCLUDEPATH := -I$(BSP_DIR)/psu_cortexa53_0/include -I. -I../misc/
LIBPATH := $(BSP_DIR)/psu_cortexa53_0/lib
CC      :=      $(CROSS)aarch64-none-elf-gcc
AS      :=      $(CROSS)aarch64-none-elf-gcc
LINKER  :=      $(CROSS)aarch64-none-elf-gcc
ECFLAGS = -march=armv8-a -DARMA53_64 -Os -flto -ffat-lto-objects
LDFLAGS := -L$(LIBPATH) -Wl,--start-group,-lxil,-lgcc,-lc,--end-group -Wl,--start-group,-lxil,-llwip4,-lgcc,-lc,--end-group -Wl,--start-group,-lxilffs,-lxil,-lgcc,-lc,--end-group
BOARD ?= .
all: $(EXEC)

$(EXEC): $(LIBS) $(OBJS) $(INCLUDES)
	$(LINKER) $(LSCRIPT) -L$(LIBPATH) -o $@ $(OBJS) $(LDFLAGS)
	rm -rf $(OBJS)

$(LIBS):
	echo "Copying BSP files"
	../misc/copy_bsp.sh $(BOARD)
	echo "Compiling bsp"
	make -C ../misc

%.o:%.c
	$(CC) $(CFLAGS) $(ECFLAGS) -c $< -o $@ $(INCLUDEPATH)
clean:
	rm -rf $(OBJS) *.elf
	rm -rf ../misc/img_rcvry_bsp
