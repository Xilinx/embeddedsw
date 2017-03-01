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
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
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
 * PmStrNode() - Converts integer node id into node string
 * @node        Node id to be converted to string
 *
 * @return      String name of given node id
 */
const char* PmStrNode(const u32 node)
{
	switch (node) {
	case NODE_APU:
		return "NODE_APU";
	case NODE_APU_0:
		return "NODE_APU_0";
	case NODE_APU_1:
		return "NODE_APU_1";
	case NODE_APU_2:
		return "NODE_APU_2";
	case NODE_APU_3:
		return "NODE_APU_3";
	case NODE_RPU:
		return "NODE_RPU";
	case NODE_RPU_0:
		return "NODE_RPU_0";
	case NODE_RPU_1:
		return "NODE_RPU_1";
	case NODE_PL:
		return "NODE_PL";
	case NODE_PLD:
		return "NODE_PLD";
	case NODE_FPD:
		return "NODE_FPD";
	case NODE_OCM_BANK_0:
		return "NODE_OCM_BANK_0";
	case NODE_OCM_BANK_1:
		return "NODE_OCM_BANK_1";
	case NODE_OCM_BANK_2:
		return "NODE_OCM_BANK_2";
	case NODE_OCM_BANK_3:
		return "NODE_OCM_BANK_3";
	case NODE_TCM_0_A:
		return "NODE_TCM_0_A";
	case NODE_TCM_0_B:
		return "NODE_TCM_0_B";
	case NODE_TCM_1_A:
		return "NODE_TCM_1_A";
	case NODE_TCM_1_B:
		return "NODE_TCM_1_B";
	case NODE_L2:
		return "NODE_L2";
	case NODE_GPU_PP_0:
		return "NODE_GPU_PP_0";
	case NODE_GPU_PP_1:
		return "NODE_GPU_PP_1";
	case NODE_USB_0:
		return "NODE_USB_0";
	case NODE_USB_1:
		return "NODE_USB_1";
	case NODE_TTC_0:
		return "NODE_TTC_0";
	case NODE_TTC_1:
		return "NODE_TTC_1";
	case NODE_TTC_2:
		return "NODE_TTC_2";
	case NODE_TTC_3:
		return "NODE_TTC_3";
	case NODE_SATA:
		return "NODE_SATA";
	case NODE_APLL:
		return "NODE_APLL";
	case NODE_VPLL:
		return "NODE_VPLL";
	case NODE_DPLL:
		return "NODE_DPLL";
	case NODE_RPLL:
		return "NODE_RPLL";
	case NODE_IOPLL:
		return "NODE_IOPLL";
	case NODE_UART_0:
		return "NODE_UART_0";
	case NODE_UART_1:
		return "NODE_UART_1";
	case NODE_SPI_0:
		return "NODE_SPI_0";
	case NODE_SPI_1:
		return "NODE_SPI_1";
	case NODE_I2C_0:
		return "NODE_I2C_0";
	case NODE_I2C_1:
		return "NODE_I2C_1";
	case NODE_SD_0:
		return "NODE_SD_0";
	case NODE_SD_1:
		return "NODE_SD_1";
	case NODE_CAN_0:
		return "NODE_CAN_0";
	case NODE_CAN_1:
		return "NODE_CAN_1";
	case NODE_ETH_0:
		return "NODE_ETH_0";
	case NODE_ETH_1:
		return "NODE_ETH_1";
	case NODE_ETH_2:
		return "NODE_ETH_2";
	case NODE_ETH_3:
		return "NODE_ETH_3";
	case NODE_ADMA:
		return "NODE_ADMA";
	case NODE_GDMA:
		return "NODE_GDMA";
	case NODE_DP:
		return "NODE_DP";
	case NODE_NAND:
		return "NODE_NAND";
	case NODE_QSPI:
		return "NODE_QSPI";
	case NODE_GPIO:
		return "NODE_GPIO";
	case NODE_DDR:
		return "NODE_DDR";
	case NODE_IPI_APU:
		return "NODE_IPI_APU";
	case NODE_IPI_RPU_0:
		return "NODE_IPI_RPU_0";
	case NODE_IPI_RPU_1:
		return "NODE_IPI_RPU_1";
	case NODE_IPI_PL_0:
		return "NODE_IPI_PL_0";
	case NODE_IPI_PL_1:
		return "NODE_IPI_PL_1";
	case NODE_IPI_PL_2:
		return "NODE_IPI_PL_2";
	case NODE_IPI_PL_3:
		return "NODE_IPI_PL_3";
	case NODE_GPU:
		return "NODE_GPU";
	case NODE_PCIE:
		return "NODE_PCIE";
	case NODE_PCAP:
		return "NODE_PCAP";
	case NODE_RTC:
		return "NODE_RTC";
	case NODE_LPD:
		return "NODE_LPD";
	case NODE_VCU:
		return "NODE_VCU";
	case NODE_EXTERN:
		return "NODE_EXTERN";
	default:
		return "ERROR_NODE";
	}
}

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
