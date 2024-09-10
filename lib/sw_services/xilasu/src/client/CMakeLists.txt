# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

if (NOT ("xilmailbox" IN_LIST BSP_LIBSRC_SUBDIRS))
  message(WARNING "XilAsu library requires XilMailbox library in client mode.")
endif()

collect (PROJECT_LIB_SOURCES xasu_aes.c)
collect (PROJECT_LIB_HEADERS xasu_aes.h)
collect (PROJECT_LIB_SOURCES xasu_client.c)
collect (PROJECT_LIB_HEADERS xasu_client.h)
collect (PROJECT_LIB_SOURCES xasu_ecc.c)
collect (PROJECT_LIB_HEADERS xasu_ecc.h)
collect (PROJECT_LIB_SOURCES xasu_rsa.c)
collect (PROJECT_LIB_HEADERS xasu_rsa.h)
collect (PROJECT_LIB_SOURCES xasu_sha2.c)
collect (PROJECT_LIB_HEADERS xasu_sha2.h)
collect (PROJECT_LIB_SOURCES xasu_sha3.c)
collect (PROJECT_LIB_HEADERS xasu_sha3.h)
collect (PROJECT_LIB_SOURCES xasu_trng.c)
collect (PROJECT_LIB_SOURCES xasu_trng.h)
collect (PROJECT_LIB_HEADERS xasu_status.h)
