###############################################################################
# Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
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

LIBPM_DIR = .
OUTS = *.o
OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
LIBPM_SRCS := $(wildcard *.c)
LIBPM_OBJS = $(addprefix $(LIBPM_DIR)/, $(LIBPM_SRCS:%.c=%.o))

libs: libxilpm.a

libxilpm.a: print_msg_xilpm $(LIBPM_OBJS)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${LIBPM_OBJS}

print_msg_xilpm:
	@echo "Compiling XilPM Library"

.PHONY: include
include:
	echo "Include files for this library have already been copied."

clean:
	rm -rf $(LIBPM_DIR)/${OBJECTS}
	rm -rf ${RELEASEDIR}/${LIB}

$(LIBPM_DIR)/%.o: $(LIBPM_DIR)/%.c $(INCLUDEFILES)
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(CUSTOM_COMPILER_FLAGS) $(INCLUDES) -c $< -o $@
