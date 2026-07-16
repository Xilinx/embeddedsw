###############################################################################
# Copyright (c) 2007 - 2022 Xilinx, Inc.  All rights reserved.
# Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
###############################################################################

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=
LIB=libxilflash.a

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

XILFLASH_DIR = .
OUTS = *.o
OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
XILFLASH_SRCS := $(wildcard *.c)
XILFLASH_OBJS = $(addprefix $(XILFLASH_DIR)/, $(XILFLASH_SRCS:%.c=%.o))

INCLUDEFILES=$(XILFLASH_DIR)/include/xilflash.h \
	     $(XILFLASH_DIR)/include/xilflash_cfi.h \
	     $(XILFLASH_DIR)/include/xilflash_intel.h \
	     $(XILFLASH_DIR)/include/xilflash_properties.h \
	     $(XILFLASH_DIR)/include/xilflash_amd.h

libs: libxilflash.a

libxilflash.a: print_msg_xilflash $(XILFLASH_OBJS)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${XILFLASH_OBJS}

print_msg_xilflash:
	@echo "Compiling XilFlash Library"

.PHONY: include
include: libxilflash_includes

libxilflash_includes:
	${CP} ${INCLUDEFILES} ${INCLUDEDIR}

clean:
	rm -rf $(XILFLASH_DIR)/${OBJECTS}
	rm -rf ${RELEASEDIR}/${LIB}

$(XILFLASH_DIR)/%.o: $(XILFLASH_DIR)/%.c $(INCLUDEFILES)
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) -c $< -o $@
