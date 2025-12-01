# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

# Core runtime files (always compiled with full functionality)
collect_by_extension(PROJECT_LIB_HEADERS *.h)
collect_by_extension(PROJECT_LIB_SOURCES *.c)

# Subsystem extensions (optional)
# When enabled, adds xilpm_subsys subdirectory which provides SubsystemOpsTable
if(XILPM_NG_SUBSYS_ENABLE)
  add_subdirectory(xilpm_subsys)
else()
  message(WARNING "Subsystem support is disabled")
endif()
