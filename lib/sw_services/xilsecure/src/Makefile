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
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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

ifeq ($(notdir $(COMPILER)) , arm-xilinx-eabi-gcc)
	EXTRA_ARCHIVE_FLAGS=rc
else
ifeq ($(notdir $(COMPILER)) , aarch64-none-elf-gcc)
	EXTRA_ARCHIVE_FLAGS=rc
else
ifeq ($(notdir $(COMPILER)) , arm-none-eabi-gcc)
	EXTRA_ARCHIVE_FLAGS=rc
else
	EXTRA_ARCHIVE_FLAGS=--create
endif
endif
endif

ifeq ($(COMPILER) , mb-gcc)
	EXTRA_ARCHIVE_FLAGS=rc
endif

RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./include/ -I${INCLUDEDIR}
SECURE_DIR = .
OBJDIR = obj/

LIB_SRCS = $(SECURE_DIR)/xsecure_sha.c \
          $(SECURE_DIR)/xsecure_aes.c \
          $(SECURE_DIR)/xsecure_rsa.c


# create SECURE_SRCS based on configured options

SECURE_SRCS = $(LIB_SRCS)

SECURE_OBJS = $(addprefix $(OBJDIR), $(SECURE_SRCS:%.c=%.o))


EXPORT_INCLUDE_FILES = $(SECURE_DIR)/xsecure_sha.h \
			$(SECURE_DIR)/xsecure_rsa.h \
			$(SECURE_DIR)/xsecure_aes.h


libs: libxilsecure.a
	cp libxilsecure.a $(RELEASEDIR)
	make clean

include:
	$(@for i in $(EXPORT_INCLUDE_FILES); do \
	${CP} -r $$i ${INCLUDEDIR}; \
	done)

clean:
	rm -rf obj/xsecure_rsa.o obj/xsecure_sha.o obj/xsecure_aes.o
	rmdir obj
	rm libxilsecure.a


libxilsecure.a: obj_dir print_msg_secure_base $(SECURE_OBJS)
	@echo "Creating archive $@"
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) $@ obj/xsecure_rsa.o obj/xsecure_sha.o obj/xsecure_aes.o


obj_dir:
	mkdir -p obj

print_msg_secure_base:
	@echo "Compiling Xilsecure Library"

$(OBJDIR)%.o: $(SECURE_DIR)/%.c
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) -c $< -o $@
