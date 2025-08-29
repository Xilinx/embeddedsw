# Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
DRIVER_LIB_VERSION = 5.2
COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=
LIB=libxil.a

CC_FLAGS = $(COMPILER_FLAGS)
ECC_FLAGS = $(EXTRA_COMPILER_FLAGS)

RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}
EXTRA_CFLAGS=

XSYSMONPSV_DIR = .
OUTS = *.o
OBJECTS = $(addsuffix .o, $(basename $(wildcard ./*c ./*/*.c)))
XSYSMONPSV_SRCS := $(wildcard ./*.c ./*/*.c)
XSYSMONPSV_OBJS = $(XSYSMONPSV_SRCS:%.c=%.o)

INCLUDEFILES= $(wildcard ./*.h ./*/*.h)

libs: sysmon

sysmon: print_msg_xsysmonpsv $(XSYSMONPSV_OBJS)
	${CP} $(XSYSMONPSV_OBJS) ${RELEASEDIR}

print_msg_xsysmonpsv:
	@echo "Compiling xsysmonpsv"

.PHONY: include
include: sysmon_includes

sysmon_includes:
	${CP} ${INCLUDEFILES} ${INCLUDEDIR}

clean:
	rm -rf ${OBJECTS}
	rm -rf ${RELEASEDIR}/${LIB}

$(XSYSMONPSV_DIR)/%.o: $(XSYSMONPSV_DIR)/%.c $(INCLUDEFILES)
	$(COMPILER) $(COMPILER_FLAGS)  $(EXTRA_COMPILER_FLAGS) $(INCLUDES) -c $< -o $@
