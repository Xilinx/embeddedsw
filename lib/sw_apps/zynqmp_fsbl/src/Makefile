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
BSP_DIR	:= ../misc/zynqmp_fsbl_bsp
A53_STATE := 64
ECFLAGS :=
LSCRIPT := -Tlscript_a53.ld
BOARD	:= zcu102
EXEC := ron_a53_fsbl.elf
LIBS = libxil.a
c_SOURCES := $(wildcard *.c)
c_SOURCES += ../misc/$(BOARD)/psu_init.c
s_SOURCES := $(wildcard *.s)
INCLUDES := $(wildcard *.h)
OBJS := $(patsubst %.c, %.o, $(c_SOURCES))
OBJS += $(patsubst %.s, %.o, $(s_SOURCES))

ifeq '$(PROC)' 'r5'
S_SOURCES := 	xfsbl_exit.S
OBJS +=		xfsbl_exit.o
CROSS   :=
CC      :=      $(CROSS)armr5-none-eabi-gcc
AS      :=      $(CROSS)armr5-none-eabi-gcc
LINKER  :=      $(CROSS)armr5-none-eabi-gcc
DUMP    :=      $(CROSS)armr5-none-eabi-objdump -xSD
CFLAGS :=  -Wall -O0 -g3 -fmessage-length=0
ECFLAGS := -g -DARMR5 -Wall -Wextra -mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16 -Os -flto -ffat-lto-objects
LSCRIPT := -Tlscript.ld
EXEC := ron_r5_fsbl.elf
INCLUDEPATH := -I$(BSP_DIR)/psu_cortexr5_0/include -I. -I../misc/$(BOARD)/$(PROC) -I../misc/$(BOARD)
LIBPATH := $(BSP_DIR)/psu_cortexr5_0/lib
LDFLAGS :=  -Wl,--start-group,-lxil,-lxilffs,-lxilsecure,-lxilpm,-lgcc,-lc,--end-group -L$(LIBPATH) -L./ -Wl,--build-id=none -mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16
endif

ifeq '$(PROC)' 'a53'
S_SOURCES := xfsbl_exit.S
S_SOURCES += xfsbl_translation_table_a53_$(A53_STATE).S
OBJS += $(patsubst %.S, %.o, $(S_SOURCES))
CROSS   :=
CFLAGS =  -Wall -O0 -g3 -fmessage-length=0 
EXEC := ron_a53_fsbl.elf
INCLUDEPATH := -I$(BSP_DIR)/psu_cortexa53_0/include -I. -I../misc/$(BOARD)/$(PROC) -I../misc/$(BOARD)
LIBPATH := $(BSP_DIR)/psu_cortexa53_0/lib
ifeq '$(A53_STATE)' '64'
CC      :=      $(CROSS)aarch64-none-elf-gcc
AS      :=      $(CROSS)aarch64-none-elf-gcc
LINKER  :=      $(CROSS)aarch64-none-elf-gcc
DUMP    :=      $(CROSS)aarch64-none-elf-objdump -xSD
ECFLAGS = -march=armv8-a -DARMA53_$(A53_STATE) -Os -flto -ffat-lto-objects
LSCRIPT := 	-Tlscript_a53.ld
LDFLAGS := -Wl,--start-group,-lxil,-lxilffs,-lxilsecure,-lxilpm,-lgcc,-lc,--end-group -L$(LIBPATH) -L./ -Wl,--build-id=none
else
CC      :=      $(CROSS)arm-none-eabi-gcc
AS      :=      $(CROSS)arm-none-eabi-gcc
LINKER  :=      $(CROSS)arm-none-eabi-gcc
DUMP    :=      $(CROSS)arm-none-eabi-objdump -xSD
ECFLAGS :=  -Wextra -march=armv7-a -mfloat-abi=hard -mfpu=vfpv3-d16 -DARMA53_$(A53_STATE) -Os -flto -ffat-lto-objects
LSCRIPT :=	-Tlscript.ld
LDFLAGS :=  -Wl,--start-group,-lxil,-lxilffs,-lxilsecure,-lxilpm,-lgcc,-lc,--end-group -L$(LIBPATH) -L./ -Wl,--build-id=none -march=armv7-a -mfloat-abi=hard -mfpu=vfpv3

endif
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

$(EXEC): $(LIBS) $(OBJS) $(INCLUDES)
	$(LINKER) -o $@ $(OBJS) $(CC_FLAGS) $(LDFLAGS) $(LSCRIPT)
	$(DUMP) $(EXEC)  > dump
	cp $(EXEC) fsbl.elf
	rm -rf $(OBJS)

$(LIBS):
	echo "Copying BSP files"
	../misc/copy_bsp.sh $(BOARD) $(PROC) $(A53_STATE)
	echo "Compiling bsp"
	make -C ../misc BOARD=$(BOARD) PROC=$(PROC) A53_STATE=$(A53_STATE)

%.o:%.c
	$(CC) $(CC_FLAGS) $(CFLAGS) $(ECFLAGS) -c $< -o $@ $(INCLUDEPATH)

%.o:%.S
	$(AS) $(CC_FLAGS) $(CFLAGS) $(ECFLAGS) -c $< -o $@ $(INCLUDEPATH)

%.o:%.s
	$(AS) $(CC_FLAGS) $(CFLAGS) $(ECFLAGS) -c $< -o $@ $(INCLUDEPATH)

clean:
	rm -rf $(OBJS) *.elf dump
