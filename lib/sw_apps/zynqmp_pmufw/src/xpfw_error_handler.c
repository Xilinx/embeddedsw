/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#include "xpfw_default.h"
#include "xpfw_error_handler.h"
#include "xpfw_resets.h"

#define ERROR_ONE_EN (PMU_GLOBAL_ERROR_INT_EN_1_DDR_ECC_MASK |\
		PMU_GLOBAL_ERROR_INT_EN_1_OCM_ECC_MASK |\
		PMU_GLOBAL_ERROR_INT_EN_1_RPU0_MASK |\
		PMU_GLOBAL_ERROR_INT_EN_1_RPU1_MASK |\
		PMU_GLOBAL_ERROR_INT_EN_1_LPD_TEMP_MASK |\
		PMU_GLOBAL_ERROR_INT_EN_1_FPD_TEMP_MASK |\
		PMU_GLOBAL_ERROR_INT_EN_1_RPU_LS_MASK |\
		PMU_GLOBAL_ERROR_INT_EN_1_RPU_CCF_MASK |\
		PMU_GLOBAL_ERROR_INT_EN_1_LPD_SWDT_MASK |\
		PMU_GLOBAL_ERROR_INT_EN_1_FPD_SWDT_MASK |\
		PMU_GLOBAL_ERROR_INT_EN_1_PWR_SUPPLY_MASK |\
		PMU_GLOBAL_ERROR_INT_EN_1_XMPU_MASK)

#define ERROR_TWO_EN (PMU_GLOBAL_ERROR_INT_EN_2_TO_MASK |\
		PMU_GLOBAL_ERROR_INT_EN_2_PL_MASK |\
		PMU_GLOBAL_ERROR_INT_EN_2_PLL_LOCK_MASK |\
		PMU_GLOBAL_ERROR_INT_EN_2_CSU_MASK |\
		PMU_GLOBAL_ERROR_INT_EN_2_PMU_UC_MASK |\
		PMU_GLOBAL_ERROR_INT_EN_2_CSU_ROM_MASK)

/*Not Enabled :   PMU_GLOBAL_ERROR_INT_EN_2_PMU_FW_MASK |\
			PMU_GLOBAL_ERROR_INT_EN_2_PMU_SERVICE_MASK |\
			PMU_GLOBAL_ERROR_STATUS_2_PMU_PB_MASK
 */

void XPfw_ErrorHandlerInit(void)
{
	/* Enable All in Error-1 */
	XPfw_Write32(PMU_GLOBAL_ERROR_EN_1, ERROR_ONE_EN);
	XPfw_Write32(PMU_GLOBAL_ERROR_INT_EN_1, ERROR_ONE_EN);

	/* PMU related errors are not enabled in Error-2*/
	XPfw_Write32(PMU_GLOBAL_ERROR_EN_2, ERROR_TWO_EN);
	XPfw_Write32(PMU_GLOBAL_ERROR_INT_EN_2, ERROR_TWO_EN);



}


void XPfw_ErrorHandlerOne(void)
{
	u32 l_ErrorOneReg;

	/* Latch the Error Flags */
	l_ErrorOneReg = XPfw_Read32(PMU_GLOBAL_ERROR_STATUS_1);

	/* Clear Error Register so that new errors can trigger an Interrupt */
	XPfw_Write32(PMU_GLOBAL_ERROR_STATUS_1,l_ErrorOneReg);

	if( (l_ErrorOneReg & PMU_GLOBAL_ERROR_STATUS_1_DDR_ECC_MASK) != 0x00000000U) {
		XPfw_ErrorEccDdr();
	}
	if( (l_ErrorOneReg & PMU_GLOBAL_ERROR_STATUS_1_OCM_ECC_MASK) != 0x00000000U) {
		XPfw_ErrorEccOcm();
	}
	if( (l_ErrorOneReg & PMU_GLOBAL_ERROR_STATUS_1_RPU0_MASK) != 0x00000000U) {
		XPfw_ErrorRpu0();
	}
	if( (l_ErrorOneReg & PMU_GLOBAL_ERROR_STATUS_1_RPU1_MASK) != 0x00000000U) {
		XPfw_ErrorRpu1();
	}
	if( (l_ErrorOneReg & PMU_GLOBAL_ERROR_STATUS_1_LPD_TEMP_MASK) != 0x00000000U) {
		XPfw_ErrorLpdTemp();
	}
	if( (l_ErrorOneReg & PMU_GLOBAL_ERROR_STATUS_1_FPD_TEMP_MASK) != 0x00000000U) {
		XPfw_ErrorFpdTemp();
	}
	if( (l_ErrorOneReg & PMU_GLOBAL_ERROR_STATUS_1_RPU_LS_MASK) != 0x00000000U) {
		XPfw_ErrorRpuLockStep();
	}
	if( (l_ErrorOneReg & PMU_GLOBAL_ERROR_STATUS_1_RPU_CCF_MASK) != 0x00000000U) {
		XPfw_ErrorRpuCcf();
	}
	if( (l_ErrorOneReg & PMU_GLOBAL_ERROR_STATUS_1_LPD_SWDT_MASK) != 0x00000000U) {
		XPfw_ErrorLpdSwdt();
	}
	if( (l_ErrorOneReg & PMU_GLOBAL_ERROR_STATUS_1_FPD_SWDT_MASK) != 0x00000000U) {
		XPfw_ErrorFpdSwdt();
	}
	if( (l_ErrorOneReg & PMU_GLOBAL_ERROR_STATUS_1_PWR_SUPPLY_MASK) != 0x00000000U) {
		XPfw_ErrorPowerSupply();
	}
	if( (l_ErrorOneReg & PMU_GLOBAL_ERROR_STATUS_1_XMPU_MASK) != 0x00000000U) {
		XPfw_ErrorXmpu();
	}

	/* Clear Error Register so that new errors can trigger an Interrupt */

	XPfw_Write32(PMU_GLOBAL_ERROR_STATUS_1,l_ErrorOneReg);

}


