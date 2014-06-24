###############################################################################
#
# Copyright (C) 2013 - 2014 Xilinx, Inc.  All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# Use of the Software is limited solely to applications:
# (a) running on a Xilinx device, or
# (b) that interact with a Xilinx device through a bus or interconnect.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Except as contained in this notice, the name of the Xilinx shall not be used
# in advertising or otherwise to promote the sale, use or other dealings in
# this Software without prior written authorization from Xilinx.
#
###############################################################################

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=

ifeq ($(COMPILER) , arm-xilinx-eabi-gcc)
	EXTRA_ARCHIVE_FLAGS=rc
else
	EXTRA_ARCHIVE_FLAGS=--create
endif

ifeq ($(COMPILER) , mb-gcc)
	EXTRA_ARCHIVE_FLAGS=rc
endif

RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./include/ -I${INCLUDEDIR} 
FATFS_DIR = .

LIB_SRCS = $(FATFS_DIR)/ff.c \
          $(FATFS_DIR)/diskio.c

          
# create FATFS_SRCS based on configured options

FATFS_SRCS = $(LIB_SRCS)

FATFS_OBJS = $(FATFS_SRCS:%.c=%.o)
          

EXPORT_INCLUDE_FILES = $(FATFS_DIR)/include/ff.h \
			$(FATFS_DIR)/include/ffconf.h \
			$(FATFS_DIR)/include/diskio.h \
			$(FATFS_DIR)/include/integer.h

			
libs: libxilffs.a
	cp libxilffs.a $(RELEASEDIR)
	make clean

include:
	@for i in $(EXPORT_INCLUDE_FILES); do \
	${CP} -r $$i ${INCLUDEDIR}; \
	done

clean:
	rm -rf obj/ff.o obj/diskio.o	
	rmdir obj
	rm libxilffs.a


libxilffs.a: obj_dir print_msg_fatfs_base $(FATFS_OBJS) 
	@echo "Creating archive $@"
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) $@ obj/ff.o obj/diskio.o
	

obj_dir:
	mkdir obj

print_msg_fatfs_base:
	@echo "Compiling XilFFs Library"

.c.o:
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) -c $< -o obj/$(@F)
