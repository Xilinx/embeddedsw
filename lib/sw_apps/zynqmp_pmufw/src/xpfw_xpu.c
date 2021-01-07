/******************************************************************************
* Copyright (c) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpfw_default.h"
#include "xpfw_xpu.h"

/* XMPU/XPPU configiguration register addresses */
#define    XMPU_DDR_0_BASE_ADDR    0xFD000000U
#define    XMPU_DDR_1_BASE_ADDR    0xFD010000U
#define    XMPU_DDR_2_BASE_ADDR    0xFD020000U
#define    XMPU_DDR_3_BASE_ADDR    0xFD030000U
#define    XMPU_DDR_4_BASE_ADDR    0xFD040000U
#define    XMPU_DDR_5_BASE_ADDR    0xFD050000U
#define    XMPU_FPD_BASE_ADDR      0xFD5D0000U
#define    XMPU_OCM_BASE_ADDR      0xFFA70000U
#define    XPPU_BASE_ADDR          0xFF980000U
#define    XPPU_POISON_OFFSET_ADDR 0xFF9CFF00U

/* XPU status register offsets */
#define    XPU_ISR_OFFSET             0x10U
#define    XPU_IER_OFFSET             0x18U
#define    XPU_ERR_STATUS_1_OFFSET    0x04U
#define    XPU_ERR_STATUS_2_OFFSET    0x08U
#define    XPU_POISON_OFFSET          0x0CU

/* XMPU error IDs to identify each error */
#define    XMPU_REG_ACC_ERR_ON_APB            0x1U
#define    XMPU_READ_PERMISSION_VIOLATION     0x2U
#define    XMPU_WRITE_PERMISSION_VIOLATION    0x4U
#define    XMPU_SECURITY_VIOLATION_ERR        0x8U

/* XPPU error IDs to identify each error */
#define    XPPU_REG_ACC_ERR_ON_APB            0x1U
#define    XPPU_MID_NOT_FOUND                 0x2U
#define    XPPU_MWRITE_PERMISSON_VIOLATION    0x4U
#define    XPPU_MID_PARITY_ERROR              0x8U
#define    XPPU_MID_ACCESS_VIOLATION          0x20U
#define    XPPU_TRUSTZONE_VIOLATION           0x40U
#define    XPPU_APPER_PARITY_ERROR            0x80U

#ifdef XPU_INTR_DEBUG_PRINT_ENABLE
struct XpuMasterID {
	u32 MasterID;
	u32 MasterIDLimit;
	char MasterName[11];
};

/* XPU master ID LUT to identify master which caused the violation */
static struct XpuMasterID XpuMasterIDLUT[] =
{
		{ 0x00U,  0x0FU,  "RPU0" },
		{ 0x10U,  0x1FU,  "RPU1" },
		{ 0x40U,  0x40U,  "PMU MB" },
		{ 0x50U,  0x50U,  "CSU MB" },
		{ 0x51U,  0x51U,  "CSU DMA" },
		{ 0x60U,  0x60U,  "USB0" },
		{ 0x61U,  0x61U,  "USB1" },
		{ 0x62U,  0x62U,  "DAP" },
		{ 0x68U,  0x6FU,  "ADMA" },
		{ 0x70U,  0x70U,  "SD0" },
		{ 0x71U,  0x71U,  "SD1" },
		{ 0x72U,  0x72U,  "NAND" },
		{ 0x73U,  0x73U,  "QSPI" },
		{ 0x74U,  0x74U,  "GEM0" },
		{ 0x75U,  0x75U,  "GEM1" },
		{ 0x76U,  0x76U,  "GEM2" },
		{ 0x77U,  0x77U,  "GEM3" },
		{ 0x80U,  0xBFU,  "APU" },
		{ 0xC0U,  0xC3U,  "SATA" },
		{ 0xC4U,  0xC4U,  "GPU" },
		{ 0xC5U,  0xC5U,  "CoreSight" },
		{ 0xD0U,  0xD0U,  "PCIe" },
		{ 0xE0U,  0xE7U,  "DPDMA" },
		{ 0xE8U,  0xEFU,  "GDMA" },
		{ 0x200U, 0x23FU, "AFI FM0" },
		{ 0x240U, 0x27FU, "AFI FM1" },
		{ 0x280U, 0x2BFU, "AFI FM2" },
		{ 0x2C0U, 0x2FFU, "AFI FM3" },
		{ 0x300U, 0x33FU, "AFI FM4" },
		{ 0x340U, 0x37FU, "AFI FM5" },
		{ 0x380U, 0x3BFU, "AFI FM LPD" },
};
#endif

struct XpuReg {
	u32 BaseAddress;
	u32 MaskAll;
	char CfgName[5];
};

