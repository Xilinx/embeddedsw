#/*************************************************************************************************
#* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
#* SPDX-License-Identifier: MIT
#*************************************************************************************************/


CC := riscv64-unknown-elf-gcc
CC_FLAGS := -MMD -MP -mabi=ilp32 -march=rv32imac_zicsr_zifencei
CFLAGS := -O2 -flto -ffat-lto-objects
# ML-DSA and ML-KEM are configured in misc/xparameters.h, which copy_bsp.sh publishes verbatim to
# the BSP include directory. Read the source rather than the published copy: this file is parsed
# before the copy_bsp.sh prerequisite runs, so on a clean tree the BSP copy does not exist yet.
ASU_XPARAMETERS := ../misc/xparameters.h
# A missing or renamed file would leave the grep empty, which is indistinguishable from PQC being
# switched off: the link would silently drop -loqs and shrink the stack under code that needs it.
# Refuse to guess, the same way CMakeLists.txt does when the xilasu config header is absent.
# Make evaluates this while parsing, so it is skipped when clean is the only goal, leaving a tree
# with the file missing still cleanable. The whole goal list is compared rather than filtered for
# clean, so that "make clean all" keeps the check, as does the default goal with an empty list.
ifneq ($(MAKECMDGOALS),clean)
ifeq ($(wildcard $(ASU_XPARAMETERS)),)
$(error asufw: $(ASU_XPARAMETERS) not found; the ML-DSA and ML-KEM build configuration \
	cannot be determined)
endif
# Inside the guard so clean reads nothing: the error above has already established that the file
# exists for every other goal, and running the grep anyway would print to stderr while cleaning.
PQC_ENABLED := $(shell grep -E '^[[:space:]]*\#define[[:space:]]+XASU_ML(DSA|KEM)_ENABLE' \
	$(ASU_XPARAMETERS))
endif

ifeq ($(strip $(PQC_ENABLED)),)
# lscript.ld defaults to the stack and heap the ML-DSA and ML-KEM paths need: the deep frames of
# liboqs signing and encapsulation, and a heap for the aligned allocations its SHA3 layer makes.
# With both features off nothing reaches the allocator and those frames are gone, so hand the
# space back to the resident instruction RAM the two sections are carved from.
PQC_SIZE_FLAGS = -Wl,--defsym=_STACK_SIZE=0x1400 -Wl,--defsym=_HEAP_SIZE=0x0
OQS_DEP =
else
# Both algorithms share one liboqs archive, so it is linked as soon as either is enabled. The BSP
# withdraws it when both are off, which is why this cannot be unconditional.
PQC_SIZE_FLAGS =
OQS_DEP = ,-loqs
endif

# --build-id=none matches CMakeLists.txt. Without it crt0.o contributes a 36-byte
# .note.gnu.build-id, which lscript.ld KEEPs in resident instruction RAM, so the two flows would
# spend different amounts of that budget on the same configuration.
LN_FLAGS = -Wl,--start-group,-lxil,-lxilasu$(OQS_DEP),-lgcc,-lc,--end-group \
	-Wl,--no-warn-rwx-segment,--gc-sections,--build-id=none \
	$(PQC_SIZE_FLAGS)

c_SOURCES := $(wildcard *.c) $(wildcard */*.c)
S_SOURCES := $(wildcard *.S) $(wildcard */*.S)
s_SOURCES := $(wildcard *.s) $(wildcard */*.s)
INCLUDES := $(wildcard *.h) $(wildcard */*.h)
OBJS := $(patsubst %.c, %.o, $(c_SOURCES))
OBJS += $(patsubst %.S, %.o, $(S_SOURCES))
OBJS += $(patsubst %.s, %.o, $(s_SOURCES))
LSCRIPT_FILE := lscript.ld
LSCRIPT := -T$(LSCRIPT_FILE)
MAP_FILE := asufw.map

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

INCLUDEPATH := -I../misc/asufw_bsp/asu_0/include -I./ -I./task -I./xfih \
	-I./sha -I./trng -I./ecc -I./aes -I./rsa_ecc -I./hmac -I./kdf -I./ecies -I./keywrap \
	-I./x509 -I./ocp -I./keymanager -I./lms -I./overlay -I./provision -I./mldsa -I./mlkem \
	-I./slhdsa -I./versal_2ve_2vm
LIBPATH := -L../misc/asufw_bsp/asu_0/lib

all: $(EXEC)

# Both of the last two are real inputs to the link: lscript.ld carries the stack, heap and overlay
# window sizes, and misc/xparameters.h selects PQC_SIZE_FLAGS and OQS_DEP. Editing either has to
# force a relink the same way editing a source does. xparameters.h already reaches the link through
# $(LIBS), so naming it here is redundant, but it keeps the input visible at the rule whose flags
# are derived from it.
$(EXEC): $(LIBS) $(OBJS) $(INCLUDES) $(LSCRIPT_FILE) $(ASU_XPARAMETERS)
	$(CC) -o $@ $(sort $(OBJS)) $(CC_FLAGS) $(CFLAGS) $(LN_FLAGS) \
		$(LIBPATH) $(LSCRIPT) -Wl,-Map=$(MAP_FILE)
$(OBJS): $(LIBS)

# The BSP carries its own copy of xparameters.h and its own objects, and neither tracks the
# originals in misc/. Because this rule used to fire only when libxil.a was absent, editing a
# feature macro such as XASU_MLDSA_ENABLE rebuilt the application against the new configuration
# while the BSP kept serving the old header, and the link flags derived from misc/xparameters.h
# flipped independently of both. That mixture links, and the resulting image can carry PQC code
# on the 0x1400 stack of a non-PQC build. Treat the BSP configuration as an input so any change
# regenerates the BSP, which in turn forces every object here to be recompiled against it.
# Everything copy_bsp.sh publishes is an input, not just the feature macros: config.make, dep.mk,
# the BSP Makefile, drivers.txt and the HSM-generated x*_g.c all change what the BSP becomes. Take
# the whole directory so a file added later is covered without editing this list, less the two
# entries that are not inputs -- asufw_bsp is the generated output itself, and Readme.txt is
# documentation. copy_bsp.sh stays in: editing the script changes the BSP it produces.
# xparameters.h is named explicitly rather than left to the glob: it is the file that gates PQC,
# and a glob that quietly matches nothing would drop it from the dependency list and let exactly
# the configuration skew this rule exists to prevent come back. Named, make stops instead.
BSP_CONFIG := $(ASU_XPARAMETERS) \
	$(filter-out ../misc/asufw_bsp ../misc/Readme.txt $(ASU_XPARAMETERS), $(wildcard ../misc/*))

$(LIBS): $(BSP_CONFIG)
	echo "Regenerating BSP for the current configuration"
	rm -rf ../misc/asufw_bsp
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
	rm -rf $(OBJS) $(LIBS) $(EXEC) $(MAP_FILE) *.o *.d */*.o */*.d
	rm -rf ../misc/asufw_bsp

-include $(DEPFILES)
