###############################################################################
#
# Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
EXTRA_COMPILER_FLAGS=
LIB= libxilloader.a

EXTRA_ARCHIVE_FLAGS=rc
RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

LIBLOADER_DIR = .
OUTS = *.o
OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
LIBLOADER_SRCS := $(wildcard *.c)
LIBLOADER_OBJS = $(addprefix $(LIBLOADER_DIR)/, $(LIBLOADER_SRCS:%.c=%.o))

INCLUDEFILES=$(LIBLOADER_DIR)/xloader.h

libs: libxilloader.a

libxilloader.a: print_msg_xilloader $(LIBLOADER_OBJS)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${LIBLOADER_OBJS}

print_msg_xilloader:
	@echo "Compiling XilLoader Library"

.PHONY: include
include: libxilloader_includes

libxilloader_includes:
	${CP} ${INCLUDEFILES} ${INCLUDEDIR}

clean:
	rm -rf $(LIBLOADER_DIR)/${OBJECTS}
	rm -rf ${RELEASEDIR}/${LIB}

$(LIBLOADER_DIR)/%.o: $(LIBLOADER_DIR)/%.c $(INCLUDEFILES)
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) -c $< -o $@
