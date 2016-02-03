
set_property (GLOBAL PROPERTY "PROJECT_LIB_EXTRA_CFLAGS")

collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
collect (PROJECT_LIB_DIRS "${CMAKE_CURRENT_BINARY_DIR}")
collect (PROJECT_INC_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include")
collect (PROJECT_INC_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include/openamp/machine/${PROJECT_MACHINE}")
collect (PROJECT_INC_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include/openamp/system/${PROJECT_SYSTEM}/machine/${PROJECT_MACHINE}")

add_subdirectory (common)
add_subdirectory (virtio)
add_subdirectory (rpmsg)
add_subdirectory (remoteproc)

if (WITH_PROXY)
  add_subdirectory (proxy)
endif (WITH_PROXY)
add_subdirectory (system)

set (OPENAMP_LIB open_amp)

collector_list (_list PROJECT_INC_DIRS)
include_directories (${_list})

get_property (_ecflags GLOBAL PROPERTY "PROJECT_LIB_EXTRA_CFLAGS")

collector_list (_list PROJECT_LIB_SOURCES)
set_property (SOURCE ${_list}
  APPEND_STRING PROPERTY COMPILE_FLAGS " ${_ecflags}")
add_library (${OPENAMP_LIB} STATIC ${_list})

install (TARGETS ${OPENAMP_LIB} ARCHIVE DESTINATION lib)
install (DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/openamp" DESTINATION include)

# vim: expandtab:ts=2:sw=2:smartindent
