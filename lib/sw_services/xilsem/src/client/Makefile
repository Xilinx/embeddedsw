###############################################################################
# Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
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

libs: libxilsem.a

libxilsem.a: print_msg_xilsem $(SEM_OBJS)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${SEM_OBJS}

print_msg_xilsem:
	@echo "Compiling XilSEM Library"

.PHONY: include
include:
	echo "Include files for this library have already been copied."

clean:
	rm -rf $(SEM_DIR)/${OBJECTS}
	rm -rf ${RELEASEDIR}/${LIB}

$(SEM_DIR)/%.o: $(SEM_DIR)/%.c $(INCLUDEFILES)
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) -c $< -o $@
