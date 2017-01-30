COMPILER=
ARCHIVER=
CP=cp
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=
LIB=libxil.a

RELEASEDIR=../../../lib
INCLUDEDIR=../../../include
INCLUDES=-I${INCLUDEDIR}

OUTS = *.o 
OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
LIBSOURCES=*.c
INCLUDEFILES=xbasic_types.h xenv.h xenv_none.h xenv_standalone.h xenv_vxworks.h xstatus.h xutil.h xversion.h xdebug.h

libs:
	echo "Compiling common"
	$(COMPILER) $(COMPILER_FLAGS) $(EXTRA_COMPILER_FLAGS) $(INCLUDES) $(LIBSOURCES)
	$(ARCHIVER) -r ${RELEASEDIR}/${LIB} ${OUTS} 
	make clean

include: 
	${CP} $(INCLUDEFILES) $(INCLUDEDIR)

clean:
	rm -rf $(OBJECTS)

