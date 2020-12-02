###############################################################################
# Copyright (c) 2016 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################
DRIVER_LIB_VERSION = v1.0

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS =
LIB=libxilfpga.a

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

RELEASEDIR=../../../lib/
INCLUDEDIR=../../../include/
INCLUDES=-I./. -I${INCLUDEDIR}

XILFPGA_DIR = ./
XILFPGA_SRCS := $(wildcard *.c)
XILFPGA_OBJS = $(addprefix $(XILFPGA_DIR), $(XILFPGA_SRCS:%.c=%.o))
libs: libxilfpga.a

libxilfpga.a: $(XILFPGA_OBJS)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${XILFPGA_OBJS}
DEPFILES := $(XILFPGA_SRCS:%.c=$(XILFPGA_DIR)%.d)

include $(wildcard $(DEPFILES))

include $(wildcard ../../../../dep.mk)

$(XILFPGA_DIR)%.o: $(XILFPGA_DIR)%.c
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) $(DEPENDENCY_FLAGS) -o $@ $<

.PHONY: include
include: libxilfpga_includes

libxilfpga_includes: $(addprefix $(INCLUDEDIR), $(wildcard *.h))

$(INCLUDEDIR)%.h: %.h
	$(CP) $< $@

clean:
	rm -rf $(XILFPGA_OBJS)
	rm -rf ${RELEASEDIR}/${LIB}
	rm -rf ${DEPFILES}
