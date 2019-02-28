DRIVER_LIB_VERSION = 1.0
COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=
LIB=libxil.a

CC_FLAGS = $(COMPILER_FLAGS)
ECC_FLAGS = $(EXTRA_COMPILER_FLAGS)

RELEASEDIR=../../../lib/
INCLUDEDIR=../../../include/
INCLUDES=-I./. -I$(INCLUDEDIR)

SRCFILES:=$(wildcard *.c)

OBJECTS = $(addprefix $(RELEASEDIR), $(addsuffix .o, $(basename $(wildcard *.c))))

libs: $(OBJECTS)

DEPFILES := $(SRCFILES:%.c=$(RELEASEDIR)%.d)

include $(wildcard $(DEPFILES))

include $(wildcard ../../../../dep.mk)

$(RELEASEDIR)%.o: %.c
	${COMPILER} $(CC_FLAGS) $(ECC_FLAGS) $(INCLUDES) $(DEPENDENCY_FLAGS) $< -o $@

.PHONY: include
include: $(addprefix $(INCLUDEDIR),$(wildcard xbasic_types.h) $(wildcard xenv.h) $(wildcard xenv_none.h) $(wildcard xenv_standalone.h) $(wildcard xenv_vxworks.h) $(wildcard xstatus.h) $(wildcard xutil.h) $(wildcard xversion.h) $(wildcard xdebug.h))

$(INCLUDEDIR)%.h: %.h
	$(CP) $< $@

clean:
	rm -rf ${OBJECTS}
	rm -rf ${DEPFILES}
