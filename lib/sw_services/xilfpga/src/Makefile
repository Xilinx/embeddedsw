###############################################################################
# Copyright (c) 2007 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=

LIB=libxilfpga.a

RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

xilfpga_DIR = .
OUTS = *.o
OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
xilfpga_SRCS := $(wildcard *.c)
xilfpga_OBJS = $(addprefix $(xilfpga_DIR)/, $(xilfpga_SRCS:%.c=%.o))

INCLUDEFILES=$(xilfpga_DIR)/$(wildcard *.h)


libs: libxilfpga.a

ifeq ($(notdir $(COMPILER)) , iccarm)
    EXTRA_ARCHIVE_FLAGS=--create
else
ifeq ($(notdir $(COMPILER)) , armclang)
      EXTRA_ARCHIVE_FLAGS=-rc
else
       EXTRA_ARCHIVE_FLAGS=rc
endif
endif

libxilfpga.a: print_msg_xilfpga $(xilfpga_OBJS)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${xilfpga_OBJS}

print_msg_xilfpga:
	@echo "Compiling xilfpga Library"

.PHONY: include
include: libxilfpga_includes

libxilfpga_includes:
	${CP} ${INCLUDEFILES} ${INCLUDEDIR}

clean:
	rm -rf $(xilfpga_DIR)/${OBJECTS}
	rm -rf ${RELEASEDIR}/${LIB}

$(xilfpga_DIR)/%.o: $(xilfpga_DIR)/%.c $(INCLUDEFILES)
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) -c $< -o $@
