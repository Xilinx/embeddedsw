###############################################################################
#
# Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
#
#
###############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.0   sa   04/05/17 First release
#
##############################################################################
#uses "xillib.tcl"

proc generate {drv_handle} {
    ::hsi::utils::define_include_file $drv_handle "xparameters.h" \
        "XTMR_Manager" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" \
        "C_MASK" "C_BRK_DELAY_RST_VALUE" "C_MASK_RST_VALUE" "C_MAGIC1" \
        "C_MAGIC2" "C_UE_IS_FATAL" "C_UE_WIDTH" "C_NO_OF_COMPARATORS" \
        "C_COMPARATORS_MASK" "C_WATCHDOG" "C_WATCHDOG_WIDTH" \
        "C_SEM_INTERFACE" "C_SEM_HEARTBEAT_WATCHDOG" \
        "C_SEM_HEARTBEAT_WATCHDOG_WIDTH" "C_BRK_DELAY_WIDTH" "C_TMR" \
        "C_TEST_COMPARATOR" "C_STRICT_MISCOMPARE" "C_USE_DEBUG_DISABLE" \
        "C_USE_TMR_DISABLE"
    ::hsi::utils::define_config_file $drv_handle "xtmr_manager_g.c" \
        "XTMR_Manager" "DEVICE_ID" "C_BASEADDR" "C_BRK_DELAY_RST_VALUE" \
        "C_MASK_RST_VALUE" "C_MAGIC1" "C_MAGIC2" "C_UE_IS_FATAL" \
        "C_UE_WIDTH" "C_NO_OF_COMPARATORS" "C_COMPARATORS_MASK" \
        "C_WATCHDOG" "C_WATCHDOG_WIDTH" "C_SEM_INTERFACE" \
        "C_SEM_HEARTBEAT_WATCHDOG" "C_SEM_HEARTBEAT_WATCHDOG_WIDTH" \
        "C_BRK_DELAY_WIDTH" "C_TMR" "C_TEST_COMPARATOR" \
        "C_STRICT_MISCOMPARE" "C_USE_DEBUG_DISABLE" "C_USE_TMR_DISABLE"

    ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" \
        "TMR_Manager" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" \
        "C_BRK_DELAY_RST_VALUE" "C_MASK_RST_VALUE" "C_MASK" "C_MAGIC1" \
        "C_MAGIC2" "C_UE_IS_FATAL" "C_UE_WIDTH" "C_NO_OF_COMPARATORS" \
        "C_COMPARATORS_MASK" "C_WATCHDOG" "C_WATCHDOG_WIDTH" \
        "C_SEM_INTERFACE" "C_SEM_HEARTBEAT_WATCHDOG" \
        "C_SEM_HEARTBEAT_WATCHDOG_WIDTH" "C_BRK_DELAY_WIDTH" "C_TMR" \
        "C_TEST_COMPARATOR" "C_STRICT_MISCOMPARE" "C_USE_DEBUG_DISABLE" \
        "C_USE_TMR_DISABLE"
}
