INCLUDEDIR=../../../include
LIBDIR=../../../lib
OBJS= *.obj
LIBSTANDALONE= $(LIBDIR)/libxilstandalone.a
LIBXIL= ${LIBDIR}/libxil.a

.PHONY: libs include clean


include:
	cp -f ../build_xilstandalone/include/* $(INCLUDEDIR)/.
libs:
	make -C ../build_xilstandalone DESTDIR=../../../ install 2>&1
	$(ARCHIVER) -xv $(LIBSTANDALONE)
	$(ARCHIVER) -r $(LIBXIL) $(OBJS)
	rm -rf $(OBJS)

clean:
	make -C ../build_xilstandalone DESTDIR=../../../ clean 2>&1
	rm -rf $(LIBSTANDALONE)
