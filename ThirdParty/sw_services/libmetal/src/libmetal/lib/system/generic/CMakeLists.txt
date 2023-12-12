collect (PROJECT_LIB_HEADERS alloc.h)
collect (PROJECT_LIB_HEADERS assert.h)
collect (PROJECT_LIB_HEADERS cache.h)
collect (PROJECT_LIB_HEADERS condition.h)
collect (PROJECT_LIB_HEADERS io.h)
collect (PROJECT_LIB_HEADERS irq.h)
collect (PROJECT_LIB_HEADERS log.h)
collect (PROJECT_LIB_HEADERS mutex.h)
collect (PROJECT_LIB_HEADERS sleep.h)
collect (PROJECT_LIB_HEADERS sys.h)

collect (PROJECT_LIB_SOURCES condition.c)
collect (PROJECT_LIB_SOURCES device.c)
collect (PROJECT_LIB_SOURCES init.c)
collect (PROJECT_LIB_SOURCES io.c)
collect (PROJECT_LIB_SOURCES irq.c)
collect (PROJECT_LIB_SOURCES shmem.c)
collect (PROJECT_LIB_SOURCES time.c)

if ("${PROJECT_MACHINE}" STREQUAL "zynqmp_a53" OR
    "${PROJECT_MACHINE}" STREQUAL "zynqmp_a72" OR
    "${PROJECT_MACHINE}" STREQUAL "zynqmp_a78" OR
    "${PROJECT_MACHINE}" STREQUAL "zynqmp_r5"  OR
    "${PROJECT_MACHINE}" STREQUAL "microblaze_generic"  OR
    "${PROJECT_MACHINE}" STREQUAL "zynq7")
  add_subdirectory(xlnx)
elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_MACHINE})
  add_subdirectory(${PROJECT_MACHINE})
endif()

