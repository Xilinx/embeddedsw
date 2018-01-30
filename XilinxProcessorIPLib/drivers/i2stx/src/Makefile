COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=-ffunction-sections -fdata-sections
EXTRA_COMPILER_FLAGS=-Wall -Wextra
LIB=libxil.a

RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
INCLUDEFILES=*.h

LIBSOURCES=*.c
OUTS = *.o


libs:
	echo "Compiling I2S Transmitter"
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) $(LIBSOURCES)
	$(ARCHIVER) -r ${RELEASEDIR}/${LIB} $(OUTS)
	make clean

include:
	 ${CP} ${INCLUDEFILES} ${INCLUDEDIR}

clean:
	rm -rf $(OBJECTS)
