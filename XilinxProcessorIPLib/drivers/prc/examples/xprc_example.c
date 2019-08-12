/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
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
/*****************************************************************************/
/**
*
* @file xprc_example.c
*
* This file contains an example using the XPrc driver to test the registers
* on the device.
*
* @note		None
*
* MODIFICATION HISTORY:
* <pre>
* Ver	 Who   Date	      Changes
* ---- ----- ------------  -----------------------------------------------
* 1.0   ms    07/18/2016     First Release
*       ms    04/05/2017     Modified comment lines notation in functions to
*                            avoid unnecessary description displayed
*                            while generating doxygen.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xprc.h"
#include "xgpio.h"
#include "xil_printf.h"
#include "xparameters.h"
#include<string.h>

/************************** Constant Definitions *****************************/

/**
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define XPRC_DEVICE_ID			XPAR_PRC_0_DEVICE_ID
#define XGPIO_USR_ACCESS_DEVICE_ID	XPAR_AXI_GPIO_USR_ACCESS_DEVICE_ID
#define XGPIO_VSM_SHIFT_DEVICE_ID	XPAR_AXI_GPIO_VSM_SHIFT_DEVICE_ID
#define XGPIO_VSM_COUNT_DEVICE_ID	XPAR_AXI_GPIO_VSM_COUNT_DEVICE_ID

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

u32 XPrc_Example(u16 DeviceId);
u32 Xprc_Check_User_Command(u16 VsmId, XGpio ShiftVsmGpio,
	XGpio CountVsmGpio);
u32 Xprc_Check_RestartWithStatus_Command(u16 VsmId);
u32 Xprc_Check_TriggerRmMapping(u16 VsmId);
u32 Xprc_Check_BsIndex_Reg(u16 VsmId);
u32 Xprc_Check_RmControl_Reg(u16 VsmId);
u32 Xprc_Check_BsSize_Reg(u16 VsmId);
u32 Xprc_Check_BsAddress_Reg(u16 VsmId);
u32 Xprc_Program_PRC(u16 VsmId);
void Xprc_Restart_VSMs(u16 VsmId);

/************************** Variable Definitions *****************************/

XPrc Prc;	/* Instance of the PRC */

