#include Makefile.config

COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=-O2 -c -g

CC_FLAGS = $(COMPILER_FLAGS)

RELEASEDIR=../../../lib
INCLUDEDIR=../../../include

CLI_DIR = ./Source
INCLUDES=-I${INCLUDEDIR} -I${CLI_DIR}

libs: libfreertos_cli.a
	cp libfreertos_cli.a $(RELEASEDIR)
	make clean

include:
	${CP} -r $(CLI_DIR)/FreeRTOS_CLI.h ${INCLUDEDIR}

clean:
	rm -rf obj/*.o
	rm -rf obj
	rm -rf libfreertos_cli.a
	echo "clean target for freecli"

CLI_SRCS = ${CLI_DIR}/FreeRTOS_CLI.c
CLI_OBJS = $(CLI_SRCS:%.c=%.o)

libfreertos_cli.a: obj_dir print_msg_cli_base $(CLI_OBJS) $(ADAPTER_OBJS)
	@echo "Creating archive $@"
	$(ARCHIVER) -r $@ obj/*.o

obj_dir:
	mkdir obj

print_msg_cli_base:
	@echo "Compiling FreeRTOS CLI"

.c.o:
	$(COMPILER) $(CC_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) -c $< -o obj/$(@F)
	