static struct XpuReg XpuRegList[] =
{
#ifdef XPAR_DDRCPSU_0_DEVICE_ID
	{
		.BaseAddress = XMPU_DDR_0_BASE_ADDR,
		.MaskAll = (u32)0xFU,
		.CfgName = "DDR0",
	},
	{
		.BaseAddress = XMPU_DDR_1_BASE_ADDR,
		.MaskAll = (u32)0xFU,
		.CfgName = "DDR1",
	},
	{
		.BaseAddress = XMPU_DDR_2_BASE_ADDR,
		.MaskAll = (u32)0xFU,
		.CfgName = "DDR2",
	},
	{
		.BaseAddress = XMPU_DDR_3_BASE_ADDR,
		.MaskAll = (u32)0xFU,
		.CfgName = "DDR3",
	},
	{
		.BaseAddress = XMPU_DDR_4_BASE_ADDR,
		.MaskAll = (u32)0xFU,
		.CfgName = "DDR4",
	},
	{
		.BaseAddress = XMPU_DDR_5_BASE_ADDR,
		.MaskAll = (u32)0xFU,
		.CfgName = "DDR5",
	},
#endif
	{
		.BaseAddress = XMPU_FPD_BASE_ADDR,
		.MaskAll = (u32)0xFU,
		.CfgName = "FPD",
	},
	{
		.BaseAddress = XMPU_OCM_BASE_ADDR,
		.MaskAll = (u32)0xFU,
		.CfgName = "OCM",
	},
	{
		.BaseAddress = XPPU_BASE_ADDR,
		.MaskAll = (u32)0xEFU,
		.CfgName = "XPPU",
	},
};


/**
 * Enable interrupts for all XMPU/XPPU Instances
 */
void XPfw_XpuIntrInit(void)
{
	u32 Idx;
	XPfw_Printf(DEBUG_DETAILED,"EM: Enabling XMPU/XPPU interrupts\r\n");
	for(Idx = 0U; Idx < ARRAYSIZE(XpuRegList);Idx++) {
		/* Enable all the Interrupts for this XMPU/XPPU Instance */
		XPfw_Write32(XpuRegList[Idx].BaseAddress + XPU_IER_OFFSET,
						XpuRegList[Idx].MaskAll);
	}
}

/**
 * Ack interrupts for all XMPU/XPPU Instances so that any new
 * interrupts occurring later can trigger an Error interrupt to PMU
 */
