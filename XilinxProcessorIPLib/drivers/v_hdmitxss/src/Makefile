COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=-ffunction-sections -fdata-sections
EXTRA_COMPILER_FLAGS=-Wall -Wextra
LIB=libxil.a

CC_FLAGS = $(COMPILER_FLAGS)
ECC_FLAGS = $(EXTRA_COMPILER_FLAGS)

RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

OUTS = *.o

INCLUDEFILES=*.h
LIBSOURCES=*.c

OBJECTS = $(addsuffix .o, $(basename $(wildcard *.c)))

libs: banner xvhdmitxss_libs clean

%.o: %.c
	${COMPILER} $(CC_FLAGS) $(ECC_FLAGS) $(INCLUDES) -o $@ $<

banner:
	echo "Compiling HDMI TX Subsystem"

xvhdmitxss_libs: ${OBJECTS}
	$(ARCHIVER) -r ${RELEASEDIR}/${LIB} ${OBJECTS}

.PHONY: include
include: xvhdmitxss_includes

xvhdmitxss_includes:
	${CP} ${INCLUDEFILES} ${INCLUDEDIR}

clean:
	rm -rf ${OBJECTS}
