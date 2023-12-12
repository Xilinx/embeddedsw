collect (PROJECT_LIB_SOURCES sys.c)

check_include_files(xintc.h HAS_XINTC)
if (HAS_XINTC)
  add_definitions(-DHAS_XINTC)
endif(HAS_XINTC)
