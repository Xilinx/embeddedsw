DRIVER_LIB_VERSION = 1.0
COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=-ffunction-sections -fdata-sections
EXTRA_COMPILER_FLAGS=-Wall -Wextra
LIB=libxil.a

CC_FLAGS = $(COMPILER_FLAGS)
ECC_FLAGS = $(EXTRA_COMPILER_FLAGS)

RELEASEDIR=../../../lib/
INCLUDEDIR=../../../include/
INCLUDES=-I./. -I$(INCLUDEDIR)

INCLUDEFILES:=$(wildcard *.h) $(wildcard ./vdma/*.h)

SRCFILES:=$(wildcard *.c) $(wildcard ./vdma/*.c)


OBJECTS = $(addprefix $(RELEASEDIR), $(addsuffix .o, $(basename ./vdma/xvprocss_vdma.c $(wildcard *.c))))


libs: $(OBJECTS)

DEPFILES := $(SRCFILES:%.c=$(RELEASEDIR)%.d)

include $(wildcard $(DEPFILES))


$(RELEASEDIR)%.o: %.c
	${COMPILER} $(CC_FLAGS) $(ECC_FLAGS) $(INCLUDES) -MMD -MP -MF"$(@:%.o=%.d)"  -MT"$(@)" $(realpath $<) -o $@

.PHONY: include
include: $(addprefix $(INCLUDEDIR),$(wildcard *.h)) $(addprefix $(INCLUDEDIR), $(subst vdma/,,$(wildcard vdma/*.h)))

$(INCLUDEDIR)%.h: vdma/%.h
	$(CP) $< $@

$(INCLUDEDIR)%.h: %.h
	$(CP) $< $@





clean:
	rm -rf ${OBJECTS}
	rm -rf $(DEPFILES)
