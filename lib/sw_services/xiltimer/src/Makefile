###############################################################################
# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################
DRIVER_LIB_VERSION = v1.0

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=

LIB=libxiltimer.a

RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

xiltimer_DIR = .
OUTS = *.o
OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
xiltimer_SRCS := $(wildcard *.c)
xiltimer_OBJS = $(addprefix $(xiltimer_DIR)/, $(xiltimer_SRCS:%.c=%.o))

INCLUDEFILES=$(xiltimer_DIR)/$(wildcard *.h)


libs: libxiltimer.a

ifeq ($(notdir $(COMPILER)) , iccarm)
    EXTRA_ARCHIVE_FLAGS=--create
else
ifeq ($(notdir $(COMPILER)) , armclang)
      EXTRA_ARCHIVE_FLAGS=-rc
else
       EXTRA_ARCHIVE_FLAGS=rc
endif
endif

libxiltimer.a: print_msg_xiltimer $(xiltimer_OBJS)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${xiltimer_OBJS}

print_msg_xiltimer:
	@echo "Compiling xiltimer Library"

.PHONY: include
include: libxiltimer_includes

libxiltimer_includes:
	${CP} ${INCLUDEFILES} ${INCLUDEDIR}

clean:
	rm -rf $(xiltimer_DIR)/${OBJECTS}
	rm -rf ${RELEASEDIR}/${LIB}

$(xiltimer_DIR)/%.o: $(xiltimer_DIR)/%.c $(INCLUDEFILES)
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) -c $< -o $@
