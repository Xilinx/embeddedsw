
set_property (GLOBAL PROPERTY "PROJECT_LIB_EXTRA_CFLAGS")

collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
collect (PROJECT_LIB_DIRS "${CMAKE_CURRENT_BINARY_DIR}")
collect (PROJECT_INC_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include")
collect (PROJECT_LIB_SOURCES version.c)


add_subdirectory (virtio)
add_subdirectory (rpmsg)
add_subdirectory (remoteproc)

if (WITH_PROXY)
  add_subdirectory (proxy)
  add_subdirectory (service/rpmsg/rpc)
endif (WITH_PROXY)

set (OPENAMP_LIB open_amp)

configure_file(version.h.in ${PROJECT_BINARY_DIR}/include/generated/openamp/version_def.h)
collect (PROJECT_INC_DIRS " ${PROJECT_BINARY_DIR}/include/generated/openamp")

if (NOT CMAKE_INSTALL_LIBDIR)
	set (CMAKE_INSTALL_LIBDIR "lib")
endif (NOT CMAKE_INSTALL_LIBDIR)

collector_list (_include PROJECT_INC_DIRS)
include_directories (${_include})

collector_list (_deps PROJECT_LIB_DEPS)

get_property (_ecflags GLOBAL PROPERTY "PROJECT_LIB_EXTRA_CFLAGS")

collector_list (_sources PROJECT_LIB_SOURCES)
set_property (SOURCE ${_sources}
  APPEND_STRING PROPERTY COMPILE_FLAGS " ${_ecflags}")

add_library (${OPENAMP_LIB} STATIC ${_sources})
set_target_properties (${OPENAMP_LIB} PROPERTIES OUTPUT_NAME  "${OPENAMP_LIB}")

install (DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/openamp" DESTINATION ${CMAKE_INCLUDE_PATH})
install (DIRECTORY "${PROJECT_BINARY_DIR}/include/generated/openamp" DESTINATION ${CMAKE_INCLUDE_PATH})

set_target_properties(${OPENAMP_LIB} PROPERTIES ADDITIONAL_CLEAN_FILES "${CMAKE_LIBRARY_PATH}/${OPENAMP_LIB}.a;${CMAKE_INCLUDE_PATH}/openamp;${clean_headers};${CMAKE_LIBRARY_PATH}/libopen_amp.a")
install(TARGETS ${OPENAMP_LIB} LIBRARY DESTINATION ${CMAKE_LIBRARY_PATH} ARCHIVE DESTINATION ${CMAKE_LIBRARY_PATH})
