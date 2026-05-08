# Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS Ecdsa.h)
collect (PROJECT_LIB_HEADERS Rsa.h)
collect (PROJECT_LIB_HEADERS xsecure_error.h)
collect (PROJECT_LIB_HEADERS xsecure_plat.h)
collect (PROJECT_LIB_HEADERS xsecure_plat_ipihandler.h)
collect (PROJECT_LIB_HEADERS xsecure_plat_kat.h)
collect (PROJECT_LIB_HEADERS xsecure_plat_kat_ipihandler.h)
collect (PROJECT_LIB_HEADERS xsecure_sha_hw.h)
collect (PROJECT_LIB_HEADERS xsecure_server_aesalginfo.h)
collect (PROJECT_LIB_HEADERS xsecure_server_ellipticalginfo.h)
collect (PROJECT_LIB_HEADERS xsecure_server_rsaalginfo.h)
collect (PROJECT_LIB_HEADERS xsecure_server_shaalginfo.h)
collect (PROJECT_LIB_HEADERS xsecure_server_hmacalginfo.h)
collect (PROJECT_LIB_HEADERS xsecure_server_lmsalginfo.h)

collect (PROJECT_LIB_SOURCES xsecure_cmd.c)
collect (PROJECT_LIB_SOURCES xsecure_plat.c)
collect (PROJECT_LIB_SOURCES xsecure_plat_ipihandler.c)
collect (PROJECT_LIB_SOURCES xsecure_plat_kat.c)
collect (PROJECT_LIB_SOURCES xsecure_plat_kat_ipihandler.c)

if (${YOCTO})
    set (STATIC_XILSECURE_LIB ${CMAKE_LIBRARY_PATH}/libxilsecure.a)
else ()
    set (STATIC_XILSECURE_LIB ${CMAKE_BINARY_DIR}/libsrc/xilsecure/src/libxilsecure.a)
endif()

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/libxilsecure_pmc.a ${STATIC_XILSECURE_LIB})
endif()
