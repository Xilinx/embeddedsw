/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc. All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*
*
******************************************************************************/

#include "xpm_board.h"
#include "xpm_common.h"
#include "xpm_core.h"
#include "xplmi_util.h"
#include "xpm_regs.h"

/*********************** Variable Definitions **************************/
static u8 WriteBuffer[BUFFER_SIZE];

/*****************************************************************************/
/**
 * This function waits for the power rail to be fully powered on to prevent
 * usage before complete. Timeout after max timeout.
 *
 * @param VoltageRailMask Mask to read power rail register value
 *
 * @return   XST_SUCCESS if successful, otherwise XST_FAILURE
 *
 * @note   This function is not PmBus dependent and can be used for any power
 *		   rail connection
 *****************************************************************************/
static XStatus XPmBoard_WaitForPowerRailUp(u32 VoltageRailMask)
{
	XStatus Status = XST_FAILURE;
	XPm_Core *Pmc;

	Pmc = (XPm_Core *)XPmDevice_GetById(XPM_DEVID_PMC);
	if (NULL == Pmc) {
		goto done;
	}

	Status = XPlmi_UtilPollForMask((Pmc->RegAddress[0] +
				     PWR_SUPPLY_STATUS_OFFSET),
				     VoltageRailMask,
				     XPLMI_TIME_OUT_DEFAULT);
	if (XST_SUCCESS != Status) {
		PmInfo("ERROR: Poll for power rail up timeout \r\n");
	}

done:
	return Status;
}

#ifdef XPAR_XIICPS_1_DEVICE_ID
/*****************************************************************************/
/**
 * This function initializes the I2C Bus
 *
 * @param Iic   I2C instance
 *
 * @return      XST_SUCCESS if successful, otherwise XST_FAILURE
 *
 * @note		IIC_BASE_ADDR is currently hardcoded to be set to PMC I2C
 *****************************************************************************/
static XStatus XPmBoard_IicInit(XIicPs *Iic)
{
	XStatus Status = XST_FAILURE;
	XIicPs_Config *Config;

	Config = XIicPs_LookupConfig(IIC_DEVICE_ID);
	if (NULL == Config) {
		PmInfo("Could not find I2C\n\r");
		goto done;
	}

	/* Initialize I2C base address and clock frequency */
	Config->BaseAddress = IIC_BASE_ADDR;
	Config->InputClockHz = IIC_CLK_FREQ_HZ;

	Status = XIicPs_CfgInitialize(Iic, Config, Config->BaseAddress);
	if (XST_SUCCESS != Status) {
		PmErr("I2C initialization failure\n\r");
		goto done;
	}

	/* Set the I2C serial clock rate */
	Status = XIicPs_SetSClk(Iic, IIC_SCLK_RATE);
	if (XST_SUCCESS != Status) {
		PmErr("Failure setting I2C clock rate\n\r");
	}

done:
	return Status;
}

/***********************************************************************/
/* This function initializes the I2C Mux to select the required channel
 *
 * @param Iic			I2C instance
 * @param MuxAddr		The address of the MUX
 * @param WriteBuffer   The channel select value
 *
 * @return XST_SUCCESS or XST_FAILURE
 ***********************************************************************/
static XStatus XPmBoard_MuxConfigure(XIicPs *Iic, u16 MuxAddr, u8 Channel)
{
	XStatus Status = XST_FAILURE;

	/* Initialize the I2C instance if it has not been done already */
	if ((u32)XIL_COMPONENT_IS_READY != Iic->IsReady) {
		Status = XPmBoard_IicInit(Iic);
		if (XST_SUCCESS != Status) {
			PmErr("I2C initialization failure\n\r");
			goto done;
		}
	}

	WriteBuffer[0] = Channel;

	/*
	 * Send configuration to Mux
	 * Wait for idle bus and check for arbitration
	 */
	do {
		while (XIicPs_BusIsBusy(Iic));

		Status = XIicPs_MasterSendPolled(Iic, WriteBuffer, 1, MuxAddr);

	} while (XST_IIC_ARB_LOST == Status);

	if (XST_SUCCESS != Status) {
		PmErr("Failure to initialize Mux\n\r");
	}

done:
	return Status;
}

/*****************************************************************************/
/**
 * This function sends PmBus commands to power up power rail
 *
 * @param RegulatorAddress   Regulator address on Pmbus to be powered up
 * @param PmcPowerSupplyMask	Register mask for given regulator
 *
 * @return   XST_SUCCESS if successful, otherwise XST_FAILURE
 *****************************************************************************/
