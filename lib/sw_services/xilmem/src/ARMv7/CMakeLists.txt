#file(GLOB SOURCES "*.c")
#file(GLOB HEADERS "*.h")\
#collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "cortexa53-32")
add_subdirectory(${ARMv8_32bit_mode})
else()
add_subdirectory(${CMAKE_SYSTEM_PROCESSOR})
endif()
#add_library(xilmem STATIC ${SOURCES})
