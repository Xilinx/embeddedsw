# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze")
  set(XPAR_XSK_MICROBLAZE_PLATFORM " ")

  # ADD the other conditions, once the cmake variables are defined for kintex and virtex
  if("${CMAKE_MACHINE}" STREQUAL "ZynqMP")
    set(XPAR_XSK_MICROBLAZE_ULTRA_PLUS " ")
  endif()
  # set the below macro, once the cmake variables are defined for kintex and virtex
  # set(XPAR_XSK_MICROBLAZE_ULTRA " ")
else()
  set(XPAR_XSK_ARM_PLATFORM " ")
endif()

option(XILSKEY_override_sysmon_cfg "Override Sysmon configuration" ON)
if(XILSKEY_override_sysmon_cfg)
  set(XSK_OVERRIDE_SYSMON_CFG " ")
endif()

if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "pmu_microblaze") OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa53") OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr5"))
  option(XILSKEY_access_secure_crit_efuse "Enables writing and reading of security critical efuses for ZynqMp" OFF)
  if(XILSKEY_access_secure_crit_efuse)
    set(XSK_ACCESS_SECURE_CRITICAL_EFUSE " ")
  endif()

  option(XILSKEY_access_user_efuse "Enables writing and reading of user efuses for ZynqMp" OFF)
  if(XILSKEY_access_user_efuse)
    set(XSK_ACCESS_USER_EFUSE " ")
  endif()

  option(XILSKEY_access_key_manage_efuse "Enables writing and reading of key management efuses for ZynqMp" OFF)
  if(XILSKEY_access_key_manage_efuse)
    set(XSK_ACCESS_KEY_MANAGE_EFUSE " ")
  endif()

  option(XILSKEY_use_puf_hd_as_user_efuse "Enables API's to use PUF Helper Data efuses as user efuses" OFF)
  if(XILSKEY_use_puf_hd_as_user_efuse)
    set(XSK_ACCESS_PUF_USER_EFUSE " ")
  endif()
endif()

SET(XILSKEY_device_series "XSK_FPGA_SERIES_ZYNQ" CACHE STRING "Device series; FPGA SERIES ZYNQ: XSK_FPGA_SERIES_ZYNQ; FPGA SERIES ULTRA: XSK_FPGA_SERIES_ULTRA; FPGA SERIES ULTRA PLUS: XSK_FPGA_SERIES_ULTRA_PLUS")

SET_PROPERTY(CACHE XILSKEY_device_series PROPERTY STRINGS "XSK_FPGA_SERIES_ZYNQ" "XSK_FPGA_SERIES_ULTRA" "XSK_FPGA_SERIES_ULTRA_PLUS")

SET(XILSKEY_device_id "0" CACHE STRING "IDCODE")

SET(XILSKEY_device_irlen "0" CACHE STRING "IR length")

SET(XILSKEY_device_numslr "1" CACHE STRING "Number of SLRs")

SET(XILSKEY_device_masterslr "0" CACHE STRING "Master SLR number")

if(NOT ("${XILSKEY_device_id}" STREQUAL "0"))
  if((NOT ${XILSKEY_device_id} MATCHES "0x0ba00477") AND
     (NOT ${XILSKEY_device_id} MATCHES "0x03822093") AND
     (NOT ${XILSKEY_device_id} MATCHES "0x03931093") AND
     (NOT ${XILSKEY_device_id} MATCHES "0x03842093") AND
     (NOT ${XILSKEY_device_id} MATCHES "0x04A62093") AND
     (NOT ${XILSKEY_device_id} MATCHES "0x04B31093") AND
     (NOT ${XILSKEY_device_id} MATCHES "0x04B51093") AND
     (NOT ${XILSKEY_device_id} MATCHES "0x0484A093"))
    set(XSK_USER_DEVICE_SERIES "${XILSKEY_device_series}")
    set(XSK_USER_DEVICE_ID "${XILSKEY_device_id}")
    set(XSK_USER_DEVICE_IRLEN "${XILSKEY_device_irlen}")
    set(XSK_USER_DEVICE_NUMSLR "${XILSKEY_device_numslr}")
    set(XSK_USER_DEVICE_MASTER_SLR "${XILSKEY_device_masterslr}")
  else()
    message("ERROR: Device IDCODE already exist by Default.")
    set(XSK_USER_DEVICE_SERIES "0")
    set(XSK_USER_DEVICE_ID "0")
    set(XSK_USER_DEVICE_IRLEN "0")
    set(XSK_USER_DEVICE_NUMSLR "0")
    set(XSK_USER_DEVICE_MASTER_SLR "0")
  endif()
else()
  message("STATUS: No Value specified, assigning default values.")
  set(XSK_USER_DEVICE_SERIES "${XILSKEY_device_series}")
  set(XSK_USER_DEVICE_ID "${XILSKEY_device_id}")
  set(XSK_USER_DEVICE_IRLEN "${XILSKEY_device_irlen}")
  set(XSK_USER_DEVICE_NUMSLR "${XILSKEY_device_numslr}")
  set(XSK_USER_DEVICE_MASTER_SLR "${XILSKEY_device_masterslr}")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xilskey_bsp_config.h.in ${CMAKE_BINARY_DIR}/include/xilskey_bsp_config.h)
