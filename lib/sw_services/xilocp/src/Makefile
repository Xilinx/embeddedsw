###############################################################################
# Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
# Copyright (c) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################
DRIVER_LIB_VERSION = v1.0

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=
LIB= libxilocp.a

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

SOCP_DIR = .
SOCP_SRCS := $(wildcard *.c)
SOCP_HDRS := $(wildcard *.h)
SOCP_OBJS = $(addprefix $(SOCP_DIR)/, $(SOCP_SRCS:%.c=%.o))

libs: libxilocp.a

print_msg_xilocp:
	@echo "Compiling XilOCP Library"

libxilocp.a: print_msg_xilocp $(SOCP_OBJS)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${SOCP_OBJS}

DEPFILES := $(SOCP_SRCS:%.c=$(SOCP_DIR)%.d)

include $(wildcard $(DEPFILES))

include $(wildcard ../../../../dep.mk)

$(SOCP_DIR)/%.o: $(SOCP_DIR)/%.c
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) $(DEPENDENCY_FLAGS) -c $< -o $@

.PHONY: include
include: libxilocp_includes

libxilocp_includes:
	${CP} ${SOCP_HDRS} ${INCLUDEDIR}

clean:
	rm -rf $(SOCP_OBJS)
	rm -rf ${RELEASEDIR}/${LIB}
	rm -rf ${DEPFILES}
