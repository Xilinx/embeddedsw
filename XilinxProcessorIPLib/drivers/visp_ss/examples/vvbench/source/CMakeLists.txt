# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.

# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_SOURCES cJSON.c)

add_subdirectory(submoduels)

collect (PROJECT_LIB_SOURCES vvbench.c)

collect (PROJECT_LIB_SOURCES vvbparser.c)

collect (PROJECT_LIB_SOURCES vvdevice.c)

collect (PROJECT_LIB_SOURCES vvformat.c)

collect (PROJECT_LIB_SOURCES vvpath.c)
