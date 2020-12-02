#!/bin/bash

# this script will copy the required bsp directories

# present working dir
WORKING_DIR=../misc

#bsp dir where files will be copied
BSP_DIR=$WORKING_DIR/versal_plm_bsp/psv_pmc_0

#processor dir
PROC_DIRNAME=cpu

# Embedded Sw dir relative path from versal_plm src
EMBEDDED_SW_DIR=$WORKING_DIR/../../../..

# selection of drivers is based on the board selected
DRIVERS_LIST="$WORKING_DIR/drivers.txt"

# drivers directory
DRIVERS_DIR=$EMBEDDED_SW_DIR/XilinxProcessorIPLib/drivers

# standalone dir, source of standalone files
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
set BSP_SEQUENTIAL_MAKEFILES += $BSP_DIR/libsrc/xilffs/src/Makefile

mkdir -p $BSP_DIR/libsrc/xilpdi
cp -r $SERVICES_DIR/xilpdi/src $BSP_DIR/libsrc/xilpdi/
cp -r $SERVICES_DIR/xilpdi/src/*.h $BSP_DIR/include/
set BSP_SEQUENTIAL_MAKEFILES += $BSP_DIR/libsrc/xilpdi/src/Makefile

mkdir -p $BSP_DIR/libsrc/xilplmi
cp -r $SERVICES_DIR/xilplmi/src $BSP_DIR/libsrc/xilplmi/
set BSP_SEQUENTIAL_MAKEFILES += $BSP_DIR/libsrc/xilplmi/src/Makefile

mkdir -p $BSP_DIR/libsrc/xilpuf
cp -r $SERVICES_DIR/xilpuf/src $BSP_DIR/libsrc/xilpuf/
cp -r $SERVICES_DIR/xilpuf/src/*.h $BSP_DIR/include/
set SP_SEQUENTIAL_MAKEFILES += $BSP_DIR/libsrc/xilpuf/src/Makefile

mkdir -p $BSP_DIR/libsrc/xilloader
cp -r $SERVICES_DIR/xilloader/src $BSP_DIR/libsrc/xilloader/
cp -r $SERVICES_DIR/xilloader/src/*.h $BSP_DIR/include/
set BSP_SEQUENTIAL_MAKEFILES += $BSP_DIR/libsrc/xilloader/src/Makefile

mkdir -p $BSP_DIR/libsrc/xilpm/src/
cp -r $SERVICES_DIR/xilpm/src/versal/common/* $BSP_DIR/libsrc/xilpm/src/
cp -r $SERVICES_DIR/xilpm/src/versal/server/* $BSP_DIR/libsrc/xilpm/src/
cp -r $SERVICES_DIR/xilpm/src/versal/common/*.h $BSP_DIR/include/
cp -r $SERVICES_DIR/xilpm/src/versal/server/*.h $BSP_DIR/include/
set BSP_SEQUENTIAL_MAKEFILES += $BSP_DIR/libsrc/xilpm/src/versal/common/Makefile

mkdir -p $BSP_DIR/libsrc/xilsecure/src/
cp -r $SERVICES_DIR/xilsecure/src/Makefile $BSP_DIR/libsrc/xilsecure/src/
cp -r $SERVICES_DIR/xilsecure/src/common/* $BSP_DIR/libsrc/xilsecure/src/
cp -r $SERVICES_DIR/xilsecure/src/versal/* $BSP_DIR/libsrc/xilsecure/src/
cp -r $SERVICES_DIR/xilsecure/src/common/*.h $BSP_DIR/include/
cp -r $SERVICES_DIR/xilsecure/src/versal/*.h $BSP_DIR/include/
mv $BSP_DIR/libsrc/xilsecure/src/libxilsecure_pmc.a $BSP_DIR/libsrc/xilsecure/src/libxilsecure.a
rm $BSP_DIR/libsrc/xilsecure/src/libxilsecure_*.a
set BSP_SEQUENTIAL_MAKEFILES += $BSP_DIR/libsrc/xilsecure/src/Makefile

mkdir -p $BSP_DIR/libsrc/xilsem
cp -r $SERVICES_DIR/xilsem/src $BSP_DIR/libsrc/xilsem/
cp -r $SERVICES_DIR/xilsem/src/*.h $BSP_DIR/include/

# copy bsp standalone code
cp -r $STANDALONE_DIR/common/*  $BSP_DIR/libsrc/standalone/src/
cp $STANDALONE_DIR/common/clocking/*  $BSP_DIR/libsrc/standalone/src/
cp $STANDALONE_DIR/microblaze/*  $BSP_DIR/libsrc/standalone/src/
cp -r $STANDALONE_DIR/profile  $BSP_DIR/libsrc/standalone/src/
cp $WORKING_DIR/bspconfig.h $BSP_DIR/libsrc/standalone/src/
cp $WORKING_DIR/microblaze_interrupts_g.c $BSP_DIR/libsrc/standalone/src/
cp $WORKING_DIR/bspconfig.h  $BSP_DIR/include/
cp $WORKING_DIR/Makefile $BSP_DIR/../
cp $WORKING_DIR/dep.mk $BSP_DIR/../
cp $WORKING_DIR/xsysmonpsv_supplylist.h $BSP_DIR/include/

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
	set BSP_SEQUENTIAL_MAKEFILES += $BSP_DIR/libsrc/$line/src/Makefile
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
cp $WORKING_DIR/xtmr_manager_g.c $BSP_DIR/libsrc/tmr_manager/src/
cp $WORKING_DIR/xtmr_inject_g.c $BSP_DIR/libsrc/tmr_inject/src/
cp $WORKING_DIR/xttcps_g.c $BSP_DIR/libsrc/ttcps/src/
cp $WORKING_DIR/xpmonpsv_g.c $BSP_DIR/libsrc/pmonpsv/src/
cp $WORKING_DIR/xgpiops_g.c $BSP_DIR/libsrc/gpiops/src/
cp $WORKING_DIR/xqspipsu_g.c $BSP_DIR/libsrc/qspipsu/src/
cp $WORKING_DIR/xrtcpsu_g.c $BSP_DIR/libsrc/rtcpsu/src/
cp $WORKING_DIR/xsdps_g.c $BSP_DIR/libsrc/sdps/src/
cp $WORKING_DIR/xuartlite_g.c $BSP_DIR/libsrc/uartlite/src/
cp $WORKING_DIR/xsysmonpsv_supplylist.h $BSP_DIR/libsrc/sysmonpsv/src/
cp $STANDALONE_DIR/common/*.h  $BSP_DIR/include/
cp $STANDALONE_DIR/common/clocking/*.h  $BSP_DIR/include/
cp $STANDALONE_DIR/microblaze/*.h  $BSP_DIR/include/

# no inbyte and outbyte present in standalone
cp $WORKING_DIR/inbyte.c $WORKING_DIR/outbyte.c  $BSP_DIR/libsrc/standalone/src/
export BSP_SEQUENTIAL_MAKEFILES
