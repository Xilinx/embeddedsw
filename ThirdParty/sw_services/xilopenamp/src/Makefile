###############################################################################
#
# Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
COMPILER_FLAGS =
EXTRA_ARCHIVE_FLAGS=rc
CC_FLAGS= -mcpu=cortex-r5
RELEASEDIR=../../../lib
#RELEASEDIR= .
INCLUDEDIR=../../../include
INCLUDES=-I./include/ -I${INCLUDEDIR}
OPENAMP_DIR = .

LIB_SRCS = $(OPENAMP_DIR)/config.c \
          $(OPENAMP_DIR)/bm_env.c \
          $(OPENAMP_DIR)/elf_loader.c \
          $(OPENAMP_DIR)/hil.c \
          $(OPENAMP_DIR)/llist.c \
          $(OPENAMP_DIR)/remote_device.c \
          $(OPENAMP_DIR)/remoteproc.c \
          $(OPENAMP_DIR)/remoteproc_loader.c \
          $(OPENAMP_DIR)/rpmsg.c \
          $(OPENAMP_DIR)/rpmsg_core.c \
          $(OPENAMP_DIR)/rpmsg_retarget.c\
          $(OPENAMP_DIR)/rsc_table_parser.c \
          $(OPENAMP_DIR)/sh_mem.c \
          $(OPENAMP_DIR)/virtio.c \
          $(OPENAMP_DIR)/virtqueue.c


# create SECURE_SRCS based on configured options

OPENAMP_SRCS = $(LIB_SRCS)

OPENAMP_OBJS = $(OPENAMP_SRCS:%.c=%.o)


EXPORT_INCLUDE_FILES:=*.h


libs: libxilopenamp.a
	cp libxilopenamp.a $(RELEASEDIR)
	make clean

include:
	${CP} ${EXPORT_INCLUDE_FILES} ${INCLUDEDIR}

clean:
	rm -rf obj/config.o obj/bm_env.o obj/rpmsg_retarget.o obj/elf_loader.o obj/hil.o obj/llist.o obj/remote_device.o obj/remoteproc.o obj/remoteproc_loader.o obj/rpmsg.o obj/rpmsg_core.o obj/rsc_table_parser.o obj/sh_mem.o obj/virtio.o obj/virtqueue.o
	rmdir obj
	rm libxilopenamp.a


libxilopenamp.a: obj_dir print_msg_secure_base $(OPENAMP_OBJS)
	@echo "Creating archive $@"
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) $@ obj/config.o obj/bm_env.o obj/rpmsg_retarget.o obj/elf_loader.o obj/hil.o obj/llist.o obj/remote_device.o obj/remoteproc.o obj/remoteproc_loader.o obj/rpmsg.o obj/rpmsg_core.o obj/rsc_table_parser.o obj/sh_mem.o obj/virtio.o obj/virtqueue.o


obj_dir:
	mkdir obj

print_msg_secure_base:
	@echo "Compiling XilOpenAmp Library"

.c.o:
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(CC_FLAGS) $(INCLUDES) -c $< -o obj/$(@F)
