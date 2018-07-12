/*
 * Copyright (C) 2014 - 2015 Xilinx, Inc.  All rights reserved.
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
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 */
#include "xpfw_config.h"
#ifdef ENABLE_PM

/*********************************************************************
 * Definitions of commonly used functions for debugging PMU Power
 * Management (PM). Conditionaly compiled code for debugging is not
 * intended to be MISRA compliant.
 *********************************************************************/

#include "pm_common.h"
#include "pm_defs.h"
#include "pm_master.h"

#ifdef DEBUG_PM

/**
 * PmStrAck() - Converts integer acknowledge into matching string
 * @ack        Acknowlegde integer id to be converted to string
 *
 * @return     String name of given acknowledge id
 */
const char* PmStrAck(const u32 ack)
{
	switch (ack) {
	case REQUEST_ACK_NO:
		return "REQUEST_ACK_NO";
	case REQUEST_ACK_BLOCKING:
		return "REQUEST_ACK_BLOCKING";
	case REQUEST_ACK_NON_BLOCKING:
		return "REQUEST_ACK_NON_BLOCKING";
	default:
		return "ERROR_ACK";
	}
}

/**
 * PmStrReason() - Converts integer reason id into matching string
 * @reason      Integer reason id to be converted
 *
 * @return      String name of given reason integer
 */
const char* PmStrReason(const u32 reason)
{
	switch (reason) {
	case ABORT_REASON_WKUP_EVENT:
		return "ABORT_REASON_WKUP_EVENT";
	case ABORT_REASON_PU_BUSY:
		return "ABORT_REASON_PU_BUSY";
	case ABORT_REASON_NO_PWRDN:
		return "ABORT_REASON_NO_PWRDN";
	case ABORT_REASON_UNKNOWN:
		return "ABORT_REASON_UNKNOWN";
	default:
		return "ERROR_REASON";
	}
}

#endif /* DEBUG_PM */

#endif
