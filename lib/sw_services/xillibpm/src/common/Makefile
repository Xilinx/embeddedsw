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
LIB= libxillibpm.a

EXTRA_ARCHIVE_FLAGS=rc
RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

LIBPM_DIR = .
OUTS = *.o
OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
LIBPM_SRCS := $(wildcard *.c)
LIBPM_OBJS = $(addprefix $(LIBPM_DIR)/, $(LIBPM_SRCS:%.c=%.o))

libs: libxillibpm.a

libxillibpm.a: print_msg_xillibpm $(LIBPM_OBJS)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${LIBPM_OBJS}

print_msg_xillibpm:
	@echo "Compiling XilLibPM Library"

.PHONY: include
include:
	echo "Include files for this library have already been copied."

clean:
	rm -rf $(LIBPM_DIR)/${OBJECTS}
	rm -rf ${RELEASEDIR}/${LIB}

$(LIBPM_DIR)/%.o: $(LIBPM_DIR)/%.c $(INCLUDEFILES)
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) -c $< -o $@
