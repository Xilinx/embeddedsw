###############################################################################
# Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################

INCLUDEDIR_ROOT=../../../include
INCLUDEDIR=$(INCLUDEDIR_ROOT)/xaiefal
INCLUDES=$(wildcard *.hpp) $(wildcard */*.hpp)
TDIRS=$(dir $(INCLUDES)
TINCLUDES=$(addprefix $(INCLUDEDIR)/,$(INCLUDES))

libs: print_msg_xaiefal

print_msg_xaiefal:
	@echo "Compiling xaiefal"

.PHONY: include
include: xaiefal_includes

xaiefal_includes: $(TINCLUDES)

$(INCLUDEDIR)/%.hpp: %.hpp
	cp $< $@

clean:
