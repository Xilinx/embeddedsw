# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

# Add xilpm_runtime only if EEMI or Subsystem is enabled
if(XILPM_NG_EEMI_ENABLE OR XILPM_NG_SUBSYS_ENABLE)
  add_subdirectory(src)
else()
  message(FATAL_ERROR "Either EEMI or Subsystem support must be enabled.")
endif()
