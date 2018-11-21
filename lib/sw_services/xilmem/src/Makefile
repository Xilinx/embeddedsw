INCLUDEDIR=../../../include
LIBDIR=../../../lib

.PHONY: libs include clean


include:
	cp -f ../build_xilmem/include/* $(INCLUDEDIR)/.
libs:
	make -C ../build_xilmem DESTDIR=../../../ install 2>&1

clean:
	make -C ../build_xilmem DESTDIR=../../../ clean 2>&1
	rm -rf $(LIBDIR)/xilmem.a
