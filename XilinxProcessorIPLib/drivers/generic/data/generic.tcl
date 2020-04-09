###############################################################################
# Copyright (C) 2009 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ------------------------------------
# 2.0     adk    12/10/13 Updated as per the New Tcl API's
###############################################################


proc generate {drv_handle} {
    
    ::hsi::utils::define_addr_params $drv_handle "xparameters.h"
}
