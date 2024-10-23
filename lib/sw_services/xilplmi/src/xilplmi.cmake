# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")
  set(XILPLMI_mode "server")
else()
  # For soft microblaze and  APU/RPU cores, mode is client.
  set(XILPLMI_mode "client")
endif()

option(XILPLMI_cache_disable "Enables/Disables Cache for XilPlmi client library." ON)
if(XILPLMI_mode STREQUAL "client")
  if(XILPLMI_cache_disable)
    set(XPLMI_CACHE_DISABLE " ")
  endif()
endif()

option(XILPLMI_plm_uart_dbg_en "Enables (if enabled in hardware design too) or Disables Debug prints from UART (log to memory done irrespectively" ON)
if(NOT XILPLMI_plm_uart_dbg_en)
  set(PLM_PRINT_NO_UART " ")
endif()

set(XILPLMI_plm_dbg_lvl "level1" CACHE STRING "Selects the debug logs level")
set_property(CACHE XILPLMI_plm_dbg_lvl PROPERTY STRINGS "level0" "level1" "level2" "level3")

Option(XILPLMI_sem_override_dbg_lvl "Override debug log level to 0 if Xilsem is present" ON)
if(XILPLMI_sem_override_dbg_lvl)
  set(PLM_SEM_PRINT_OVERRIDE " ")
endif()

if("${XILPLMI_plm_dbg_lvl}" STREQUAL "level0")
  set(PLM_PRINT_LEVEL0 " ")
elseif("${XILPLMI_plm_dbg_lvl}" STREQUAL "level1")
  set(PLM_DEBUG " ")
elseif("${XILPLMI_plm_dbg_lvl}" STREQUAL "level2")
  set(PLM_DEBUG_INFO " ")
elseif("${XILPLMI_plm_dbg_lvl}" STREQUAL "level3")
  set(PLM_DEBUG_DETAILED " ")
endif()

set(XILPLMI_plm_mode "release" CACHE STRING "Selects between Release and Debug modes")
set_property(CACHE XILPLMI_plm_mode PROPERTY STRINGS "debug" "release")
if("${XILPLMI_plm_mode}" STREQUAL "debug")
  set(PLM_DEBUG_MODE " ")
endif()

option(XILPLMI_plm_perf_en "Enables or Disables Boot time measurement" ON)
if (XILPLMI_plm_perf_en)
  set(PLM_PRINT_PERF " ")
endif()

option(XILPLMI_plm_qspi_en "Enables (if enabled in hardware design too) or Disables QSPI boot mode" ON)
if (NOT XILPLMI_plm_qspi_en)
  set(PLM_QSPI_EXCLUDE " ")
endif()

option(XILPLMI_plm_sd_en "Enables (if enabled in hardware design too) or Disables SD boot mode" ON)
if (NOT XILPLMI_plm_sd_en)
  set(PLM_SD_EXCLUDE " ")
endif()

option(XILPLMI_plm_ospi_en "Enables (if enabled in hardware design too) or Disables OSPI boot mode" ON)
if (NOT XILPLMI_plm_ospi_en)
  set(PLM_OSPI_EXCLUDE " ")
endif()

option(XILPLMI_plm_sem_en "Enables (if enabled in hardware design too) or Disables SEM feature" ON)
if (NOT XILPLMI_plm_sem_en)
  set(PLM_SEM_EXCLUDE " ")
endif()

option(XILPLMI_plm_secure_en "Enables or Disbales Secure features" ON)
if (NOT XILPLMI_plm_secure_en)
  set(PLM_SECURE_EXCLUDE " ")
endif()

option(XILPLMI_plm_usb_en "Enables (if enabled in hardware design too) or disables USB boot mode" OFF)
if (NOT XILPLMI_plm_usb_en)
  set(PLM_USB_EXCLUDE " ")
endif()

if("${CMAKE_MACHINE}" STREQUAL "Versal")
  option(XILPLMI_plm_nvm_en "Enables or Disables NVM handlers" OFF)
elseif("${CMAKE_MACHINE}" STREQUAL "VersalNet")
  option(XILPLMI_plm_nvm_en "Enables or Disables NVM handlers" ON)
