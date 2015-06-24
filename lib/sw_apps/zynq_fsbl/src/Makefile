###############################################################################
#
# Copyright (C) 2012 - 2014 Xilinx, Inc.  All rights reserved.
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal 
# in the Software without restriction, including without limitation the rights 
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell  
# copies of the Software, and to permit persons to whom the Software is 
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in 
# all copies or substantial portions of the Software.
#
# Use of the Software is limited solely to applications: 
# (a) running on a Xilinx device, or 
# (b) that interact with a Xilinx device through a bus or interconnect.  
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF 
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
# SOFTWARE.
#
# Except as contained in this notice, the name of the Xilinx shall not be used
# in advertising or otherwise to promote the sale, use or other dealings in 
# this Software without prior written authorization from Xilinx.
#
###############################################################################


CC := arm-xilinx-eabi-gcc
CC_FLAGS :=  
CFLAGS := 
ECFLAGS := 
LDFLAGS := 

c_SOURCES := $(wildcard *.c)
S_SOURCES := $(wildcard *.S)
s_SOURCES := $(wildcard *.s)
INCLUDES := $(wildcard *.h)
OBJS := $(patsubst %.c, %.o, $(c_SOURCES))
OBJS += $(patsubst %.S, %.o, $(S_SOURCES))
OBJS += $(patsubst %.s, %.o, $(s_SOURCES))
LSCRIPT := -Tlscript.ld

BSP_DIR	:= ../misc/

LIBS := libxil.a 
EXEC := fsbl.elf

INCLUDEPATH := -I$(BSP_DIR)/ps7_cortexa9_0/include -I.
LIBPATH := $(BSP_DIR)/ps7_cortexa9_0/lib

BOARD	:= zc702
DEPS	:= depends

all: $(EXEC)

$(EXEC): $(DEPS) $(LIBS) $(OBJS) $(INCLUDES)
	$(LINKER) -o $@ $(OBJS) $(CC_FLAGS) $(LDFLAGS) $(LSCRIPT)
	rm -rf $(OBJS) 

$(DEPS):
ifeq "$(CC)" "arm-xilinx-eabi-gcc"
	cp $(BSP_DIR)/$(BOARD)/ps7_init.* .
AS=arm-xilinx-eabi-gcc
LINKER=arm-xilinx-eabi-gcc
LDFLAGS = -lrsa -Wl,--start-group,-lxil,-lgcc,-lc,--end-group -Wl,--start-group,-lxilffs,-lxil,-lgcc,-lc,--end-group -L$(LIBPATH) -L./
endif 
ifeq "$(CC)" "armcc"
	cp $(BSP_DIR)/$(BOARD)/ps7_init.* .
AS=armasm
LINKER=armlink 
CFLAGS += -c --c99 --wchar32
CC_FLAGS	+= --cpu=Cortex-A9 --fpu=VFPv3_FP16
LDFLAGS = --entry=_vector_table "$(LIBPATH)/libxil.a(*.o)" --no_search_dynamic_libraries --userlibpath=$(LIBPATH),. --library=xil,xilffs,rsa
LSCRIPT = --scatter="scatter.scat"
endif
	
	
$(LIBS): 
	echo "Copying BSP files"
	$(BSP_DIR)/copy_bsp.sh "$(BOARD)" "$(CC)"
	echo "Compiling bsp"
	if [ $(CC) == "arm-xilinx-eabi-gcc" ]; then \
		make -C $(BSP_DIR) -k all "CC=arm-xilinx-eabi-gcc" "AR=arm-xilinx-eabi-ar" "C_FLAGS=-O2 -c" "EC_FLAGS=-g"; \
	elif [ $(CC) == "armcc" ]; then \
        make -C $(BSP_DIR) -k all "CC=armcc" "AR=armar" "C_FLAGS= -O2 -c" "EC_FLAGS=--debug --wchar32"; \
	fi;

%.o:%.c
	$(CC) $(CC_FLAGS) $(CFLAGS) $(ECFLAGS) -c $< -o $@ $(INCLUDEPATH)

%.o:%.S
	if [ $(CC) == "arm-xilinx-eabi-gcc" ]; then \
		$(AS) $(CC_FLAGS) -c $< -o $@ $(INCLUDEPATH); \
	elif [ $(CC) == "armcc" ]; then \
        $(CC) $(INCLUDEPATH) -E -o fsbl_handoff.s fsbl_handoff.S; \
		$(AS) $(CC_FLAGS) -c fsbl_handoff.s -o fsbl_handoff.o $(INCLUDEPATH); \
		rm fsbl_handoff.s; \
	fi;
	
	
%.o:%.s
	$(AS) $(CC_FLAGS) $(CFLAGS) $(ECFLAGS) -c $< -o $@ $(INCLUDEPATH)

clean:
	rm -rf $(OBJS) $(BSP_DIR)/ps7_cortexa9_0 $(EXEC)

