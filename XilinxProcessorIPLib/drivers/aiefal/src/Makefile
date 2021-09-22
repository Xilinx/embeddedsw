###############################################################################
# Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################

INCLUDEDIR_ROOT=../../../include
INCLUDEDIR=$(INCLUDEDIR_ROOT)/xaiefal
INCLUDES=$(wildcard *.hpp) $(wildcard */*.hpp)
TDIRS=$(addsuffix /,$(addprefix $(INCLUDEDIR)/,$(dir $(wildcard */*.hpp))))
TINCLUDES=$(addprefix $(INCLUDEDIR)/,$(INCLUDES))

ifneq (,$(findstring Windows,$(OS)))
	IS_WIN=1
else
	IS_WIN=0
endif

libs: print_msg_xaiefal

print_msg_xaiefal:
	@echo "Compiling xaiefal"

.PHONY: include
include: xaiefal_includes

xaiefal_includes: $(INCLUDEDIR) $(TDIRS) $(TINCLUDES)

ifeq (0,$(IS_WIN))
$(INCLUDEDIR):
	mkdir $@
$(INCLUDEDIR)/%/: $(INCLUDEDIR)
	mkdir $@
else
$(INCLUDEDIR):
	"mkdir" -p $(subst /,\,$@)
$(INCLUDEDIR)/%/: $(INCLUDEDIR)
	"mkdir" -p $(subst /,\,$@)
endif

$(INCLUDEDIR)/%.hpp: %.hpp $(INCLUDEDIR) $(TDIRS)
	cp $< $@

clean:
