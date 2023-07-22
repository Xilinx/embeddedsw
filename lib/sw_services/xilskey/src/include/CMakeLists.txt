# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

if(NOT ${XILSKEY_use_puf_hd_as_user_efuse})
  collect (PROJECT_LIB_HEADERS xilskey_eps_zynqmp_puf.h)
endif()

collect (PROJECT_LIB_HEADERS xilskey_eps_zynqmp.h)
collect (PROJECT_LIB_HEADERS xilskey_utils.h)

if(NOT("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "pmu_microblaze"))
  collect (PROJECT_LIB_HEADERS xilskey_bbram.h)
endif()

if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa9") OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze"))
  collect (PROJECT_LIB_HEADERS xilskey_epl.h)
  collect (PROJECT_LIB_HEADERS xilskey_eps.h)
endif()