static XStatus XPmBoard_PowerUpRail(u8 RegulatorAddress,
				     u32 PmcPowerSupplyMask)
{
	XStatus Status = XST_FAILURE;

	Status = XPmBus_WriteByte(&IicInstance, RegulatorAddress,
				     ON_OFF_CONFIG, OP_POW_CTRL_CONFIG);
	if (XST_SUCCESS != Status) {
		PmErr("Failed to configure power regulator\r\n");
		goto done;
	}

	Status = XPmBus_WriteByte(&IicInstance, RegulatorAddress,
				     OPERATION, PM_OP_POWER_UP);
	if (XST_SUCCESS != Status) {
		PmErr("Failed to power up power rail\r\n");
		goto done;
	}

	Status = XPmBoard_WaitForPowerRailUp(PmcPowerSupplyMask);

done:
	return Status;
}

/*****************************************************************************/
/**
 * This function sends PmBus commands to power down power rail
 *
 * @param RegulatorAddress   Regulator address on Pmbus to be powered down
 *
 * @return   XST_SUCCESS if successful, otherwise XST_FAILURE
 *****************************************************************************/
static XStatus XPmBoard_PowerDownRail(u8 RegulatorAddress)
{
	XStatus Status = XST_FAILURE;

	Status = XPmBus_WriteByte(&IicInstance, RegulatorAddress,
				     ON_OFF_CONFIG, OP_POW_CTRL_CONFIG);
	if (XST_SUCCESS != Status) {
		PmErr("Failed to configure power regulator\r\n");
		goto done;
	}

	Status = XPmBus_WriteByte(&IicInstance, RegulatorAddress,
				     OPERATION, PM_OP_POWER_DOWN);
	if (XST_SUCCESS != Status) {
		PmErr("Failed to power down power rail\n");
	}

done:
	return Status;
}
#endif /* XPAR_XIICPS_1_DEVICE_ID */

/*****************************************************************************/
/**
 * This function is used to control the power rails
 *
 * @param Function  Action to be performed on power rail
 * @param PowerRegulatorId  Id given to a particular power rail
 *
 * @return   XST_SUCCESS if successful, otherwise XST_FAILURE
 *****************************************************************************/
XStatus XPmBoard_ControlRail(const enum power_rail_function Function,
				     const enum power_rail_id PowerRegulatorId)
{
	XStatus Status = XST_FAILURE;

#ifdef CUSTOM_PMBUS
	u8 MuxChannel;
	u8 RegulatorAddress;
	u32 PmcPowerSupplyMask;

	switch (PowerRegulatorId) {
	case POWER_RAIL_FPD:
		RegulatorAddress = PSFP_REGULATOR_ADDR;
		MuxChannel = MUX_SEL_CHANNEL_0;
		PmcPowerSupplyMask = PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_FPD_MASK;
		break;
	/*TODO: Add cases for other power rails i.e. LPD */
	default:
		PmErr("Invalid Regulator Id\n\r");
		goto done;
	}
#ifdef XPAR_XIICPS_1_DEVICE_ID
	/* Configure Mux */
	Status = XPmBoard_MuxConfigure(&IicInstance, I2C0_MUX_ADDR, MuxChannel);
	if (XST_SUCCESS != Status) {
		PmErr("Failure initializing I2C Mux\r\n");
		goto done;
	}

	switch (Function) {
	case RAIL_POWER_UP:
		/* Send PMBus commands to Power up rail */
		Status = XPmBoard_PowerUpRail(RegulatorAddress, PmcPowerSupplyMask);
		if (XST_SUCCESS != Status) {
			PmErr("Failure powering up FPD power rail\n\r");
			goto done;
		}
		break;
	case RAIL_POWER_DOWN:
		/* Send PMC_I2C command to turn off power rail */
		Status = XPmBoard_PowerDownRail(RegulatorAddress);
		if (XST_SUCCESS != Status) {
			PmErr("Failure turning off FPD power rail\n\r");
			goto done;
		}
		break;
	/* TODO: Add cases for other power rail actions */
	default:
		PmErr("Invalid Function Id\n\r");
		goto done;
	}
#endif /* XPAR_XIICPS_1_DEVICE_ID */
#endif /* CUSTOM_PMBUS */
	Status = XST_SUCCESS;
#ifdef CUSTOM_PMBUS
done:
#endif
	return Status;
}
