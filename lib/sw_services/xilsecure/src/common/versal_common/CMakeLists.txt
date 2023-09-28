# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

add_subdirectory(common)
if("${XILSECURE_mode}" STREQUAL "client")
  if (NOT ("xilmailbox" IN_LIST BSP_LIBSRC_SUBDIRS))
    message(WARNING "XilSecure library requires xilmailbox library in client mode.")
  endif()
  add_subdirectory(client)
else()
  add_subdirectory(server)
endif()
