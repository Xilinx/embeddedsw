/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "sleep.h"
#include "xil_util.h"
#include "xplmi.h"
#include "xpm_common.h"
#include "xpm_rail.h"
#include "xpm_powerdomain.h"
#include "xpm_debug.h"
#include "xpm_pmc.h"
#include "xpm_node.h"
#include "xpm_regs.h"
#include "xpm_power.h"
#include "xpm_regulator.h"
#include "xplmi_sysmon.h"
#include "xplmi_scheduler.h"

#if defined (XPAR_XIICPS_0_DEVICE_ID) || defined (XPAR_XIICPS_1_DEVICE_ID) || \
    defined (XPAR_XIICPS_2_DEVICE_ID) || defined (XPAR_XIICPS_0_BASEADDR)  || \
    defined (XPAR_XIICPS_1_BASEADDR)  || defined (XPAR_XIICPS_2_BASEADDR)
#include "xiicps.h"

#define IIC_SCLK_RATE		400000

XIicPs *XPmRail_GetIicInstance(void)
{
	static XIicPs IicInstance;

	return &IicInstance;
}

XStatus I2CInitialize(XIicPs *Iic, const u32 ControllerID)
{
	XStatus Status = XST_FAILURE;
	XIicPs_Config *Config;
	const XPm_Device *Device;
	u16 I2CDeviceId;

	Device = XPmDevice_GetById(ControllerID);
	if (NULL == Device) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	/* Request the I2C controller */
	Status = XPm_RequestDevice(PM_SUBSYS_PMC, ControllerID,
				   (u32)PM_CAP_ACCESS, XPM_MAX_QOS, 0,
				   XPLMI_CMD_SECURE);
	if (XST_SUCCESS != Status) {
		goto done;
	}

#ifndef SDT
#ifdef XPAR_XIICPS_0_DEVICE_ID
	if ((u32)XPAR_XIICPS_0_BASEADDR == Device->Node.BaseAddress) {
		I2CDeviceId = XPAR_XIICPS_0_DEVICE_ID;
	} else
#endif
#ifdef XPAR_XIICPS_1_DEVICE_ID
	if ((u32)XPAR_XIICPS_1_BASEADDR == Device->Node.BaseAddress) {
		I2CDeviceId = XPAR_XIICPS_1_DEVICE_ID;
	} else
#endif
#ifdef XPAR_XIICPS_2_DEVICE_ID
	if ((u32)XPAR_XIICPS_2_BASEADDR == Device->Node.BaseAddress) {
		I2CDeviceId = XPAR_XIICPS_2_DEVICE_ID;
	} else
#endif
	{
		Status = XST_FAILURE;
		goto done;
	}

	Config = XIicPs_LookupConfig(I2CDeviceId);
#else
	Config = XIicPs_LookupConfig(Device->Node.BaseAddress);
#endif
	if (NULL == Config) {
		Status = XST_FAILURE;
		goto done;
	}

	Status = XIicPs_CfgInitialize(Iic, Config, Config->BaseAddress);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	XIicPs_Reset(Iic);

	/* Set the I2C serial clock rate */
	Status = XIicPs_SetSClk(Iic, IIC_SCLK_RATE);

done:
	return Status;
}
#endif /* XPAR_XIICPS_0_DEVICE_ID || XPAR_XIICPS_1_DEVICE_ID || \
	  XPAR_XIICPS_2_DEVICE_ID || XPAR_XIICPS_0_BASEADDR  || \
	  XPAR_XIICPS_1_BASEADDR  || XPAR_XIICPS_2_BASEADDR */

#if defined (RAIL_CONTROL)
#if defined (XPAR_XIICPS_0_DEVICE_ID) || defined (XPAR_XIICPS_1_DEVICE_ID) || \
    defined (XPAR_XIICPS_2_DEVICE_ID) || defined (XPAR_XIICPS_0_BASEADDR)  || \
    defined (XPAR_XIICPS_1_BASEADDR)  || defined (XPAR_XIICPS_2_BASEADDR)
