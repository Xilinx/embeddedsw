/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

#ifndef _PM_DEFS_H_
#define _PM_DEFS_H_

#include "xpm_nodeid.h"
#include "xpm_client_nodeidwrapper.h"
#include "xpm_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * NOTE: All macros defined in this file is to just maintain the
 *       compatibility with existing ZynqMP application.
 *       This macros will be deprecated in future.
 */

/* Reset action IDs */
#define XILPM_RESET_ACTION_RELEASE	PM_RESET_ACTION_RELEASE
#define XILPM_RESET_ACTION_ASSERT	PM_RESET_ACTION_ASSERT
#define XILPM_RESET_ACTION_PULSE	PM_RESET_ACTION_PULSE

/* Requirement limits */
#define MAX_CAPABILITY			XPM_MAX_CAPABILITY
#define MAX_LATENCY			XPM_MAX_LATENCY
#define MAX_QOS				XPM_MAX_QOS
#define MIN_CAPABILITY			XPM_MIN_CAPABILITY
#define MIN_LATENCY			XPM_MIN_LATENCY
#define MIN_QOS				XPM_MIN_QOS

/* System shutdown macros */
#define PMF_SHUTDOWN_TYPE_SHUTDOWN	PM_SHUTDOWN_TYPE_SHUTDOWN
#define PMF_SHUTDOWN_TYPE_RESET		PM_SHUTDOWN_TYPE_RESET

#define PMF_SHUTDOWN_SUBTYPE_SUBSYSTEM	PM_SHUTDOWN_SUBTYPE_RST_SUBSYSTEM
#define PMF_SHUTDOWN_SUBTYPE_PS_ONLY	PM_SHUTDOWN_SUBTYPE_RST_PS_ONLY
#define PMF_SHUTDOWN_SUBTYPE_SYSTEM	PM_SHUTDOWN_SUBTYPE_RST_SYSTEM

/* Callback IDs */
/* NOTE: This macros are currently not supported for Versal. It may come in future */
#define PM_NOTIFY_STL_NO_OP		(34U)

/**
 *  PM Acknowledge Request Types
 */
/* TODO: Add support for this macros in future */
enum XPmRequestAck {
	REQUEST_ACK_NO = 1,
	REQUEST_ACK_BLOCKING,
	REQUEST_ACK_NON_BLOCKING,
	REQUEST_ACK_CB_CERROR,
};

#ifdef __cplusplus
}
#endif

#endif /* _PM_DEFS_H_ */
