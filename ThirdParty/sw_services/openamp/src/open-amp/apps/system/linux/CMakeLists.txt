collect (APP_COMMON_SOURCES helper.c)

if (WITH_REMOTEPROC_MASTER)
  message(FATAL_ERROR "Remoteproc master is not currently supported on ${MACHINE}/${SYSTEM}.")
else (WITH_REMOTEPROC_MASTER)
  set (_linker_script "${CMAKE_CURRENT_SOURCE_DIR}/linker_remote.ld")
endif (WITH_REMOTEPROC_MASTER)

set_property (GLOBAL PROPERTY APP_LINKER_SCRIPT "${_linker_script}")

find_library(LIBXIL_LIB NAMES xil PATHS ${CMAKE_FIND_ROOT_PATH})
get_filename_component(LIBXIL_LIB_DIR ${LIBXIL_LIB} DIRECTORY)
collect(PROJECT_LIB_DIRS ${LIBXIL_LIB_DIR})
