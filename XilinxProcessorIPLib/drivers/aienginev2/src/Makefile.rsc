###############################################################################
# Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################

CROSSCOMPILE ?=
RSCMETA ?= aie_rsc_meta.bin
GEN_RSCMETA ?= gen_aie_rsc_meta.bin

# use objcopy instead of ld is to make sure the AI engine resource meta data
# is linked as read only data
OBJCOPY = $(CROSSCOMPILE)objcopy
OBJ = $(subst .bin,.o,$(GEN_RSCMETA))

OUTFMT=elf64-x86-64
OUTARCH=i386
ifneq (,$(findstring aarch64,$(CROSSCOMPILE)))
  OUTFMT=elf64-littleaarch64
  OUTARCH=aarch64
endif
ifneq (,$(findstring armr5,$(CROSSCOMPILE)))
  OUTFMT=elf32-littlearm
  OUTARCH=arm
endif
ifneq (,$(findstring mb-,$(CROSSCOMPILE)))
  OUTFMT=elf32-microblazeel
  OUTARCH=microblazeel
endif
ifneq (,$(findstring microblaze,$(CROSSCOMPILE)))
  OUTFMT=elf32-microblazeel
  OUTARCH=microblazeel
endif

all: addrsc

addrsc: $(OBJ)

$(OBJ): $(GEN_RSCMETA)
	$(OBJCOPY) -I binary -O $(OUTFMT) -B $(OUTARCH) --rename-section .data=.rodata,alloc,load,readonly,data,contents $< $@

$(GEN_RSCMETA): $(RSCMETA)
	cp $< $@

clean:
	rm -f $(OBJ) $(GEN_RSCMETA)

.PHONY: all addrsc
