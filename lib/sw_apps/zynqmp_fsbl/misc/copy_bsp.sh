#!/bin/bash

# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT

# this script will copy the required bsp directories
# this will take board and compiler as arguments
#

BOARD=$1
PROC=$2
A53_STATE=$3
CROSS_COMP=$4
#processor dir name
if [ $PROC == "a53" ]; then
	PROC_DIRNAME="cpu_cortexa53"
elif [ $PROC == "r5" ]; then
	PROC_DIRNAME="cpu_cortexr5"
fi

# present working dir
WORKING_DIR=../misc

#bsp dir where files will be copied
if [ $PROC == "a53" ]; then
	BSP_DIR=$WORKING_DIR/zynqmp_fsbl_bsp/psu_cortexa53_0
elif [ $PROC == "r5" ]; then
	BSP_DIR=$WORKING_DIR/zynqmp_fsbl_bsp/psu_cortexr5_0
fi

BOARD_DIR=$WORKING_DIR/"$BOARD"

# Embedded Sw dir relative path from FSBL src
EMBEDDED_SW_DIR=$WORKING_DIR/../../../..

# selection of drivers is based on the board selected
DRIVERS_LIST="$BOARD_DIR/drivers.txt"

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
mkdir -p $BSP_DIR/libsrc/xilffs
cp -r $SERVICES_DIR/xilffs/src $BSP_DIR/libsrc/xilffs/
cp -r $SERVICES_DIR/xilffs/src/include/* $BSP_DIR/include/

mkdir -p $BSP_DIR/libsrc/xilsecure/src/
cp -r $SERVICES_DIR/xilsecure/src/Makefile $BSP_DIR/libsrc/xilsecure/src/
cp -r $SERVICES_DIR/xilsecure/src/common/all/* $BSP_DIR/libsrc/xilsecure/src/
cp -r $SERVICES_DIR/xilsecure/src/zynqmp/* $BSP_DIR/libsrc/xilsecure/src/
cp -r $SERVICES_DIR/xilsecure/src/common/all/*.h $BSP_DIR/include/
cp -r $SERVICES_DIR/xilsecure/src/zynqmp/*.h $BSP_DIR/include/

cp -r $SERVICES_DIR/xilpm/ $BSP_DIR/libsrc/
cp -r $SERVICES_DIR/xilpm/src/zynqmp/client/common/* $BSP_DIR/libsrc/xilpm/src/
cp -r $SERVICES_DIR/xilpm/src/zynqmp/client/common/*.h $BSP_DIR/include/
if [ -e $BOARD_DIR/pm_cfg_obj.c ]; then
	cp $BOARD_DIR/pm_cfg_obj.c $BSP_DIR/libsrc/xilpm/src/
else
	cp $WORKING_DIR/pm_cfg_obj.c  $BSP_DIR/libsrc/xilpm/src/
fi

rm -rf $BSP_DIR/libsrc/xilpm/src/zynqmp/
rm -rf $BSP_DIR/libsrc/xilpm/src/versal/

# copy bsp standalone code
cp -f $STANDALONE_DIR/arm/common/*.h $BSP_DIR/libsrc/standalone/src/
cp -f $STANDALONE_DIR/arm/common/*.c $BSP_DIR/libsrc/standalone/src/
cp -f $STANDALONE_DIR/common/*.c $BSP_DIR/libsrc/standalone/src/
cp -f $STANDALONE_DIR/common/*.h $BSP_DIR/libsrc/standalone/src/
mkdir -p $BSP_DIR/libsrc/standalone/src/includes_ps

if [ $PROC == "a53" ]; then
	if [ $A53_STATE == "64" ]; then
		cp -f $STANDALONE_DIR/arm/ARMv8/64bit/*.c $BSP_DIR/libsrc/standalone/src/
		cp -f $STANDALONE_DIR/arm/ARMv8/64bit/*.h $BSP_DIR/libsrc/standalone/src/
		if [ "$CROSS_COMP" == "armclang" ]; then
			cp -f $STANDALONE_DIR/arm/ARMv8/64bit/armclang/* $BSP_DIR/libsrc/standalone/src/
			cp -f $STANDALONE_DIR/arm/ARMv8/64bit/platform/ZynqMP/armclang/* $BSP_DIR/libsrc/standalone/src/
			cp -f $WORKING_DIR/Makefile_standalone_armclang $BSP_DIR/libsrc/standalone/src/Makefile
		else
			cp -f $STANDALONE_DIR/arm/ARMv8/64bit/gcc/* $BSP_DIR/libsrc/standalone/src/
                        cp -f $STANDALONE_DIR/arm/ARMv8/64bit/platform/ZynqMP/gcc/* $BSP_DIR/libsrc/standalone/src/
			cp -f $STANDALONE_DIR/arm/common/gcc/* $BSP_DIR/libsrc/standalone/src/
		fi
		cp $WORKING_DIR/bspconfig.h $BSP_DIR/include
		cp $WORKING_DIR/bspconfig.h $BSP_DIR/libsrc/standalone/src/
		#Copy xilpm src for apu
		cp $SERVICES_DIR/xilpm/src/zynqmp/client/apu/* $BSP_DIR/libsrc/xilpm/src/
		cp $SERVICES_DIR/xilpm/src/zynqmp/client/apu/*.h $BSP_DIR/include/

	elif [ $A53_STATE == "32" ]; then
		cp -f $STANDALONE_DIR/arm/common/gcc/* $BSP_DIR/libsrc/standalone/src/
		cp -f $STANDALONE_DIR/arm/ARMv8/32bit/*.c $BSP_DIR/libsrc/standalone/src/
		cp -f $STANDALONE_DIR/arm/ARMv8/32bit/*.h $BSP_DIR/libsrc/standalone/src/
		cp -f $STANDALONE_DIR/arm/ARMv8/32bit/gcc/* $BSP_DIR/libsrc/standalone/src/
		cp -f $STANDALONE_DIR/arm/ARMv8/32bit/platform/ZynqMP/* $BSP_DIR/libsrc/standalone/src/
		cp -f $STANDALONE_DIR/arm/ARMv8/32bit/*.h $BSP_DIR/include
		cp $WORKING_DIR/bspconfig32.h $BSP_DIR/include/bspconfig.h
		cp $WORKING_DIR/bspconfig32.h $BSP_DIR/libsrc/standalone/src/bspconfig.h
		#Copy xilpm src for apu
		cp -rf $SERVICES_DIR/xilpm/src/zynqmp/client/apu/* $BSP_DIR/libsrc/xilpm/src/
		cp -rf $SERVICES_DIR/xilpm/src/zynqmp/client/apu/*.h $BSP_DIR/include/
	fi
		cp -rf $STANDALONE_DIR/arm/ARMv8/includes_ps/platform/ZynqMP/* $BSP_DIR/libsrc/standalone/src/includes_ps/
		cp -rf $STANDALONE_DIR/arm/ARMv8/includes_ps/platform/ZynqMP/* $BSP_DIR/include/
elif [ $PROC == "r5" ]; then
	cp -f $STANDALONE_DIR/arm/common/gcc/* $BSP_DIR/libsrc/standalone/src/
	cp -f $STANDALONE_DIR/arm/cortexr5/*.c $BSP_DIR/libsrc/standalone/src/
	cp -f $STANDALONE_DIR/arm/cortexr5/*.h $BSP_DIR/libsrc/standalone/src/
	cp -f $STANDALONE_DIR/arm/cortexr5/gcc/* $BSP_DIR/libsrc/standalone/src/
	cp -f $STANDALONE_DIR/arm/cortexr5/platform/ZynqMP/xparameters_ps.h $BSP_DIR/libsrc/standalone/src/
	cp -f $STANDALONE_DIR/arm/cortexr5/platform/ZynqMP/mpu_r5.c $BSP_DIR/libsrc/standalone/src/mpu.c
	cp -f $STANDALONE_DIR/arm/cortexr5/platform/CortexR5/xil_mpu_r5.c $BSP_DIR/libsrc/standalone/src/xil_mpu.c
	cp -f $STANDALONE_DIR/arm/cortexr5/platform/CortexR5/xil_mpu_r5.h $BSP_DIR/libsrc/standalone/src/xil_mpu.h

	#include files
	cp -f $STANDALONE_DIR/arm/cortexr5/*.h $BSP_DIR/include
	cp $WORKING_DIR/bspconfig32.h $BSP_DIR/include/bspconfig.h
	cp $WORKING_DIR/bspconfig32.h $BSP_DIR/libsrc/standalone/src/bspconfig.h

	#copy xilpm src for rpu
	cp -rf $SERVICES_DIR/xilpm/src/zynqmp/client/rpu/* $BSP_DIR/libsrc/xilpm/src/
	cp -rf $SERVICES_DIR/xilpm/src/zynqmp/client/rpu/*.h $BSP_DIR/include/

	#copy includes_ps from a53 directory
	cp -rf $STANDALONE_DIR/arm/ARMv8/includes_ps/platform/ZynqMP/* $BSP_DIR/libsrc/standalone/src/includes_ps/
	cp -rf $STANDALONE_DIR/arm/ARMv8/includes_ps/platform/ZynqMP/* $BSP_DIR/include/
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
	cp -rf $DRIVERS_DIR/$line/src $BSP_DIR/libsrc/$line
	 #copy the driver include files
	cp -rf $DRIVERS_DIR/$line/src/*.h $BSP_DIR/include/

	# copy all the HSM generated driver files DRIVER_g.c
	#   cp $BOARD_DIR/x"$line"_g.c $BSP_DIR/libsrc/$line/src/

done < $DRIVERS_LIST

#copy the processor code.
if [ -d $BSP_DIR/libsrc/$PROC_DIRNAME/src ]; then
	echo "$PROC_DIRNAME directory already exists"
else
	mkdir -p $BSP_DIR/libsrc/$PROC_DIRNAME
fi
cp -r $DRIVERS_DIR/$PROC_DIRNAME/src $BSP_DIR/libsrc/$PROC_DIRNAME/src

#copy the xparameters.h
cp $BOARD_DIR/$PROC/xparameters*.h $BSP_DIR/include/

# other dependencies which are required
cp $WORKING_DIR/config.make $BSP_DIR/libsrc/standalone/src/
cp -rf $STANDALONE_DIR/arm/common/*.h $BSP_DIR/include/
cp -rf $STANDALONE_DIR/arm/common/gcc/*.h $BSP_DIR/include/
cp -rf $STANDALONE_DIR/common/*.h $BSP_DIR/include/
# no inbyte and outbyte present in standalone
cp $WORKING_DIR/inbyte.c $BSP_DIR/libsrc/standalone/src/
cp $WORKING_DIR/outbyte.c $BSP_DIR/libsrc/standalone/src/
cp $BOARD_DIR/xcsudma_g.c $BSP_DIR/libsrc/csudma/src/
cp $BOARD_DIR/xusbpsu_g.c $BSP_DIR/libsrc/usbpsu/src/

if [ $PROC == "a53" ]; then
	cp ../misc/zcu102/a53/xipipsu_g.c $BSP_DIR/libsrc/ipipsu/src/
elif [ $PROC == "r5" ]; then
	cp ../misc/zcu102/r5/xipipsu_g.c $BSP_DIR/libsrc/ipipsu/src/
fi