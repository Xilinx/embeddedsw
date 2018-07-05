###############################################################################
# Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
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
