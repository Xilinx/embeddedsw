/******************************************************************************
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_debug.h"
#include "xpm_device.h"
#include "xpm_rail.h"
#include "xpm_regulator.h"

#define RENESAS_ISL69260_ID	0x49d28100
#define RENESAS_ISL69260_NB	4
#define INFINEON_XDPE15284D_ID	0x8a03
#define INFINEON_XDPE15284D_NB	2
#define TI_TPS53689_ID		0x544953689000
#define TI_TPS53689_NB		6

#define PMBUS_IC_DEVICE_ID	0xAD
#define DEVICE_ID_LEN_MAX	8

/*
 * eFuse values for VID power rails
 */
u8 VCCINT_VIDTable[] = { \
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10 \
};

u8 VCC_FPD_VIDTable[] = { \
	48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63 \
};

u8 VCC_CPM5N_VIDTable[] = { \
	48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63 \
};

static struct {
	u32 RailId;
	u32 RegulatorId;
	u64 IcDeviceId;
	u8 IcDeviceId_Bytes;
} MV_Regulators[] = { \
	{ PM_POWER_VCCINT_PL, 0x442c001, RENESAS_ISL69260_ID, RENESAS_ISL69260_NB }, \
	{ PM_POWER_VCCINT_PL, 0x442c002, INFINEON_XDPE15284D_ID, INFINEON_XDPE15284D_NB }, \
	{ PM_POWER_VCCINT_PL, 0x442c003, TI_TPS53689_ID, TI_TPS53689_NB }, \
	{ PM_POWER_VCCINT_CPM5N, 0x442c004, RENESAS_ISL69260_ID, RENESAS_ISL69260_NB }, \
	{ PM_POWER_VCCINT_CPM5N, 0x442c005, INFINEON_XDPE15284D_ID, INFINEON_XDPE15284D_NB }, \
	{ PM_POWER_VCCINT_CPM5N, 0x442c006, TI_TPS53689_ID, TI_TPS53689_NB }, \
};
#if defined (XPAR_XIICPS_0_DEVICE_ID) || defined (XPAR_XIICPS_1_DEVICE_ID) || \
    defined (XPAR_XIICPS_2_DEVICE_ID) || defined (XPAR_XIICPS_0_BASEADDR)  || \
    defined (XPAR_XIICPS_1_BASEADDR)  || defined (XPAR_XIICPS_2_BASEADDR)

/*
 * TBD - This routine should be moved to common code when a use case for it is
 * identified that applies to all Versal platforms.
 */
static XStatus I2CRead(XIicPs *Iic, u16 SlaveAddr, u8 *Buffer, s32 ByteCount)
{
	XStatus Status = XST_FAILURE;

	(void) XIicPs_SetOptions(Iic, XIICPS_REP_START_OPTION);

	Status = I2CWrite(Iic, SlaveAddr, Buffer, 1);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	(void) XIicPs_ClearOptions(Iic, XIICPS_REP_START_OPTION);

	/* Continuously try to receive in case of arbitration */
	do {
		if (0 != Iic->IsRepeatedStart) {
			Status = I2CIdleBusWait(Iic);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}

		Status = XIicPs_MasterRecvPolled(Iic, Buffer, ByteCount, SlaveAddr);
	} while (XST_IIC_ARB_LOST == Status);

	if (XST_SUCCESS != Status) {
		PmErr("I2C read failure\r\n");
		goto done;
	}

	/* wait until the bus is idle again */
	Status = I2CIdleBusWait(Iic);

done:
	return Status;
}

/*
 * This routine identifies which one of possible regulator vendors
 * is used to power the rail.  This identification is done by issuing
 * IC_DEVICE_ID PMBus command and comparing the read value with the
 * values listed in the 'MV_Regulators' structure.
 */
