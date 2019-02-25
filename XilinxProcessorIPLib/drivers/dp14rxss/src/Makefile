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

INCLUDEFILES=*.h ./dp/*.h ./iic/*.h ./hdcp1x/*.h ./hdcp22/*.h
LIBSOURCES=*.c ./dp/*.c ./iic/*.c ./hdcp1x/*.c ./hdcp22/*.c

OBJECTS = $(addsuffix .o, $(basename ./dp/xdprxss_dprx.c ./iic/xdprxss_iic.c ./hdcp1x/xdprxss_hdcp1x.c ./hdcp22/xdprxss_hdcp22.c $(wildcard *.c)))

libs: banner xdprxss_libs clean

%.o: %.c
	${COMPILER} $(CC_FLAGS) $(ECC_FLAGS) $(INCLUDES) -o $@ $<

banner:
	echo "Compiling DisplayPort Receiver Subsystem"

xdprxss_libs: ${OBJECTS}
	$(ARCHIVER) -r ${RELEASEDIR}/${LIB} ${OBJECTS}

.PHONY: include
include: xdprxss_includes

xdprxss_includes:
	${CP} ${INCLUDEFILES} ${INCLUDEDIR}

clean:
	rm -rf ${OBJECTS}
