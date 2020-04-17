###############################################################################
# Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=
LIB= libxilplmi.a

EXTRA_ARCHIVE_FLAGS=rc
RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

LIBPLMI_DIR = .
OUTS = *.o
OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
LIBPLMI_SRCS := $(wildcard *.c)
LIBPLMI_OBJS = $(addprefix $(LIBPLMI_DIR)/, $(LIBPLMI_SRCS:%.c=%.o))

INCLUDEFILES=$(LIBPLMI_DIR)/*.h

libs: libxilplmi.a

libxilplmi.a: print_msg_xilplmi $(LIBPLMI_OBJS)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${LIBPLMI_OBJS}

print_msg_xilplmi:
	@echo "Compiling XilPLMI Library"

.PHONY: include
include: libxilplmi_includes

libxilplmi_includes:
	${CP} ${INCLUDEFILES} ${INCLUDEDIR}

clean:
	rm -rf $(LIBPLMI_DIR)/${OBJECTS}
	rm -rf ${RELEASEDIR}/${LIB}

$(LIBPLMI_DIR)/%.o: $(LIBPLMI_DIR)/%.c $(INCLUDEFILES)
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) -c $< -o $@
