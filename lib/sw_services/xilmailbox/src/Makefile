###############################################################################
# Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=
LIB= libxilmailbox.a

EXTRA_ARCHIVE_FLAGS=rc
RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

LIBMBOX_DIR = .
OUTS = *.o
OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
LIBMBOX_SRCS := $(wildcard *.c)
LIBMBOX_OBJS = $(addprefix $(LIBMBOX_DIR)/, $(LIBMBOX_SRCS:%.c=%.o))

INCLUDEFILES=$(LIBMBOX_DIR)/*.h

libs: libxilmailbox.a

libxilmailbox.a: print_msg_xilmailbox $(LIBMBOX_OBJS)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${LIBMBOX_OBJS}

print_msg_xilmailbox:
	@echo "Compiling xilmailbox Library"

.PHONY: include
include: libxilmailbox_includes

libxilmailbox_includes:
	${CP} ${INCLUDEFILES} ${INCLUDEDIR}

clean:
	rm -rf $(LIBMBOX_DIR)/${OBJECTS}
	rm -rf ${RELEASEDIR}/${LIB}

$(LIBMBOX_DIR)/%.o: $(LIBMBOX_DIR)/%.c $(INCLUDEFILES)
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) -c $< -o $@
