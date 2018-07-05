###############################################################################
# Copyright (c) 2013 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=
LIB= libxilskey.a

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

RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

SKEY_DIR = .
OUTS = *.o
OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
SKEY_SRCS := $(wildcard *.c)
SKEY_OBJS = $(addprefix $(SKEY_DIR)/, $(SKEY_SRCS:%.c=%.o))

INCLUDEFILES=$(wildcard *.h)

libs: libxilskey.a

libxilskey.a: print_msg_xilskey $(SKEY_OBJS)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${SKEY_OBJS}

print_msg_xilskey:
	@echo "Compiling Xilskey Library"

.PHONY: include
include: libxilskey_includes

libxilskey_includes:
	${CP} ${INCLUDEFILES} ${INCLUDEDIR}

clean:
	rm -rf $(SKEY_DIR)/${OBJECTS}
	rm -rf ${RELEASEDIR}/${LIB}

$(SKEY_DIR)/%.o: $(SKEY_DIR)/%.c $(INCLUDEFILES)
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) -c $< -o $@
