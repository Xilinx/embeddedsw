###############################################################################
# Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
# Copyright (c) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################
DRIVER_LIB_VERSION = v1.0

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS =
EXTRA_COMPILER_FLAGS=
LIB=libxilnvm.a

ifeq ($(notdir $(COMPILER)) , iccarm)
	EXTRA_ARCHIVE_FLAGS=--create
else
ifeq ($(notdir $(COMPILER)) , armclang)
	EXTRA_ARCHIVE_FLAGS=-rc
else
	EXTRA_ARCHIVE_FLAGS=rc
endif
endif

RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

NVM_DIR = .
NVM_SRCS := $(wildcard *.c)
NVM_HDRS := $(wildcard *.h)
NVM_OBJS = $(addprefix $(NVM_DIR)/, $(NVM_SRCS:%.c=%.o))

libs: libxilnvm.a

print_msg_nvm:
	@echo "Compiling XilNVM Library"

libxilnvm.a: print_msg_nvm $(NVM_OBJS)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${NVM_OBJS}

DEPFILES := $(NVM_SRCS:%.c=$(NVM_DIR)%.d)

include $(wildcard $(DEPFILES))

include $(wildcard ../../../../dep.mk)

$(NVM_DIR)/%.o: $(NVM_DIR)/%.c
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) $(DEPENDENCY_FLAGS) -c $< -o $@

.PHONY: include
include: libxilnvm_includes

libxilnvm_includes:
	${CP} ${NVM_HDRS} ${INCLUDEDIR}

clean:
	rm -rf $(NVM_OBJS)
	rm -rf ${RELEASEDIR}/${LIB}
	rm -rf ${DEPFILES}
