###############################################################################
# Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
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
