###############################################################################
# Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################

if (WITH_XAIEDRV_FIND)
  find_package (Libxaiengine REQUIRED)
  collect (PROJECT_INC_DIRS "${LIBXAIENGINE_INCLUDE_DIR}")
  collect (PROJECT_LIB_DIRS "${LIBXAIENGINE_LIB_DIR}")
  collect (PROJECT_LIB_DEPS "${LIBXAIENGINE_LIB}")
else()
  collect (PROJECT_INC_DIRS "${CMAKE_BINARY_DIR}/aienginev2-src/include")
  collect (PROJECT_LIB_DIRS "${CMAKE_BINARY_DIR}/aienginev2-src/src")
  collect (PROJECT_LIB_DEPS "xaiengine")
endif (WITH_XAIEDRV_FIND)
