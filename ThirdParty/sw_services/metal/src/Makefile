INCLUDEDIR=../../../include
.PHONY: libs
.PHONY: include
include:
	mkdir -p $(INCLUDEDIR)/metal
	cp -R -f ../build_metal/lib/metal/*.h $(INCLUDEDIR)/metal/.
libs:
	make -C ../build_metal DESTDIR=../../../ install 2>&1
