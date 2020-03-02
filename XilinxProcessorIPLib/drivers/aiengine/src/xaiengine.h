/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include <xaiengine/xaieconfig.h>
#include <xaiengine/xaiedma_shim.h>
#include <xaiengine/xaiedma_tile.h>
#include <xaiengine/xaiegbl.h>
#include <xaiengine/xaiegbl_defs.h>
#include <xaiengine/xaiegbl_params.h>
#include <xaiengine/xaiegbl_reginit.h>
#include <xaiengine/xaielib.h>
#include <xaiengine/xaielib_npi.h>
#include <xaiengine/xaiepm_clock.h>
#include <xaiengine/xaietile_core.h>
#include <xaiengine/xaietile_error.h>
#include <xaiengine/xaietile_event.h>
#include <xaiengine/xaietile_lock.h>
#include <xaiengine/xaietile_mem.h>
#include <xaiengine/xaietile_noc.h>
#include <xaiengine/xaietile_perfcnt.h>
#include <xaiengine/xaietile_pl.h>
#include <xaiengine/xaietile_plif.h>
#include <xaiengine/xaietile_shim.h>
#include <xaiengine/xaietile_strm.h>
#include <xaiengine/xaietile_timer.h>
#include <xaiengine/xparameters_aie.h>

#ifdef __AIESIM__
#include <xaiengine/xaiesim.h>
#include <xaiengine/xaiesim_elfload.h>
#endif

#ifdef __cplusplus
}
#endif
