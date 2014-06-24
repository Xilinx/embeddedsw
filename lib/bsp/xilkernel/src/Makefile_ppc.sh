##############################################################################
#
# (c) Copyright 2010 Xilinx, Inc. All rights reserved.
#
# This file contains confidential and proprietary information of Xilinx, Inc.
# and is protected under U.S. and international copyright and other
# intellectual property laws.
#
# DISCLAIMER
# This disclaimer is not a license and does not grant any rights to the
# materials distributed herewith. Except as otherwise provided in a valid
# license issued to you by Xilinx, and to the maximum extent permitted by
# applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
# FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
# IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
# MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
# and (2) Xilinx shall not be liable (whether in contract or tort, including
# negligence, or under any other theory of liability) for any loss or damage
# of any kind or nature related to, arising under or in connection with these
# materials, including for any direct, or any indirect, special, incidental,
# or consequential loss or damage (including loss of data, profits, goodwill,
# or any type of loss or damage suffered as a result of any action brought by
# a third party) even if such damage or loss was reasonably foreseeable or
# Xilinx had been advised of the possibility of the same.
#
# CRITICAL APPLICATIONS
# Xilinx products are not designed or intended to be fail-safe, or for use in
# any application requiring fail-safe performance, such as life-support or
# safety devices or systems, Class III medical devices, nuclear facilities,
# applications related to the deployment of airbags, or any other applications
# that could lead to death, personal injury, or severe property or
# environmental damage (individually and collectively, "Critical
# Applications"). Customer assumes the sole risk and liability of any use of
# Xilinx products in Critical Applications, subject only to applicable laws
# and regulations governing limitations on product liability.
#
# THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
# AT ALL TIMES.
#
# Top level Makefile
#
# $Id: Makefile_mb.sh,v 1.1.2.1 2011/08/25 12:12:50 anirudh Exp $
#
##############################################################################

#
# Processor architecture
# microblaze
#
ARCH = microblaze

SYSTEMDIR = ../../..

TOPDIR = .

ARCH_PREFIX = mb

#
# gnu tools for Makefile
#
CC = $(ARCH_PREFIX)-gcc
AR = $(ARCH_PREFIX)-ar
CP = cp

# export ARCH ARCH_PREFIX ARCH_DEFINE CC AR CP

#
# Compiler, linker and other options.
#
CFLAGS = ${COMPILER_FLAGS} -D__XMK__ ${EXTRA_COMPILER_FLAGS}

#
# System project directories.
#

LIBDIR = $(SYSTEMDIR)/lib
INCLUDEDIR = $(SYSTEMDIR)/include

# ELF process's system call wrapper library
LIBSYSCALL = ${TOPDIR}/libsyscall.a

# Kernel library. Contains system call wrappers and the entire kernel
LIBXILKERNEL = ${LIBDIR}/libxil.a
LIBXILKERNEL_OLD = ${TOPDIR}/libxilkernel.a

INCLUDES = -I${TOPDIR}/include -I$(INCLUDEDIR)
LIBS = -L${TOPDIR} -L$(LIBDIR)

# These objects go into libxil.a for linking with kernel bundled threads
KERNEL_AR_OBJS = ${TOPDIR}/src/sys/*.o ${TOPDIR}/src/ipc/*.o ${TOPDIR}/syscall/arch/${ARCH}/*.o ${TOPDIR}/syscall/*.o ${TOPDIR}/syscall/arch/${ARCH}/*.o ${TOPDIR}/syscall/*.o ${TOPDIR}/src/arch/${ARCH}/debugmon.o
KERNEL_AR_OBJS_2 = ${TOPDIR}/src/arch/${ARCH}/*.o 

# These objects go into the ELF process system call wrapper library - libsyscall.a
LIBSYSCALL_OBJS = ${TOPDIR}/syscall/arch/${ARCH}/*.o ${TOPDIR}/syscall/*.o

INCLUDEFILES = ${TOPDIR}/include/* 

STANDALONE_STDIN_SRC = inbyte.c 
STANDALONE_STDOUT_SRC = outbyte.c
STANDALONE_STDIN_OBJ = inbyte.o
STANDALONE_STDOUT_OBJ = outbyte.o 

LIBXIL = libxil.a

#SUBDIRS = dir_src 
SUBDIRS = 

libs: real_libs
	echo "Compiling xilkernel"

real_libs: dir_syscall dir_src standalone rellibs

dir_src:
	$(MAKE) -C src CFLAGS="$(CFLAGS)" ARCH="$(ARCH)" CC="$(CC)" AR="$(AR)" all

dir_syscall: 
	$(MAKE) -C syscall CFLAGS="$(CFLAGS)" ARCH="$(ARCH)" CC="$(CC)" AR="$(AR)" libs


#
# This target is required because of the way inbyte.c and outbyte.c get
# generated under xilkernel instead of standalone. This is due to the behavior
# of standard tcl library routines xhandle_stdin and xhandle_stdout
#

standalone:

# Does not work on SOL !!
#.PHONY: include

include:  dummy
	$(CP) -rf $(INCLUDEFILES) $(INCLUDEDIR)

dummy:

rellibs: standalone
	$(AR) -r ${LIBSYSCALL} $(LIBSYSCALL_OBJS)
	$(CP)  $(LIBSYSCALL) $(LIBDIR)
	$(AR) -r ${LIBXILKERNEL} $(KERNEL_AR_OBJS)
	$(AR) -r ${LIBXILKERNEL_OLD} ${KERNEL_AR_OBJS_2}
	$(CP)  $(LIBXILKERNEL_OLD) $(LIBDIR)

clean:
	$(MAKE) -C src ARCH="$(ARCH)" clean
	$(MAKE) -C syscall ARCH="$(ARCH)" clean
	rm -f ${LIBSYSCALL}
