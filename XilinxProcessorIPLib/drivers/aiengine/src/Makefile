###############################################################################
# Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS= -D__AIEBAREMTL__
EXTRA_COMPILER_FLAGS=
EXTRA_ARCHIVE_FLAGS=rc
LIB=libxil.a

RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

XAIE_DIR = .
OUTS = *.o
OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
XAIE_SRCS_EXCLUDE := xaieio.c xaietile_proc.c
XAIE_SRCS := $(filter-out $(XAIE_SRCS_EXCLUDE), $(wildcard *.c))
XAIE_OBJS = $(addprefix $(XAIE_DIR)/, $(XAIE_SRCS:%.c=%.o))

INCLUDEFILES= $(wildcard *.h)

libs: xaie

xaie: print_msg_xaie $(XAIE_OBJS)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${XAIE_OBJS}

print_msg_xaie:
	@echo "Compiling xaie"

.PHONY: include
include: xaie_includes

xaie_includes:
	${CP} xaiengine.h ${INCLUDEDIR}
	${CP} ${INCLUDEFILES} ${INCLUDEDIR}/xaiengine

clean:
	rm -rf $(XAIE_DIR)/${OBJECTS}
	rm -rf ${RELEASEDIR}/${LIB}

$(XAIE_DIR)/%.o: $(XAIE_DIR)/%.c $(INCLUDEFILES)
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) -c $< -o $@
