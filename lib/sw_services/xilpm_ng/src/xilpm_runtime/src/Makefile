###############################################################################
# Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################

COMPILER=
ARCHIVER=
CP=cp
XILPM_RUNTIME_COMPILER_FLAGS= $(_CC_XILPM_FLAGS) -Os -fno-strict-aliasing -c -mcpu=v11.0 -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare
XILPM_RUNTIME_EXTRA_COMPILER_FLAGS=-g  -ffunction-sections -fdata-sections -Wall -Wextra -flto -ffat-lto-objects
CUSTOM_COMPILER_FLAGS=
ifeq ($(findstring gcc, $(COMPILER)), gcc)
CUSTOM_COMPILER_FLAGS+=-Wall -Wextra -Warray-bounds=2 -Wformat=2 -Wno-main -Wimplicit-function-declaration -Wmissing-prototypes -Wmissing-braces -Wmultistatement-macros -Wparentheses -Wreturn-type -Wsequence-point -Wbool-compare -Wbool-operation -Wchar-subscripts -Wenum-compare -Winit-self -fno-strict-aliasing -Wtautological-compare -Wuninitialized -Wunused-function -Wunused-label -Wunused-variable -Wcast-function-type -Wenum-conversion -Wmissing-parameter-type -Wtype-limits -Wshift-negative-value -Wshadow -Wdouble-promotion -fno-common -Wnull-dereference -Wmissing-include-dirs -Wswitch-default -Wswitch-enum -Wswitch-bool -Wshift-overflow=2 -Wswitch-outside-range -Wswitch-unreachable -Wunused-parameter -Wduplicated-branches -Wduplicated-cond -Wdiv-by-zero -Wshadow=local -Wpointer-arith -Wunused-macros -Wdangling-else -Waggressive-loop-optimizations -Wstrict-prototypes -Woverflow -Wredundant-decls -Wvla -Wlogical-op -Wconversion -Wundef
endif
LIB= libxilpm_runtime.a

XILPM_RUNTIME_EEMI_SRCS := xpm_runtime_eemi.c
XILPM_RUNTIME_SUBSYS_SRCS := xpm_runtime_eemi_subsys.c xpm_runtime_mem_subsys.c

# Check if XilPM_Runtime (EEMI) or XilPM_Runtime (Subsys) is enabled
ifneq ($(filter -DXILPM_RUNTIME_EEMI, $(_CC_XILPM_FLAGS)),)
	# Remove XilPM_Runtime (Subsys) specific source files
	XILPM_FILTERED_SRCS := $(XILPM_RUNTIME_SUBSYS_SRCS)
	XILPM_RUNTIME_BANNER := 1
	XILPM_RUNTIME_BANNER_STRING := "XilPm_Runtime: EEMI"
else ifneq ($(filter -DXILPM_RUNTIME_SUBSYS, $(_CC_XILPM_FLAGS)),)
	# Remove XilPM_Runtime (EEMI) specific source files
	XILPM_FILTERED_SRCS := $(XILPM_RUNTIME_EEMI_SRCS)
	XILPM_RUNTIME_BANNER := 2
	XILPM_RUNTIME_BANNER_STRING := "XilPm_Runtime: SUBSYS"
endif

EXTRA_ARCHIVE_FLAGS=rc
RELEASEDIR=../../../lib
INCLUDEDIR=$(shell realpath ../../../include)
INCLUDES=-I${INCLUDEDIR}/
XILPM_DIR = .
OUTS = *.o
OBJECTS = $(addsuffix .o, $(basename $(wildcard *.c)))
XILPM_SRCS = $(filter-out $(XILPM_FILTERED_SRCS), $(wildcard *.c))
XILPM_OBJS = $(addprefix $(XILPM_DIR)/, $(XILPM_SRCS:%.c=%.o))

libs: libxilpm_runtime.a

libxilpm_runtime.a: print_msg_xilpm_runtime $(XILPM_OBJS)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${XILPM_OBJS}

print_msg_xilpm_runtime:
	@echo "**********************************"
	@echo "**********************************"
	@echo "Compiling [$(XILPM_RUNTIME_BANNER_STRING)] Library"
	@echo "**********************************"
	@echo "**********************************"

.PHONY: include
include:
	echo "Include files for this library have already been copied."

clean:
	rm -rf $(XILPM_DIR)/${OBJECTS}
	rm -rf ${RELEASEDIR}/${LIB}

$(XILPM_DIR)/%.o: $(XILPM_DIR)/%.c $(INCLUDEFILES)
	$(COMPILER) $(XILPM_RUNTIME_COMPILER_FLAGS) $(XILPM_RUNTIME_EXTRA_COMPILER_FLAGS) $(CUSTOM_COMPILER_FLAGS) -DXILPM_RUNTIME_BANNER=$(XILPM_RUNTIME_BANNER) $(INCLUDES) -c $< -o $@
