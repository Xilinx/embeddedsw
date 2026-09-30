###############################################################################
# Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
# Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################
DRIVER_LIB_VERSION = v1.0

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=
ifeq ($(findstring gcc, $(COMPILER)), gcc)
CUSTOM_COMPILER_FLAGS=-Wall -Wextra -Warray-bounds=2 -Wformat=2 -Wno-main -Wimplicit-function-declaration -Wmissing-prototypes -Wmissing-braces -Wmultistatement-macros -Wparentheses -Wreturn-type -Wsequence-point -Wbool-compare -Wbool-operation -Wchar-subscripts -Wenum-compare -Winit-self -fno-strict-aliasing -Wtautological-compare -Wuninitialized -Wunused-function -Wunused-label -Wunused-variable -Wcast-function-type -Wenum-conversion -Wmissing-parameter-type -Wtype-limits -Wshift-negative-value -Wshadow -Wdouble-promotion -Wundef -fno-common -Wconversion -Wnull-dereference -Wmissing-include-dirs -Wswitch-default -Wswitch-enum -Wswitch-bool -Wshift-overflow=2 -Wswitch-outside-range -Wswitch-unreachable -Wunused-parameter -Wduplicated-branches -Wduplicated-cond -Wdiv-by-zero -Wshadow=local -Wpointer-arith -Wunused-macros -Wconversion -Wdangling-else -Wlogical-op -Waggressive-loop-optimizations -Wstrict-prototypes -Woverflow -Wredundant-decls -Wvla
else
CUSTOM_COMPILER_FLAGS=
endif
ifeq ($(findstring mb-gcc, $(COMPILER)), mb-gcc)
CUSTOM_COMPILER_FLAGS+= -mlittle-endian
endif
LIB= libxilpuf.a

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

SPUF_DIR = .
SPUF_SRCS := $(wildcard *.c)
SPUF_HDRS := $(wildcard *.h)
SPUF_OBJS = $(addprefix $(SPUF_DIR)/, $(SPUF_SRCS:%.c=%.o))

libs: libxilpuf.a

print_msg_xilpuf:
	@echo "Compiling XilPuf Library"

libxilpuf.a: print_msg_xilpuf $(SPUF_OBJS)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${SPUF_OBJS}

DEPFILES := $(SPUF_SRCS:%.c=$(SPUF_DIR)%.d)

include $(wildcard $(DEPFILES))

include $(wildcard ../../../../dep.mk)

$(SPUF_DIR)/%.o: $(SPUF_DIR)/%.c $(INCLUDEFILES)
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(CUSTOM_COMPILER_FLAGS) $(INCLUDES) $(DEPENDENCY_FLAGS) -c $< -o $@

.PHONY: include
include: libxilpuf_includes

libxilpuf_includes:
	${CP} ${SPUF_HDRS} ${INCLUDEDIR}

clean:
	rm -rf $(SPUF_OBJS)
	rm -rf ${RELEASEDIR}/${LIB}
	rm -rf ${DEPFILES}