XStatus I2CIdleBusWait(XIicPs *Iic)
{
	XStatus Status = XST_FAILURE;
	u32 Timeout;

	Timeout = 100000;
	while (0 != XIicPs_BusIsBusy(Iic)) {
		usleep(10);
		Timeout--;

		if (0U == Timeout) {
			PmErr("ERROR: I2C bus idle wait timeout\r\n");
			goto done;
		}
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus I2CWrite(XIicPs *Iic, u16 SlaveAddr, u8 *Buffer, s32 ByteCount)
{
	XStatus Status = XST_FAILURE;

	/* Continuously try to send in case of arbitration */
	do {
		if (0 != Iic->IsRepeatedStart) {
			Status = I2CIdleBusWait(Iic);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}

		Status = XIicPs_MasterSendPolled(Iic, Buffer, ByteCount, SlaveAddr);
	} while (XST_IIC_ARB_LOST == Status);

	if (XST_SUCCESS != Status) {
		PmErr("I2C write failure\r\n");
	}

done:
	return Status;
}

static XStatus XPmRail_PMBusControl(const XPm_Rail *Rail, u8 Mode)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	XIicPs *Iic;
	u8 WriteBuffer[3] = {0};
	const XPm_Regulator *Regulator;
	u16 RegulatorSlaveAddress, MuxAddress;
	u32 i = 0, j = 0, k = 0, BytesLen = 0, ByteIndex = 0;
	u32 ControllerID;
	u8 PI;

	PI = Rail->ParentIndex;
	Regulator = (XPm_Regulator *)XPmRegulator_GetById(Rail->ParentIds[PI]);
	if (NULL == Regulator) {
		DbgErr = XPM_INT_ERR_INVALID_ARGS;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Iic = XPmRail_GetIicInstance();
	if ((u32)XIL_COMPONENT_IS_READY != Iic->IsReady) {
		ControllerID = Regulator->Cntrlr[XPM_I2C_CNTRLR]->Node.Id;
		Status = I2CInitialize(Iic, ControllerID);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_I2C_INIT;
			goto done;
		}
	}

	RegulatorSlaveAddress = (u16)Regulator->I2cAddress;
	for (i = 0; i < Regulator->Config.CmdLen; i++) {
		MuxAddress = (u16)Regulator->Config.CmdArr[j];
		j++;
		BytesLen = Regulator->Config.CmdArr[j];

		/*
		 * First 2 bytes of a word are Mux address and length,
		 * the I2C payload should not be more than 2 bytes.
		 */
		if (BytesLen > 2u) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}

		j++;
		for (k = 0; k < BytesLen; k++) {
			WriteBuffer[k] = Regulator->Config.CmdArr[j];
			j++;
		}

		if (j >= (u32)MAX_I2C_COMMAND_LEN) {
			Status = XST_INVALID_PARAM;
			goto done;
		}

		Status = I2CWrite(Iic, MuxAddress, WriteBuffer, (s32)BytesLen);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_I2C_WRITE;
			goto done;
		}
	}

	Status = XST_FAILURE;
	while (ByteIndex < ((u32)Rail->I2cModes[PI][Mode].CmdLen * 4U)) {
		BytesLen = Rail->I2cModes[PI][Mode].CmdArr[ByteIndex];

		/*
		 * If the next BytesLen is 0, it means we are in the middle of
		 * last payload word with no more I2C command to process.
		 */
		if (0U == BytesLen) {
			break;
		}

		if (BytesLen > 3u) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}

		ByteIndex++;
		for (k = 0; k < BytesLen; k++) {
			WriteBuffer[k] = Rail->I2cModes[PI][Mode].CmdArr[ByteIndex];
			ByteIndex++;
		}

		Status = I2CWrite(Iic, RegulatorSlaveAddress, WriteBuffer, (s32)BytesLen);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_I2C_WRITE;
			goto done;
		}
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
#else
static XStatus XPmRail_PMBusControl(XPm_Rail *Rail, u8 Mode)
{
	(void) Rail;
	(void) Mode;

	return XST_SUCCESS;
}
#endif /* XPAR_XIICPS_0_DEVICE_ID || XPAR_XIICPS_1_DEVICE_ID || \
	  XPAR_XIICPS_2_DEVICE_ID || XPAR_XIICPS_0_BASEADDR  || \
	  XPAR_XIICPS_1_BASEADDR  || XPAR_XIICPS_2_BASEADDR */

