###############################################################################
# Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################

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
LIB= libxilplmi.a

EXTRA_ARCHIVE_FLAGS=rc
RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

LIBPLMI_DIR = .
OUTS = *.o
OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
LIBPLMI_SRCS := $(wildcard *.c)
LIBPLMI_OBJS = $(addprefix $(LIBPLMI_DIR)/, $(LIBPLMI_SRCS:%.c=%.o))

INCLUDEFILES=$(LIBPLMI_DIR)/*.h

libs: libxilplmi.a

libxilplmi.a: print_msg_xilplmi $(LIBPLMI_OBJS)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${LIBPLMI_OBJS}

print_msg_xilplmi:
	@echo "Compiling XilPLMI Library"

.PHONY: include
include: libxilplmi_includes

libxilplmi_includes:
	${CP} ${INCLUDEFILES} ${INCLUDEDIR}

clean:
	rm -rf $(LIBPLMI_DIR)/${OBJECTS}
	rm -rf ${RELEASEDIR}/${LIB}

$(LIBPLMI_DIR)/%.o: $(LIBPLMI_DIR)/%.c $(INCLUDEFILES)
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(CUSTOM_COMPILER_FLAGS) $(INCLUDES) -c $< -o $@
