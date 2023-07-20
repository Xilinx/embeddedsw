# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS Ecdsa.h)
collect (PROJECT_LIB_HEADERS xsecure_ellipticplat.h)
collect (PROJECT_LIB_HEADERS xsecure_error.h)
collect (PROJECT_LIB_HEADERS xsecure_hmac.h)
collect (PROJECT_LIB_HEADERS xsecure_plat.h)
collect (PROJECT_LIB_HEADERS xsecure_plat_kat.h)
collect (PROJECT_LIB_HEADERS xsecure_sha_hw.h)
collect (PROJECT_LIB_HEADERS xsecure_trng.h)
collect (PROJECT_LIB_HEADERS xsecure_trng_hw.h)
collect (PROJECT_LIB_HEADERS SoftSHA.h)

collect (PROJECT_LIB_SOURCES xsecure_cmd.c)
collect (PROJECT_LIB_SOURCES xsecure_ellipticplat.c)
collect (PROJECT_LIB_SOURCES xsecure_hmac.c)
collect (PROJECT_LIB_SOURCES xsecure_plat.c)
collect (PROJECT_LIB_SOURCES xsecure_plat_kat.c)
collect (PROJECT_LIB_SOURCES xsecure_trng.c)

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")
  collect (PROJECT_LIB_HEADERS xsecure_trng_ipihandler.h)
  collect (PROJECT_LIB_SOURCES xsecure_trng_ipihandler.c)
  collect (PROJECT_LIB_HEADERS xsecure_plat_kat_ipihandler.h)
  collect (PROJECT_LIB_SOURCES xsecure_plat_kat_ipihandler.c)
  collect (PROJECT_LIB_HEADERS xsecure_plat_ipihandler.h)
  collect (PROJECT_LIB_SOURCES xsecure_plat_ipihandler.c)
  collect (PROJECT_LIB_HEADERS xsecure_sha384.h)
  collect (PROJECT_LIB_SOURCES xsecure_sha384.c)
endif()

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/libxilsecure_pmc.a ${CMAKE_LIBRARY_PATH}/libxilsecure_plat.a)
endif()

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa78")
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/libxilsecure_a78_64.a ${CMAKE_LIBRARY_PATH}/libxilsecure_plat.a)
endif()

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr52")
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/libxilsecure_r52.a ${CMAKE_LIBRARY_PATH}/libxilsecure_plat.a)
endif()