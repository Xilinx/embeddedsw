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
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
#
#
###############################################################################

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS= -D__AIEBAREMTL__
EXTRA_COMPILER_FLAGS=
EXTRA_ARCHIVE_FLAGS=rc
LIB=libxil.a

RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

XAIE_DIR = .
OUTS = *.o
OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
XAIE_SRCS_EXCLUDE := xaieio.c xaietile_proc.c
XAIE_SRCS := $(filter-out $(XAIE_SRCS_EXCLUDE), $(wildcard *.c))
XAIE_OBJS = $(addprefix $(XAIE_DIR)/, $(XAIE_SRCS:%.c=%.o))

INCLUDEFILES= $(wildcard *.h)

libs: xaie

xaie: print_msg_xaie $(XAIE_OBJS)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${XAIE_OBJS}

print_msg_xaie:
	@echo "Compiling xaie"

.PHONY: include
include: xaie_includes

xaie_includes:
	mkdir $(INCLUDEDIR)/xaiengine -p
	${CP} xaiengine.h ${INCLUDEDIR}
	${CP} ${INCLUDEFILES} ${INCLUDEDIR}/xaiengine

clean:
	rm -rf $(XAIE_DIR)/${OBJECTS}
	rm -rf ${RELEASEDIR}/${LIB}

$(XAIE_DIR)/%.o: $(XAIE_DIR)/%.c $(INCLUDEFILES)
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) -c $< -o $@
