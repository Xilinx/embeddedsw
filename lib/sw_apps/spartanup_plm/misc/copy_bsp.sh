#!/bin/bash

# Copyright (c) 2024, Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

# this script will copy the required bsp directories

# present working dir
WORKING_DIR=../misc

#bsp dir where files will be copied
BSP_DIR=$WORKING_DIR/spartanup_plm_bsp/psxl_pmcl_0

#processor dir
PROC_DIRNAME=cpu

# Embedded Sw dir relative path from spartanup_plm src
EMBEDDED_SW_DIR=$WORKING_DIR/../../../../

# selection of drivers is based on the board selected
DRIVERS_LIST="$WORKING_DIR/drivers.txt"

# drivers directory
DRIVERS_DIR=$EMBEDDED_SW_DIR/XilinxProcessorIPLib/drivers

# standalone dir, source of standalone files
STANDALONE_DIR=$EMBEDDED_SW_DIR/lib/bsp/standalone/src

# libraries dir
SERVICES_DIR=$EMBEDDED_SW_DIR/lib/sw_services

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
cp -r $STANDALONE_DIR/common/*  $BSP_DIR/libsrc/standalone/src/
cp $STANDALONE_DIR/common/clocking/*  $BSP_DIR/libsrc/standalone/src/
cp $STANDALONE_DIR/riscv/*  $BSP_DIR/libsrc/standalone/src/
cp -r $STANDALONE_DIR/profile  $BSP_DIR/libsrc/standalone/src/
cp $WORKING_DIR/bspconfig.h $BSP_DIR/libsrc/standalone/src/
cp $WORKING_DIR/bspconfig.h  $BSP_DIR/include/
cp $WORKING_DIR/Makefile $BSP_DIR/../
cp $WORKING_DIR/dep.mk $BSP_DIR/../

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
    #copy the driver include files
    cp -r $DRIVERS_DIR/$line/src/*.h $BSP_DIR/include/
# copy all the HSM generated driver files DRIVER_g.c
	cp $WORKING_DIR/x"$line"_g.c $BSP_DIR/libsrc/$line/src/
done < $DRIVERS_LIST

#copy the processor code.
if [ -d $BSP_DIR/libsrc/$PROC_DIRNAME/src ]; then
	echo "$PROC_DIRNAME directory already exists"
else
	mkdir -p $BSP_DIR/libsrc/$PROC_DIRNAME
fi
cp -r $DRIVERS_DIR/$PROC_DIRNAME/src $BSP_DIR/libsrc/$PROC_DIRNAME/src

#copy the xparameters.h
cp $WORKING_DIR/xparameters*.h $BSP_DIR/include/

# other dependencies which are required
cp $WORKING_DIR/config.make $BSP_DIR/libsrc/standalone/src/
cp $STANDALONE_DIR/common/*.h  $BSP_DIR/include/
cp $STANDALONE_DIR/common/clocking/*.h  $BSP_DIR/include/
cp $STANDALONE_DIR/riscv/*.h  $BSP_DIR/include/

# no outbyte present in standalone
cp $WORKING_DIR/outbyte.c  $BSP_DIR/libsrc/standalone/src/
