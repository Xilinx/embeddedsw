INCLUDEDIR=../../../include
LIBDIR=../../../lib
.PHONY: libs
.PHONY: include
include:
	mkdir -p $(INCLUDEDIR)/metal
	cp -R -f ../build_libmetal/lib/metal/*.h $(INCLUDEDIR)/metal/.
libs:
	make -C ../build_libmetal DESTDIR=../../../ install 2>&1
clean:
	make -C ../build_libmetal DESTDIR=../../../ clean 2>&1
	rm -rf $(LIBDIR)/libmetal.a
