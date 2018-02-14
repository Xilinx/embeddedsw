collect (PROJECT_LIB_HEADERS alloc.h)
collect (PROJECT_LIB_HEADERS condition.h)
collect (PROJECT_LIB_HEADERS io.h)
collect (PROJECT_LIB_HEADERS irq.h)
collect (PROJECT_LIB_HEADERS mutex.h)
collect (PROJECT_LIB_HEADERS sys.h)

collect (PROJECT_LIB_SOURCES alloc.c)
collect (PROJECT_LIB_SOURCES cache.c)
collect (PROJECT_LIB_SOURCES condition.c)
collect (PROJECT_LIB_SOURCES device.c)
collect (PROJECT_LIB_SOURCES init.c)
collect (PROJECT_LIB_SOURCES io.c)
collect (PROJECT_LIB_SOURCES irq.c)
collect (PROJECT_LIB_SOURCES shmem.c)
collect (PROJECT_LIB_SOURCES sleep.c)
collect (PROJECT_LIB_SOURCES time.c)

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_MACHINE})
  add_subdirectory(${PROJECT_MACHINE})
endif (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_MACHINE})

# vim: expandtab:ts=2:sw=2:smartindent
