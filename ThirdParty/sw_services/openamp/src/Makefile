INCLUDEDIR=../../../include
.PHONY: libs
.PHONY: include
include:
	rsync -a open-amp/lib/include/openamp/ $(INCLUDEDIR)/openamp/
libs:
	make -C ../build_openamp DESTDIR=../../../ install 2>&1
