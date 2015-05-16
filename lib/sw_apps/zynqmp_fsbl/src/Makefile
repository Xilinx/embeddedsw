#/******************************************************************************
#*
#* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
#*
#* Permission is hereby granted, free of charge, to any person obtaining a copy
#* of this software and associated documentation files (the "Software"), to deal
#* in the Software without restriction, including without limitation the rights
#* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#* copies of the Software, and to permit persons to whom the Software is
#* furnished to do so, subject to the following conditions:
#*
#* The above copyright notice and this permission notice shall be included in
#* all copies or substantial portions of the Software.
#*
#* Use of the Software is limited solely to applications:
#* (a) running on a Xilinx device, or
#* (b) that interact with a Xilinx device through a bus or interconnect.
#*
#* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
#* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
#* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#* SOFTWARE.
#*
#* Except as contained in this notice, the name of the Xilinx shall not be used
#* in advertising or otherwise to promote the sale, use or other dealings in
#* this Software without prior written authorization from Xilinx.
#*
#******************************************************************************/

PROC ?= a53
CROSS ?=
BSP_DIR	:= .

ECFLAGS :=
LSCRIPT := -Tlscript_a53.ld
BOARD	:=
EXEC := ron_a53_fsbl.elf

c_SOURCES := $(wildcard *.c)
S_SOURCES := $(wildcard *.S)
s_SOURCES := $(wildcard *.s)
INCLUDES := $(wildcard *.h)
OBJS := $(patsubst %.c, %.o, $(c_SOURCES))
OBJS += $(patsubst %.S, %.o, $(S_SOURCES))
OBJS += $(patsubst %.s, %.o, $(s_SOURCES))

ifeq '$(PROC)' 'r5'
CROSS   :=
CC      :=      $(CROSS)arm-none-eabi-gcc
AS      :=      $(CROSS)arm-none-eabi-gcc
LINKER  :=      $(CROSS)arm-none-eabi-gcc
DUMP    :=      $(CROSS)arm-none-eabi-objdump -xSD
CFLAGS =  -Wall -O0 -g3 -fmessage-length=0 \
		  -mcpu=cortex-r5 -mfloat-abi=softfp
LSCRIPT := -Tlscript.ld
EXEC := ron_r5_fsbl.elf
INCLUDEPATH := -I$(BSP_DIR)/psu_cortexr5_0/include -I.
LIBPATH := $(BSP_DIR)/psu_cortexr5_0/lib
LDFLAGS := -Wl,--start-group,-lxil,-lxilffs,-lgcc,-lc,--end-group -L$(LIBPATH) -L./ -Wl,--build-id=none
endif

ifeq '$(PROC)' 'a53'
CROSS   :=
CC      :=      $(CROSS)aarch64-none-elf-gcc
AS      :=      $(CROSS)aarch64-none-elf-gcc
LINKER  :=      $(CROSS)aarch64-none-elf-gcc
DUMP    :=      $(CROSS)aarch64-none-elf-objdump -xSD
CFLAGS =  -Wall -O0 -g3 -fmessage-length=0 \
		  -march=armv8-a -DXFSBL_A53
LSCRIPT := -Tlscript_a53.ld
EXEC := ron_a53_fsbl.elf
INCLUDEPATH := -I$(BSP_DIR)/psu_cortexa53_0/include -I.
LIBPATH := $(BSP_DIR)/psu_cortexa53_0/lib
LDFLAGS := -Wl,--start-group,-lxil,-lxilffs,-lgcc,-lc,--end-group -L$(LIBPATH) -L./ -Wl,--build-id=none
endif

ifeq "$(CC)" "armcc"
AS=armasm
LINKER=armlink
CFLAGS += -c --c99 --wchar32
CC_FLAGS	+= --cpu=Cortex-A9 --fpu=VFPv3_FP16
LDFLAGS = --entry=_vector_table "$(LIBPATH)/libxil.a(*.o)" --no_search_dynamic_libraries --userlibpath=$(LIBPATH),. --library=xil
LSCRIPT = --scatter="scatter.scat"
endif

all: $(EXEC)

$(EXEC): $(OBJS) $(INCLUDES)
	$(LINKER) -o $@ $(OBJS) $(CC_FLAGS) $(LDFLAGS) $(LSCRIPT)
	$(DUMP) $(EXEC)  > dump
	cp $(EXEC) fsbl.elf
	rm -rf $(OBJS)

%.o:%.c
	$(CC) $(CC_FLAGS) $(CFLAGS) $(ECFLAGS) -c $< -o $@ $(INCLUDEPATH)

%.o:%.S
	$(AS) $(CC_FLAGS) $(CFLAGS) $(ECFLAGS) -c $< -o $@ $(INCLUDEPATH)

%.o:%.s
	$(AS) $(CC_FLAGS) $(CFLAGS) $(ECFLAGS) -c $< -o $@ $(INCLUDEPATH)

clean:
	rm -rf $(OBJS) *.elf dump
