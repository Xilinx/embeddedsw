###############################################################################
# Copyright (c) 2017 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=
LIB= libxilpdi.a

EXTRA_ARCHIVE_FLAGS=rc
RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

PDI_DIR = .
OUTS = *.o
OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
PDI_SRCS := $(wildcard *.c)
PDI_OBJS = $(addprefix $(PDI_DIR)/, $(PDI_SRCS:%.c=%.o))

INCLUDEFILES=$(PDI_DIR)/xilpdi.h

libs: libxilpdi.a

libxilpdi.a: print_msg_xilpdi $(PDI_OBJS)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${PDI_OBJS}

print_msg_xilpdi:
	@echo "Compiling Xilpdi Library"

.PHONY: include
include: libxilpdi_includes

libxilpdi_includes:
	${CP} ${INCLUDEFILES} ${INCLUDEDIR}

clean:
	rm -rf $(PDI_DIR)/${OBJECTS}
	rm -rf ${RELEASEDIR}/${LIB}

$(PDI_DIR)/%.o: $(PDI_DIR)/%.c $(INCLUDEFILES)
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) -c $< -o $@
