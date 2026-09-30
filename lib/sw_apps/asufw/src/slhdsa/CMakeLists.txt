# Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
# SLH-DSA core for ASUFW — hash backends via XSha + DMA.

collect(PROJECT_LIB_SOURCES xslhdsa.c)
collect(PROJECT_LIB_HEADERS xslhdsa.h)
collect(PROJECT_LIB_HEADERS xslhdsa_addr.h)

collect(PROJECT_LIB_SOURCES xslhdsa_core.c)
collect(PROJECT_LIB_HEADERS xslhdsa_core.h)

collect(PROJECT_LIB_SOURCES xslhdsa_hash.c)
collect(PROJECT_LIB_HEADERS xslhdsa_hash.h)

collect(PROJECT_LIB_SOURCES xslhdsa_yield.c)
collect(PROJECT_LIB_HEADERS xslhdsa_yield.h)