void XPfw_ErrorHandlerTwo(void)
{
	u32 l_ErrorTwoReg;

	/* Latch the Error Flags */
	l_ErrorTwoReg = XPfw_Read32(PMU_GLOBAL_ERROR_STATUS_2);

	/* Clear Error Register so that new errors can trigger an Interrupt */
	XPfw_Write32(PMU_GLOBAL_ERROR_STATUS_2,l_ErrorTwoReg);

	if( (l_ErrorTwoReg & PMU_GLOBAL_ERROR_STATUS_2_TO_MASK) != 0x00000000U) {
		XPfw_ErrorTimeOut();
	}
	if( (l_ErrorTwoReg & PMU_GLOBAL_ERROR_STATUS_2_PL_MASK) != 0x00000000U) {
		XPfw_ErrorPL();
	}
	if( (l_ErrorTwoReg & PMU_GLOBAL_ERROR_STATUS_2_PLL_LOCK_MASK) != 0x00000000U) {
		XPfw_ErrorPLL();
	}
	if( (l_ErrorTwoReg & PMU_GLOBAL_ERROR_STATUS_2_CSU_MASK) != 0x00000000U) {
		XPfw_ErrorCsu();
	}
	if( (l_ErrorTwoReg & PMU_GLOBAL_ERROR_STATUS_2_PMU_UC_MASK) != 0x00000000U) {
		XPfw_ErrorPmuUncorrectable();
	}
	if( (l_ErrorTwoReg & PMU_GLOBAL_ERROR_STATUS_2_PMU_FW_MASK) != 0x00000000U) {
		XPfw_ErrorPmuFw();
	}
	if( (l_ErrorTwoReg & PMU_GLOBAL_ERROR_STATUS_2_PMU_SERVICE_MASK) != 0x00000000U) {
		XPfw_ErrorPmuServiceMode();
	}
	if( (l_ErrorTwoReg & PMU_GLOBAL_ERROR_STATUS_2_PMU_PB_MASK) != 0x00000000U) {
		XPfw_ErrorPmuPreBoot();
	}
	if( (l_ErrorTwoReg & PMU_GLOBAL_ERROR_STATUS_2_CSU_ROM_MASK) != 0x00000000U) {
		XPfw_ErrorCsuRom();
	}




}




void XPfw_ErrorEccDdr(void) {
	fw_printf("XPFW: Error Handler Triggered: %s\r\n",__func__);
}

void XPfw_ErrorEccOcm(void) {
	fw_printf("XPFW: Error Handler Triggered: %s\r\n",__func__);
}

void XPfw_ErrorRpu0(void) {
	fw_printf("XPFW: Error Handler Triggered: %s\r\n",__func__);
}

void XPfw_ErrorRpu1(void) {
	fw_printf("XPFW: Error Handler Triggered: %s\r\n",__func__);
}

void XPfw_ErrorLpdTemp(void) {
	fw_printf("XPFW: Error Handler Triggered: %s\r\n",__func__);
}

void XPfw_ErrorFpdTemp(void) {
	fw_printf("XPFW: Error Handler Triggered: %s\r\n",__func__);
}

void XPfw_ErrorRpuLockStep(void) {
	fw_printf("XPFW: Error Handler Triggered: %s\r\n",__func__);
	XPfw_ResetRpu();
}

void XPfw_ErrorRpuCcf(void) {
	fw_printf("XPFW: Error Handler Triggered: %s\r\n",__func__);
}

void XPfw_ErrorLpdSwdt(void) {
	fw_printf("XPFW: Error Handler Triggered: %s\r\n",__func__);
}

void XPfw_ErrorFpdSwdt(void) {
	fw_printf("XPFW: Error Handler Triggered: %s\r\n",__func__);
}

void XPfw_ErrorPowerSupply(void) {
	fw_printf("XPFW: Error Handler Triggered: %s\r\n",__func__);
}

void XPfw_ErrorXmpu(void) {
	fw_printf("XPFW: Error Handler Triggered: %s\r\n",__func__);
}

void XPfw_ErrorTimeOut(void) {
	fw_printf("XPFW: Error Handler Triggered: %s\r\n",__func__);
}

void XPfw_ErrorPL(void) {
	fw_printf("XPFW: Error Handler Triggered: %s\r\n",__func__);
}

void XPfw_ErrorPLL(void) {
	fw_printf("XPFW: Error Handler Triggered: %s\r\n",__func__);
}

void XPfw_ErrorCsu(void) {
	fw_printf("XPFW: Error Handler Triggered: %s\r\n",__func__);
}

void XPfw_ErrorPmuUncorrectable(void) {
	fw_printf("XPFW: Error Handler Triggered: %s\r\n",__func__);
}

void XPfw_ErrorPmuFw(void) {
	fw_printf("XPFW: Error Handler Triggered: %s\r\n",__func__);
}

void XPfw_ErrorPmuServiceMode(void) {
	fw_printf("XPFW: Error Handler Triggered: %s\r\n",__func__);
}

void XPfw_ErrorPmuPreBoot(void)
{
	fw_printf( " XPFW: Error Handler Triggered: %s\r\n " ,__func__);
}

void XPfw_ErrorCsuRom(void) {
	fw_printf("XPFW: Error Handler Triggered: %s\r\n",__func__);
}