/*****************************************************************************/
/**
*
* This is the main function to call the example.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	u32 Status;

	/* Run the selftest example */
	Status = XPrc_Example((u16)XPRC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("PRC Example is failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran PRC Example\r\n");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function does a minimal test on the PRC device and driver as a
* design example.
*
* @param	DeviceId is the XPAR_<prc_instance>_DEVICE_ID value from
*		xparameters.h.
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if failed
*
* @note		None.
*
******************************************************************************/
u32 XPrc_Example(u16 DeviceId)
{
	u32 Status;
	XPrc_Config *XPrcCfgPtr;
	XGpio_Config *XUsrAccessGpioCfgPtr;
	XGpio UsrAccessGpio;
		/* Instance of the GPIO that's connected to the USR_ACCESS primitive */
	XGpio_Config *XShiftVsmGpioCfgPtr;
	XGpio ShiftVsmGpio;
		/* Instance of the GPIO attached to the PRC's Shift VSM's outputs */
	XGpio_Config *XCountVsmGpioCfgPtr;
	XGpio CountVsmGpio;
		/* Instance of the GPIO attached to the PRC's Count VSM's outputs */
	u16 VsmId;	/* A loop variable used to iterate round VSMs in the PRC */
	u16 RmId;	/* A loop variable used to iterate round RMs in the PRC */

	/*
	 * Initialize the PRC driver so that it's ready to use.
	 * Look up the configuration in the config table, then initialize it.
	 */
	XPrcCfgPtr = XPrc_LookupConfig(DeviceId);
	if (NULL == XPrcCfgPtr) {
		return XST_FAILURE;
	}

	Status = XPrc_CfgInitialize(&Prc, XPrcCfgPtr, XPrcCfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built
	 * correctly
	 */
	Status = XPrc_SelfTest(&Prc);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Set up the USR_ACCESS GPIO */
	XUsrAccessGpioCfgPtr = XGpio_LookupConfig(
			(u16)XGPIO_USR_ACCESS_DEVICE_ID);
	if (NULL == XUsrAccessGpioCfgPtr) {
		return XST_FAILURE;
	}

	Status = XGpio_CfgInitialize(&UsrAccessGpio, XUsrAccessGpioCfgPtr,
			XUsrAccessGpioCfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Set up the Shift VSM GPIO */
	XShiftVsmGpioCfgPtr = XGpio_LookupConfig((u16)
			XGPIO_VSM_SHIFT_DEVICE_ID);
	if (NULL == XShiftVsmGpioCfgPtr) {
		return XST_FAILURE;
	}

	Status = XGpio_CfgInitialize(&ShiftVsmGpio, XShiftVsmGpioCfgPtr,
			XShiftVsmGpioCfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*Set up the Count VSM GPIO */
	XCountVsmGpioCfgPtr = XGpio_LookupConfig(
			(u16)XGPIO_VSM_COUNT_DEVICE_ID);
	if (NULL == XCountVsmGpioCfgPtr) {
		return XST_FAILURE;
	}

	Status = XGpio_CfgInitialize(&CountVsmGpio, XCountVsmGpioCfgPtr,
			XCountVsmGpioCfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Print the PRC's status when the program starts
	 * This should only print anything if debug is turned on
	 */
	for (VsmId = 0; VsmId < XPrc_GetNumberOfVsms(&Prc); VsmId++) {
		XPrc_PrintVsmStatus(&Prc, VsmId, "    ");
	}


	/*
	 * We can only reprogram VSMs when they are in shutdown, so shutdown
	 * any active VSMs
	 */
	for (VsmId = 0; VsmId < XPrc_GetNumberOfVsms(&Prc); VsmId++) {
		if(XPrc_IsVsmInShutdown(&Prc, VsmId) == 0) {
			XPrc_SendShutdownCommand(&Prc, VsmId);
			while(XPrc_IsVsmInShutdown(&Prc, VsmId) == 0);
		}
	}

	for (VsmId = 0; VsmId < XPrc_GetNumberOfVsms(&Prc); VsmId++) {
		XPrc_PrintVsmStatus(&Prc, VsmId, "    ");
	}

	Xprc_Check_User_Command(VsmId, ShiftVsmGpio, CountVsmGpio);
	Xprc_Check_RestartWithStatus_Command(VsmId);
	Xprc_Check_TriggerRmMapping(VsmId);
	Xprc_Check_BsIndex_Reg(VsmId);
	Xprc_Check_RmControl_Reg(VsmId);
	Xprc_Check_BsSize_Reg(VsmId);
	Xprc_Check_BsAddress_Reg(VsmId);
	Xprc_Program_PRC(VsmId);
	Xprc_Restart_VSMs(VsmId);


	/*
	 * The ICAP has lower priority than JTAG and SDK and Vivado HW
	 * Debugger both use JTAG to control the device and get status. This
	 * can occasionally mean that the first partial bitstream load will
	 * fail if the SDK program is reloaded without reconfiguring the entire
	 * device.
	 *
	 * This code attempts to detect that case and flush it before we enter
	 * the actual test phase.

	 * NOTE: If you press F11 in SDK to reload the program this can cause
	 * error messages to appear.
	 * These can be ignored.
	 */

	while(XGpio_DiscreteRead(&UsrAccessGpio, 1) != 0){
		XPrc_SendSwTrigger (&Prc, 0, 0);
	}

	/*
	 * Send triggers and check that each one loaded the correct
	 * bitstream
	 */
	for (VsmId = 0; VsmId < XPrc_GetNumberOfVsms(&Prc); VsmId++) {
		for (RmId = 0; RmId < XPrc_GetNumRms(&Prc, VsmId); RmId++) {
			u32 ExpectedId = 0;
			u32 ActualId = 0;

			/*
			 * Send a software trigger to the PRC to get it to load
			 * the partial bitstream for VsmID, RmId.
			 */
			XPrc_SendSwTrigger (&Prc, VsmId, RmId);

			/* Now wait until the PRC has loaded it */

			/* Read the status register once */
			u32 Status = XPrc_ReadStatusReg(&Prc, VsmId);

			/*
			 * Use the value twice.  This ensures that both
			 * the RmId and State fields come from the same read.
			 * If only one field is required, pass &Prc instead of
			 *  NULL, and VsmId instead of Status to these
			 * functions and they will read the register rather
			 * than parse the pre-read data
			 */
			s32 RmIdInPrc = XPrc_GetRmIdFromStatus(NULL, Status);
			u8  State     = XPrc_GetVsmState      (NULL, Status);

			if (State == XPRC_SR_STATE_FULL && RmIdInPrc == RmId) {
				break;
			}

			/* Work out what USR_ACCESS shoudl change to */
			ExpectedId = (VsmId << 16 ) | RmId;

			/* Get the USR_ACCESS setting from the GPIO */
			ActualId = XGpio_DiscreteRead(&UsrAccessGpio, 1);

			if (ExpectedId != ActualId) {
				return XST_FAILURE;
			}
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is used to test the User Command.
*
* @param	VsmId is the identifier of the VSM to access.
* @param	ShiftVsmGpio is Instance of the GPIO attached to the PRC's
*		Shift VSM.
* @param	CountVsmGpio is Instance of the GPIO attached to the PRC's
*		Count VSM.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		This is only valid for this particular configuration of the
*		PRC so it cannot be used in a general PRC test case.
*		This is configuration dependent because each VSM has its own
*		GPIO.
*
******************************************************************************/
u32 Xprc_Check_User_Command(u16 VsmId, XGpio ShiftVsmGpio, XGpio CountVsmGpio)
{
	u8 Rm_Shutdown_Req;
	u8 Rm_Decouple;
	u8 Sw_Shutdown_Req;
	u8 Sw_Startup_Req;
	u8 Rm_Reset;

	u8 Rm_Shutdown_Req_Actual;
	u8 Rm_Decouple_Actual;
	u8 Sw_Shutdown_Req_Actual;
	u8 Sw_Startup_Req_Actual;
	u8 Rm_Reset_Actual;

	u32 gpio_value;
	u8  i;

	XGpio *pGpio;		/* A pointer to a GPIO */

	for (VsmId = 0; VsmId < XPrc_GetNumberOfVsms(&Prc); VsmId++) {

		/*
		 * Work out which VSM this is and select the correct GPIO
		 * instance to use
		 */
		if (VsmId == 0) {
			pGpio = &ShiftVsmGpio;
		} else if (VsmId == 1) {
			pGpio = &CountVsmGpio;
		}

		for (i = 0; i < 32; i++) {
			Rm_Shutdown_Req = (i & (1 <<
				XPRC_CR_USER_CONTROL_RM_SHUTDOWN_REQ_BIT)) >>
				XPRC_CR_USER_CONTROL_RM_SHUTDOWN_REQ_BIT;
			Rm_Decouple = (i & (1 <<
				XPRC_CR_USER_CONTROL_RM_DECOUPLE_BIT)) >>
				XPRC_CR_USER_CONTROL_RM_DECOUPLE_BIT;
			Sw_Shutdown_Req = (i & (1 <<
				XPRC_CR_USER_CONTROL_SW_SHUTDOWN_REQ_BIT)) >>
				XPRC_CR_USER_CONTROL_SW_SHUTDOWN_REQ_BIT;
			Sw_Startup_Req = (i & (1 <<
				XPRC_CR_USER_CONTROL_SW_STARTUP_REQ_BIT)) >>
				XPRC_CR_USER_CONTROL_SW_STARTUP_REQ_BIT;
			Rm_Reset = (i & (1 <<
				XPRC_CR_USER_CONTROL_RM_RESET_BIT)) >>
				XPRC_CR_USER_CONTROL_RM_RESET_BIT;

			XPrc_SendUserControlCommand(&Prc, VsmId,
				Rm_Shutdown_Req, Rm_Decouple, Sw_Shutdown_Req,
				Sw_Startup_Req, Rm_Reset);
			gpio_value = XGpio_DiscreteRead(pGpio, 1);

			Rm_Shutdown_Req_Actual = (gpio_value & (1 <<
				XPRC_CR_USER_CONTROL_RM_SHUTDOWN_REQ_BIT)) >>
				XPRC_CR_USER_CONTROL_RM_SHUTDOWN_REQ_BIT;
			Rm_Decouple_Actual = (gpio_value & (1 <<
				XPRC_CR_USER_CONTROL_RM_DECOUPLE_BIT)) >>
				XPRC_CR_USER_CONTROL_RM_DECOUPLE_BIT;
			Sw_Shutdown_Req_Actual = (gpio_value & (1 <<
				XPRC_CR_USER_CONTROL_SW_SHUTDOWN_REQ_BIT)) >>
				XPRC_CR_USER_CONTROL_SW_SHUTDOWN_REQ_BIT;
			Sw_Startup_Req_Actual = (gpio_value & (1 <<
				XPRC_CR_USER_CONTROL_SW_STARTUP_REQ_BIT )) >>
				XPRC_CR_USER_CONTROL_SW_STARTUP_REQ_BIT ;
			Rm_Reset_Actual = (gpio_value & (1 <<
				XPRC_CR_USER_CONTROL_RM_RESET_BIT)) >>
				XPRC_CR_USER_CONTROL_RM_RESET_BIT;

			/*
			 * What signals will be enabled?
			 * RM_SHUTOWN, RM_DECOUPLE and RM_RESET are alwas
			 * enabled SW_SHUTDOWN and SW_STARTUP are only enabled
			 * if there's some kind of control interface.  Of
			 * course, there will be or else we'd never be able to
			 *  test it in this program :-)
			 */

			if(Rm_Shutdown_Req_Actual != Rm_Shutdown_Req) {
				return XST_FAILURE;
			}

			if(Rm_Decouple_Actual != Rm_Decouple) {
				return XST_FAILURE;
			}

			if(Rm_Reset_Actual != Rm_Reset) {
				return XST_FAILURE;
			}

			if(Sw_Startup_Req_Actual != Sw_Startup_Req) {
				return XST_FAILURE;
			}

			if(Sw_Shutdown_Req_Actual != Sw_Shutdown_Req) {
				return XST_FAILURE;
			}
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is used to test the Restart with Status Command.
* Flip the VSM state and set a new RM. Check the status is as expected.
* Flip the VSM state back to what it was and set the original RM. Check the
* status is as expected
*
* @param	VsmId is the identifier of the VSM to access.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		None.
*
******************************************************************************/
u32 Xprc_Check_RestartWithStatus_Command(u16 VsmId)
{
	s32 IntitialRmId;
	s32 RmId;
	s32 RmId_New;
	u8  State;
	u32 Status;
	u8  NewState;
	u8  ExpectedState;
	char strOppositeState[6];
	char strExpectedState[6];
	u8 Phase;


	for (VsmId = 0; VsmId < XPrc_GetNumberOfVsms(&Prc); VsmId++) {

		/*
		 * Make sure the VSM is active so I can read the empty/full
		 * state
		 */
		XPrc_SendRestartWithNoStatusCommand(&Prc, VsmId);
		while(XPrc_IsVsmInShutdown(&Prc, VsmId) == 1);

		/* Get the currently loaded RM (if any) */
		Status = XPrc_ReadStatusReg(&Prc, VsmId);
		IntitialRmId = XPrc_GetRmIdFromStatus(NULL, Status);
		State = XPrc_GetVsmState(NULL, Status);

		/* Now shut the VSM down */
		XPrc_SendShutdownCommand(&Prc, VsmId);
		while(XPrc_IsVsmInShutdown(&Prc, VsmId) == 0);

		/*
		 * Get the currently loaded RM and make sure it's the same as
		 * it was when the VSM was active. Shutting it down shouldn't
		 * have changed that.
		 */
		Status = XPrc_ReadStatusReg    (&Prc, VsmId);
		RmId   = XPrc_GetRmIdFromStatus(NULL, Status);

		if(RmId != IntitialRmId) {
			return XST_FAILURE;
		}

		for (Phase = 0; Phase < 2; Phase++) {
			if (Phase == 0) {
			/* *************** Phase 1 ***************
			 * Invert the state of the VSM and give it a new RmId
			 * regardless of the state.
			 */
			RmId_New = IntitialRmId + 1;
				if (RmId_New >= XPrc_GetNumRms(&Prc, VsmId)) {
					RmId_New = 0;
				}
			} else {

				/* *************** Phase 2 ***************
				 * Restore the initial state of the VSM and
				 * restore the RmId it originally had
				 */
				RmId_New = IntitialRmId;
			}

			if(State == XPRC_SR_STATE_FULL) {
				/* The PRC is full, so restart it empty */
				NewState      = XPRC_CR_VS_EMPTY;
				ExpectedState = XPRC_SR_STATE_EMPTY;
				strcpy(strExpectedState, "Empty");
				strcpy(strOppositeState, "Full");
			} else {
				/* The PRC was empty, so restart it full */
				NewState      = XPRC_CR_VS_FULL;
				ExpectedState = XPRC_SR_STATE_FULL;
				strcpy(strExpectedState, "Full");
				strcpy(strOppositeState, "Empty");
			}

			XPrc_SendRestartWithStatusCommand(&Prc, VsmId,
				NewState, RmId_New);
			while(XPrc_IsVsmInShutdown(&Prc, VsmId) == 1);

			Status = XPrc_ReadStatusReg(&Prc, VsmId);
			RmId   = XPrc_GetRmIdFromStatus(NULL, Status);
			State  = XPrc_GetVsmState      (NULL, Status);

			if(State != ExpectedState) {
				return XST_FAILURE;
			}

			if(RmId != RmId_New) {
				return XST_FAILURE;
			}

			/* Shut it down again */
			XPrc_SendShutdownCommand(&Prc, VsmId);
			while(XPrc_IsVsmInShutdown(&Prc, VsmId) == 0);
		}

	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is used to test that we can set/get the Trigger to RM mapping
* registers. For each trigger allocated, set the value of the RM it will load.
*
* @param	VsmId is the identifier of the VSM to access.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		The width of this register is set by the number of allocated
*		RMs, so we can't use any number greater than that or else it
*		will alias.
*
******************************************************************************/
u32 Xprc_Check_TriggerRmMapping(u16 VsmId)
{
	u16 TriggerId;	/* A loop variable used to iterate round triggers in PRC */
	u16 RmId;		/* A loop variable used to iterate round RMs in the PRC */
	u16 VsmId_tmp;	/* A loop variable used to iterate round VSMs in the PRC */
	u16 TriggerId_tmp;
					/* A loop variable used to iterate round triggers in PRC */
	u16 MaxRmId;


	/* Start by setting all of the triggers in all VSms to zero */
	for (VsmId = 0; VsmId < XPrc_GetNumberOfVsms(&Prc); VsmId++) {
		for (TriggerId = 0; TriggerId < XPrc_GetNumTriggersAllocated
		(&Prc, VsmId); TriggerId++) {
			XPrc_SetTriggerToRmMapping(&Prc, VsmId, TriggerId, 0);
		}
	}

	/* Check they are all zero in all VSMs */
	for (VsmId = 0; VsmId < XPrc_GetNumberOfVsms(&Prc); VsmId++) {
		for (TriggerId = 0; TriggerId < XPrc_GetNumTriggersAllocated
		(&Prc, VsmId); TriggerId++) {
			RmId = XPrc_GetTriggerToRmMapping(&Prc, VsmId,
				TriggerId);
			if (RmId != 0) {
				return XST_FAILURE;
			}
		}
	}

	/*
	 * Now check each trigger register can be updated without changing
	 * any other trigger register
	 */
	for (VsmId = 0; VsmId < XPrc_GetNumberOfVsms(&Prc); VsmId++) {
		MaxRmId = XPrc_GetNumRmsAllocated(&Prc, VsmId)-1;

		for (TriggerId = 0; TriggerId < XPrc_GetNumTriggersAllocated
		(&Prc, VsmId); TriggerId++) {
			XPrc_SetTriggerToRmMapping(&Prc, VsmId, TriggerId,
				MaxRmId);

			RmId = XPrc_GetTriggerToRmMapping(&Prc, VsmId,
				TriggerId);
			if (RmId != MaxRmId) {
				return XST_FAILURE;
			}

			/* Make sure all the rest are 0 */
			for (VsmId_tmp = 0; VsmId_tmp < XPrc_GetNumberOfVsms
			(&Prc); VsmId_tmp++) {
				for (TriggerId_tmp = 0; TriggerId_tmp <
				XPrc_GetNumTriggersAllocated(&Prc,
				VsmId_tmp); TriggerId_tmp++) {
					if (VsmId_tmp == VsmId && TriggerId_tmp
					== TriggerId) {
						continue;
					}
					RmId = XPrc_GetTriggerToRmMapping(&Prc,
						VsmId_tmp, TriggerId_tmp);
					if (RmId != 0) {
						return XST_FAILURE;
					}
				}
			}
			/* Now set this one back to 0 */
			XPrc_SetTriggerToRmMapping(&Prc, VsmId, TriggerId, 0);
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is used to test that we can set/get the BS Index registers.
*
* @param	VsmId is the identifier of the VSM to access.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		None.
*
******************************************************************************/
u32 Xprc_Check_BsIndex_Reg(u16 VsmId)
{
	u16 RmId;	/* A loop variable used to iterate round RMs in the PRC */
	u16 VsmId_tmp;
	u16 RmId_tmp;
	u16 BsIndex;
	u16 MaxRmId;

	/*
	 * The maximum value for a BsIndex register is the number of RMs
	 * allocated -1.
	 * We can't use any number greater than that or else it will alias.
	 *
	 * Start by setting all of the BS Index registers in all VSms to zero
	 */
	for (VsmId = 0; VsmId < XPrc_GetNumberOfVsms(&Prc); VsmId++) {
		for (RmId = 0; RmId < XPrc_GetNumRmsAllocated(&Prc,VsmId);
		RmId++) {
			XPrc_SetRmBsIndex(&Prc, VsmId, RmId, 0);
		}
	}

	/* Check they are all zero in all VSMs */
	for (VsmId = 0; VsmId < XPrc_GetNumberOfVsms(&Prc); VsmId++) {
		for (RmId = 0; RmId < XPrc_GetNumRmsAllocated(&Prc, VsmId);
		RmId++) {
			BsIndex = XPrc_GetRmBsIndex(&Prc, VsmId, RmId);
			if (BsIndex != 0) {
				return XST_FAILURE;
			}
		}
	}

	/*
	 * Now check each BsIndex can be updated without changing any
	 * other BsIndex register
	 */
	for (VsmId = 0; VsmId < XPrc_GetNumberOfVsms(&Prc); VsmId++) {
		MaxRmId = XPrc_GetNumRmsAllocated(&Prc, VsmId)-1;
		for (RmId = 0; RmId < XPrc_GetNumRmsAllocated(&Prc, VsmId);
		RmId++) {
			XPrc_SetRmBsIndex(&Prc, VsmId, RmId, MaxRmId);
			BsIndex = XPrc_GetRmBsIndex(&Prc, VsmId, RmId);
			if (BsIndex != MaxRmId) {
				return XST_FAILURE;
			}

			/* Make sure all the rest are 0 */
			for (VsmId_tmp = 0; VsmId_tmp < XPrc_GetNumberOfVsms
			(&Prc); VsmId_tmp++) {
				for (RmId_tmp = 0; RmId_tmp <
				XPrc_GetNumRmsAllocated(&Prc, VsmId_tmp);
				RmId_tmp++) {
					if (VsmId_tmp == VsmId && RmId_tmp
					== RmId) {
						continue;
					}
					BsIndex = XPrc_GetRmBsIndex(&Prc,
						VsmId_tmp, RmId_tmp);
						if (BsIndex != 0) {
							return XST_FAILURE;
						}
				}
			}
			/* Now set this one back to 0 */
			XPrc_SetRmBsIndex(&Prc, VsmId, RmId, 0);
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is used to test that we can set/get the RM Control registers.
*
* @param	VsmId is the identifier of the VSM to access.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		The RM Control register contains the following fields:
*		Name              | Min value  | Max value |
*		Shutdown Required |     0      |     3     |
*		Startup  Required |     0      |     1     |
*		Reset    Required |     0      |     3     |
*		Reset Duration    |     0      |   255     |
*
******************************************************************************/
u32 Xprc_Check_RmControl_Reg(u16 VsmId)
{
	u16 RmId;	/* A loop variable used to iterate round RMs in the PRC */
	u16 VsmId_tmp;
	u16 RmId_tmp;
	u8 ShutdownRequired;
	u8 StartupRequired;
	u8 ResetRequired;
	u8 ResetDuration;

	/*
	 * Start by setting all of the Rm Control registers in all VSms
	 * to zero
	 */
	for (VsmId = 0; VsmId < XPrc_GetNumberOfVsms(&Prc); VsmId++) {
		for (RmId = 0; RmId < XPrc_GetNumRmsAllocated(&Prc, VsmId);
		RmId++) {
			XPrc_SetRmControl(&Prc, VsmId, RmId, 0, 0, 0, 0);
		}
	}

	/* Check they are all zero in all VSMs */
	for (VsmId = 0; VsmId < XPrc_GetNumberOfVsms(&Prc); VsmId++) {
		for (RmId = 0; RmId < XPrc_GetNumRmsAllocated(&Prc, VsmId);
		RmId++) {
			XPrc_GetRmControl(&Prc, VsmId, RmId, &ShutdownRequired,
			&StartupRequired, &ResetRequired, &ResetDuration);

			if (ShutdownRequired != 0) {
				return XST_FAILURE;
			}

			if (StartupRequired != 0) {
				return XST_FAILURE;
			}

			if (ResetRequired != 0) {
				return XST_FAILURE;
			}

			if (ResetDuration != 0) {
				return XST_FAILURE;
			}
		}
	}

	/*
	 * Now check each RM Control register can be updated without changing
	 * any other RM Control register
	 */
	for (VsmId = 0; VsmId < XPrc_GetNumberOfVsms(&Prc); VsmId++) {
		for (RmId = 0; RmId < XPrc_GetNumRmsAllocated(&Prc, VsmId);
		RmId++) {
			XPrc_SetRmControl(&Prc, VsmId, RmId, 3, 1, 2, 255);
			XPrc_GetRmControl(&Prc, VsmId, RmId, &ShutdownRequired,
			&StartupRequired, &ResetRequired, &ResetDuration);

			if (ShutdownRequired != 3) {
				return XST_FAILURE;
			}

			if (StartupRequired != 1) {
				return XST_FAILURE;
			}

			if (ResetRequired != 2) {
				return XST_FAILURE;
			}

			if (ResetDuration != 255) {
				return XST_FAILURE;
			}

			/*  Make sure all the rest are 0 */
			for (VsmId_tmp = 0; VsmId_tmp < XPrc_GetNumberOfVsms
			(&Prc); VsmId_tmp++) {
				for (RmId_tmp = 0; RmId_tmp <
				XPrc_GetNumRmsAllocated(&Prc, VsmId_tmp);
				RmId_tmp++) {
					if (VsmId_tmp == VsmId && RmId_tmp ==
					RmId) {
						continue;
					}
					XPrc_GetRmControl(&Prc, VsmId_tmp,
					RmId_tmp, &ShutdownRequired,
					&StartupRequired, &ResetRequired,
					&ResetDuration);

					if (ShutdownRequired != 0) {
						return XST_FAILURE;
					}

					if (StartupRequired != 0) {
						return XST_FAILURE;
					}

					if (ResetRequired != 0) {
						return XST_FAILURE;
					}

					if (ResetDuration != 0) {
						return XST_FAILURE;
					}
				}
			}
			/* Now set this one back to 0 */
			XPrc_SetRmControl(&Prc, VsmId, RmId, 0, 0, 0, 0);
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is used to test that we can set/get the BS Size registers.
*
* @param	VsmId is the identifier of the VSM to access.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		None.
*
******************************************************************************/
u32 Xprc_Check_BsSize_Reg(u16 VsmId)
{
	u16 RmId;	/* A loop variable used to iterate round RMs in the PRC */
	u16 VsmId_tmp;
	u16 RmId_tmp;
	u32 BsSize;
	u32 MaxBsSize = -1;

	/* Start by setting all of the BS Size registers in all VSms to zero */
	for (VsmId = 0; VsmId < XPrc_GetNumberOfVsms(&Prc); VsmId++) {
		for (RmId = 0; RmId < XPrc_GetNumRmsAllocated(&Prc, VsmId);
		RmId++) {
			XPrc_SetBsSize(&Prc, VsmId, RmId, 0);
		}
	}

	/* Check they are all zero in all VSMs */
	for (VsmId = 0; VsmId < XPrc_GetNumberOfVsms(&Prc); VsmId++) {
		for (RmId = 0; RmId < XPrc_GetNumRmsAllocated(&Prc, VsmId);
		RmId++) {
			BsSize = XPrc_GetBsSize(&Prc, VsmId, RmId);
			if (BsSize != 0) {
				return XST_FAILURE;
			}
		}
	}

	/*
	 * Now check each BsSize can be updated without changing any other
	 * BsSize register
	 */
	for (VsmId = 0; VsmId < XPrc_GetNumberOfVsms(&Prc); VsmId++) {
		for (RmId = 0; RmId < XPrc_GetNumRmsAllocated(&Prc, VsmId);
		RmId++) {
			XPrc_SetBsSize(&Prc, VsmId, RmId, MaxBsSize);

			BsSize = XPrc_GetBsSize(&Prc, VsmId, RmId);
			if (BsSize != MaxBsSize) {
				return XST_FAILURE;
			}

			/* Make sure all the rest are 0 */
			for (VsmId_tmp = 0; VsmId_tmp < XPrc_GetNumberOfVsms
			(&Prc); VsmId_tmp++) {
				for (RmId_tmp = 0; RmId_tmp <
				XPrc_GetNumRmsAllocated(&Prc, VsmId_tmp);
				RmId_tmp++) {
					if (VsmId_tmp == VsmId && RmId_tmp
					== RmId) {
						continue;
					}
					BsSize = XPrc_GetBsSize(&Prc,
						VsmId_tmp, RmId_tmp);
					if (BsSize != 0) {
						return XST_FAILURE;
					}
				}
			}
			/* Now set this one back to 0 */
			XPrc_SetBsSize(&Prc, VsmId, RmId, 0);
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is used to test that we can set/get the BS Address registers.
*
* @param	VsmId is the identifier of the VSM to access.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		None.
*
******************************************************************************/
u32 Xprc_Check_BsAddress_Reg(u16 VsmId)
{
	u16 RmId;	/* A loop variable used to iterate round RMs in the PRC */
	u16 VsmId_tmp;
	u16 RmId_tmp;
	u32 BsAddress;
	u32 MaxBsAddress = -1;

	/*
	 * Start by setting all of the BS Address registers in all VSms to
	 * zero
	 */
	for (VsmId = 0; VsmId < XPrc_GetNumberOfVsms(&Prc); VsmId++) {
		for (RmId = 0; RmId < XPrc_GetNumRmsAllocated(&Prc, VsmId);
		RmId++) {
			XPrc_SetBsAddress(&Prc, VsmId, RmId, 0);
		}
	}

	/* Check they are all zero in all VSMs */
	for (VsmId = 0; VsmId < XPrc_GetNumberOfVsms(&Prc); VsmId++) {
		for (RmId = 0; RmId < XPrc_GetNumRmsAllocated(&Prc, VsmId);
		RmId++) {
			BsAddress = XPrc_GetBsAddress(&Prc, VsmId, RmId);
			if (BsAddress != 0) {
				return XST_FAILURE;
			}
		}
	}

	/*
	 * Now check each BsAddress can be updated without changing any other
	 * BsAddress register
	 */
	for (VsmId = 0; VsmId < XPrc_GetNumberOfVsms(&Prc); VsmId++) {
		for (RmId = 0; RmId < XPrc_GetNumRmsAllocated(&Prc, VsmId);
		RmId++) {
			XPrc_SetBsAddress(&Prc, VsmId, RmId, MaxBsAddress);

			BsAddress = XPrc_GetBsAddress(&Prc, VsmId, RmId);
			if (BsAddress != MaxBsAddress) {
				return XST_FAILURE;
			}

			/* Make sure all the rest are 0 */
			for (VsmId_tmp = 0; VsmId_tmp < XPrc_GetNumberOfVsms
			(&Prc); VsmId_tmp++) {
				for (RmId_tmp = 0; RmId_tmp <
				XPrc_GetNumRmsAllocated(&Prc, VsmId_tmp);
				RmId_tmp++) {
					if (VsmId_tmp == VsmId && RmId_tmp ==
					RmId) {
						continue;
					}
					BsAddress = XPrc_GetBsAddress(&Prc,
						VsmId_tmp, RmId_tmp);
					if (BsAddress != 0) {
						return XST_FAILURE;
					}
				}
			}
		/* Now set this one back to 0 */
		XPrc_SetBsAddress(&Prc, VsmId, RmId, 0);
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is used to Program the PRC.
*
* @param	VsmId is the identifier of the VSM to access.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		For each allocated RM in each VSM, generate a partial bitstream
*		and store it in memory. Program the PRC to point at it.
*
******************************************************************************/
u32 Xprc_Program_PRC(u16 VsmId)
{
	u16 RmId;	/* A loop variable used to iterate round RMs in the PRC */
	u32 Bitstream_Dummy[28];
	u32 *ptr;

	for (VsmId = 0; VsmId < XPrc_GetNumberOfVsms(&Prc); VsmId++) {
		for (RmId = 0; RmId < XPrc_GetNumRms(&Prc, VsmId); RmId++) {
			u32 id = 0;
			u32 NumWordsInBS = 28;
			u32 BsSize = NumWordsInBS*4;
			ptr =  Bitstream_Dummy;
			if (ptr == NULL) {
				return XST_FAILURE;
			}

			id = (VsmId << 16 ) | RmId;

			/*
			 * A bitstream looks like this
			 *   ffff ffff   : Dummy word
			 *   0000 00bb   : Bus Width Sync Word
			 *   1122 0044   : Bus Width Detect
			 *   ffff ffff   : Dummy word
			 *   ffff ffff   : Dummy word
			 *   aa99 5566   : Sync Word
			 *   2000 0000   : NOOP
			 *   3001 a001   : Write 1 word to user access
			 *   <  id   >   :
			 *   2000 0000   : NOOP
			 *   3000 8001   : Desync command
			 *   0000 000d   : Desync command
			 *   2000 0000   : NOOP
			 */

			ptr[0] = 0xffffffff;
			ptr[1] = 0x000000bb;
			ptr[2] = 0x11220044;
			ptr[3] = 0xffffffff;
			ptr[4] = 0xffffffff;
			ptr[5] = 0xaa995566;
			ptr[6] = 0x20000000;
			ptr[7] = 0x3001a001;
			ptr[8] = id;
			ptr[9] = 0x20000000;
			ptr[10] = 0x30008001;
			ptr[11] = 0x0000000d;
			ptr[12] = 0x20000000;
			ptr[13] = 0x20000000;
			ptr[14] = 0x20000000;
			ptr[15] = 0x20000000;
			ptr[16] = 0x20000000;
			ptr[17] = 0x20000000;
			ptr[18] = 0x20000000;
			ptr[19] = 0x20000000;
			ptr[20] = 0x20000000;
			ptr[21] = 0x20000000;
			ptr[22] = 0x20000000;
			ptr[23] = 0x20000000;
			ptr[24] = 0x20000000;
			ptr[25] = 0x20000000;
			ptr[26] = 0x20000000;
			ptr[27] = 0x20000000;

			XPrc_SetRmControl(&Prc, VsmId, RmId, 0, 0, 0, 0);

			/*
			 * This is a 7 series project so BsIndex is always
			 * the RmId.  It will have been set correctly when
			 * the core was configured, but we'll set it here
			 * because we distrubed it above in the register test
			 */
			XPrc_SetRmBsIndex(&Prc, VsmId, RmId, RmId);

			XPrc_SetBsSize   (&Prc, VsmId, RmId, BsSize);
			XPrc_SetBsAddress(&Prc, VsmId, RmId, (u32)ptr);
		}
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is used to Restart the VSMs.
*
* @param	VsmId is the identifier of the VSM to access.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		None.
*
******************************************************************************/
void Xprc_Restart_VSMs(u16 VsmId)
{
	for (VsmId = 0; VsmId < XPrc_GetNumberOfVsms(&Prc); VsmId++) {
		XPrc_SendRestartWithNoStatusCommand(&Prc, VsmId);
		/* Wait until it has left the shutdown state */
		while(XPrc_IsVsmInShutdown(&Prc, VsmId) == 1);
	}
}