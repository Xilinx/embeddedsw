###############################################################################
# Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=
LIB= libxilsem.a

EXTRA_ARCHIVE_FLAGS=rc
RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

SEM_DIR = .
OUTS = *.o
OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
SEM_SRCS := $(wildcard *.c)
SEM_OBJS = $(addprefix $(SEM_DIR)/, $(SEM_SRCS:%.c=%.o))

libs:
	cp $(SEM_DIR)/libxilsem.a $(RELEASEDIR)

.PHONY: include
include:
	echo "Include files for this library have already been copied."

clean:
	rm -rf ${RELEASEDIR}/${LIB}
