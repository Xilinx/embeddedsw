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

INCLUDEFILES:=$(wildcard *.h) $(wildcard ./dp/*.h) $(wildcard ./vtc/*.h) $(wildcard ./hdcp1x/*.h) $(wildcard ./dualsplitter/*.h)

SRCFILES:=$(wildcard *.c) $(notdir $(wildcard ./dp/*.c)) $(notdir $(wildcard ./vtc/*.c)) $(notdir $(wildcard ./hdcp1x/*.c)) $(notdir $(wildcard ./dualsplitter/*.c))

OBJECTS = $(addprefix $(RELEASEDIR), $(addsuffix .o, $(basename $(wildcard *.c)) $(basename $(notdir $(wildcard ./dp/*.c))) $(basename $(notdir $(wildcard ./vtc/*.c))) $(basename $(notdir $(wildcard ./hdcp1x/*.c))) $(basename $(notdir $(wildcard ./dualsplitter/*.c)))))

libs: $(OBJECTS)

DEPFILES := $(SRCFILES:%.c=$(RELEASEDIR)%.d)

include $(wildcard $(DEPFILES))

include $(wildcard ../../../../dep.mk)

$(RELEASEDIR)%.o: ./dp/%.c
	${COMPILER} $(CC_FLAGS) $(ECC_FLAGS) $(INCLUDES)  $(DEPENDENCY_FLAGS)  $< -o $@

$(RELEASEDIR)%.o: ./vtc/%.c
	${COMPILER} $(CC_FLAGS) $(ECC_FLAGS) $(INCLUDES)  $(DEPENDENCY_FLAGS)  $< -o $@

$(RELEASEDIR)%.o: ./hdcp1x/%.c
	${COMPILER} $(CC_FLAGS) $(ECC_FLAGS) $(INCLUDES)  $(DEPENDENCY_FLAGS)  $< -o $@

$(RELEASEDIR)%.o: ./dualsplitter/%.c
	${COMPILER} $(CC_FLAGS) $(ECC_FLAGS) $(INCLUDES)  $(DEPENDENCY_FLAGS)  $< -o $@

$(RELEASEDIR)%.o: %.c
	${COMPILER} $(CC_FLAGS) $(ECC_FLAGS) $(INCLUDES)  $(DEPENDENCY_FLAGS)  $< -o $@

.PHONY: include
include: $(addprefix $(INCLUDEDIR),$(wildcard *.h)) $(addprefix $(INCLUDEDIR), $(subst dp/,,$(wildcard dp/*.h))) $(addprefix $(INCLUDEDIR), $(subst vtc/,,$(wildcard vtc/*.h))) $(addprefix $(INCLUDEDIR), $(subst hdcp1x/,,$(wildcard hdcp1x/*.h))) $(addprefix $(INCLUDEDIR), $(subst dualsplitter/,,$(wildcard dualsplitter/*.h)))

$(INCLUDEDIR)%.h: dp/%.h
	$(CP) $< $@

$(INCLUDEDIR)%.h: vtc/%.h
	$(CP) $< $@

$(INCLUDEDIR)%.h: hdcp1x/%.h
	$(CP) $< $@

$(INCLUDEDIR)%.h: dualsplitter/%.h
	$(CP) $< $@

$(INCLUDEDIR)%.h: %.h
	$(CP) $< $@

clean:
	rm -rf ${OBJECTS}
	rm -rf $(DEPFILES)