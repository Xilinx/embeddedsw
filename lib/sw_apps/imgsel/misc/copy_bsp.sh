#******************************************************************************
#* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
#* SPDX-License-Identifier: MIT
#******************************************************************************

#!/bin/bash

# this script will copy the required bsp directories
# this will take board and compiler as arguments
#

BOARD=$1
#processor dir name
PROC_DIRNAME="cpu_cortexa53"

# present working dir
WORKING_DIR=../../misc

#bsp dir where files will be copied
BSP_DIR=$WORKING_DIR/imgsel_bsp/psu_cortexa53_0

# Embedded Sw dir relative path from FSBL src
EMBEDDED_SW_DIR=$WORKING_DIR/../../../..

# selection of drivers is based on the board selected
DRIVERS_LIST="../../misc/$BOARD/drivers.txt"

# drivers directory
DRIVERS_DIR=$EMBEDDED_SW_DIR/XilinxProcessorIPLib/drivers

# standalone dir, source of standalone files
STANDALONE_DIR=$EMBEDDED_SW_DIR/lib/bsp/standalone/src

# libraries dir
SERVICES_DIR=$EMBEDDED_SW_DIR/lib/sw_services

BSP_SEQUENTIAL_MAKEFILES=

# Copy psu_init.c file from SOM folder
if [ $BOARD = "som" ]; then
	cp $WORKING_DIR/$BOARD/psu_init.c .
fi
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

# copy bsp standalone code
cp -f $STANDALONE_DIR/arm/common/*.h $BSP_DIR/libsrc/standalone/src/
cp -f $STANDALONE_DIR/arm/common/*.c $BSP_DIR/libsrc/standalone/src/
cp -f $STANDALONE_DIR/common/*.c $BSP_DIR/libsrc/standalone/src/
cp -f $STANDALONE_DIR/common/*.h $BSP_DIR/libsrc/standalone/src/
cp ../../misc/inbyte.c $BSP_DIR/libsrc/standalone/src/
cp ../../misc/outbyte.c $BSP_DIR/libsrc/standalone/src/
if [ $BOARD = "som" ]; then
	cp -f $STANDALONE_DIR/common/clocking/*.c $BSP_DIR/libsrc/standalone/src/
	cp -f $STANDALONE_DIR/common/clocking/*.h $BSP_DIR/libsrc/standalone/src/
fi
mkdir -p $BSP_DIR/libsrc/standalone/src/includes_ps

cp -f $STANDALONE_DIR/arm/ARMv8/64bit/*.c $BSP_DIR/libsrc/standalone/src/
cp -f $STANDALONE_DIR/arm/ARMv8/64bit/*.h $BSP_DIR/libsrc/standalone/src/
cp -f $STANDALONE_DIR/arm/ARMv8/64bit/gcc/* $BSP_DIR/libsrc/standalone/src/
cp -f $STANDALONE_DIR/arm/ARMv8/64bit/platform/ZynqMP/gcc/* $BSP_DIR/libsrc/standalone/src/
cp -f $STANDALONE_DIR/arm/common/gcc/* $BSP_DIR/libsrc/standalone/src/
cp ../../misc/bspconfig.h $BSP_DIR/include
cp ../../misc/bspconfig.h $BSP_DIR/libsrc/standalone/src/
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
	 #copy the driver include files
	cp -rf $DRIVERS_DIR/$line/src/*.h $BSP_DIR/include/

	# copy all the HSM generated driver files DRIVER_g.c
	if [ -f ../../misc/x"$line"_g.c ]; then
		cp ../../misc/x"$line"_g.c $BSP_DIR/libsrc/$line/src/
	fi
	BSP_SEQUENTIAL_MAKEFILES="$BSP_SEQUENTIAL_MAKEFILES $BSP_DIR/libsrc/$line/src/Makefile"

done < $DRIVERS_LIST

#copy the processor code.
if [ -d $BSP_DIR/libsrc/$PROC_DIRNAME/src ]; then
	echo "$PROC_DIRNAME directory already exists"
else
	mkdir -p $BSP_DIR/libsrc/$PROC_DIRNAME
fi
cp -r $DRIVERS_DIR/$PROC_DIRNAME/src $BSP_DIR/libsrc/$PROC_DIRNAME/src

#copy the xparameters.h
cp ../../misc/$BOARD/xparameters*.h $BSP_DIR/include/

# other dependencies which are required
cp $WORKING_DIR/config.make $BSP_DIR/libsrc/standalone/src/
cp -rf $STANDALONE_DIR/arm/common/*.h $BSP_DIR/include/
cp -rf $STANDALONE_DIR/arm/common/gcc/*.h $BSP_DIR/include/
cp -rf $STANDALONE_DIR/common/*.h $BSP_DIR/include/
export BSP_SEQUENTIAL_MAKEFILES
