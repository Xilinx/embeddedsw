#!/bin/bash

# Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
# Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT

# this script will copy the required bsp directories
# this will take board and compiler as arguments
#

# processor dir name
PROC_DIRNAME="cpu_cortexa53"

# present working dir
WORKING_DIR=../misc

# bsp dir where files will be copied
BSP_DIR=$WORKING_DIR/img_rcvry_bsp/psu_cortexa53_0


# Embedded Sw dir relative path from Image recovery src
EMBEDDED_SW_DIR=$WORKING_DIR/../../../..

# selection of drivers is based on the board selected
DRIVERS_LIST=$WORKING_DIR/drivers.txt

# drivers directory
DRIVERS_DIR=$EMBEDDED_SW_DIR/XilinxProcessorIPLib/drivers

# standalone dir, source of standalone files
STANDALONE_DIR=$EMBEDDED_SW_DIR/lib/bsp/standalone/src

# libraries dir
SERVICES_DIR=$EMBEDDED_SW_DIR/lib/sw_services

THIRD_PARTY_SERVICES_DIR=$EMBEDDED_SW_DIR/ThirdParty/sw_services

BSP_SEQUENTIAL_MAKEFILES=

# creation of BSP folders required
if [ -d $BSP_DIR ]; then
	echo "BSP directory already exists"
else
	mkdir -p $BSP_DIR/code
	mkdir -p $BSP_DIR/include
	mkdir -p $BSP_DIR/lib
	mkdir -p $BSP_DIR/libsrc
fi

# create bsp standalone/src folder
if [ -d $BSP_DIR/libsrc/standalone/src ]; then
	echo "Standalone directory already exists"
else
	mkdir -p $BSP_DIR/libsrc/standalone/src
fi

# copy the libraries required
mkdir -p $BSP_DIR/libsrc/xilffs
cp -r $SERVICES_DIR/xilffs/src $BSP_DIR/libsrc/xilffs/
cp -r $SERVICES_DIR/xilffs/src/include/* $BSP_DIR/include/
BSP_SEQUENTIAL_MAKEFILES="$BSP_SEQUENTIAL_MAKEFILES $BSP_DIR/libsrc/xilffs/src/Makefile"

mkdir -p $BSP_DIR/libsrc/lwip213/src/
cp -rf $THIRD_PARTY_SERVICES_DIR/lwip213/src/* $BSP_DIR/libsrc/lwip213/src/
cp $WORKING_DIR/tools/xtopology_g.c $BSP_DIR/libsrc/lwip213/src/contrib/ports/xilinx/netif/
cp $WORKING_DIR/tools/Makefile.config $BSP_DIR/libsrc/lwip213/src/
cp $WORKING_DIR/tools/lwipopts.h $BSP_DIR/libsrc/lwip213/src/contrib/ports/xilinx/include/
cp $WORKING_DIR/tools/xlwipconfig.h $BSP_DIR/libsrc/lwip213/src/contrib/ports/xilinx/include/
BSP_SEQUENTIAL_MAKEFILES="BSP_SEQUENTIAL_MAKEFILES $BSP_DIR/libsrc/lwip213/src/Makefile"

# copy bsp standalone code
mkdir -p $BSP_DIR/libsrc/standalone/src/includes_ps
cp -rf $STANDALONE_DIR/ $BSP_DIR/libsrc/standalone/
cp -f $STANDALONE_DIR/arm/ARMv8/64bit/*.c $BSP_DIR/libsrc/standalone/src/
cp -f $STANDALONE_DIR/arm/ARMv8/64bit/*.h $BSP_DIR/libsrc/standalone/src/
cp -f $STANDALONE_DIR/arm/ARMv8/64bit/gcc/* $BSP_DIR/libsrc/standalone/src/
cp -f $STANDALONE_DIR/arm/ARMv8/64bit/platform/ZynqMP/gcc/* $BSP_DIR/libsrc/standalone/src/
cp -f $STANDALONE_DIR/arm/common/gcc/*.c $BSP_DIR/libsrc/standalone/src/
cp -f $STANDALONE_DIR/arm/common/gcc/*.h $BSP_DIR/libsrc/standalone/src/
cp -f $STANDALONE_DIR/arm/common/*.c $BSP_DIR/libsrc/standalone/src/
cp -f $STANDALONE_DIR/arm/common/*.h $BSP_DIR/libsrc/standalone/src/
cp $WORKING_DIR/tools/bspconfig.h $BSP_DIR/include
cp $WORKING_DIR/tools/bspconfig.h $BSP_DIR/libsrc/standalone/src/
cp -rf $STANDALONE_DIR/arm/ARMv8/includes_ps/platform/ZynqMP/* $BSP_DIR/libsrc/standalone/src/includes_ps/
cp -rf $STANDALONE_DIR/arm/ARMv8/includes_ps/platform/ZynqMP/* $BSP_DIR/include/

# copy the bsp drivers
while read line
do
    # copy driver code to bsp
	if [ -d $BSP_DIR/libsrc/$line/src ]; then
		echo "$line directory already exists"
	else
		 mkdir -p $BSP_DIR/libsrc/$line
	fi
	cp -rf $DRIVERS_DIR/$line/src $BSP_DIR/libsrc/$line
	 # copy the driver include files
	cp -rf $DRIVERS_DIR/$line/src/*.h $BSP_DIR/include/

	# copy all the HSM generated driver files DRIVER_g.c
	if [[ -f $WORKING_DIR/tools/$1/x"$line"_g.c ]]; then
		cp $WORKING_DIR/tools/$1/x"$line"_g.c $BSP_DIR/libsrc/$line/src/
	elif [[ -f $WORKING_DIR/tools/x"$line"_g.c ]]; then
		cp $WORKING_DIR/tools/x"$line"_g.c $BSP_DIR/libsrc/$line/src/
	fi
	BSP_SEQUENTIAL_MAKEFILES="$BSP_SEQUENTIAL_MAKEFILES $BSP_DIR/libsrc/$line/src/Makefile"
done < $DRIVERS_LIST

# copy the processor code.
if [ -d $BSP_DIR/libsrc/$PROC_DIRNAME/src ]; then
	echo "$PROC_DIRNAME directory already exists"
else
	mkdir -p $BSP_DIR/libsrc/$PROC_DIRNAME
fi
cp -r $DRIVERS_DIR/$PROC_DIRNAME/src $BSP_DIR/libsrc/$PROC_DIRNAME/src

# copy the xparameters.h
cp $WORKING_DIR/tools/$1/xparameters*.h $BSP_DIR/include/

# other dependencies which are required
cp $WORKING_DIR/tools/config.make $BSP_DIR/libsrc/standalone/src/
cp -rf $STANDALONE_DIR/arm/common/*.h $BSP_DIR/include/
cp -rf $STANDALONE_DIR/arm/common/gcc/*.h $BSP_DIR/include/
cp -rf $STANDALONE_DIR/common/*.h $BSP_DIR/include/
cp -f $STANDALONE_DIR/common/*.c $BSP_DIR/libsrc/standalone/src/
cp -f $STANDALONE_DIR/common/*.h $BSP_DIR/libsrc/standalone/src/
cp -f $STANDALONE_DIR/common/clocking/* $BSP_DIR/libsrc/standalone/src/
# no inbyte and outbyte present in standalone
cp $WORKING_DIR/tools/inbyte.c $BSP_DIR/libsrc/standalone/src/
cp $WORKING_DIR/tools/outbyte.c $BSP_DIR/libsrc/standalone/src/
export BSP_SEQUENTIAL_MAKEFILES
