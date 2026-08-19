#/*************************************************************************************************
#* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
#* SPDX-License-Identifier: MIT
#*************************************************************************************************/


CC := riscv64-unknown-elf-gcc
CC_FLAGS := -MMD -MP -mabi=ilp32 -march=rv32imac_zicsr_zifencei
CFLAGS := -O2 -flto -ffat-lto-objects
LN_FLAGS = -Wl,--start-group,-lxil,-lxilasu,-lgcc,-lc,--end-group -Wl,--no-warn-rwx-segment,--gc-sections

c_SOURCES := $(wildcard *.c) $(wildcard */*.c)
S_SOURCES := $(wildcard *.S) $(wildcard */*.S)
s_SOURCES := $(wildcard *.s) $(wildcard */*.s)
INCLUDES := $(wildcard *.h) $(wildcard */*.h)
OBJS := $(patsubst %.c, %.o, $(c_SOURCES))
OBJS += $(patsubst %.S, %.o, $(S_SOURCES))
OBJS += $(patsubst %.s, %.o, $(s_SOURCES))
LSCRIPT := -Tlscript.ld

# These driver translation units are relocated into the DDR-backed overlay
# sections by the OVERLAY blocks in lscript.ld.
#   -fno-lto : keep each a distinct object with real, linker-selectable
#              machine-code sections. LTO would merge them into the combined
#              image and defeat the per-object section placement.
#   -msmall-data-limit=0 : disable the small-data area for these objects. Their
#              .text sits in the IRAM overlay window while their data
#              (.rodata/.data/.bss) sits in the far data RAM overlay window.
#              gp-relative small-data addressing assumes data sits within +/-2KB
#              of __global_pointer$, which is not true across the overlay
#              windows, so it must be disabled to force absolute/PC-relative
#              data references.
OVERLAY_SOURCES := ecc/xecc.c \
	rsa_ecc/xrsa.c \
	rsa_ecc/xrsa_ecc.c \
	rsa_ecc/xrsa_eccinfo.c \
	rsa_ecc/xrsa_padding.c \
	lms/xlms.c \
	lms/xlms_core.c \
	lms/xlms_ots.c \
	ecies/xecies.c \
	overlay/xoverlay_sections.c
OVERLAY_OBJS := $(patsubst %.c, %.o, $(OVERLAY_SOURCES))
$(OVERLAY_OBJS): CFLAGS += -fno-lto -msmall-data-limit=0

CURRENT_DIR = $(shell pwd)
DEPFILES := $(patsubst %.o, %.d, $(OBJS))
LIBS := ../misc/asufw_bsp/asu_0/lib/libxil.a
EXEC := asufw.elf

INCLUDEPATH := -I../misc/asufw_bsp/asu_0/include -I./ -I./task -I./xfih -I./sha -I./trng -I./ecc -I./aes -I./rsa_ecc -I./hmac -I./kdf -I./ecies -I./keywrap -I./x509 -I./ocp -I./keymanager -I./lms -I./overlay -I./provision
LIBPATH := -L../misc/asufw_bsp/asu_0/lib

all: $(EXEC)

$(EXEC): $(LIBS) $(OBJS) $(INCLUDES)
	$(CC) -o $@ $(sort $(OBJS)) $(CC_FLAGS) $(CFLAGS) $(LN_FLAGS) $(LIBPATH) $(LSCRIPT)
$(OBJS): $(LIBS)

$(LIBS):
	echo "Copying BSP files"
	../misc/copy_bsp.sh
	echo "Compiling bsp"
	$(MAKE) -C ../misc/asufw_bsp/

%.o:%.c
	$(CC) $(CFLAGS) $(CC_FLAGS) -c $< -o $@ $(INCLUDEPATH)

%.o:%.S
	$(CC) $(CFLAGS) $(CC_FLAGS) -c $< -o $@ $(INCLUDEPATH)

%.o:%.s
	$(CC) $(CFLAGS) $(CC_FLAGS) -c $< -o $@ $(INCLUDEPATH)

clean:
	rm -rf $(OBJS) $(LIBS) $(EXEC) *.o *.d */*.o */*.d
	rm -rf ../misc/asufw_bsp

-include $(DEPFILES)
