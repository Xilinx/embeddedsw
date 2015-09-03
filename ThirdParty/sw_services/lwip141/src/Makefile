include Makefile.lwip
include Makefile.adapter
include Makefile.config

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=
EXTRA_ARCHIVE_FLAGS=rc
LIB=liblwip4.a

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
OUTS = ./*.o
OBJ_DIR = .
LWIP_OBJ1 = $(LWIP_OBJS) $(ADAPTER_OBJS)
VPATH = $(LWIP_DIR)/src/core/ $(LWIP_DIR)/src/core/ipv4/ $(LWIP_DIR)/src/core/ipv6 \
	$(LWIP_DIR)/src/core/snmp $(LWIP_DIR)/src/netif $(LWIP_DIR)/src/api $(PORT) $(PORT)/netif
INCLUDEFILES = $(ADAPTER_INCLUDES) $(LWIP_INCLUDES)

libs: liblwip4.a

liblwip4.a: print_msg_lwip $(LWIP_OBJ1)
	$(ARCHIVER) $(EXTRA_ARCHIVE_FLAGS) ${RELEASEDIR}/${LIB} $(OUTS)

print_msg_lwip:
	@echo "Compiling lwip src and adapter files"

.PHONY: include
include: liblwip4_includes

liblwip4_includes:
	${CP} -r $(LWIP_DIR)/src/include/ipv4/lwip ${INCLUDEDIR}
	${CP} -r $(LWIP_DIR)/src/include/lwip ${INCLUDEDIR}
	${CP} -r $(LWIP_DIR)/src/include/netif ${INCLUDEDIR}
	${CP} -r contrib/ports/xilinx/include/netif ${INCLUDEDIR}
	${CP} -r contrib/ports/xilinx/include/arch ${INCLUDEDIR}
	${CP} -r contrib/ports/xilinx/include/lwipopts.h ${INCLUDEDIR}
	${CP} -r contrib/ports/xilinx/include/xlwipconfig.h ${INCLUDEDIR}

clean:
	rm -rf ${OUTS}
	rm -rf ${RELEASEDIR}/${LIB}

$(OBJ_DIR)/%.o: $(OBJ_DIR)/%.c $(INCLUDEFILES)
	$(GCC_COMPILER) $(CC_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) -c $< -o $@

