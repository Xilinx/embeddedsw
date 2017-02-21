###############################################################################
#
# Copyright (C) 2013 - 2015 Xilinx, Inc.  All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# Use of the Software is limited solely to applications:
# (a) running on a Xilinx device, or
# (b) that interact with a Xilinx device through a bus or interconnect.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Except as contained in this notice, the name of the Xilinx shall not be used
# in advertising or otherwise to promote the sale, use or other dealings in
# this Software without prior written authorization from Xilinx.
#
###############################################################################

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS =
LIB=libxilsecure.a

EXTRA_ARCHIVE_FLAGS=rc

RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

SECURE_DIR = .
OUTS = *.o
OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
SECURE_SRCS := $(wildcard *.c)
SECURE_OBJS = $(addprefix $(SECURE_DIR)/, $(SECURE_SRCS:%.c=%.o))

INCLUDEFILES=$(SECURE_DIR)/xsecure_sha.h \
		$(SECURE_DIR)/xsecure_aes.h \
		$(SECURE_DIR)/xsecure_rsa.h \
		$(SECURE_DIR)/xsecure_hw.h \
		$(SECURE_DIR)/xsecure_sha2.h \
		$(SECURE_DIR)/xsecure.h

libs: libxilsecure.a

libxilsecure.a: print_msg_secure $(SECURE_OBJS)
	cp $(SECURE_DIR)/$(LIB) $(RELEASEDIR)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} ${SECURE_OBJS}

print_msg_secure:
	@echo "Compiling XilSecure Library"

.PHONY: include
include: libxilsecure_includes

libxilsecure_includes:
	${CP} ${INCLUDEFILES} ${INCLUDEDIR}

clean:
	rm -rf $(SECURE_DIR)/${OBJECTS}
	rm -rf ${RELEASEDIR}/${LIB}

$(SECURE_DIR)/%.o: $(SECURE_DIR)/%.c $(INCLUDEFILES)
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) -c $< -o $@
