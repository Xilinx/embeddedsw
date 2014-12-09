#/******************************************************************************
#*
#* Copyright (C) 2014 Xilinx, Inc. All rights reserved.
#*
#* This file contains confidential and proprietary information  of Xilinx, Inc.
#* and is protected under U.S. and  international copyright and other
#* intellectual property  laws.
#*
#* DISCLAIMER
#* This disclaimer is not a license and does not grant any  rights to the
#* materials distributed herewith. Except as  otherwise provided in a valid
#* license issued to you by  Xilinx, and to the maximum extent permitted by
#* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND  WITH ALL
#* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES  AND CONDITIONS, EXPRESS,
#* IMPLIED, OR STATUTORY, INCLUDING  BUT NOT LIMITED TO WARRANTIES OF
#* MERCHANTABILITY, NON-  INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
#* and
#* (2) Xilinx shall not be liable (whether in contract or tort,  including
#* negligence, or under any other theory of liability) for any loss or damage
#* of any kind or nature  related to, arising under or in connection with these
#* materials, including for any direct, or any indirect,  special, incidental,
#* or consequential loss or damage  (including loss of data, profits,
#* goodwill, or any type of  loss or damage suffered as a result of any
#* action brought  by a third party) even if such damage or loss was
#* reasonably foreseeable or Xilinx had been advised of the  possibility
#* of the same.
#*
#* CRITICAL APPLICATIONS
#* Xilinx products are not designed or intended to be fail-  safe, or for use
#* in any application requiring fail-safe  performance, such as life-support
#* or safety devices or  systems, Class III medical devices, nuclear
#* facilities,  applications related to the deployment of airbags, or any
#* other applications that could lead to death, personal  injury, or severe
#* property or environmental damage  (individually and collectively,
#* "Critical  Applications"). Customer assumes the sole risk and  liability
#* of any use of Xilinx products in Critical  Applications, subject only to
#* applicable laws and  regulations governing limitations on product liability.
#*
#* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS  PART
#* OF THIS FILE AT ALL TIMES.
#*
#******************************************************************************/

c_SOURCES := $(wildcard *.c)
INCLUDES := $(wildcard *.h)
EXECS := xnandps8_example.elf
OBJS := $(patsubst %.c, %.o, $(c_SOURCES))

ifeq '$(PROC)' 'r5'
BSP_DIR	:= ../../../../build/target/cortex-r5
LSCRIPT_DIR := ../../../../build/scripts/bsp_utils
CC      :=       ${CROSS_COMPILE}gcc
AS      :=       ${CROSS_COMPILE}gcc
LINKER  :=       ${CROSS_COMPILE}gcc
AR	:=	 ${CROSS_COMPILE}ar
LIBPATH :=	$(BSP_DIR)/ps8_cortexr5_0/lib
DUMP    :=       ${CROSS_COMPILE}objdump -xSD
INCLUDEPATH := -I$(BSP_DIR)/ps8_cortexr5_0/include -I. -I./include/
CFLAGS =  -Wall -O0 -g3 -fmessage-length=0 \
		  -mcpu=cortex-r5 -mfloat-abi=softfp
LDFLAGS := -Wl,--start-group,-lxil,-lxilffs,-lgcc,-lc,--end-group -L$(LIBPATH) -L./
LSCRIPT := -T$(LSCRIPT_DIR)/LinkScr.ld
endif

ifeq '$(PROC)' 'a53'
BSP_DIR	:= ../../../../build/target/cortex-a53
LSCRIPT_DIR := ../../../../build/scripts/bsp_utils
CC      :=       ${CROSS_COMPILE}gcc
AS      :=       ${CROSS_COMPILE}gcc
LINKER  :=       ${CROSS_COMPILE}gcc
AR	:=	 ${CROSS_COMPILE}gcc
DUMP    :=       ${CROSS_COMPILE}objdump -xSD
INCLUDEPATH := -I$(BSP_DIR)/ps8_cortexa53_0/include -I. -I./include/
LIBPATH := $(BSP_DIR)/ps8_cortexa53_0/lib
CFLAGS =  -Wall -O0 -g3 -fmessage-length=0 \
		  -march=armv8-a
LSCRIPT := -T$(LSCRIPT_DIR)/DDRA53_LinkScr.ld
LDFLAGS := -Wl,--start-group,-lxil,-lxilffs,-lgcc,-lc,--end-group -L$(LIBPATH) -L./
endif

all: $(EXECS)

%.elf: %.o $(INCLUDES)
	$(LINKER) $< $(CC_FLAGS) $(LDFLAGS) $(LSCRIPT) -o $@
	$(DUMP) $@ > dump

%.o:%.c
	$(CC) $(CC_FLAGS) $(CFLAGS) $(ECFLAGS) -c $< -o $@ $(INCLUDEPATH)

clean:
	rm -rf $(OBJS) *.elf dump
