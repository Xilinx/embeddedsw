# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect_by_extension(PROJECT_LIB_HEADERS *.h)
collect_by_extension(PROJECT_LIB_SOURCES *.c)

if(XILPM_NG_SUBSYS_ENABLE)
  # Enable Subsystem support (if enabled) - includes EEMI + Subsystem
  add_subdirectory(xilpm_subsys)
else()
  # Enable EEMI support as the baseline
  add_subdirectory(xilpm_eemi)
endif()
