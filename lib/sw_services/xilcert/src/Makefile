###############################################################################
# Copyright (c) 2023, Xilinx, Inc.  All rights reserved.
# Copyright (c) 2023 -2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################
DRIVER_LIB_VERSION = v1.0

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=
LIB= libxilcert.a

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

SCERT_DIR = .
SCERT_SRCS := $(wildcard *.c)
SCERT_HDRS := $(wildcard *.h)
SCERT_OBJS = $(addprefix $(SCERT_DIR)/, $(SCERT_SRCS:%.c=%.o))

libs: libxilcert.a

print_msg_xilcert:
	@echo "Compiling XilCert Library"

libxilcert.a: print_msg_xilcert $(SCERT_OBJS)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${SCERT_OBJS}

DEPFILES := $(SCERT_SRCS:%.c=$(SCERT_DIR)%.d)

include $(wildcard $(DEPFILES))

include $(wildcard ../../../../dep.mk)

$(SCERT_DIR)/%.o: $(SCERT_DIR)/%.c
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) $(DEPENDENCY_FLAGS) -c $< -o $@

.PHONY: include
include: libxilcert_includes

libxilcert_includes:
	${CP} ${SCERT_HDRS} ${INCLUDEDIR}

clean:
	rm -rf $(SCERT_OBJS)
	rm -rf ${RELEASEDIR}/${LIB}
	rm -rf ${DEPFILES}
