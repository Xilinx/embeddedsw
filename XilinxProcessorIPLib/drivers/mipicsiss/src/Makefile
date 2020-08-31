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

INCLUDEFILES:=*.h

SRCFILES:=$(wildcard *.c)
ASSEMBLY_SRCFILES:=$(wildcard *.S)


ASSEMBLY_OBJECTS  = $(addprefix $(RELEASEDIR), $(addsuffix .o, $(basename $(wildcard *.S))))
OBJECTS = $(addprefix $(RELEASEDIR), $(addsuffix .o, $(basename $(wildcard *.c))))


libs: $(OBJECTS) $(ASSEMBLY_OBJECTS)

DEPFILES := $(SRCFILES:%.c=$(RELEASEDIR)%.d)

include $(wildcard $(DEPFILES))

ASSEMBLY_DEPFILES := $(ASSEMBLY_SRCFILES:%.S=$(RELEASEDIR)%.d)

include $(wildcard $(ASSEMBLY_DEPFILES))


$(RELEASEDIR)%.o: %.c
	${COMPILER} $(CC_FLAGS) $(ECC_FLAGS) $(INCLUDES) -MMD -MP -MF"$(@:%.o=%.d)"  -MT"$(@)" $< -o $@

$(RELEASEDIR)%.o: %.S
	${COMPILER} $(CC_FLAGS) $(ECC_FLAGS) $(INCLUDES) -MMD -MP -MF"$(@:%.o=%.d)"  -MT"$(@)" $< -o $@


.PHONY: include
include: $(addprefix $(INCLUDEDIR),$(wildcard *.h))

$(INCLUDEDIR)%.h: %.h
	$(CP) $< $@


clean:
	rm -rf ${OBJECTS}
	rm -rf $(DEPFILES)
