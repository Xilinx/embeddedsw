###############################################################################
# Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=
LIB= libxilocp.a

RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

SOCP_DIR = .
OUTS = *.o
OBJECTS = $(addsuffix .o, $(basename $(wildcard *.c)))
SOCP_SRCS := $(wildcard *.c)
SOCP_OBJS = $(addprefix $(SOCP_DIR)/, $(SOCP_SRCS:%.c=%.o))

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

libs: libxilocp.a

libxilocp.a: print_msg_xilocp $(SOCP_OBJS)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${SOCP_OBJS}

print_msg_xilocp:
	@echo "Compiling XilOcp Library"

.PHONY: include
include: libxilocp_includes

libxilocp_includes:
	${CP} ${INCLUDEFILES} ${INCLUDEDIR}

clean:
	rm -rf $(SOCP_DIR)/${OBJECTS}
	rm -rf ${RELEASEDIR}/${LIB}

$(SOCP_DIR)/%.o: $(SOCP_DIR)/%.c $(INCLUDEFILES)
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) -c $< -o $@
