# Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS Ecdsa.h)
collect (PROJECT_LIB_HEADERS Rsa.h)
collect (PROJECT_LIB_HEADERS xsecure_error.h)
collect (PROJECT_LIB_HEADERS xsecure_mgf.h)
collect (PROJECT_LIB_HEADERS xsecure_plat.h)
collect (PROJECT_LIB_HEADERS xsecure_plat_ipihandler.h)
collect (PROJECT_LIB_HEADERS xsecure_plat_kat.h)
collect (PROJECT_LIB_HEADERS xsecure_plat_kat_ipihandler.h)
collect (PROJECT_LIB_HEADERS xsecure_sha_hw.h)

collect (PROJECT_LIB_SOURCES xsecure_cmd.c)
collect (PROJECT_LIB_SOURCES xsecure_mgf.c)
collect (PROJECT_LIB_SOURCES xsecure_plat.c)
collect (PROJECT_LIB_SOURCES xsecure_plat_ipihandler.c)
collect (PROJECT_LIB_SOURCES xsecure_plat_kat.c)
collect (PROJECT_LIB_SOURCES xsecure_plat_kat_ipihandler.c)

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/libxilsecure_pmc.a ${CMAKE_LIBRARY_PATH}/libxilsecure_plat.a)
endif()
