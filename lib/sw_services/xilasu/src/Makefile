###############################################################################
# Copyright (C) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
###############################################################################

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS =
ifeq ($(findstring gcc, $(COMPILER)), gcc)
CUSTOM_COMPILER_FLAGS=-Wall -Wextra -Warray-bounds=2 -Wformat=2 -Wno-main -Wimplicit-function-declaration -Wmissing-prototypes -Wmissing-braces -Wmultistatement-macros -Wparentheses -Wreturn-type -Wsequence-point -Wbool-compare -Wbool-operation -Wchar-subscripts -Wenum-compare -Winit-self -fno-strict-aliasing -Wtautological-compare -Wuninitialized -Wunused-function -Wunused-label -Wunused-variable -Wcast-function-type -Wenum-conversion -Wmissing-parameter-type -Wtype-limits -Wshift-negative-value -Wshadow -Wdouble-promotion -Wundef -fno-common -Wconversion -Wnull-dereference -Wmissing-include-dirs -Wswitch-default -Wswitch-enum -Wswitch-bool -Wshift-overflow=2 -Wswitch-outside-range -Wswitch-unreachable -Wunused-parameter -Wduplicated-branches -Wduplicated-cond -Wdiv-by-zero -Wshadow=local -Wpointer-arith -Wunused-macros -Wconversion -Wdangling-else -Wlogical-op -Waggressive-loop-optimizations -Wstrict-prototypes -Woverflow -Wredundant-decls -Wvla
else
CUSTOM_COMPILER_FLAGS=
endif
LIB=libxilasu.a

RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

# A single prebuilt liboqs drop serves both ML-DSA and ML-KEM, so it is published as soon as
# either one is enabled and withdrawn when both are off, so that turning PQC off cannot leave a
# stale archive behind for the firmware link to pick up.
OQS_LIB=liboqs.a
ASU_XPARAMETERS=${INCLUDEDIR}/xparameters.h
# An absent header would leave the grep empty, which is indistinguishable from both features being
# switched off, and that branch deletes liboqs.a from the BSP lib directory. Refuse to guess, the
# same way CMakeLists.txt does when its config header is missing. copy_bsp.sh publishes this header
# before the BSP is built, so reaching this error means the BSP itself is incomplete.
# Make evaluates this while parsing, so it is skipped when clean is the only goal, leaving a BSP
# with the header missing still cleanable. The whole goal list is compared rather than filtered for
# clean, so that "make clean libs" keeps the check, as does the default goal with an empty list.
# The grep sits inside the guard so clean reads nothing; clean withdraws liboqs.a either way.
ifneq ($(MAKECMDGOALS),clean)
ifeq ($(wildcard $(ASU_XPARAMETERS)),)
$(error xilasu: $(ASU_XPARAMETERS) not found; the ML-DSA and ML-KEM build configuration \
	cannot be determined)
endif
PQC_ENABLED := $(shell grep -E '^[[:space:]]*\#define[[:space:]]+XASU_ML(DSA|KEM)_ENABLE' \
	$(ASU_XPARAMETERS))
endif

ASU_DIR = .
OUTS = *.o
OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
ASU_SRCS := $(wildcard *.c)
ASU_OBJS = $(addprefix $(ASU_DIR)/, $(ASU_SRCS:%.c=%.o))

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

libs: libxilasu.a publish_liboqs

libxilasu.a: print_msg_secure $(ASU_OBJS)
ifneq ("$(wildcard $(ASU_DIR)/$(LIB))","")
	 cp $(ASU_DIR)/$(LIB) $(RELEASEDIR)
	 $(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${ASU_OBJS}
else
	 $(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${ASU_OBJS}
endif

.PHONY: publish_liboqs
publish_liboqs:
ifneq ($(strip $(PQC_ENABLED)),)
	 @if [ ! -f $(ASU_DIR)/$(OQS_LIB) ]; then \
		echo "xilasu: ML-DSA or ML-KEM is enabled but $(OQS_LIB) is missing;" \
			"cannot link" >&2; \
		exit 1; \
	 fi
	 cp $(ASU_DIR)/$(OQS_LIB) $(RELEASEDIR)
else
	 rm -f ${RELEASEDIR}/${OQS_LIB}
endif

print_msg_secure:
	@echo "Compiling Xilasu Library"

.PHONY: include
include: libxilasu_includes

libxilasu_includes:
	${CP} ${INCLUDEFILES} ${INCLUDEDIR}

clean:
	rm -rf $(ASU_DIR)/${OBJECTS}
	rm -rf ${RELEASEDIR}/${LIB}
	rm -rf ${RELEASEDIR}/${OQS_LIB}

$(ASU_DIR)/%.o: $(ASU_DIR)/%.c $(INCLUDEFILES)
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(CUSTOM_COMPILER_FLAGS) $(INCLUDES) -c $< -o $@
