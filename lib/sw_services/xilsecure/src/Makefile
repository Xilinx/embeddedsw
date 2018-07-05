###############################################################################
# Copyright (c) 2013 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS =
LIB=libxilsecure.a

RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

SECURE_DIR = .
OUTS = *.o
OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
SECURE_SRCS := $(wildcard *.c)
SECURE_OBJS = $(addprefix $(SECURE_DIR)/, $(SECURE_SRCS:%.c=%.o))

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

libs: libxilsecure.a

libxilsecure.a: print_msg_secure $(SECURE_OBJS)
ifneq ("$(wildcard $(SECURE_DIR)/$(LIB))","")
	 cp $(SECURE_DIR)/$(LIB) $(RELEASEDIR)
	 $(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${SECURE_OBJS}
else
	 $(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${SECURE_OBJS}
endif

print_msg_secure:
	@echo "Compiling XilSecure Library"

.PHONY: include
include: libxilsecure_includes

libxilsecure_includes:
	${CP} ${INCLUDEFILES} ${INCLUDEDIR}

clean:
	rm -rf $(SECURE_DIR)/${OBJECTS}
	rm -rf ${RELEASEDIR}/${LIB}

$(SECURE_DIR)/%.o: $(SECURE_DIR)/%.c $(INCLUDEFILES)
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) -c $< -o $@
