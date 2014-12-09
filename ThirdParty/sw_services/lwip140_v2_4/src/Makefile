include Makefile.lwip
include Makefile.adapter
include Makefile.config

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=

ifeq ($(CONFIG_PROCESSOR_LITTLE_ENDIAN), y)
	ENDIAN_FLAGS=-DPROCESSOR_LITTLE_ENDIAN
else
	ENDIAN_FLAGS=-DPROCESSOR_BIG_ENDIAN
endif

CC_FLAGS = $(COMPILER_FLAGS)
CC_FLAGS += $(ENDIAN_FLAGS)

RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I${INCLUDEDIR} -I$(LWIP_DIR)/src/include -I$(LWIP_DIR)/src/include/ipv4 -I$(PORT)/include

libs: liblwip4.a
	cp liblwip4.a $(RELEASEDIR)
	make clean

include:
	${CP} -r $(LWIP_DIR)/src/include/ipv4/lwip ${INCLUDEDIR}
	${CP} -r $(LWIP_DIR)/src/include/lwip ${INCLUDEDIR}
	${CP} -r $(LWIP_DIR)/src/include/netif ${INCLUDEDIR}
	${CP} -r contrib/ports/xilinx/include/netif ${INCLUDEDIR}
	${CP} -r contrib/ports/xilinx/include/arch ${INCLUDEDIR}
	${CP} -r contrib/ports/xilinx/include/lwipopts.h ${INCLUDEDIR}
	${CP} -r contrib/ports/xilinx/include/xlwipconfig.h ${INCLUDEDIR}

clean:
	rm -rf obj/*.o
	rm -rf obj
	rm -rf liblwip4.a
	echo "clean target for lwip"

liblwip4.a: obj_dir print_msg_lwip_base $(LWIP_OBJS) print_msg_lwip_adapter $(ADAPTER_OBJS)
	@echo "Creating archive $@"
	$(ARCHIVER) rc $@ obj/*.o

obj_dir:
	mkdir obj

print_msg_lwip_base:
	@echo "Compiling lwIP"

print_msg_lwip_adapter:
	@echo "Compiling lwIP adapter for Xilinx MAC"

.c.o:
	$(GCC_COMPILER) $(CC_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) -c $< -o obj/$(@F)
