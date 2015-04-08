###############################################################################
#
# Copyright (C) 2014 Xilinx, Inc. All rights reserved.
#
# This file contains confidential and proprietary information  of Xilinx, Inc.
# and is protected under U.S. and  international copyright and other
# intellectual property  laws.
#
# DISCLAIMER
# This disclaimer is not a license and does not grant any  rights to the
# materials distributed herewith. Except as  otherwise provided in a valid
# license issued to you by  Xilinx, and to the maximum extent permitted by
# applicable law:
# (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND  WITH ALL FAULTS, AND
# XILINX HEREBY DISCLAIMS ALL WARRANTIES  AND CONDITIONS, EXPRESS, IMPLIED,
# OR STATUTORY, INCLUDING  BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
# NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE
# and
# (2) Xilinx shall not be liable (whether in contract or tort,  including
# negligence, or under any other theory of liability) for any loss or damage of
# any kind or nature  related to, arising under or in connection with these
# materials, including for any direct, or any indirect,  special, incidental,
# or consequential loss or damage  (including loss of data, profits, goodwill,
# or any type of  loss or damage suffered as a result of any action brought
# by a third party) even if such damage or loss was  reasonably foreseeable
# or Xilinx had been advised of the  possibility of the same.
#
# CRITICAL APPLICATIONS
# Xilinx products are not designed or intended to be fail-safe, or for use in
# any application requiring fail-safe  performance, such as life-support or
# safety devices or  systems, Class III medical devices, nuclear facilities,
# applications related to the deployment of airbags, or any  other applications
# that could lead to death, personal  injury, or severe property or environmental
# damage  (individually and collectively, "Critical  Applications").
# Customer assumes the sole risk and liability of any use of Xilinx products in
# Critical  Applications, subject only to applicable laws and  regulations
# governing limitations on product liability.
#
# THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
# AT ALL TIMES.
#
###############################################################################
include config.make

AS=aarch64-none-elf-as
CC=aarch64-none-elf-gcc
AR=aarch64-none-elf-ar
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=
LIB=libxil.a

LIB=libxil.a

CC_FLAGS = $(subst -pg, -DPROFILING, $(COMPILER_FLAGS))
ECC_FLAGS = $(subst -pg, -DPROFILING, $(EXTRA_COMPILER_FLAGS))


#The following flags are required for PEEP. We can remove them later
ECC_FLAGS	+= -march=armv8-a


RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

OUTS = *.o

INCLUDEFILES=*.h

libs: $(LIBS)

standalone_libs: $(LIBSOURCES)
	echo "Compiling standalone A53"
	$(CC) $(CC_FLAGS) $(ECC_FLAGS) $(INCLUDES) $^
	$(ARCHIVER) -r ${RELEASEDIR}/${LIB} ${OUTS}

profile_libs:
	$(MAKE) -C profile COMPILER_FLAGS="$(COMPILER_FLAGS)" EXTRA_COMPILER_FLAGS="$(EXTRA_COMPILER_FLAGS)" COMPILER="$(CC)" ARCHIVER="$(ARCHIVER)" AS="$(AS)" libs

.PHONY: include
include: standalone_includes profile_includes

standalone_includes:
	${CP} ${INCLUDEFILES} ${INCLUDEDIR}

profile_includes:
	$(MAKE) -C profile COMPILER_FLAGS="$(COMPILER_FLAGS)" EXTRA_COMPILER_FLAGS="$(EXTRA_COMPILER_FLAGS)" COMPILER="$(CC)" ARCHIVER="$(ARCHIVER)" AS="$(AS)" include

clean:
	rm -rf ${OUTS}
	$(MAKE) -C profile COMPILER_FLAGS="$(COMPILER_FLAGS)" EXTRA_COMPILER_FLAGS="$(EXTRA_COMPILER_FLAGS)" COMPILER="$(CC)" ARCHIVER="$(ARCHIVER)" AS="$(AS)" clean
