######################################################################
# Copyright (c) 2011 Xilinx, Inc.  All rights reserved. 
# 
# Xilinx, Inc. 
# XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A 
# COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS 
# ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR 
# STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION 
# IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE 
# FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION. 
# XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO 
# THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO 
# ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE 
# FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY 
# AND FITNESS FOR A PARTICULAR PURPOSE. 
######################################################################


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
	cp $(BSP_DIR)/$(BOARD)/ps7_init* .
	$(LINKER) -o $@ $(OBJS) $(CC_FLAGS) $(LDFLAGS) $(LSCRIPT)
	rm -rf $(OBJS) 

$(DEPS):
ifeq "$(CC)" "arm-xilinx-eabi-gcc"
AS=arm-xilinx-eabi-gcc
LINKER=arm-xilinx-eabi-gcc
LDFLAGS = -lrsa -Wl,--start-group,-lxil,-lgcc,-lc,--end-group -Wl,--start-group,-lxilffs,-lxil,-lgcc,-lc,--end-group -L$(LIBPATH) -L./
endif 
ifeq "$(CC)" "armcc"
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

