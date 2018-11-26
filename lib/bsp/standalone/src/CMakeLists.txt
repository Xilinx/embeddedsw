cmake_minimum_required(VERSION 2.8.9)
project(xilstandalone)

set (CMAKE_INSTALL_LIBDIR "lib")
function (collector_create name base)
  set_property (GLOBAL PROPERTY "COLLECT_${name}_LIST")
  set_property (GLOBAL PROPERTY "COLLECT_${name}_BASE" "${base}")
endfunction (collector_create)

function (collector_list var name)
  get_property (_list GLOBAL PROPERTY "COLLECT_${name}_LIST")
  set (${var} "${_list}" PARENT_SCOPE)
endfunction (collector_list)

function (collector_base var name)
  get_property (_base GLOBAL PROPERTY "COLLECT_${name}_BASE")
  set (${var} "${_base}" PARENT_SCOPE)
endfunction (collector_base)

function (collect name)
  collector_base (_base ${name})
  string(COMPARE NOTEQUAL "${_base}" "" _is_rel)
  set (_list)
  foreach (s IN LISTS ARGN)
    if (_is_rel)
      get_filename_component (s "${s}" ABSOLUTE)
      file (RELATIVE_PATH s "${_base}" "${s}")
    endif (_is_rel)
    list (APPEND _list "${s}")
  endforeach ()
  set_property (GLOBAL APPEND PROPERTY "COLLECT_${name}_LIST" "${_list}")
endfunction (collect)

collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")

#Bring the headers into the project
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)

enable_language(C ASM)
#add_subdirectory(${CMAKE_SYSTEM_PROCESSOR})
add_subdirectory(common)
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze")
add_subdirectory(microblaze)
<<<<<<< HEAD:lib/bsp/standalone/src/CMakeLists.txt
=======
elseif (("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa53") 
	OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa72"))
add_subdirectory(ARMv8)
>>>>>>> xilmem fix for psu_pmu processor:lib/sw_services/xilmem/src/CMakeLists.txt
else()
add_subdirectory(arm)
endif()

collect (PROJECT_LIB_SOURCES outbyte.c)
collect (PROJECT_LIB_SOURCES inbyte.c)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include)

#Compile it as a static library
add_library(xilstandalone STATIC ${_sources})
set_target_properties(xilstandalone PROPERTIES LINKER_LANGUAGE C)
install(TARGETS xilstandalone LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/build ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
