collect (PROJECT_INC_DIRS "${CMAKE_CURRENT_SOURCE_DIR}")
collect (PROJECT_LIB_DIRS "${CMAKE_CURRENT_BINARY_DIR}")
include_directories (${CMAKE_CURRENT_SOURCE_DIR})
file (GLOB C_SRCS *.c)

set (_cflags "${CMAKE_C_FLAGS}")
set_source_files_properties(${C_SRCS} PROPERTIES COMPILE_FLAGS "${_cflags}")
add_library (baremetal STATIC ${C_SRCS})
