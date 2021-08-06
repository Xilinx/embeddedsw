/******************************************************************************
* Copyright (c) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "sleep.h"
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

#if defined (XPAR_XIICPS_0_DEVICE_ID) || defined (XPAR_XIICPS_1_DEVICE_ID) || \
    defined (XPAR_XIICPS_2_DEVICE_ID)
#include "xiicps.h"

#define IIC_SCLK_RATE		400000
#define I2C_SLEEP_US 		1000U

static XIicPs IicInstance;

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
	usleep(I2C_SLEEP_US);

done:
	return Status;
}

static XStatus I2CInitialize(XIicPs *Iic)
{
	XStatus Status = XST_FAILURE;
	XIicPs_Config *Config;
	const XPm_Device *Device;
	u16 I2CDeviceId;

	/* Request the PMC_I2C device */
	Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_I2C_PMC,
				   (u32)PM_CAP_ACCESS, XPM_MAX_QOS, 0,
				   XPLMI_CMD_SECURE);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Device = XPmDevice_GetById(PM_DEV_I2C_PMC);
	if (NULL == Device) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

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
	if (NULL == Config) {
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

static XStatus I2CIdleBusWait(XIicPs *Iic)
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

static XStatus I2CWrite(XIicPs *Iic, u16 SlaveAddr, u8 *Buffer, s32 ByteCount)
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

XStatus XPmRail_Control(XPm_Rail *Rail, u8 State)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u8 WriteBuffer[2] = {0};
	u32 NodeIdx = NODEINDEX(Rail->Power.Node.Id);
	XPm_Regulator *Regulator;
	u16 RegulatorSlaveAddress, MuxAddress;
	u32 i = 0, j = 0, k = 0, BytesLen = 0, Mode = 0;

	if (State == Rail->Power.Node.State) {
		Status = XST_SUCCESS;
		goto done;
	}

	if ((0U != Rail->ParentId) &&
	    ((u32)XPM_NODESUBCL_POWER_REGULATOR == NODESUBCLASS(Rail->ParentId))) {
		Regulator = (XPm_Regulator *)XPmRegulator_GetById(Rail->ParentId);
		if (Regulator == NULL) {
			Status = XST_INVALID_PARAM;
			goto done;
		}
	} else {
		PmDbg("Rail topology information unavailable so rail can not be controlled.\r\n");
		Status = XST_SUCCESS;
		goto done;
	}

	if ((u32)XIL_COMPONENT_IS_READY != IicInstance.IsReady) {
		if (Regulator->ParentId == PM_DEV_I2C_PMC) {
			Status = I2CInitialize(&IicInstance);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_I2C_INIT;
				goto done;
			}
		} else {
			PmDbg("Regulator not supported.\r\n");
			Status = XST_SUCCESS;
			goto done;
		}
	}

	RegulatorSlaveAddress = (u16)Regulator->Node.BaseAddress;
	for (i = 0; i < Regulator->Config.CmdLen; i++) {
		MuxAddress = (u16)Regulator->Config.CmdArr[j];
		j++;
		BytesLen = Regulator->Config.CmdArr[j];
		j++;
		for (k = 0; k < BytesLen; k++) {
			WriteBuffer[k] = Regulator->Config.CmdArr[j];
			j++;
		}
		Status = I2CWrite(&IicInstance, MuxAddress, WriteBuffer,
				  (s32)BytesLen);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_I2C_WRITE;
			goto done;
		}
	}

	i = 0; j = 0; k = 0;

	Mode = ((u8)XPM_POWER_STATE_ON == State) ? 1U : 0U;
	for (i = 0; i < Rail->I2cModes[Mode].CmdLen; i++) {
		BytesLen = Rail->I2cModes[Mode].CmdArr[j];
		j++;
		for (k = 0; k < BytesLen; k++) {
			WriteBuffer[k] = Rail->I2cModes[Mode].CmdArr[j];
			j++;
		}

		Status = I2CWrite(&IicInstance, RegulatorSlaveAddress, WriteBuffer,
				  (s32)BytesLen);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_I2C_WRITE;
			goto done;
		}
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

XStatus XPmRail_Control(XPm_Rail *Rail, u8 State)
{
	(void)Rail;
	(void)State;

	return XST_SUCCESS;
}

#endif

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
	u32 BaseAddress = 0;
	u32 Type, i, j, k;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	u32 NodeIndex = NODEINDEX(RailId);
	if ((u32)XPM_NODEIDX_POWER_MAX <= NodeIndex) {
		DbgErr = XPM_INT_ERR_INVALID_NODE_IDX;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (1U < NumArgs) {
		Type = Args[1];
		switch (Type) {
		case (u32)XPM_RAILTYPE_MODE_PMBUS:
			Rail->ParentId = Args[2];
			Rail->NumModes = (u8)Args[3];
			if (Rail->NumModes > (u8)MAX_MODES) {
				DbgErr = XPM_INT_ERR_INVALID_PARAM;
				Status = XST_INVALID_PARAM;
				goto done;
			}

			k = 4;
			/* Format as below:
			 * add node rail , parent regulator,
			 * num_modes_supported, mode0 id+len of command bytes,
			 * i2c commands,
			 * mode1 id+len of command bytes, i2c commands.
			 *
			 * For example,
			 * pm_add_node 0x432802b 0x1 0x442c002 0x2 0x300
			 *             0x02000002 0x01021a02 0x00 0x301
			 *             0x02000002 0x01021a02 0x80
			 */
			for (i = 0U; i < Rail->NumModes; i++) {
				Rail->I2cModes[i].CmdLen = (u8)(Args[k] >> 8) &
							   0xFFU;
				k++;
				for (j = 0; j < Rail->I2cModes[i].CmdLen; j++) {
					Status = Xil_SecureMemCpy(&Rail->I2cModes[i].CmdArr[j * 4U], 4U,
							   &Args[k], 4U);
					k++;
					if (XST_SUCCESS != Status) {
						goto done;
					}
				}
			}

			Status = XST_SUCCESS;
			break;
		case (u32)XPM_RAILTYPE_PGOOD:
			Rail->Source = (XPm_PgoodSource)Args[2];
			BaseAddress =  Args[3];
			Rail->Power.Node.BaseAddress = Args[3];
			Status = XST_SUCCESS;
			break;
		default:
			DbgErr = XPM_INT_ERR_INVALID_PARAM;
			Status = XST_INVALID_PARAM;
			break;
		}

		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	if (NULL == XPmPower_GetById(RailId)) {
		Status = XPmPower_Init(&Rail->Power, RailId, BaseAddress, NULL);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_POWER_DOMAIN_INIT;
		}
	}

	Rail->Power.Node.State = (u8)XPM_POWER_STATE_ON;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
