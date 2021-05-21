###############################################################################
# Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=
CUSTOM_COMPILER_FLAGS=-Werror
LIB= libxilpm.a

EXTRA_ARCHIVE_FLAGS=rc
RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

XILPM_DIR = .
OUTS = *.o
OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
XILPM_SRCS := $(wildcard *.c)
XILPM_OBJS = $(addprefix $(XILPM_DIR)/, $(XILPM_SRCS:%.c=%.o))

libs: libxilpm.a

libxilpm.a: print_msg_xilpm $(XILPM_OBJS)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${XILPM_OBJS}

print_msg_xilpm:
	@echo "Compiling XilPM Library"

.PHONY: include
include:
	echo "Include files for this library have already been copied."

clean:
	rm -rf $(XILPM_DIR)/${OBJECTS}
	rm -rf ${RELEASEDIR}/${LIB}

$(XILPM_DIR)/%.o: $(XILPM_DIR)/%.c $(INCLUDEFILES)
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(CUSTOM_COMPILER_FLAGS) $(INCLUDES) -c $< -o $@
