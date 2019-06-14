###############################################################################
#
# Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
LIB= libxilmailbox.a

EXTRA_ARCHIVE_FLAGS=rc
RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

LIBMBOX_DIR = .
OUTS = *.o
OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
LIBMBOX_SRCS := $(wildcard *.c)
LIBMBOX_OBJS = $(addprefix $(LIBMBOX_DIR)/, $(LIBMBOX_SRCS:%.c=%.o))

INCLUDEFILES=$(LIBMBOX_DIR)/*.h

libs: libxilmailbox.a

libxilmailbox.a: print_msg_xilmailbox $(LIBMBOX_OBJS)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${LIBMBOX_OBJS}

print_msg_xilmailbox:
	@echo "Compiling xilmailbox Library"

.PHONY: include
include: libxilmailbox_includes

libxilmailbox_includes:
	${CP} ${INCLUDEFILES} ${INCLUDEDIR}

clean:
	rm -rf $(LIBMBOX_DIR)/${OBJECTS}
	rm -rf ${RELEASEDIR}/${LIB}

$(LIBMBOX_DIR)/%.o: $(LIBMBOX_DIR)/%.c $(INCLUDEFILES)
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) -c $< -o $@