#if defined (XPAR_XGPIOPS_0_DEVICE_ID) || defined (XPAR_XGPIOPS_1_DEVICE_ID)
static XStatus XPmRail_GPIOControl(const XPm_Rail *Rail, u8 Mode)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_Regulator *Regulator;
	XPm_CntrlrType Type;
	XPm_Node Controller;
	u16 Offset;
	u32 Mask, Value;
	u8 Found = 0;
	static u8 GPIOCntlrInitialized;

	/*
	 * Look for GPIO meta-data required to transition the rail to the new
	 * power mode.
	 */
	for (u8 i = 0; i < MAX_MODES; i++) {
		if (Rail->GPIOModes[i].ModeNumber == Mode) {
			Offset = Rail->GPIOModes[i].Offset;
			Mask = Rail->GPIOModes[i].Mask;
			Value = Rail->GPIOModes[i].Value;
			Found = 1;
			break;
		}
	}

	if (1U != Found) {
		DbgErr = XPM_INT_ERR_INVALID_ARGS;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Regulator = (XPm_Regulator *)XPmRegulator_GetById(Rail->ParentIds[0]);
	if (NULL == Regulator) {
		DbgErr = XPM_INT_ERR_INVALID_ARGS;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Type = XPM_GPIO_CNTRLR;
	Controller = Regulator->Cntrlr[Type]->Node;

	/*
	 * On the first invocation of this routine, we need to request the
	 * GPIO controller to be assigned to the PMC subsystem.
	 */
	if (0U == GPIOCntlrInitialized) {
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, Controller.Id,
					   (u32)PM_CAP_ACCESS, XPM_MAX_QOS, 0,
					   XPLMI_CMD_SECURE);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_DEVICE_REQUEST;
			goto done;
		}

		GPIOCntlrInitialized = 1;
	}

	/*
	 * Set GPIO lines.  If GPIO lines of this controller is shared with
	 * other applications, the Offset value should point to its 'Maskable
	 * Output Data' register so the hardware could guarantee its atomicity.
	 */
	XPm_RMW32(Controller.BaseAddress + Offset, Mask, Value);
	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
#else
static XStatus XPmRail_GPIOControl(XPm_Rail *Rail, u8 Mode)
{
	(void) Rail;
	(void) Mode;

	return XST_SUCCESS;
}
#endif /* XPAR_XGPIOPS_0_DEVICE_ID || XPAR_XGPIOPS_1_DEVICE_ID */

static XStatus WaitForPowerRailUp(u32 VoltageRailMask)
{
	XStatus Status = XST_FAILURE;
	XPm_Pmc *Pmc;

	Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);
	if (NULL == Pmc) {
		goto done;
	}

	Status = XPlmi_UtilPollForMask((Pmc->PmcGlobalBaseAddr +
					PWR_SUPPLY_STATUS_OFFSET),
					VoltageRailMask,
					XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		PmErr("Poll for power rail up timeout\r\n");
		goto done;
	}

	/*
	 * TODO: A delay is needed for a power rail to stabilize.
	 * The value of this delay is board-dependent and should
	 * come from cdo.
	 */
	usleep(1000U);

done:
	return Status;
}

