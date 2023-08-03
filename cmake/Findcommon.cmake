# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

option(NON_YOCTO "Non Yocto embeddedsw FLOW" OFF)
set(CMAKE_POLICY_DEFAULT_CMP0140 OLD)

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

function(LIST_INDEX index match PROP)
    foreach (prop ${PROP})
        if ("${prop}" STREQUAL "${match}")
            set(index ${index} PARENT_SCOPE)
            return()
        endif()
        MATH(EXPR index "${index}+1")
    endforeach()
endfunction()

function(get_headers headers)
    foreach(header ${_headers})
	string(REPLACE "/" ";" PATH_LIST ${header})
	list(GET PATH_LIST -1 header)
	list(APPEND clean_headers ${CMAKE_INCLUDE_PATH}/${header})
	set(clean_headers ${clean_headers} PARENT_SCOPE)
    endforeach()
    return(${clean_headers})
endfunction()

macro(subdirlist result curdir)
    file(GLOB subdirs RELATIVE ${curdir} ${curdir}/*)
    set(dirlist "")
    foreach(subdir ${subdirs})
	if(IS_DIRECTORY ${curdir}/${subdir})
	    list(APPEND dirlist ${subdir})
	endif()
    endforeach()
    set(${result} ${dirlist})
endmacro()

function (linker_gen path)
    if (NOT EXISTS "${CMAKE_SOURCE_DIR}/lscript.ld")
	if (NOT DEFINED STACK_SIZE)
            if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze")
	        set(STACK_SIZE 0x400)
	    else()
	        set(STACK_SIZE 0x2000)
	    endif()
	endif()
	if (NOT DEFINED HEAP_SIZE)
            if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze")
	        set(HEAP_SIZE 0x400)
	    else()
	        set(HEAP_SIZE 0x2000)
	    endif()
	endif()
        SET(MEM_NODE_INSTANCES "${TOTAL_MEM_CONTROLLERS}" CACHE STRING "Memory Controller")
        SET_PROPERTY(CACHE MEM_NODE_INSTANCES PROPERTY STRINGS "${TOTAL_MEM_CONTROLLERS}")
        set(CUSTOM_LINKER_FILE "None" CACHE STRING "Custom Linker Script")
        list(LENGTH MEM_NODE_INSTANCES _len)
        if (${_len} EQUAL 1)
            set(DDR ${MEM_NODE_INSTANCES})
        endif()
        if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa72")
          OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa78")
          OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa53")
          OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa53-32")
          OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "aarch64"))
            configure_file(${path}/lscript_a53.ld.in ${CMAKE_SOURCE_DIR}/lscript.ld)
            list(APPEND LINKER_FILE_PATH ${path}/lscript_a53.ld.in)
            list(APPEND LINKER_FILE lscript_a53.ld.in)
        endif()

        if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze") OR
          ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblazeel") OR
          ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze") OR
          ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "pmu_microblaze"))
            configure_file(${path}/lscript_mb.ld.in ${CMAKE_SOURCE_DIR}/lscript.ld)
            list(APPEND LINKER_FILE_PATH ${path}/lscript_mb.ld.in)
            list(APPEND LINKER_FILE lscript_mb.ld.in)
        endif()

        if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr5"))
            if("${CMAKE_MACHINE}" STREQUAL "Versal")
                configure_file(${path}/lscript_versal_r5.ld.in ${CMAKE_SOURCE_DIR}/lscript.ld)
                list(APPEND LINKER_FILE_PATH ${path}/lscript_versal_r5.ld.in)
                list(APPEND LINKER_FILE lscript_versal_r5.ld.in)
            else()
                configure_file(${path}/lscript_r5.ld.in ${CMAKE_SOURCE_DIR}/lscript.ld)
                list(APPEND LINKER_FILE_PATH ${path}/lscript_r5.ld.in)
                list(APPEND LINKER_FILE lscript_r5.ld.in)
            endif()
        endif()

        if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr52"))
            configure_file(${path}/lscript_versal_net_r5.ld.in ${CMAKE_SOURCE_DIR}/lscript.ld)
            list(APPEND LINKER_FILE_PATH ${path}/lscript_versal_net_r5.ld.in)
            list(APPEND LINKER_FILE lscript_versal_net_r5.ld.in)
        endif()

        if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa9"))
            configure_file(${path}/lscript_a9.ld.in ${CMAKE_SOURCE_DIR}/lscript.ld)
            list(APPEND LINKER_FILE_PATH ${path}/lscript_a9.ld.in)
            list(APPEND LINKER_FILE lscript_a9.ld.in)
        endif()

        if (NOT "${CUSTOM_LINKER_FILE}" STREQUAL "None")
            execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CUSTOM_LINKER_FILE} ${CMAKE_SOURCE_DIR}/)
        endif()
        if (${NON_YOCTO})
            file (REMOVE_RECURSE ${path})
        endif()
    endif()
endfunction(linker_gen)

function(gen_exheader path drvname addr prefix)
    string(TOUPPER ${drvname} DRVNAME)
    set(ADDR_DEFINE "#define X${DRVNAME}_BASEADDRESS ${addr}U")

    if (NOT ${prefix})
        set(DRVNAME X${DRVNAME})
        configure_file(${path}/example.h.in ${CMAKE_SOURCE_DIR}/${prefix}${drvname}_example.h)
    else()
        configure_file(${path}/example.h.in ${CMAKE_SOURCE_DIR}/${drvname}_example.h)
    endif()
endfunction(gen_exheader)

function(gen_bspconfig)
    execute_process(COMMAND lopper $ENV{SYSTEM_DTFILE} -- baremetal_bspconfig_xlnx ${ESW_MACHINE} ${CMAKE_SOURCE_DIR}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/common
  RESULT_VARIABLE output)
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/common/MemConfig.cmake ${CMAKE_SOURCE_DIR}/MemConfig.cmake)
endfunction(gen_bspconfig)

function(get_drvlist)
    execute_process(COMMAND lopper $ENV{SYSTEM_DTFILE} -- baremetaldrvlist_xlnx ${ESW_MACHINE} ${CMAKE_SOURCE_DIR}/../../
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  RESULT_VARIABLE output)
endfunction(get_drvlist)

function(gen_drvconfig drvname)
	if ("${drvname}" STREQUAL "uartps" OR
	    "${drvname}" STREQUAL "uartlite" OR
	    "${drvname}" STREQUAL "uartpsv")
    execute_process(COMMAND lopper $ENV{SYSTEM_DTFILE} -- baremetalconfig_xlnx ${ESW_MACHINE} ${CMAKE_SOURCE_DIR}/${drvname}/src/ stdin
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/${drvname}/src/
  RESULT_VARIABLE output)
    else()
        execute_process(COMMAND lopper $ENV{SYSTEM_DTFILE} -- baremetalconfig_xlnx ${ESW_MACHINE} ${CMAKE_SOURCE_DIR}/${drvname}/src/
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/${drvname}/src/
  RESULT_VARIABLE output)
    endif()
endfunction(gen_drvconfig)

function(gen_xparams)
  execute_process(COMMAND lopper $ENV{SYSTEM_DTFILE} -- baremetal_xparameters_xlnx ${ESW_MACHINE} ${CMAKE_SOURCE_DIR}/../../
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  RESULT_VARIABLE output)
endfunction(gen_xparams)

function(gen_linker)
  execute_process(COMMAND lopper $ENV{SYSTEM_DTFILE} -- baremetallinker_xlnx ${ESW_MACHINE} ${CMAKE_SOURCE_DIR}/
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  RESULT_VARIABLE output)
endfunction(gen_linker)

function(gen_libconfig)
  execute_process(COMMAND lopper $ENV{SYSTEM_DTFILE} -- bmcmake_metadata_xlnx.py ${ESW_MACHINE} ${CMAKE_SOURCE_DIR}/ hwcmake_metadata ${CMAKE_SOURCE_DIR}/../../../../
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  RESULT_VARIABLE output)
endfunction(gen_libconfig)

macro(print_elf_size size_command app_name)
  if(DEFINED ${size_command})
    add_custom_command(
      TARGET ${app_name}.elf POST_BUILD
      COMMAND ${${size_command}} --format=berkeley ${app_name}.elf
      COMMAND ${${size_command}} --format=berkeley ${app_name}.elf > ${CMAKE_BINARY_DIR}/${app_name}.elf.size
      VERBATIM)
  endif()
endmacro()

function(find_project_type src_ext PROJECT_TYPE)
  set(cpp_extensions ".cpp;.cc;.CPP;.c++;.cxx")
  foreach(extension ${cpp_extensions})
    if (${extension} IN_LIST src_ext)
      set(PROJECT_TYPE "c++" PARENT_SCOPE)
      return()
    endif()
  endforeach()
  set(PROJECT_TYPE "c" PARENT_SCOPE)
endfunction()
