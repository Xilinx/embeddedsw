###############################################################################
# Copyright (c) 2012 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=
LIB=libxilisf.a

ifeq ($(notdir $(COMPILER)) , iccarm)
	EXTRA_ARCHIVE_FLAGS=--create
else
ifeq ($(notdir $(COMPILER)) , armcc)
	EXTRA_ARCHIVE_FLAGS=--create
else
	EXTRA_ARCHIVE_FLAGS=rc
endif
endif


RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

XILISF_DIR = .
OUTS = *.o
OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
XILISF_SRCS := $(wildcard *.c)
XILISF_OBJS = $(addprefix $(XILISF_DIR)/, $(XILISF_SRCS:%.c=%.o))

INCLUDEFILES=$(XILISF_DIR)/include/xilisf.h \
	     $(XILISF_DIR)/include/xilisf_atmel.h \
	     $(XILISF_DIR)/include/xilisf_intelstm.h

libs: libxilisf.a

libxilisf.a: print_msg_xilisf $(XILISF_OBJS)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${XILISF_OBJS}

print_msg_xilisf:
	@echo "Compiling XilIsf Library"
	@echo ""; echo "WARNING: Xilisf library is being deprecated from 2020.1 release. It will be made obsolete in 2021.1 release."
	@echo "please use driver examples instead."; echo ""

.PHONY: include
include: libxilisf_includes

libxilisf_includes:
	${CP} ${INCLUDEFILES} ${INCLUDEDIR}

clean:
	rm -rf $(XILISF_DIR)/${OBJECTS}
	rm -rf ${RELEASEDIR}/${LIB}

$(XILISF_DIR)/%.o: $(XILISF_DIR)/%.c $(INCLUDEFILES)
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) -c $< -o $@