void XPfw_XpuIntrAck(void)
{
	u32 Idx;
	u32 XpuIntSts = 0U;

#ifdef XPU_INTR_DEBUG_PRINT_ENABLE
	u32 Addr = 0U;
	u32 MasterID = 0U;
	u32 PoisonReg = 0U;
	u32 Offset = 0U;
	u32 MasterIdx;
#endif

	for (Idx = 0U; Idx < ARRAYSIZE(XpuRegList); Idx++) {
		if (XpuIntSts != 0U) {
			break;
		}
		XpuIntSts = XPfw_Read32(XpuRegList[Idx].BaseAddress + XPU_ISR_OFFSET);

#ifdef XPU_INTR_DEBUG_PRINT_ENABLE
		Addr = XPfw_Read32(XpuRegList[Idx].BaseAddress + XPU_ERR_STATUS_1_OFFSET);
		MasterID = XPfw_Read32(XpuRegList[Idx].BaseAddress + XPU_ERR_STATUS_2_OFFSET);
		PoisonReg = XPfw_Read32(XpuRegList[Idx].BaseAddress + XPU_POISON_OFFSET);
		if ((Idx < (ARRAYSIZE(XpuRegList) - 1U)) && (XpuIntSts != 0U)) {

			switch (XpuIntSts) {

				case XMPU_REG_ACC_ERR_ON_APB:
				{
					XPfw_Printf(DEBUG_DETAILED,"EM: XMPU %s Register Access "
							"Error on APB\r\n", XpuRegList[Idx].CfgName);
				}
				break;

				case XMPU_READ_PERMISSION_VIOLATION:
				{
					XPfw_Printf(DEBUG_DETAILED,"EM: XMPU %s Read permission "
							"violation occurred\r\n", XpuRegList[Idx].CfgName);
				}
				break;

				case XMPU_WRITE_PERMISSION_VIOLATION:
				{
					XPfw_Printf(DEBUG_DETAILED,"EM: XMPU %s Write permission "
							"violation occurred\r\n", XpuRegList[Idx].CfgName);
				}
				break;

				case XMPU_SECURITY_VIOLATION_ERR:
				{
					XPfw_Printf(DEBUG_DETAILED,"EM: XMPU %s Security violation"
							" occurred\r\n", XpuRegList[Idx].CfgName);
				}
				break;

				default:
					/* Empty default case */
					break;
			}

			XPfw_Printf(DEBUG_DETAILED,"EM: Address of poisoned operation: "
					"0x%x%s\r\n",Addr,"XXX");

			for(MasterIdx = 0U; MasterIdx < ARRAYSIZE(XpuMasterIDLUT);
					++MasterIdx) {

				if ((MasterID >= XpuMasterIDLUT[MasterIdx].MasterID) &&
					  (MasterID <= XpuMasterIDLUT[MasterIdx].MasterIDLimit)) {

					XPfw_Printf(DEBUG_DETAILED,"EM: Master Device of poisoned "
							"operation: %s\r\n",
							XpuMasterIDLUT[MasterIdx].MasterName);
					break;
				}
			}

			XPfw_Printf(DEBUG_DETAILED,"EM: Poison register: 0x%x\r\n",
					PoisonReg);
		} else if ((Idx == (ARRAYSIZE(XpuRegList) - 1U)) && (XpuIntSts != 0U)) {

			Offset = XPfw_Read32(XPPU_POISON_OFFSET_ADDR);

			switch (XpuIntSts) {

				case XPPU_REG_ACC_ERR_ON_APB:
				{
					XPfw_Printf(DEBUG_DETAILED,"EM: XPPU Register access error"
						" on APB. A register access was requested to an "
						"unimplemented register location\r\n");
				}
				break;

				case XPPU_MID_NOT_FOUND:
				{
					XPfw_Printf(DEBUG_DETAILED,"EM: XPPU Master ID "
						"not found\r\n");
				}
				break;

				case XPPU_MWRITE_PERMISSON_VIOLATION:
				{
					XPfw_Printf(DEBUG_DETAILED,"EM: XPPU Read permission "
						"violation. Master attempted a write, but the master "
						"has read-only permission\r\n");
				}
				break;

				case XPPU_MID_PARITY_ERROR:
				{
					XPfw_Printf(DEBUG_DETAILED,"EM: XPPU Master ID parity "
							"error\r\n");
				}
				break;

				case XPPU_MID_ACCESS_VIOLATION:
				{
					XPfw_Printf(DEBUG_DETAILED,"EM: XPPU Master ID access "
							"violation\r\n");
				}
				break;

				case XPPU_TRUSTZONE_VIOLATION:
				{
					XPfw_Printf(DEBUG_DETAILED,"EM: XPPU TrustZone Violation. "
						"A non-secure master attempted to access a secure "
						"memory location\r\n");
				}
				break;

				case XPPU_APPER_PARITY_ERROR:
				{
					XPfw_Printf(DEBUG_DETAILED,"EM: XPPU Aperture parity "
							"Error\r\n");
				}
				break;

				default:
					/* Empty default case */
					break;
			}

			Addr = (Addr << 12) | Offset;
			XPfw_Printf(DEBUG_DETAILED,"EM: Address of poisoned operation: "
					"0x%x\r\n",Addr);
			for(MasterIdx = 0U; MasterIdx < ARRAYSIZE(XpuMasterIDLUT);
					++MasterIdx) {

				if ((MasterID >= XpuMasterIDLUT[MasterIdx].MasterID) &&
					  (MasterID <= XpuMasterIDLUT[MasterIdx].MasterIDLimit)) {

					XPfw_Printf(DEBUG_DETAILED,"EM: Master Device of poisoned "
						"operation: %s\r\n",
						XpuMasterIDLUT[MasterIdx].MasterName);
					break;
				}
			}
			XPfw_Printf(DEBUG_DETAILED,"EM: Poison register : 0x%x\r\n",
					PoisonReg);
		} else {
			/* For MISRA C compliance */
		}
#endif
		/* Ack the Interrupts */
		XPfw_Write32(XpuRegList[Idx].BaseAddress + XPU_ISR_OFFSET,
						XpuRegList[Idx].MaskAll);
	}
}

/**
 * A placeholder for handling XPPU/XMPU errors
 * This routine is called when ever a XMPU/XPPU error occurs
 * It prints a message if debug is enabled and Acks the errors
 *
 * @param ErrorId is the error identifier passed by Error Manager
 */
void XPfw_XpuIntrHandler(u8 ErrorId)
{
	XPfw_Printf(DEBUG_DETAILED,
		"============================================================\r\n");
	XPfw_Printf(DEBUG_DETAILED,"EM: XMPU/XPPU violation occurred "
			"(ErrorId: %d)\r\n", ErrorId);
	XPfw_XpuIntrAck();
	XPfw_Printf(DEBUG_DETAILED,
		"============================================================\r\n");
}
