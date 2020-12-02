###############################################################################
# Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
###############################################################################
#
# Modification History
#
#  Ver   Who      Date       Changes
# ----- ----  ----------  -----------------------------------------------
#  1.0   ms    07/14/16     FirstRelease
#  1.1   ms    01/16/17     Updated the parameter naming from
#                           XPAR_PR_DECOUPLER_NUM_INSTANCES to
#                           XPAR_XPRD_NUM_INSTANCES to avoid compilation
#                           failure for XPAR_PR_DECOUPLER_NUM_INSTANCES as
#                           the tools are generating XPAR_XPRD_NUM_INSTANCES
#                           in the generated xprd_g.c for fixing MISRA-C
#                           files. This is a fix for CR-966099 based on the
#                           update in the tools.
#
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
    ::hsi::utils::define_zynq_include_file $drv_handle "xparameters.h" "XPRD" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR"

    ::hsi::utils::define_zynq_config_file $drv_handle "xprd_g.c" "XPrd" "DEVICE_ID" "C_BASEADDR"

    ::hsi::utils::define_zynq_canonical_xpars $drv_handle "xparameters.h" "PR_DECOUPLER" "DEVICE_ID" "C_BASEADDR"

}
