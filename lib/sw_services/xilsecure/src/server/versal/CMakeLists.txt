# Copyright (C) 2023 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS Ecdsa.h)
collect (PROJECT_LIB_HEADERS xsecure_error.h)
collect (PROJECT_LIB_HEADERS xsecure_plat.h)
collect (PROJECT_LIB_SOURCES xsecure_plat.c)
collect (PROJECT_LIB_HEADERS xsecure_sha_hw.h)
collect (PROJECT_LIB_SOURCES xsecure_plat_kat.c)
collect (PROJECT_LIB_HEADERS xsecure_plat_kat.h)
if( NOT (
		("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa72") OR
		("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr5")
	)
  )
  collect (PROJECT_LIB_SOURCES xsecure_plat_ipihandler.c)
  collect (PROJECT_LIB_HEADERS xsecure_plat_ipihandler.h)
  collect (PROJECT_LIB_SOURCES xsecure_cmd.c)
endif()

if (${YOCTO})
    set (STATIC_XILSECURE_LIB ${CMAKE_LIBRARY_PATH}/libxilsecure.a)
else ()
    set (STATIC_XILSECURE_LIB ${CMAKE_BINARY_DIR}/libsrc/xilsecure/src/libxilsecure.a)
endif()

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/libxilsecure_pmc.a ${STATIC_XILSECURE_LIB})
endif()

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa72")
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/libxilsecure_a72_64.a ${STATIC_XILSECURE_LIB})
endif()

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr5")
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/libxilsecure_r5.a ${STATIC_XILSECURE_LIB})
endif()
