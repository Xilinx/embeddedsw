# Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

# Platform-specific client code for Versal_2VE_2VM: APU/RPU init plus the
# platform headers (xpm_*.h) that the shared pm_api_sys.h includes. The headers
# are collected so they are staged into the BSP include path for the client.
collect (PROJECT_LIB_SOURCES pm_client.c)
collect (PROJECT_LIB_HEADERS pm_client.h)
collect (PROJECT_LIB_HEADERS xpm_defs.h)
collect (PROJECT_LIB_HEADERS xpm_err.h)
collect (PROJECT_LIB_HEADERS xpm_error_node.h)
collect (PROJECT_LIB_HEADERS xpm_nodeid.h)
collect (PROJECT_LIB_HEADERS xpm_ver.h)
