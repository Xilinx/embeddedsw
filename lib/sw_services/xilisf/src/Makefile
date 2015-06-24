###############################################################################
#
# Copyright (C) 2012 - 2015 Xilinx, Inc.  All rights reserved.
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
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=

RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I${INCLUDEDIR} 
ISF_DIR = .

LIB_SRCS = $(ISF_DIR)/xilisf.c \
          $(ISF_DIR)/xilisf_read.c \
          $(ISF_DIR)/xilisf_erase.c \
          $(ISF_DIR)/xilisf_write.c \
          $(ISF_DIR)/xilisf_spr.c 
          
# create ISF_SRCS based on configured options

ISF_SRCS = $(LIB_SRCS)

ISF_OBJS = $(ISF_SRCS:%.c=%.o)
          

EXPORT_INCLUDE_FILES = $(ISF_DIR)/include/xilisf.h \
			$(ISF_DIR)/include/xilisf_atmel.h \
			$(ISF_DIR)/include/xilisf_intelstm.h 

			
libs: libxilisf.a
	cp libxilisf.a $(RELEASEDIR)
	make clean

include: 
	@for i in $(EXPORT_INCLUDE_FILES); do \
	echo ${CP} -r $$i ${INCLUDEDIR}; \
	${CP} -r $$i ${INCLUDEDIR}; \
	done

clean:
	rm -rf obj/*.o	
	rmdir obj
	rm libxilisf.a


libxilisf.a: obj_dir print_msg_isf_base $(ISF_OBJS) 
	@echo "Creating archive $@"
	$(ARCHIVER) rc $@ obj/*.o

obj_dir:
	mkdir obj

print_msg_isf_base:
	@echo "Compiling XilIsf Library"

.c.o:
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) -c $< -o obj/$(@F)
