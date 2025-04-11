###############################################################################
# Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
###############################################################################

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=-Os -g
EXTRA_COMPILER_FLAGS=
CUSTOM_COMPILER_FLAGS=
PLFM_COMPILER_FLAGS= $(_CC_XILPM_FLAGS) -Os -fno-strict-aliasing -c -mcpu=v11.0 -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare
XILPM_BOOT_EXTRA_COMPILER_FLAGS=-g -ffunction-sections -fdata-sections -Wall -Wextra -flto -ffat-lto-objects
ifeq ($(findstring gcc, $(COMPILER)), gcc)
CUSTOM_COMPILER_FLAGS+=-Wall -Wextra -Warray-bounds=2 -Wformat=2 -Wno-main -Wimplicit-function-declaration -Wmissing-prototypes -Wmissing-braces -Wmultistatement-macros -Wparentheses -Wreturn-type -Wsequence-point -Wbool-compare -Wbool-operation -Wchar-subscripts -Wenum-compare -Winit-self -fno-strict-aliasing -Wtautological-compare -Wuninitialized -Wunused-function -Wunused-label -Wunused-variable -Wcast-function-type -Wenum-conversion -Wmissing-parameter-type -Wtype-limits -Wshift-negative-value -Wshadow -Wdouble-promotion -fno-common -Wnull-dereference -Wmissing-include-dirs -Wswitch-default -Wswitch-enum -Wswitch-bool -Wshift-overflow=2 -Wswitch-outside-range -Wswitch-unreachable -Wunused-parameter -Wduplicated-branches -Wduplicated-cond -Wdiv-by-zero -Wshadow=local -Wpointer-arith -Wunused-macros -Wdangling-else -Waggressive-loop-optimizations -Wstrict-prototypes -Woverflow -Wredundant-decls -Wvla -Wlogical-op -Wconversion -Wundef
endif
LIB= libxilpm.a

EXTRA_ARCHIVE_FLAGS=rc
RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES= -I${INCLUDEDIR}

XILPM_DIR = .
OUTS = *.o
OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
XILPM_SRCS := $(wildcard *.c)
XILPM_OBJS = $(addprefix $(XILPM_DIR)/, $(XILPM_SRCS:%.c=%.o))

libs: libxilpm.a

libxilpm.a: print_msg_xilpm $(XILPM_OBJS)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${XILPM_OBJS}

print_msg_xilpm:
	@echo "Compiling XilPM Library"

.PHONY: include
include:
	echo "Include files for this library have already been copied."

clean:
	rm -rf $(XILPM_DIR)/${OBJECTS}
	rm -rf ${RELEASEDIR}/${LIB}

$(XILPM_DIR)/%.o: $(XILPM_DIR)/%.c $(INCLUDEFILES)
	$(COMPILER)  $(PLFM_COMPILER_FLAGS) $(XILPM_BOOT_EXTRA_COMPILER_FLAGS) $(CUSTOM_COMPILER_FLAGS) $(INCLUDES) -c $< -o $@
