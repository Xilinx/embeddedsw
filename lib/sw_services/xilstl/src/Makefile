###############################################################################
# Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=
LIB= libxilstl.a

EXTRA_ARCHIVE_FLAGS=rc
RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

STL_DIR = .
OUTS = *.o
OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
STL_SRCS := $(wildcard *.c)
STL_OBJS = $(addprefix $(STL_DIR)/, $(STL_SRCS:%.c=%.o))

libs:
	cp $(STL_DIR)/libxilstl.a $(RELEASEDIR)

.PHONY: include
include:
	echo "Include files for this library have already been copied."

clean:
	rm -rf ${RELEASEDIR}/${LIB}
