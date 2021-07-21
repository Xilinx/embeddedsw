#!/bin/bash

# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

# this script will copy the required bsp directories

# this will take board and compiler as arguments
#
BOARD=$1
COMPILER=$2

# present working dir
WORKING_DIR=../misc

BSP_DIR=$WORKING_DIR/ps7_cortexa9_0
BOARD_DIR=$WORKING_DIR/"$BOARD"

# Embedded Sw dir relative path from FSBL src
EMBEDDED_SW_DIR=$WORKING_DIR/../../../..

# selection of drivers is based on the board selected
DRIVERS_LIST="$BOARD_DIR/drivers.txt"

# drivers directory 
DRIVERS_DIR=$EMBEDDED_SW_DIR/XilinxProcessorIPLib/drivers

# standalone dir 
STANDALONE_DIR=$EMBEDDED_SW_DIR/lib/bsp/standalone/src

# libraries dir 
SERVICES_DIR=$EMBEDDED_SW_DIR/lib/sw_services

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

# copy bsp standalone code
if [ -d $BSP_DIR/libsrc/standalone/src ]; then 
	echo "Standalone directory already exists"
else
	mkdir -p $BSP_DIR/libsrc/standalone/src
fi

cp -rf $STANDALONE_DIR/profile  $BSP_DIR/libsrc/standalone/src/
cp $STANDALONE_DIR/common/*.h  $BSP_DIR/libsrc/standalone/src/
cp $STANDALONE_DIR/common/*.c  $BSP_DIR/libsrc/standalone/src/
cp $STANDALONE_DIR/arm/cortexa9/*.c  $BSP_DIR/libsrc/standalone/src/
cp $STANDALONE_DIR/arm/cortexa9/*.h  $BSP_DIR/libsrc/standalone/src/
cp $STANDALONE_DIR/arm/common/*.h   $BSP_DIR/libsrc/standalone/src/
cp $STANDALONE_DIR/arm/common/*.c   $BSP_DIR/libsrc/standalone/src/
cp $STANDALONE_DIR/arm/common/gcc/*  $BSP_DIR/libsrc/standalone/src/

if [ $COMPILER == "arm-none-eabi-gcc" ]; then
	cp $STANDALONE_DIR/arm/cortexa9/gcc/*  $BSP_DIR/libsrc/standalone/src/
elif [ $COMPILER == "armcc" ]; then 
	cp $STANDALONE_DIR/cortexa9/armcc/*  $BSP_DIR/libsrc/standalone/src/
fi
	

# copy the bsp drivers
while read line
do
    # copy driver code to bsp
    if [ -d $BSP_DIR/libsrc/$line/src ]; then
        echo "$line directory already exists"
    else
        mkdir -p $BSP_DIR/libsrc/$line
    fi
    cp -r $DRIVERS_DIR/$line/src $BSP_DIR/libsrc/$line
# copy all the HSM generated driver files DRIVER_g.c
#    cp $BOARD_DIR/x"$line"_g.c $BSP_DIR/libsrc/$line/src/
    BSP_SEQUENTIAL_MAKEFILES="$BSP_SEQUENTIAL_MAKEFILES $BSP_DIR/libsrc/$line/src/Makefile"

done < $DRIVERS_LIST

# copy the libraries required
cp -r $SERVICES_DIR/xilffs/ $BSP_DIR/libsrc/
cp -r $SERVICES_DIR/xilffs/src/include/* $BSP_DIR/include/
BSP_SEQUENTIAL_MAKEFILES="$BSP_SEQUENTIAL_MAKEFILES $BSP_DIR/libsrc/xilffs/src/Makefile"

cp -r $SERVICES_DIR/xilrsa/ $BSP_DIR/libsrc/
cp -r $SERVICES_DIR/xilrsa/src/include/* $BSP_DIR/include/
BSP_SEQUENTIAL_MAKEFILES="$BSP_SEQUENTIAL_MAKEFILES $BSP_DIR/libsrc/xilrsa/src/Makefile"

#copy the xparameters.h
cp $BOARD_DIR/xparameters.h $BSP_DIR/include/
cp $BOARD_DIR/bspconfig.h $BSP_DIR/include/

# other dependencies which are required
cp $WORKING_DIR/config.make $BSP_DIR/libsrc/standalone/src/ 
cp $STANDALONE_DIR/common/*  $BSP_DIR/include/
# no inbyte and outbyte present in standalone
cp $BOARD_DIR/inbyte.c $BOARD_DIR/outbyte.c  $BSP_DIR/libsrc/standalone/src/
export BSP_SEQUENTIAL_MAKEFILES