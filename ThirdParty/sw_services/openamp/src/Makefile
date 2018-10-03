INCLUDEDIR=../../../include
LIBDIR=../../../lib

.PHONY: libs include clean

include:
	cp -R -f open-amp/lib/include/openamp $(INCLUDEDIR)/.
libs:
	make -C ../build_openamp DESTDIR=../../../ install 2>&1
clean:
	make -C ../build_openamp DESTDIR=../../../ clean 2>&1
	rm -rf $(LIBDIR)/libopen_amp.a
