COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=
# LIB=libxil.a
LIB=libxil.a
LEVEL=0

RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I${INCLUDEDIR}

OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
LIBSOURCES=$(wildcard *.c)
INCLUDEFILES=$(wildcard *.h)

libs:
	echo "Compiling hwicap"
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) $(LIBSOURCES)
	$(ARCHIVER) -r ${RELEASEDIR}/${LIB} $(OBJECTS)
#	make clean

include:
	${CP} $(INCLUDEFILES) $(INCLUDEDIR)

clean:
	rm -rf $(OBJECTS)

