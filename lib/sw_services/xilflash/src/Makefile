###############################################################################
#
# Copyright (C) 2007 - 2015 Xilinx, Inc.  All rights reserved.
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
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# 
#
###############################################################################

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=
LIB=libxilflash.a

ifeq ($(notdir $(COMPILER)) , iccarm)
	EXTRA_ARCHIVE_FLAGS=--create
else
ifeq ($(notdir $(COMPILER)) , armcc)
	EXTRA_ARCHIVE_FLAGS=--create
else
ifeq ($(notdir $(COMPILER)) , armclang)
	EXTRA_ARCHIVE_FLAGS=-rc
else
	EXTRA_ARCHIVE_FLAGS=rc
endif
endif
endif


RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

XILFLASH_DIR = .
OUTS = *.o
OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
XILFLASH_SRCS := $(wildcard *.c)
XILFLASH_OBJS = $(addprefix $(XILFLASH_DIR)/, $(XILFLASH_SRCS:%.c=%.o))

INCLUDEFILES=$(XILFLASH_DIR)/include/xilflash.h \
	     $(XILFLASH_DIR)/include/xilflash_cfi.h \
	     $(XILFLASH_DIR)/include/xilflash_intel.h \
	     $(XILFLASH_DIR)/include/xilflash_properties.h \
	     $(XILFLASH_DIR)/include/xilflash_amd.h

libs: libxilflash.a

libxilflash.a: print_msg_xilflash $(XILFLASH_OBJS)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${XILFLASH_OBJS}

print_msg_xilflash:
	@echo "Compiling XilFlash Library"

.PHONY: include
include: libxilflash_includes

libxilflash_includes:
	${CP} ${INCLUDEFILES} ${INCLUDEDIR}

clean:
	rm -rf $(XILFLASH_DIR)/${OBJECTS}
	rm -rf ${RELEASEDIR}/${LIB}

$(XILFLASH_DIR)/%.o: $(XILFLASH_DIR)/%.c $(INCLUDEFILES)
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) -c $< -o $@
