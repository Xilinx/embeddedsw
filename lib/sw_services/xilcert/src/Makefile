###############################################################################
# Copyright (c) 2023, Xilinx, Inc.  All rights reserved.
# Copyright (c) 2023, Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=
LIB= libxilcert.a

RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

SCERT_DIR = .
OUTS = *.o
OBJECTS = $(addsuffix .o, $(basename $(wildcard *.c)))
SCERT_SRCS := $(wildcard *.c)
SCERT_OBJS = $(addprefix $(SCERT_DIR)/, $(SCERT_SRCS:%.c=%.o))

INCLUDEFILES=$(wildcard *.h)
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

libs: libxilcert.a

libxilcert.a: print_msg_xilcert $(SCERT_OBJS)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${SCERT_OBJS}

print_msg_xilcert:
	@echo "Compiling XilCert Library"

.PHONY: include
include: libxilcert_includes

libxilcert_includes:
	${CP} ${INCLUDEFILES} ${INCLUDEDIR}

clean:
	rm -rf $(SCERT_DIR)/${OBJECTS}
	rm -rf ${RELEASEDIR}/${LIB}

$(SCERT_DIR)/%.o: $(SCERT_DIR)/%.c $(INCLUDEFILES)
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) -c $< -o $@
