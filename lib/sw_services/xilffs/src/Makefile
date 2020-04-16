###############################################################################
# Copyright (c) 2013 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS =
LIB=libxilffs.a

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

FATFS_DIR = .
OUTS = *.o
OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
FATFS_SRCS := $(wildcard *.c)
FATFS_OBJS = $(addprefix $(FATFS_DIR)/, $(FATFS_SRCS:%.c=%.o))

INCLUDEFILES=$(FATFS_DIR)/include/ff.h \
			$(FATFS_DIR)/include/ffconf.h \
			$(FATFS_DIR)/include/diskio.h \
			$(FATFS_DIR)/include/integer.h

libs: libxilffs.a

libxilffs.a: print_msg_fatfs $(FATFS_OBJS)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${FATFS_OBJS}

print_msg_fatfs:
	@echo "Compiling XilFFs Library"

.PHONY: include
include: libxilffs_includes

libxilffs_includes:
	${CP} ${INCLUDEFILES} ${INCLUDEDIR}

clean:
	rm -rf $(FATFS_DIR)/${OBJECTS}
	rm -rf ${RELEASEDIR}/${LIB}

$(FATFS_DIR)/%.o: $(FATFS_DIR)/%.c $(INCLUDEFILES)
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) -c $< -o $@
