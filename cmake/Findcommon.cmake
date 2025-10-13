
# Copyright (C) 2023 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

option(YOCTO "Yocto based embeddedsw FLOW" OFF)
set(CMAKE_POLICY_DEFAULT_CMP0140 OLD)

if (YOCTO)
find_package(commonmeta QUIET)
ADD_COMPILE_DEFINITIONS(-DYOCTO_FLOW)
endif()

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

function(collect_by_extension name extension)
  # Retrieve the collector's base directory
  collector_base(_base "${name}")

  string(COMPARE NOTEQUAL "${_base}" "" _has_base)
  if(NOT _has_base)
    message(FATAL_ERROR "No base directory set for collector '${name}'")
  endif()

  # Current directory where this function is called
  set(current_dir "${CMAKE_CURRENT_LIST_DIR}")

  # Collect files matching the extension in the current directory and subdirectories
  file(GLOB files CONFIGURE_DEPENDS "${current_dir}/${extension}")

  # Initialize list to store adjusted file paths
  set(_list)

  # Process each file
  foreach(s IN LISTS files)
    # Ensure we have the absolute path (GLOB_RECURSE provides absolute paths)
    get_filename_component(abs_s "${s}" ABSOLUTE)

    # Adjust the path to be relative to the collector's base directory
    file(RELATIVE_PATH rel_s "${_base}" "${abs_s}")

    # Append the adjusted path to the list
    list(APPEND _list "${rel_s}")
  endforeach()

  # Append the adjusted paths to the collector's list property
  set_property(GLOBAL APPEND PROPERTY "COLLECT_${name}_LIST" "${_list}")
endfunction()


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
    if (NOT EXISTS "${USER_LINKER_SCRIPT}")
	if ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze")
		if (DEFINED BASE_VECTOR)
			MATH(EXPR V_RESET "${BASE_VECTOR}" OUTPUT_FORMAT HEXADECIMAL)
			MATH(EXPR V_EXCEPTION "${BASE_VECTOR} + 0x8" OUTPUT_FORMAT HEXADECIMAL)
			MATH(EXPR V_INTERRUPT "${BASE_VECTOR} + 0x10" OUTPUT_FORMAT HEXADECIMAL)
			MATH(EXPR V_HWEXCEPTION "${BASE_VECTOR} + 0x20" OUTPUT_FORMAT HEXADECIMAL)
		else()
			set(V_RESET 0x0)
			set(V_EXCEPTION 0x8)
			set(V_INTERRUPT 0x10)
			set(V_HWEXCEPTION 0x20)
		endif()
	endif()
	if (NOT DEFINED STACK_SIZE)
		if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze") OR
			("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze_riscv"))
	        set(STACK_SIZE 0x400)
	    else()
	        set(STACK_SIZE 0x2000)
	    endif()
	endif()
	if (NOT DEFINED HEAP_SIZE)
		if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze") OR
			("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze_riscv"))
	        set(HEAP_SIZE 0x800)
	    else()
	        set(HEAP_SIZE 0x2000)
	    endif()
	endif()
        SET(MEM_NODE_INSTANCES "${TOTAL_MEM_CONTROLLERS}" CACHE STRING "Memory Controller")
        SET_PROPERTY(CACHE MEM_NODE_INSTANCES PROPERTY STRINGS "${TOTAL_MEM_CONTROLLERS}")
        set(CUSTOM_LINKER_FILE "None" CACHE STRING "Custom Linker Script")
        if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa72")
          OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa78")
          OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa53")
          OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "aarch64"))
            configure_file(${path}/lscript_a53.ld.in ${CMAKE_SOURCE_DIR}/lscript.ld)
            list(APPEND LINKER_FILE_PATH ${path}/lscript_a53.ld.in)
            list(APPEND LINKER_FILE lscript_a53.ld.in)
        endif()

        if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa53-32")
            configure_file(${path}/lscript_a53_32.ld.in ${CMAKE_SOURCE_DIR}/lscript.ld)
            list(APPEND LINKER_FILE_PATH ${path}/lscript_a53_32.ld.in)
            list(APPEND LINKER_FILE lscript_a53_32.ld.in)
        endif()

        if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze") OR
          ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblazeel") OR
          ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze") OR
          ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "pmu_microblaze"))
            configure_file(${path}/lscript_mb.ld.in ${CMAKE_SOURCE_DIR}/lscript.ld)
            list(APPEND LINKER_FILE_PATH ${path}/lscript_mb.ld.in)
            list(APPEND LINKER_FILE lscript_mb.ld.in)
        endif()

        if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze_riscv"))
            configure_file(${path}/lscript_mb_riscv.ld.in ${CMAKE_SOURCE_DIR}/lscript.ld)
            list(APPEND LINKER_FILE_PATH ${path}/lscript_mb_riscv.ld.in)
            list(APPEND LINKER_FILE lscript_mb_riscv.ld.in)
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
            set(linker_config_filename "lscript_r52_generic.ld.in")
            if("${CMAKE_SUBMACHINE}" STREQUAL "VersalNet")
                set(linker_config_filename "lscript_versal_net_r5.ld.in")
            endif()
            configure_file(${path}/${linker_config_filename} ${CMAKE_SOURCE_DIR}/lscript.ld)
            list(APPEND LINKER_FILE_PATH ${path}/${linker_config_filename})
            list(APPEND LINKER_FILE ${linker_config_filename})
        endif()

        if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa9"))
            configure_file(${path}/lscript_a9.ld.in ${CMAKE_SOURCE_DIR}/lscript.ld)
            list(APPEND LINKER_FILE_PATH ${path}/lscript_a9.ld.in)
            list(APPEND LINKER_FILE lscript_a9.ld.in)
        endif()

        if (NOT "${CUSTOM_LINKER_FILE}" STREQUAL "None")
            execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CUSTOM_LINKER_FILE} ${CMAKE_SOURCE_DIR}/)
        endif()
	if (NOT ${YOCTO})
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