endif()
if (NOT XILPLMI_plm_nvm_en)
  set(PLM_NVM_EXCLUDE " ")
endif()

if("${CMAKE_MACHINE}" STREQUAL "Versal")
  option(XILPLMI_plm_puf_en "Enables or Disables PUF handlers" OFF)
elseif("${CMAKE_MACHINE}" STREQUAL "VersalNet")
  option(XILPLMI_plm_puf_en "Enables or Disables PUF handlers" ON)
endif()
if (NOT XILPLMI_plm_puf_en)
  set(PLM_PUF_EXCLUDE " ")
endif()

option(XILPLMI_plm_stl_en "Enables or Disables STL" OFF)
if (XILPLMI_plm_stl_en)
  set(PLM_ENABLE_STL " ")
endif()

if ("${CMAKE_MACHINE}" STREQUAL "VersalNet")
  option(XILPLMI_plm_ocp_en "Enables or Disables OCP" ON)
  if (NOT XILPLMI_plm_ocp_en)
    set(PLM_OCP_EXCLUDE " ")
  endif()
endif()

if("${CMAKE_MACHINE}" STREQUAL "Versal")
  option(XILPLMI_ssit_plm_to_plm_comm_en "Enables or Disables SSIT PLM to PLM communication (valid only for Versal)" ON)
  option(XILPLMI_ssit_secure_plm_to_plm_comm_en "Enables or Disables SSIT secure PLM to PLM communication (valid only for Versal)" OFF)
  if (XILPLMI_ssit_plm_to_plm_comm_en)
    set(PLM_ENABLE_PLM_TO_PLM_COMM " ")
    if (XILPLMI_ssit_secure_plm_to_plm_comm_en)
    set(PLM_ENABLE_SECURE_PLM_TO_PLM_COMM " ")
  endif()
  endif()
endif()

option(XILPLMI_plm_add_ppks_en "Enables or Disables additional PPKs" OFF)
#Part list for which additional PPK support is not enabled by default
set(part_list "vc1502" "vc1702" "vc1802" "vc1902" "vc2602" "vc2802" "ve1752" "ve2202" "ve2302" "ve2602" "ve2802"
    "vh1522" "vh1542" "vh1582" "vh1742" "vh1782" "vp1102" "vp1202" "vp1402" "vp1502" "vp1552" "vp1702" "vp1802"
    "vp2502" "vp2802" "vm1102" "vm1302" "vm1402" "vm1502" "vm1802" "vm2202" "vm2302" "vm2502" "vm2902" "vn3716")

string(SUBSTRING "${DEVICE_ID}" 2 -1 PartName)
list(FIND part_list "${PartName}" index)

if (XILPLMI_plm_add_ppks_en OR (index EQUAL -1))
  set(PLM_EN_ADD_PPKS " ")
endif()

option(XILPLMI_plm_ecdsa_en "Enables or Disables ECDSA handlers" ON)
if (NOT XILPLMI_plm_ecdsa_en)
  set(PLM_ECDSA_EXCLUDE " ")
endif()

option(XILPLMI_plm_rsa_en "Enables or Disables RSA handlers" ON)
if (NOT XILPLMI_plm_rsa_en)
  set(PLM_RSA_EXCLUDE " ")
endif()

option(XILPLMI_timestamp_en "Enables or Disables Timestamp prints" ON)
if (NOT XILPLMI_timestamp_en)
  set(PLM_BANNER_TIMESTAMP_EXCLUDE " ")
endif()

option(XILPLMI_cfi_selective_read_en "Enables or disables CFI selective read" OFF)
if (XILPLMI_cfi_selective_read_en)
  set(PLM_ENABLE_CFI_SELECTIVE_READ " ")
endif()

set(XILPLMI_user_modules_count "0" CACHE STRING "Number of User Modules.")
set(XILPLMI_USER_MODULES_COUNT "${XILPLMI_user_modules_count}")

set(XILPLMI_plm_version_user_defined "0" CACHE STRING "User defined PLM version.")
set(XILPLMI_PLM_VERSION_USER_DEFINED "${XILPLMI_plm_version_user_defined}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xplmi_bsp_config.h.in ${CMAKE_BINARY_DIR}/include/xplmi_bsp_config.h)
