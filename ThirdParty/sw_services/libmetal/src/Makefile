INCLUDEDIR=../../../include
LIBDIR=../../../lib

.PHONY: libs include clean

include:
	cp -R -f ../build_libmetal/lib/include/metal $(INCLUDEDIR)/.
libs:
	make -C ../build_libmetal DESTDIR=../../../ install 2>&1
clean:
	make -C ../build_libmetal DESTDIR=../../../ clean 2>&1
	rm -rf $(LIBDIR)/libmetal.a
