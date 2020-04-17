###############################################################################
# Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=
LIB= libxilpuf.a

RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

SPUF_DIR = .
OUTS = *.o
OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
SPUF_SRCS := $(wildcard *.c)
SPUF_OBJS = $(addprefix $(SPUF_DIR)/, $(SPUF_SRCS:%.c=%.o))

INCLUDEFILES=$(wildcard *.h)
ifeq ($(notdir $(COMPILER)) , iccarm)
    EXTRA_ARCHIVE_FLAGS=--create
else
ifeq ($(notdir $(COMPILER)) , armcc)
EXTRA_ARCHIVE_FLAGS=--create
else
ifeq ($(notdir $(COMPILER)) , armclang)
      EXTRA_ARCHIVE_FLAGS=-rc
else
       EXTRA_ARCHIVE_FLAGS=rc
endif
endif
endif


libs: libxilpuf.a

libxilpuf.a: print_msg_xilpuf $(SPUF_OBJS)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${SPUF_OBJS}

print_msg_xilpuf:
	@echo "Compiling XilPuf Library"

.PHONY: include
include: libxilpuf_includes

libxilpuf_includes:
	${CP} ${INCLUDEFILES} ${INCLUDEDIR}

clean:
	rm -rf $(SPUF_DIR)/${OBJECTS}
	rm -rf ${RELEASEDIR}/${LIB}

$(SPUF_DIR)/%.o: $(SPUF_DIR)/%.c $(INCLUDEFILES)
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) -c $< -o $@
