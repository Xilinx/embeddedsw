# Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

find_package(common)
add_subdirectory(common)
if("${XILSECURE_mode}" STREQUAL "client")
  if (NOT ("xilmailbox" IN_LIST BSP_LIBSRC_SUBDIRS))
   cmake_warning("XilSecure library requires xilmailbox library in client mode.")
  endif()
  add_subdirectory(client)
else()
  add_subdirectory(server)
endif()
