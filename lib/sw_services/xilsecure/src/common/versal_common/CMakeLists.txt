# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

add_subdirectory(common)
if(("${XILSECURE_mode}" STREQUAL "client") AND (NOT "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze"))
  if (NOT ("xilmailbox" IN_LIST BSP_LIBSRC_SUBDIRS))
    message(FATAL_ERROR "This library requires xilmailbox library in the Board Support Package.")
  endif()
  add_subdirectory(client)
else()
  add_subdirectory(server)
endif()
