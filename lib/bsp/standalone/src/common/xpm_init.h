/******************************************************************************
*Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
*SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpm_init.h
* @addtogroup xpm_init xpm_init APIs
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------- -------- -----------------------------------------------
*  1.0  gm      14/06/23 Initial release.
* </pre>
******************************************************************************/

#ifndef XPM_INIT_H
#define XPM_INIT_H

/************************** Constant Definitions *****************************/

#if defined  (XPM_SUPPORT)
#if defined  (VERSAL)
#define	MAX_NODE_COUNT	33
#endif
#if defined  (VERSAL_NET)
#define	MAX_NODE_COUNT	38
#endif

/**************************** Type Definitions *******************************/

typedef struct {
    u64 BaseAddress;
    u64 NodeId;
    u64 ResetId;
} XpmNodeInfo;

/************************** Function Prototypes ******************************/

u64 XpmGetNodeId(u64 BaseAddress);
u64 XpmGetResetId(u64 BaseAddress);

#endif
#endif
