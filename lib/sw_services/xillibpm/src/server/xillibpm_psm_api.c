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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 *
 ******************************************************************************/

#include "xillibpm_defs.h"
#include "xillibpm_psm_api.h"
#include "xplmi_modules.h"
#include "xpm_common.h"
#include "xpm_core.h"
#include "xpm_device.h"
#include "xpm_ipi.h"

static XPlmi_ModuleCmd XPlmi_PsmCmds[PSM_API_MAX+1];
static XPlmi_Module XPlmi_Psm =
{
	XPLMI_MODULE_XILPSM_ID,
	XPlmi_PsmCmds,
	PSM_API_MAX+1,
};

static int XPm_ProcessPsmCmd(XPlmi_Cmd * Cmd)
{
	u32 Status = XST_SUCCESS;
	u32 *Pload = Cmd->Payload;

	PmDbg("Processing Cmd %x\n\r", Cmd->CmdId);

	switch (Cmd->CmdId & 0xFF) {
		case PM_PWR_DWN_EVENT:
			Status = XPm_PwrDwnEvent(Pload[0]);
			break;
		case PM_WAKE_UP_EVENT:
			Status = XPm_WakeUpEvent(Pload[0]);
			break;
		default:
			Status = XST_INVALID_PARAM;
			break;
	}

	Cmd->Response[0] = Status;

	if (XST_SUCCESS == Status) {
		Cmd->ResumeHandler = NULL;
	} else {
		PmErr("Error %d while processing command 0x%x\r\n", Status, Cmd->CmdId);
		PmDbg("Command payload: 0x%x, 0x%x, 0x%x, 0x%x\r\n",
				Pload[0], Pload[1], Pload[2], Pload[3]);
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief	Initialize PSM module which processes IPI from PSM
 *
 * @param	None
 *
 * @return	None
 *
 * @note	None
 *
 ****************************************************************************/
void XPm_PsmModuleInit(void)
{
	u32 Idx;

	for (Idx = 1; Idx < XPlmi_Psm.CmdCnt; Idx++) {
		XPlmi_PsmCmds[Idx].Handler = XPm_ProcessPsmCmd;
	}
	XPlmi_ModuleRegister(&XPlmi_Psm);
}

/****************************************************************************/
/**
 * @brief This Function will power up processor by sending IPI to PSM for
 *       performing direct power up operation.
 *
 * @param DeviceId	Device ID of processor
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code.
 *
 * @note none
 *
 ****************************************************************************/
XStatus XPm_DirectPwrUp(const u32 DeviceId)
{
	XStatus Status = XST_SUCCESS;
	u32 Payload[PAYLOAD_ARG_CNT];

	Payload[0] = PSM_API_DIRECT_PWR_UP;
	Payload[1] = DeviceId;

	Status = XPm_IpiSend(PSM_IPI_INT_MASK, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPm_IpiReadStatus(PSM_IPI_INT_MASK);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief This Function will power down processor by sending IPI to PSM for
 *       performing direct power down operation.
 *
 * @param DeviceId	Device ID of processor
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code.
 *
 * @note none
 *
 ****************************************************************************/
XStatus XPm_DirectPwrDwn(const u32 DeviceId)
{
	XStatus Status = XST_SUCCESS;
	u32 Payload[PAYLOAD_ARG_CNT];

	Payload[0] = PSM_API_DIRECT_PWR_DWN;
	Payload[1] = DeviceId;

	Status = XPm_IpiSend(PSM_IPI_INT_MASK, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPm_IpiReadStatus(PSM_IPI_INT_MASK);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief This Function is called by PSM to perform actions to finish suspend
 *       procedur of processor.
 *
 * @param DeviceId	Device ID of processor
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code.
 *
 * @note none
 *
 ****************************************************************************/
XStatus XPm_PwrDwnEvent(const u32 DeviceId)
{
	XStatus Status = XST_SUCCESS;
	XPm_Core *Core;
	XPm_Subsystem *Subsystem;
	u32 SubsystemId;

	if ((XPM_NODECLASS_DEVICE != NODECLASS(DeviceId)) ||
	    (XPM_NODESUBCL_DEV_CORE != NODESUBCLASS(DeviceId))) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Core = (XPm_Core *)XPmDevice_GetById(DeviceId);

	if (XPM_DEVSTATE_SUSPENDING != Core->Device.Node.State) {
		Status = XST_FAILURE;
		goto done;
	}

	if (NULL != Core->CoreOps->PowerDown) {
		Status = Core->CoreOps->PowerDown(Core);
	}

	SubsystemId = XPmDevice_GetSubsystemIdOfCore((XPm_Device *)Core);

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XST_FAILURE;
		goto done;
	}

	if (SUSPENDING == Subsystem->State) {
		Status = XPmRequirement_UpdateScheduled(Subsystem, TRUE);

		XPmSubsystem_SetState(SubsystemId, OFFLINE);
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief This Function is called by PSM to wake processor.
 *
 * @param DeviceId	Device ID of processor
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code.
 *
 * @note none
 *
 ****************************************************************************/
XStatus XPm_WakeUpEvent(const u32 DeviceId)
{
	return XPm_RequestWakeUp(XPM_SUBSYSID_PMC, DeviceId, 0, 0, 0);
}
