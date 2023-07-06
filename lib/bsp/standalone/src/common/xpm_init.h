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
#ifdef VERSAL_NET
#define	MAX_NODE_COUNT	38
#elif defined(versal)
#define	MAX_NODE_COUNT	33
#endif

/**************************** Type Definitions *******************************/

typedef struct {
    UINTPTR BaseAddress;
    UINTPTR NodeId;
    UINTPTR ResetId;
} XpmNodeInfo;

/************************** Function Prototypes ******************************/

UINTPTR XpmGetNodeId(UINTPTR BaseAddress);
UINTPTR XpmGetResetId(UINTPTR BaseAddress);

#endif
#endif
