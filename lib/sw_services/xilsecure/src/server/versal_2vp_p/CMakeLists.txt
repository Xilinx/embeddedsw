# Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

# Include headers from versal_2ve_2vm directory
set(VERSAL_2VE_2VM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../versal_2ve_2vm)

# Add include directory for versal_2ve_2vm headers
include_directories(${VERSAL_2VE_2VM_DIR})

# Collect headers from versal_2ve_2vm
collect (PROJECT_LIB_HEADERS ${VERSAL_2VE_2VM_DIR}/Ecdsa.h)
collect (PROJECT_LIB_HEADERS ${VERSAL_2VE_2VM_DIR}/Rsa.h)

# Collect local headers
collect (PROJECT_LIB_HEADERS xsecure_error.h)
collect (PROJECT_LIB_HEADERS xsecure_plat.h)
collect (PROJECT_LIB_HEADERS xsecure_plat_ipihandler.h)
collect (PROJECT_LIB_HEADERS xsecure_plat_kat.h)
collect (PROJECT_LIB_HEADERS xsecure_plat_kat_ipihandler.h)
collect (PROJECT_LIB_HEADERS xsecure_sha_hw.h)

# Collect local sources
collect (PROJECT_LIB_SOURCES xsecure_cmd.c)
collect (PROJECT_LIB_SOURCES xsecure_plat.c)
collect (PROJECT_LIB_SOURCES xsecure_plat_ipihandler.c)
collect (PROJECT_LIB_SOURCES xsecure_plat_kat.c)
collect (PROJECT_LIB_SOURCES xsecure_plat_kat_ipihandler.c)

# Handle libxilsecure_pmc.a library from versal_2ve_2vm
if (${YOCTO})
    set (STATIC_XILSECURE_LIB ${CMAKE_LIBRARY_PATH}/libxilsecure.a)
else ()
    set (STATIC_XILSECURE_LIB ${CMAKE_BINARY_DIR}/libsrc/xilsecure/src/libxilsecure.a)
endif()

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${VERSAL_2VE_2VM_DIR}/libxilsecure_pmc.a ${STATIC_XILSECURE_LIB})
endif()
