DRIVER_LIB_VERSION = 1.0
COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=-Wall -Wextra
LIB=libxil.a

CC_FLAGS = $(COMPILER_FLAGS)
ECC_FLAGS = $(EXTRA_COMPILER_FLAGS)

RELEASEDIR=../../../lib/
INCLUDEDIR=../../../include/
INCLUDES=-I./. -I$(INCLUDEDIR)

INCLUDEFILES:=$(wildcard *.h) $(wildcard ./dp/*.h) $(wildcard ./iic/*.h) $(wildcard ./hdcp1x/*.h) $(wildcard ./hdcp22/*.h)

SRCFILES:=$(wildcard *.c) $(notdir $(wildcard ./dp/*.c)) $(notdir $(wildcard ./iic/*.c)) $(notdir $(wildcard ./hdcp1x/*.c)) $(notdir $(wildcard ./hdcp22/*.c))

OBJECTS = $(addprefix $(RELEASEDIR), $(addsuffix .o, $(basename $(wildcard *.c)) $(basename $(notdir $(wildcard ./dp/*.c))) $(basename $(notdir $(wildcard ./iic/*.c))) $(basename $(notdir $(wildcard ./hdcp1x/*.c))) $(basename $(notdir $(wildcard ./hdcp22/*.c)))))

libs: $(OBJECTS)

DEPFILES := $(SRCFILES:%.c=$(RELEASEDIR)%.d)

include $(wildcard $(DEPFILES))

include $(wildcard ../../../../dep.mk)

$(RELEASEDIR)%.o: ./dp/%.c
	${COMPILER} $(CC_FLAGS) $(ECC_FLAGS) $(INCLUDES)  $(DEPENDENCY_FLAGS)  $< -o $@

$(RELEASEDIR)%.o: ./iic/%.c
	${COMPILER} $(CC_FLAGS) $(ECC_FLAGS) $(INCLUDES)  $(DEPENDENCY_FLAGS)  $< -o $@

$(RELEASEDIR)%.o: ./hdcp1x/%.c
	${COMPILER} $(CC_FLAGS) $(ECC_FLAGS) $(INCLUDES)  $(DEPENDENCY_FLAGS)  $< -o $@

$(RELEASEDIR)%.o: ./hdcp22/%.c
	${COMPILER} $(CC_FLAGS) $(ECC_FLAGS) $(INCLUDES)  $(DEPENDENCY_FLAGS)  $< -o $@

$(RELEASEDIR)%.o: %.c
	${COMPILER} $(CC_FLAGS) $(ECC_FLAGS) $(INCLUDES)  $(DEPENDENCY_FLAGS)  $< -o $@

.PHONY: include
include: $(addprefix $(INCLUDEDIR),$(wildcard *.h)) $(addprefix $(INCLUDEDIR), $(subst dp/,,$(wildcard dp/*.h))) $(addprefix $(INCLUDEDIR), $(subst iic/,,$(wildcard iic/*.h))) $(addprefix $(INCLUDEDIR), $(subst hdcp1x/,,$(wildcard hdcp1x/*.h))) $(addprefix $(INCLUDEDIR), $(subst hdcp22/,,$(wildcard hdcp22/*.h)))

$(INCLUDEDIR)%.h: dp/%.h
	$(CP) $< $@

$(INCLUDEDIR)%.h: iic/%.h
	$(CP) $< $@

$(INCLUDEDIR)%.h: hdcp1x/%.h
	$(CP) $< $@

$(INCLUDEDIR)%.h: hdcp22/%.h
	$(CP) $< $@

$(INCLUDEDIR)%.h: %.h
	$(CP) $< $@

clean:
	rm -rf ${OBJECTS}
	rm -rf $(DEPFILES)