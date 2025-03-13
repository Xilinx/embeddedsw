###############################################################################
# Copyright (c) 2025, Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################
DRIVER_LIB_VERSION = v1.0

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=
LIB= libxiltpm.a

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

STPM_DIR = .
STPM_SRCS := $(wildcard *.c)
STPM_HDRS := $(wildcard *.h)
STPM_OBJS = $(addprefix $(STPM_DIR)/, $(STPM_SRCS:%.c=%.o))

libs: libxiltpm.a

print_msg_xiltpm:
	@echo "Compiling XilTpm Library"

libxiltpm.a: print_msg_xiltpm $(STPM_OBJS)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${STPM_OBJS}

DEPFILES := $(STPM_SRCS:%.c=$(STPM_DIR)%.d)

include $(wildcard $(DEPFILES))

include $(wildcard ../../../../dep.mk)

$(STPM_DIR)/%.o: $(STPM_DIR)/%.c
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) $(DEPENDENCY_FLAGS) -c $< -o $@

.PHONY: include
include: libxiltpm_includes

libxiltpm_includes:
	${CP} ${STPM_HDRS} ${INCLUDEDIR}

clean:
	rm -rf $(STPM_OBJS)
	rm -rf ${RELEASEDIR}/${LIB}
	rm -rf ${DEPFILES}
