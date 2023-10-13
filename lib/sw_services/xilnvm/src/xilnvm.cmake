# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")
  set(XILNVM_mode "server")
elseif(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze") OR ("${CMAKE_MACHINE}" STREQUAL "VersalNet"))
  # For soft microblaze and Versal_net APU/RPU cores, mode is client.
  set(XILNVM_mode "client")
else()
  set(XILNVM_mode "client" CACHE STRING "Enables A72/R5 server and client mode support for XilNvm library for Versal")
  set_property(CACHE XILNVM_mode PROPERTY STRINGS "client" "server")
endif()

option(XILNVM_use_puf_hd_as_user_efuse "Enables API's to use PUF Helper data efuses as user efuses." OFF)
if(XILNVM_use_puf_hd_as_user_efuse)
  set(XNVM_ACCESS_PUF_USER_DATA " ")
endif()

option(XILNVM_cache_disable "Enables/Disables Cache for XilNvm client library." ON)
if(XILNVM_mode STREQUAL "client")
  if(XILNVM_cache_disable)
    set(XNVM_CACHE_DISABLE " ")
  endif()
endif()

option(XILNVM_en_add_ppks "Enables or Disables additional PPKs" OFF)
#Part list for which additional PPK support is not enabled by default
set(part_list "vc1502" "vc1702" "vc1802" "vc1902" "vc2602" "vc2802" "ve1752" "ve2202" "ve2302" "ve2602" "ve2802"
    "vh1522" "vh1542" "vh1582" "vh1742" "vh1782" "vp1102" "vp1202" "vp1402" "vp1502" "vp1552" "vp1702" "vp1802"
    "vp2502" "vp2802" "vm1102" "vm1302" "vm1402" "vm1502" "vm1802" "vm2202" "vm2302" "vm2502" "vm2902" "vn3716")
string(SUBSTRING "${DEVICE_ID}" 2 -1 PartName)
list(FIND part_list "${PartName}" index)

if (XILNVM_en_add_ppks OR (index EQUAL -1))
  set(PLM_EN_ADD_PPKS " ")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xilnvm_bsp_config.h.in ${CMAKE_BINARY_DIR}/include/xilnvm_bsp_config.h)
