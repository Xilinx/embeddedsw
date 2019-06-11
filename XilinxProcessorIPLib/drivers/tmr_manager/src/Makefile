COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=
LIB=libxil.a
LEVEL=0

RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I./. -I${INCLUDEDIR}

INCLUDEFILES=*.h
LIBSOURCES=*.c *.S

OUTS = *.o
OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
ASSEMBLY_OBJECTS  = $(addsuffix .o, $(basename $(wildcard *.S)))

libs:
	echo "Compiling tmr_manager"
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) $(LIBSOURCES)
	$(ARCHIVER) -r ${RELEASEDIR}/${LIB} $(OBJECTS) $(ASSEMBLY_OBJECTS)
	make clean

include:
	${CP} $(INCLUDEFILES) $(INCLUDEDIR)

clean:
	rm -rf ${OBJECTS}
	rm -rf ${ASSEMBLY_OBJECTS}