function(add_dependency_on_bsp sources)
  if (NOT ${YOCTO})
    file(GLOB bsp_archives "${CMAKE_LIBRARY_PATH}/*.a")
    set_source_files_properties(${${sources}} OBJECT_DEPENDS "${bsp_archives}")
  endif()
endfunction()

function(split_string_by_length INPUT_STRING CHUNK_SIZE OUTPUT_LIST)
    string(LENGTH "${INPUT_STRING}" STRING_LENGTH)
    set(INDEX 0)
    set(RESULT)

    while(INDEX LESS STRING_LENGTH)
        math(EXPR NEXT_INDEX "${INDEX} + ${CHUNK_SIZE}")
        string(SUBSTRING "${INPUT_STRING}" ${INDEX} ${CHUNK_SIZE} CHUNK)
        list(APPEND RESULT "${CHUNK}")
        set(INDEX "${NEXT_INDEX}")
    endwhile()

    set(${OUTPUT_LIST} "${RESULT}" PARENT_SCOPE)
endfunction()

function(print_build_info_target target_name compiler_flags compile_defs linker_flags linker_command)
    split_string_by_length("Compiler Flags: ${compiler_flags}  ${compile_defs}" 100 cflag_print)
    split_string_by_length("Linker Flags: ${linker_flags} ${linker_command}" 100 linker_flag_print)

    add_custom_target(print_build_info ALL
        COMMAND ${CMAKE_COMMAND} -E echo ""
        COMMAND ${CMAKE_COMMAND} -E cmake_echo_color ${cflag_print}
        COMMAND ${CMAKE_COMMAND} -E echo ""
        COMMAND ${CMAKE_COMMAND} -E cmake_echo_color ${linker_flag_print}
        COMMAND ${CMAKE_COMMAND} -E echo ""
        COMMAND ${CMAKE_COMMAND} -E echo "==========================================="
        COMMENT "==========Application Build Information =========="
        VERBATIM
    )
    add_dependencies(${target_name} print_build_info)
endfunction()

macro(remove_pg)
if(${proc_extra_compiler_flags} MATCHES "^()?-pg$")
    remove_definitions(-pg)
endif()
endmacro()

string(ASCII 27 Esc)
set(ColourReset "${Esc}[m")
set(BoldRed     "${Esc}[1;31m")
set(BoldYellow  "${Esc}[1;33m")
macro(cmake_error message)
    message("${BoldRed} ERROR: ${message} ${ColourReset}")
endmacro()

macro(cmake_warning message)
    message("${BoldYellow} WARNING: ${message} ${ColourReset}")
endmacro()