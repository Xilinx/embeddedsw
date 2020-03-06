COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=-Wall -Wextra
LIB=libxil.a

CC_FLAGS = $(COMPILER_FLAGS)
ECC_FLAGS = $(EXTRA_COMPILER_FLAGS)

RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

OUTS = *.o

INCLUDEFILES=$(wildcard *.h) ./vtc/*.h ./dualsplitter/*.h ./dp/*.h ./hdcp1x/*.h
LIBSOURCES=$(wildcard *.c) ./vtc/*.c ./dualsplitter/*.c ./dp/*.c ./hdcp1x/*.c

OBJECTS = $(addsuffix .o, $(basename ./vtc/xdptxss_vtc.c ./dualsplitter/xdptxss_dualsplitter.c ./dp/xdptxss_dptx.c ./hdcp1x/xdptxss_hdcp1x.c $(wildcard *.c)))

libs: banner xdptxss_libs clean

%.o: %.c
	${COMPILER} $(CC_FLAGS) $(ECC_FLAGS) $(INCLUDES) -o $@ $<

banner:
	echo "Compiling DisplayPort Transmitter Subsystem"

xdptxss_libs: ${OBJECTS}
	$(ARCHIVER) -r ${RELEASEDIR}/${LIB} ${OBJECTS}

.PHONY: include
include: xdptxss_includes

xdptxss_includes:
	${CP} ${INCLUDEFILES} ${INCLUDEDIR}

clean:
	rm -rf ${OBJECTS}