XStatus XPmRail_Control(XPm_Rail *Rail, u8 State, u8 Mode)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 NodeIdx = NODEINDEX(Rail->Power.Node.Id);
	u8 PI;

	if (Mode >= MAX_MODES) {
		DbgErr = XPM_INT_ERR_INVALID_PARAM;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Power state transition is either to ON or OFF */
	if (((u8)XPM_POWER_STATE_ON != State) &&
	    ((u8)XPM_POWER_STATE_OFF != State)) {
		DbgErr = XPM_INT_ERR_INVALID_PARAM;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/*
	 * Mode 0 is reserved for turning the power rail OFF and mode 1
	 * is reserved for turning the power rail ON.  For other modes,
	 * the power state remains in XPM_POWER_STATE_ON.
	 */
	if (((0U == Mode) || (1U == Mode)) &&
	    (State == Rail->Power.Node.State)) {
		Status = XST_SUCCESS;
		goto done;
	}

	/*
	 * For any mode greater than 1, only proceed if the current power state
	 * is in XPM_POWER_STATE_ON.
	 */
	if ((1U < Mode) && ((u8)XPM_POWER_STATE_ON != Rail->Power.Node.State)) {
		Status = XST_SUCCESS;
		goto done;
	}

	PI = Rail->ParentIndex;
	if (0U == Rail->ParentIds[PI]) {
		PmDbg("Rail topology information unavailable so rail can not be controlled.\r\n");
		Status = XST_SUCCESS;
		goto done;
	}

	if (XPM_RAILTYPE_MODE_PMBUS == Rail->ControlType[PI][Mode]) {
		Status = XPmRail_PMBusControl(Rail, Mode);
		if (XST_SUCCESS != Status) {
			goto done;
		}

	} else if (XPM_RAILTYPE_MODE_GPIO == Rail->ControlType[PI][Mode]) {
		Status = XPmRail_GPIOControl(Rail, Mode);
		if (XST_SUCCESS != Status) {
			goto done;
		}

	} else {
		Status = XST_SUCCESS;
		goto done;
	}

	/* TBD: Mask can come through topology */
	if ((u8)XPM_POWER_STATE_ON == State) {
		if ((u32)XPM_NODEIDX_POWER_VCCINT_PSFP == NodeIdx) {
			Status = WaitForPowerRailUp(PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_FPD_MASK);
		} else if ((u32)XPM_NODEIDX_POWER_VCCINT_PSLP == NodeIdx) {
			Status = WaitForPowerRailUp(PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_LPD_MASK);
		} else if ((u32)XPM_NODEIDX_POWER_VCCINT_PL == NodeIdx) {
			Status = WaitForPowerRailUp(PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_PL_MASK);
		} else if ((u32)XPM_NODEIDX_POWER_VCCINT_RAM == NodeIdx) {
			Status = WaitForPowerRailUp(PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_RAM_MASK);
		} else if ((u32)XPM_NODEIDX_POWER_VCCAUX == NodeIdx) {
			Status = WaitForPowerRailUp(PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCAUX_MASK);
		} else if ((u32)XPM_NODEIDX_POWER_VCCINT_SOC == NodeIdx) {
			Status = WaitForPowerRailUp(PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_SOC_MASK);
		} else {
			/* Required by MISRA */
		}
	}

	Rail->Power.Node.State = State;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
#else
XStatus XPmRail_Control(XPm_Rail *Rail, u8 State, u8 Mode)
{
	(void) Mode;

	Rail->Power.Node.State = State;
	return XST_SUCCESS;
}
#endif /* RAIL_CONTROL */

#if defined (VERSAL_DVS)
/*
 * This routine is invoked periodically by the task scheduler and its
 * task is to determine if a voltage adjustment is needed, based on current
 * temperature and current voltage level.  The argument passed points to power
 * rail that needs to be adjusted.
 */
static int XPmRail_CyclicTempVoltAdj(void *Arg)
{
	int Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	XPm_Rail *Rail = (XPm_Rail *)Arg;
	u32 UpperTempThresh, LowerTempThresh;
	u8 UpperVoltMode, LowerVoltMode;
	u32 CurrentTemp;
	u8 *CurrentVoltMode;
	XSysMonPsv *SysMonInstPtr = XPlmi_GetSysmonInst();

	/* Validate that the argument passed in is a power rail */
	if ((u32)XPM_NODETYPE_POWER_RAIL != NODETYPE(Rail->Power.Node.Id)) {
		DbgErr = XPM_INT_ERR_INVALID_ARGS;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	UpperTempThresh = Rail->TempVoltAdj->UpperTempThresh;
	LowerTempThresh = Rail->TempVoltAdj->LowerTempThresh;
	UpperVoltMode = Rail->TempVoltAdj->UpperVoltMode;
	LowerVoltMode = Rail->TempVoltAdj->LowerVoltMode;
	CurrentVoltMode = &Rail->TempVoltAdj->CurrentVoltMode;

	/*
	 * If Root SysMon is not initialized yet, skip the cycle until
	 * it is initialized.
	 */
	if (0U == SysMonInstPtr->Config.BaseAddress) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Read DEVICE_TEMP_MAX register through SysMon driver */
	CurrentTemp = XSysMonPsv_ReadDeviceTemp(SysMonInstPtr, XSYSMONPSV_VAL);

	/*
	 * If the current temperature is at or below lower threshold and
	 * we are not in upper voltage mode, make an adjustment to higher
	 * voltage.  Similarly, if the temperature is at or above upper
	 * threshold and not in lower voltage mode, make an adjustment
	 * to lower voltage.  If the temperature is between lower and upper
	 * threshold, no adjustment is needed.
	 */
	if ((XSysMonPsv_FixedToFloat(CurrentTemp) <=
	     XSysMonPsv_FixedToFloat(LowerTempThresh)) &&
	    (*CurrentVoltMode != UpperVoltMode)) {
		PmDbg("Current temperature is 0x%x, Set voltage to upper mode "
		      "%d\n\r", CurrentTemp, UpperVoltMode);
		Status = XPmRail_Control(Rail, (u8)XPM_POWER_STATE_ON,
					 UpperVoltMode);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_RAIL_UPPER_VOLT;
			goto done;
		}

		*CurrentVoltMode = UpperVoltMode;
	} else if ((XSysMonPsv_FixedToFloat(CurrentTemp) >=
		    XSysMonPsv_FixedToFloat(UpperTempThresh)) &&
		   (*CurrentVoltMode != LowerVoltMode)) {
		PmDbg("Current temperature is 0x%x, Set voltage to lower mode "
		      "%d\n\r", CurrentTemp, LowerVoltMode);
		Status = XPmRail_Control(Rail, (u8)XPM_POWER_STATE_ON,
					 LowerVoltMode);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_RAIL_LOWER_VOLT;
			goto done;
		}

		*CurrentVoltMode = LowerVoltMode;
	} else if ((XSysMonPsv_FixedToFloat(CurrentTemp) <
		    XSysMonPsv_FixedToFloat(UpperTempThresh)) &&
		   (XSysMonPsv_FixedToFloat(CurrentTemp) >
		    XSysMonPsv_FixedToFloat(LowerTempThresh)) &&
		   (0U == *CurrentVoltMode)) {
		PmDbg("Current temperature is 0x%x, Set voltage to lower mode "
		      "%d\n\r", CurrentTemp, LowerVoltMode);
		Status = XPmRail_Control(Rail, (u8)XPM_POWER_STATE_ON,
					 LowerVoltMode);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_RAIL_LOWER_VOLT;
			goto done;
		}

		*CurrentVoltMode = LowerVoltMode;
	} else {
		Status = XST_SUCCESS;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/*
 * This routine verifies whether the controllers (I2C and/or GPIO) that have
 * been specified in the CDO command responsible for controlling the power
 * rail have been enabled in the design.
 */
static XStatus XPmRail_VerifyController(const XPm_Rail *Rail)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_Regulator *Regulator;
	u8 CntrlrEnabled = 0;
	u8 PI;

	if (NULL == Rail) {
		DbgErr = XPM_INT_ERR_INVALID_ARGS;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	PI = Rail->ParentIndex;
	Regulator = (XPm_Regulator *)XPmRegulator_GetById(Rail->ParentIds[PI]);
	if (NULL == Regulator) {
		DbgErr = XPM_INT_ERR_INVALID_ARGS;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	for (u8 i = 0; i < (u8)XPM_MAX_NUM_CNTRLR; i++) {
		if (NULL == Regulator->Cntrlr[i]) {
			continue;
		}

		CntrlrEnabled = 0;
		if ((u8)XPM_I2C_CNTRLR == i) {
#ifdef XPAR_XIICPS_0_DEVICE_ID
			if ((u32)XPAR_XIICPS_0_BASEADDR ==
			    Regulator->Cntrlr[i]->Node.BaseAddress) {
				CntrlrEnabled = 1;
			} else
#endif
#ifdef XPAR_XIICPS_1_DEVICE_ID
			if ((u32)XPAR_XIICPS_1_BASEADDR ==
			    Regulator->Cntrlr[i]->Node.BaseAddress) {
				CntrlrEnabled = 1;
			} else
#endif
#ifdef XPAR_XIICPS_2_DEVICE_ID
			if ((u32)XPAR_XIICPS_2_BASEADDR ==
			    Regulator->Cntrlr[i]->Node.BaseAddress) {
				CntrlrEnabled = 1;
			} else
#endif
			{
				/* Required by MISRA */
			}

		} else if ((u8)XPM_GPIO_CNTRLR == i) {
#ifdef XPAR_XGPIOPS_0_DEVICE_ID
			if ((u32)XPAR_XGPIOPS_0_BASEADDR ==
			    Regulator->Cntrlr[i]->Node.BaseAddress) {
				CntrlrEnabled = 1;
			} else
#endif
#ifdef XPAR_XGPIOPS_1_DEVICE_ID
			if ((u32)XPAR_XGPIOPS_1_BASEADDR ==
			    Regulator->Cntrlr[i]->Node.BaseAddress) {
				CntrlrEnabled = 1;
			} else
#endif
			{
				/* Required by MISRA */
			}

		} else {
			/* Required by MISRA */
		}

		/*
		 * The device that has been defined in the CDO to
		 * control the regulator has not been enabled in
		 * the design.
		 */
		if (0U == CntrlrEnabled) {
			DbgErr = XPM_INT_ERR_RAIL_CONTROLLER_DISABLED;
			Status = XST_INVALID_PARAM;
			goto done;
		}
	}

	/*
	 * We will satisfy the following condition if no CDO command has
	 * been parsed to define which device is controlling the regulator.
	 */
	if (0U == CntrlrEnabled) {
		DbgErr = XPM_INT_ERR_INVALID_ARGS;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/*
 * This routine is invoked if the add node command for a power rail indicates
 * that voltage adjustment for the rail at different temperature is required.
 * This routine schedules a task to perform that periodic monitoring.
 */
static XStatus XPmRail_InitTempVoltAdj(const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 NodeId;
	XPm_Rail *Rail;
	static XPmRail_TempVoltAdj VCCINT_PL_TempVoltAdj;

	if (NumArgs < 6U) {
		DbgErr = XPM_INT_ERR_INVALID_ARGS;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/*
	 * Current support is for VCCINT_PL power rail only
	 */
	NodeId = Args[0];
	if (PM_POWER_VCCINT_PL != NodeId) {
		DbgErr = XPM_INT_ERR_INVALID_ARGS;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/*
	 * Args[2] holds the lower temperature threshold and Args[4] holds
	 * the upper temperature threshold.  Validate that value of lower
	 * threshold is less than value of upper threshold.
	 */
	if (XSysMonPsv_FixedToFloat(Args[2]) >= XSysMonPsv_FixedToFloat(Args[4])) {
		DbgErr = XPM_INT_ERR_INVALID_ARGS;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	VCCINT_PL_TempVoltAdj.LowerTempThresh = Args[2];
	VCCINT_PL_TempVoltAdj.LowerVoltMode = (u8)Args[5];
	VCCINT_PL_TempVoltAdj.UpperTempThresh = Args[4];
	VCCINT_PL_TempVoltAdj.UpperVoltMode = (u8)Args[3];

	/*
	 * Schedule a task to be run every 100ms to monitor the current
	 * temperature and make voltage adjustment, if needed.
	 */
	Rail = (XPm_Rail *)XPmPower_GetById(NodeId);
	Status = XPmRail_VerifyController(Rail);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_INVALID_ARGS;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Rail->TempVoltAdj = &VCCINT_PL_TempVoltAdj;
	Status = XPlmi_SchedulerAddTask(0x0U, XPmRail_CyclicTempVoltAdj, NULL,
					100U, XPLM_TASK_PRIORITY_0, Rail,
					XPLMI_PERIODIC_TASK);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
#endif /* VERSAL_DVS */

#if defined (RAIL_CONTROL)
static XStatus XPmRail_InitI2CMode(const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	XPm_Rail *Rail;
	const XPm_Regulator *Regulator;
	u32 i, j, k;
	u8 NumModes, Mode;
	const u32 CopySize = 4U;
	u8 PI;

	Rail = (XPm_Rail *)XPmPower_GetById(Args[0]);
	if (NULL == Rail) {
		DbgErr = XPM_INT_ERR_INVALID_ARGS;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Regulator = (XPm_Regulator *)XPmRegulator_GetById(Args[2]);
	if (NULL == Regulator) {
		DbgErr = XPM_INT_ERR_INVALID_ARGS;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	for (PI = 0; PI < MAX_PARENTS; PI++) {
		if (0U == Rail->ParentIds[PI]) {
			Rail->ParentIds[PI] = Args[2];
			break;
		}
	}

	if (PI == MAX_PARENTS) {
		DbgErr = XPM_INT_ERR_INVALID_ARGS;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	NumModes = (u8)Args[3];
	if (MAX_MODES < NumModes) {
		DbgErr = XPM_INT_ERR_INVALID_ARGS;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	k = 4;

	/* Format as below:
	 * add node rail, parent regulator,
	 * num_modes_supported, mode0 id+len of command bytes,
	 * i2c commands,
	 * mode1 id+len of command bytes, i2c commands.
	 *
	 * For example,
	 * pm_add_node 0x432802b 0x1 0x442c002 0x2 0x300
	 *	       0x02000002 0x01021a02 0x00 0x301
	 *	       0x02000002 0x01021a02 0x80
	 */
	for (i = 0U; i < NumModes; i++) {
		Mode = (u8)(Args[k] & 0xFFU);
		if (MAX_MODES <= Mode) {
			DbgErr = XPM_INT_ERR_INVALID_ARGS;
			Status = XST_INVALID_PARAM;
			goto done;
		}

		Rail->I2cModes[PI][Mode].CmdLen = (u8)(Args[k] >> 8) & 0xFFU;
		Rail->ControlType[PI][Mode] = XPM_RAILTYPE_MODE_PMBUS;
		k++;
		for (j = 0; j < Rail->I2cModes[PI][Mode].CmdLen; j++) {
			if (k >= NumArgs) {
				DbgErr = XPM_INT_ERR_INVALID_ARGS;
				Status = XST_INVALID_PARAM;
				goto done;
			}

			Status = Xil_SMemCpy(&Rail->I2cModes[PI][Mode].CmdArr[j * 4U],
					     CopySize, &Args[k], CopySize, CopySize);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_INVALID_PARAM;
				goto done;
			}

			k++;
		}
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus XPmRail_InitGPIOMode(const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	XPm_Rail *Rail;
	const XPm_Regulator *Regulator;
	u8 NumModes, Mode;
	u32 Index = 0U;

	/* Format as below:
	 * add node rail id, type, parent regulator id, num_modes,
	 * first mode id | second mode id
	 *
	 * e.g.
	 * pm_add_node 0x4328030 0x4 0x442c001 0x2 0x302 0x0044 0x800 0x0
	 *					   0x303 0x0044 0x800 0x800
	 * arg0: VCCINT_PL rail id = 0x4328030
	 * arg1: type = 0x4 (GPIO)
	 * arg2: regulator id = 0x442c001 (VCCINT_PL regulator id)
	 * arg3: num of modes = 0x2 (mode2: lower voltage; mode3: upper voltage)
	 * arg4: len | mode2 = 0x302 0x0044 0x800 0x0
	 * arg5: len | mode3 = 0x303 0x0044 0x800 0x800
	 */

	Rail = (XPm_Rail *)XPmPower_GetById(Args[Index]);
	if (NULL == Rail) {
		DbgErr = XPM_INT_ERR_INVALID_ARGS;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Index = Index + 2U;
	Regulator = (XPm_Regulator *)XPmRegulator_GetById(Args[Index]);
	if (NULL == Regulator) {
		DbgErr = XPM_INT_ERR_INVALID_ARGS;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Rail->ParentIds[0] = Args[Index];
	Index++;
	NumModes = (u8)Args[Index];
	if (MAX_MODES < NumModes) {
		DbgErr = XPM_INT_ERR_INVALID_ARGS;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Index++;

	/*
	 * In addition to 4 Args that have been processed, we expect to have 4 words of
	 * data for each power mode.
	 */
	if ((((u32)NumModes * 4U) + Index) > NumArgs) {
		DbgErr = XPM_INT_ERR_INVALID_ARGS;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	for (u8 i = 0; i < NumModes; i++) {
		/* 3 words of data is expected for setting a GPIO data register */
		if (((Args[Index] >> 8U) & 0xFFU) != 3U) {
			DbgErr = XPM_INT_ERR_INVALID_ARGS;
			Status = XST_INVALID_PARAM;
			goto done;
		}

		Mode = (u8)(Args[Index] & 0xFFU);
		if (MAX_MODES <= Mode) {
			DbgErr = XPM_INT_ERR_INVALID_ARGS;
			Status = XST_INVALID_PARAM;
			goto done;
		}

		Rail->ControlType[0][Mode] = XPM_RAILTYPE_MODE_GPIO;
		Rail->GPIOModes[Mode].ModeNumber = Mode;
		Index++;
		Rail->GPIOModes[Mode].Offset = (u16)Args[Index];
		Index++;
		Rail->GPIOModes[Mode].Mask = Args[Index];
		Index++;
		Rail->GPIOModes[Mode].Value = Args[Index];
		Index++;
	}

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
#endif /* RAIL_CONTROL */

/****************************************************************************/
/**
 * @brief  Initialize rail node base class
 *
 * @param  Rail: Pointer to an uninitialized power rail struct
 * @param  RailId: Node Id assigned to a Power Rail node
 * @param  Args: Arguments for power rail
 * @param  NumArgs: Number of arguments for power rail
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note Args is dependent on the rail type. Passed arguments will be different
 *		 for mode type and power good types.
 *
 ****************************************************************************/
XStatus XPmRail_Init(XPm_Rail *Rail, u32 RailId, const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 BaseAddress = 0;
	u8 Type;

	u32 NodeIndex = NODEINDEX(RailId);
	if ((u32)XPM_NODEIDX_POWER_MAX <= NodeIndex) {
		DbgErr = XPM_INT_ERR_INVALID_ARGS;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (3U < NumArgs) {
		Type = (u8)Args[1];
		switch (Type) {
		case XPM_RAILTYPE_PGOOD:
			Rail->Source = (XPm_PgoodSource)Args[2];
			BaseAddress =  Args[3];
			Rail->Power.Node.BaseAddress = Args[3];
			Status = XST_SUCCESS;
			break;
#if defined (RAIL_CONTROL)
		case XPM_RAILTYPE_MODE_PMBUS:
			Status = XPmRail_InitI2CMode(Args, NumArgs);
			break;
#if defined (VERSAL_DVS)
		case XPM_RAILTYPE_TEMPVOLTADJ:
			Status = XPmRail_InitTempVoltAdj(Args, NumArgs);
			break;
#endif /* VERSAL_DVS */
		case XPM_RAILTYPE_MODE_GPIO:
			Status = XPmRail_InitGPIOMode(Args, NumArgs);
			break;
#endif /* RAIL_CONTROL */
		default:
			DbgErr = XPM_INT_ERR_INVALID_ARGS;
			Status = XST_INVALID_PARAM;
			break;
		}

		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_INVALID_ARGS;
			goto done;
		}
	}

	if (NULL == XPmPower_GetById(RailId)) {
		Status = XPmPower_Init(&Rail->Power, RailId, BaseAddress, NULL);
	}

	Rail->Power.Node.State = (u8)XPM_POWER_STATE_ON;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
