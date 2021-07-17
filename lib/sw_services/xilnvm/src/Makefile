###############################################################################
# Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS =
LIB=libxilnvm.a

RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

NVM_DIR = .
OUTS = *.o
OBJECTS = $(addsuffix .o, $(basename $(wildcard *.c)))
NVM_SRCS := $(wildcard *.c)
NVM_OBJS = $(addprefix $(NVM_DIR)/, $(NVM_SRCS:%.c=%.o))

INCLUDEFILES := $(wildcard *.h)

ifeq ($(notdir $(COMPILER)) , iccarm)
	EXTRA_ARCHIVE_FLAGS=--create
else
ifeq ($(notdir $(COMPILER)) , armclang)
	EXTRA_ARCHIVE_FLAGS=-rc
else
	EXTRA_ARCHIVE_FLAGS=rc
endif
endif

libs: libxilnvm.a

libxilnvm.a: print_msg_nvm $(NVM_OBJS)
ifneq ("$(wildcard $(NVM_DIR)/$(LIB))","")
	 cp $(NVM_DIR)/$(LIB) $(RELEASEDIR)
	 $(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${NVM_OBJS}
else
	 $(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${NVM_OBJS}
endif

print_msg_nvm:
	@echo "Compiling XilNvm Library"

.PHONY: include
include: libxilnvm_includes

libxilnvm_includes:
	${CP} ${INCLUDEFILES} ${INCLUDEDIR}

clean:
	rm -rf $(NVM_DIR)/${OBJECTS}
	rm -rf ${RELEASEDIR}/${LIB}

$(NVM_DIR)/%.o: $(NVM_DIR)/%.c $(INCLUDEFILES)
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) -c $< -o $@
