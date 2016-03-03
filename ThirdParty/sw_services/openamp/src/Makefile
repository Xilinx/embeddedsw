INCLUDEDIR=../../../include
.PHONY: libs
.PHONY: include
include:
	cp -R -f open-amp/lib/include/openamp $(INCLUDEDIR)/.
libs:
	make -C ../build_openamp DESTDIR=../../../ install 2>&1
