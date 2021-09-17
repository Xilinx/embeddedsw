###############################################################################
# Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################

set (CMAKE_SYSTEM_PROCESSOR "aarch64"            CACHE STRING "")
set (CROSS_PREFIX           "aarch64-linux-gnu-" CACHE STRING "")
include (cross-linux-g++)

# vim: expandtab:ts=2:sw=2:smartindent