static XStatus XPmRail_IdentifyVendor(XPm_Rail *Rail)
{
	XStatus Status = XST_FAILURE;
	static XIicPs *Iic;
	const XPm_Regulator *Regulator;
	u32 RailId = Rail->Power.Node.Id;
	u32 ControllerId;
	u16 I2cAddress;
	u8 Buffer[DEVICE_ID_LEN_MAX];
	u8 Bytes;
	u64 IcDeviceId;

	for (u8 i = 0; i < (sizeof(MV_Regulators)/sizeof(MV_Regulators[0])); i++) {
		if (MV_Regulators[i].RailId == RailId) {
			for (u8 j = 0; j < MAX_PARENTS; j++) {
				if (Rail->ParentIds[j] == MV_Regulators[i].RegulatorId) {
					Regulator = XPmRegulator_GetById(Rail->ParentIds[j]);
					I2cAddress = Regulator->I2cAddress;
					Bytes = MV_Regulators[i].IcDeviceId_Bytes;

					if (NULL == Iic) {
						Iic = XPmRail_GetIicInstance();
					}

					if ((u32)XIL_COMPONENT_IS_READY != Iic->IsReady) {
						ControllerId = Regulator->Cntrlr[XPM_I2C_CNTRLR]->Node.Id;
						Status = I2CInitialize(Iic, ControllerId);
						if (XST_SUCCESS != Status) {
							goto done;
						}
					}

					/* zero out the storage used */
					IcDeviceId = 0;
					Status = XPlmi_MemSetBytes(&Buffer, sizeof(Buffer), 0U, sizeof(Buffer));
					if (XST_SUCCESS != Status) {
						goto done;
					}

					Buffer[0] = PMBUS_IC_DEVICE_ID;
					Status = I2CRead(Iic, I2cAddress, Buffer, (s32)Bytes);
					if (XST_SUCCESS != Status) {
						goto done;
					}

					for (s8 k = ((s8)Bytes - 1); k >= 0; k--) {
						IcDeviceId = (IcDeviceId << 8) | Buffer[k];
					}

					if (IcDeviceId == MV_Regulators[i].IcDeviceId) {
						Rail->ParentIndex = j;
						break;
					}
				}
			}
		}
	}

	Status = XST_SUCCESS;

done:
	return Status;
}
#else
static XStatus XPmRail_IdentifyVendor(XPm_Rail *Rail)
{
	(void)Rail;
	PmWarn("Unable to Identify Regulator Vendor of Rail 0x%x.\
		Using default Regulator: 0x%x\r\n", Rail->Power.Node.Id, Rail->ParentIds[0]);
	return XST_SUCCESS;
}
#endif
XStatus XPmRail_AdjustVID(XPm_Rail *Rail)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_Device *EfuseCache;
	u32 VID;
	u8 ValidOverride = 0;
	u8 VIDIndex, Index;
	u8 Found = 0;

	/* if this rail has already been VID adjusted, then nothing more to do */
	if (1U == Rail->VIDAdjusted) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* A non-zero value in RTCA location indicates an override value */
	VID = XPm_In32(XPLMI_RTCFG_VID_OVERRIDE);
	if (0U != VID) {
		if ((0U == ((VID >> VID_CNTRL_OFFSET) & VID_CNTRL_MASK))) {
			Status = XST_SUCCESS;
			goto done;
		}

		/* Override control value should be valid for plan 1 */
		if (1U == ((VID >> VID_CNTRL_OFFSET) & VID_CNTRL_MASK)) {
			ValidOverride = 1;
		} else {
			DbgErr = XPM_INT_ERR_INVALID_VID;
			goto done;
		}
	}

	/* No override value is present so read the eFuse location */
	if (0U == ValidOverride) {
		EfuseCache = XPmDevice_GetById(PM_DEV_EFUSE_CACHE);
		if (NULL == EfuseCache) {
			DbgErr = XPM_INT_ERR_INVALID_DEVICE;
			goto done;
		}

		VID = XPm_In32(EfuseCache->Node.BaseAddress + EFUSE_CACHE_VID);
	}

	switch (Rail->Power.Node.Id) {
	case PM_POWER_VCCINT_PL:
		VIDIndex = ((VID >> VID_VCCINT_VCC_HNICX_OFFSET) &
			    VID_VCCINT_VCC_HNICX_MASK);

		/* No adjustment from default voltage if VID value is 0 */
		if (0U == VIDIndex) {
			Status = XST_SUCCESS;
			goto done;
		}

		for (Index = 0; Index < sizeof(VCCINT_VIDTable); Index++) {
			if (VCCINT_VIDTable[Index] == VIDIndex) {
				Found = 1;
				break;
			}
		}

		if (!Found) {
			DbgErr = XPM_INT_ERR_INVALID_VID;
			goto done;
		}

		break;
	case PM_POWER_VCCINT_PSFP:
		VIDIndex = ((VID >> VID_VCC_FPD_OFFSET) & VID_VCC_FPD_MASK);

		/* No adjustment from default voltage if VID value is 0 */
		if (0U == VIDIndex) {
			Status = XST_SUCCESS;
			goto done;
		}

		for (Index = 0; Index < sizeof(VCC_FPD_VIDTable); Index++) {
			if (VCC_FPD_VIDTable[Index] == VIDIndex) {
				Found = 1;
				break;
			}
		}

		if (!Found) {
			DbgErr = XPM_INT_ERR_INVALID_VID;
			goto done;
		}

		break;
	case PM_POWER_VCCINT_CPM5N:
		VIDIndex = ((VID >> VID_VCC_CPM5N_OFFSET) & VID_VCC_CPM5N_MASK);

		/* No adjustment from default voltage if VID value is 0 */
		if (0U == VIDIndex) {
			Status = XST_SUCCESS;
			goto done;
		}

		for (Index = 0; Index < sizeof(VCC_CPM5N_VIDTable); Index++) {
			if (VCC_CPM5N_VIDTable[Index] == VIDIndex) {
				Found = 1;
				break;
			}
		}

		if (!Found) {
			DbgErr = XPM_INT_ERR_INVALID_VID;
			goto done;
		}

		break;
	default:
		Status = XST_SUCCESS;
		goto done;
	}

	Status = XPmRail_IdentifyVendor(Rail);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPmRail_Control(Rail, (u8)XPM_POWER_STATE_ON, (Index + 2));
	if (XST_SUCCESS == Status) {
		Rail->VIDAdjusted = 1;
	} else {
		DbgErr = XPM_INT_ERR_RAIL_VID;
		goto done;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
