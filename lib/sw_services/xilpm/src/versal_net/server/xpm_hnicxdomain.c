/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_common.h"
#include "xpm_hnicxdomain.h"
#include "xpm_debug.h"

XStatus XPmHnicxDomain_Init(XPm_HnicxDomain *Hnicxd, u32 Id, u32 BaseAddress,
                         XPm_Power *Parent)
{
        XStatus Status = XST_FAILURE;
        u16 DbgErr = XPM_INT_ERR_UNDEFINED;

        Status = XPmPowerDomain_Init(&Hnicxd->Domain, Id, BaseAddress, Parent, NULL);
        if (XST_SUCCESS != Status) {
                DbgErr = XPM_INT_ERR_POWER_DOMAIN_INIT;
        }

        XPm_PrintDbgErr(Status, DbgErr);
        return Status;
}
