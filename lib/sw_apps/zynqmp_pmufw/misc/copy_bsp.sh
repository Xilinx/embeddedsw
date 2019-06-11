#!/bin/bash

# this script will copy the required bsp directories

# present working dir
WORKING_DIR=../misc

#bsp dir where files will be copied
BSP_DIR=$WORKING_DIR/zynqmp_pmufw_bsp/psu_pmu_0

#processor dir
PROC_DIRNAME=cpu

# Embedded Sw dir relaive path from pmufw src
EMBEDDED_SW_DIR=$WORKING_DIR/../../../..

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
mkdir -p $BSP_DIR/libsrc/xilfpga/src/
cp -r $SERVICES_DIR/xilfpga/src/* $BSP_DIR/libsrc/xilfpga/src
cp -r $SERVICES_DIR/xilfpga/src/interface/zynqmp/* $BSP_DIR/libsrc/xilfpga/src
cp -r $SERVICES_DIR/xilfpga/src/*.h $BSP_DIR/include/
cp -r $SERVICES_DIR/xilfpga/src/interface/zynqmp/*.h $BSP_DIR/include/
rm -r $BSP_DIR/libsrc/xilfpga/src/interface/
mkdir -p $BSP_DIR/libsrc/xilsecure/src/
cp -r $SERVICES_DIR/xilsecure/src/Makefile $BSP_DIR/libsrc/xilsecure/src/
cp -r $SERVICES_DIR/xilsecure/src/common/* $BSP_DIR/libsrc/xilsecure/src/
cp -r $SERVICES_DIR/xilsecure/src/zynqmp/* $BSP_DIR/libsrc/xilsecure/src/
cp -r $SERVICES_DIR/xilsecure/src/common/*.h $BSP_DIR/include/
cp -r $SERVICES_DIR/xilsecure/src/zynqmp/*.h $BSP_DIR/include/
cp -r $SERVICES_DIR/xilskey/ $BSP_DIR/libsrc/
cp -r $SERVICES_DIR/xilskey/src/*.h $BSP_DIR/include/
cp -r $SERVICES_DIR/xilskey/src/include/*.h $BSP_DIR/include/

# copy bsp standalone code
cp  $STANDALONE_DIR/common/*  $BSP_DIR/libsrc/standalone/src/
cp  $STANDALONE_DIR/microblaze/*  $BSP_DIR/libsrc/standalone/src/
cp -r $STANDALONE_DIR/profile  $BSP_DIR/libsrc/standalone/src/
cp  $WORKING_DIR/bspconfig.h  $BSP_DIR/include
cp  $WORKING_DIR/Makefile $BSP_DIR/../
cp  $WORKING_DIR/xfpga_config.h $BSP_DIR/include

#remove _g.c files
#rm $BSP_DIR/libsrc/standalone/src/microblaze_interrupts_g.c

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
mv $BSP_DIR/libsrc/xilsecure/src/xsecure_sha2_pmu.a $BSP_DIR/libsrc/xilsecure/src/libxilsecure.a

# other dependencies which are required
cp $WORKING_DIR/config.make $BSP_DIR/libsrc/standalone/src/
cp $STANDALONE_DIR/common/*.h  $BSP_DIR/include/
cp $STANDALONE_DIR/microblaze/*.h  $BSP_DIR/include/
cp $STANDALONE_DIR/profile/*.h  $BSP_DIR/include/

# no inbyte and outbyte present in standalone
cp $WORKING_DIR/inbyte.c $WORKING_DIR/outbyte.c  $BSP_DIR/libsrc/standalone/src/


