/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022-2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xplmi_util.h"
#include "xplmi_dma.h"
#include "xpm_common.h"
#include "xpm_aie.h"
#include "xpm_regs.h"
#include "xpm_bisr.h"
#include "xpm_device.h"
#include "xpm_debug.h"

/* TODO: Remove once tools supports add_node commands for AIE partition device */
#include "xpm_aiedevice.h"

#define AIE_POLL_TIMEOUT 0X1000000U

#define COL_SHIFT 23U
#define ROW_SHIFT 18U
#define AIE1_TILE_BADDR(NocAddr, col, row)	\
	(((u64)(NocAddr)) + ((u64)(col) << COL_SHIFT) + ((u64)(row) << ROW_SHIFT))

#define AIE2_COL_SHIFT 25U
#define AIE2_ROW_SHIFT 20U
#define AIE2_TILE_BADDR(NocAddr, col, row)	\
	(((u64)(NocAddr)) + ((u64)(col) << AIE2_COL_SHIFT) + ((u64)(row) << AIE2_ROW_SHIFT))

#define AIE_CORE_STATUS_DONE_MASK   (1UL<<20U)

#define XPM_AIE_OPS                    0U
#define XPM_AIE2_OPS                   1U
#define XPM_AIE_OPS_MAX                2U

#define AieWrite64(addr, val) swea(addr, val)
#define AieRead64(addr) lwea(addr)

#define AIE1_PROG_MEM_SIZE		(0x4000U)
#define AIE1_DATA_MEM_SIZE		(0x8000U)

static XPm_AieDomain *PmAieDomain;
static XStatus Aie2_Zeroization(const XPm_Device *AieDev, u32 ColStart, u32 ColEnd);

static inline void AieRMW64(u64 addr, u32 Mask, u32 Value)
{
	u32 l_val;
	l_val = AieRead64(addr);
	l_val = (l_val & (~Mask)) | (Mask & Value);
	AieWrite64(addr, l_val);
}

/* Buffer to hold AIE data memory zeroization elf */
/**
 * NOTE: If ProgramMem[] is updated in future, then check if the current
 * AieWaitForCoreDone() implementation is still valid or needs to be
 * updated to use events.
 */
static const u32 ProgramMem[] __attribute__ ((aligned(16))) = {
	0x0600703fU,
	0x0a000804U,
	0x000018c0U,
	0x603803f7U,
	0x00000203U,
	0x400c9803U,
	0x13201803U,
	0x31009803U,
	0x200003f7U,
	0x00000277U,
	0x800003f7U,
	0x00000257U,
	0x00000000U,
	0x39200000U,
	0x0000003dU,
	0x00000000U,
	0x00000000U,
	0x00000000U,
	0x40000000U,
	0x00001888U,
	0x00000000U,
	0x00000000U,
	0x00000000U,
	0x0000079aU,
	0x00000000U,
	0x00000000U,
	0x00000000U,
	0x00002614U,
	0x00000000U,
	0x00000000U,
	0x07428800U,
	0x00000000U,
	0x00010001U,
	0x00010001U,
	0x00030001U,
	0x00011000U,
	0x00010001U,
	0x00010001U,
	0x00010001U,
	0x00010001U,
	};

/*****************************************************************************/
/**
 * This function provides a delay for specified duration
 *
 * @param MicroSeconds Duration in micro seconds
 * @return
 *****************************************************************************/
static inline void AieWait(u32 MicroSeconds)
{
	usleep(MicroSeconds);
}


/*****************************************************************************/
/**
 * This function is used to enable AIE Core
 *
 * @param AieDomain Handle to AIE domain instance
 * @param Col Column index of the Core
 * @param Row Row index of the Core
 *
 * @return N/A
 *****************************************************************************/
static void AieCoreEnable(const XPm_AieDomain *AieDomain, u32 Col, u32 Row)
{
	u64 TileBaseAddress = AIE1_TILE_BADDR(AieDomain->Array.NocAddress, Col, Row);

	/* Release reset to the Core */
	AieWrite64(TileBaseAddress + AIE_CORE_CONTROL_OFFSET, 0U);

	/* Enable the Core */
	AieWrite64(TileBaseAddress + AIE_CORE_CONTROL_OFFSET, 1U);
}

/*****************************************************************************/
/**
 * This function waits for a Core's DONE bit to be set
 *
 * @param AieDomain Handle to AIE domain instance
 * @param Col Column index of the Core
 * @param Row Row index of the Core
 *
 * @return Status Code
 *****************************************************************************/
static XStatus AieWaitForCoreDone(const XPm_AieDomain *AieDomain, u32 Col, u32 Row)
{
	u64 TileBaseAddress = AIE1_TILE_BADDR(AieDomain->Array.NocAddress, Col, Row);
	u64 StatusRegAddr = TileBaseAddress + AIE_CORE_STATUS_OFFSET;
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = XPlmi_UtilPollForMask64((u32)(StatusRegAddr>>32),
				(u32)(StatusRegAddr), AIE_CORE_STATUS_DONE_MASK, 10U);
	if (Status != XST_SUCCESS) {
		DbgErr = XPM_INT_ERR_AIE_CORE_STATUS_TIMEOUT;
		PmInfo("ERROR: Poll for Done timeout \r\n");
	}

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/*****************************************************************************/
/**
 * This function loads a core's program memory with zeroization elf
 *
 * @param AieDomain Handle to AIE domain instance
 * @param Col Column index of the Core
 * @param Row Row index of the Core
 *
 * @return Status Code
 *****************************************************************************/
static XStatus ProgramCore(const XPm_AieDomain *AieDomain, u32 Col, u32 Row,
			   const u32 *PrgData, u32 NumOfWords)
{
	u64 TileBaseAddress = AIE1_TILE_BADDR(AieDomain->Array.NocAddress, Col, Row);
	u64 PrgAddr = TileBaseAddress + AIE_PROGRAM_MEM_OFFSET;

	return XPlmi_DmaXfr((u64)(0U)|(u32)PrgData, PrgAddr, NumOfWords, XPLMI_PMCDMA_0);
}

/*****************************************************************************/
/**
 * This function is used to cycle reset to the entire AIE array
 *
 * @return
 *****************************************************************************/
static XStatus ArrayReset(u32 BaseAddress)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = XPm_PcsrWrite(BaseAddress, ME_NPI_REG_PCSR_MASK_ME_ARRAY_RESET_MASK,
					ME_NPI_REG_PCSR_MASK_ME_ARRAY_RESET_MASK);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_ARRAY_RESET;
		goto done;
	}

	/* Wait for reset to propagate (1us) */
	AieWait(1U);

	Status = XPm_PcsrWrite(BaseAddress, ME_NPI_REG_PCSR_MASK_ME_ARRAY_RESET_MASK, 0U);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_ARRAY_RESET_RELEASE;
	}

	/* Wait for reset to propagate (1us) */
	AieWait(1U);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to scrub ECC enabled memories in the entire AIE array
 * Parameter: Action: ECC_SCRUB_DISABLE - Disable ECC scrub (disable PMEM Scrub using False event in all the Tiles)
 *		      ECC_SCRUB_ENABLE  - Enable ECC scrub (enable PMEM Scrub using True event in all the Tiles)
 *
 * @return
 *****************************************************************************/
static void TriggerEccScrub(const XPm_AieDomain *AieDomain, u32 Action)
{
	u16 StartCol = AieDomain->Array.StartCol;
	u16 EndCol = StartCol + AieDomain->Array.NumColsAdjusted;
	u16 StartRow = AieDomain->Array.StartRow;
	u16 EndRow = StartRow + AieDomain->Array.NumRowsAdjusted;

	for (u16 col = StartCol; col < EndCol; col++) {
		for (u16 row = StartRow; row < EndRow; row++) {
			u64 TileBaseAddress = AIE1_TILE_BADDR(AieDomain->Array.NocAddress, col, row);
			AieWrite64(TileBaseAddress + AIE_CORE_ECC_SCRUB_EVENT_OFFSET, Action);
		}
	}
}

/*****************************************************************************/
/**
 * This function clock gates ME tiles clock column-wise
 *
 * @param AieDomain Handle to AIE domain instance
 *
 * @return N/A
 *****************************************************************************/
static void AieClkGateByCol(const XPm_AieDomain *AieDomain)
{
	u16 StartCol = 0;	/* always start from col 0 */
	u16 EndCol = StartCol + AieDomain->Array.NumCols;	/* always use total no. of cols */
	u16 StartRow = 0U;	/* Shim row is always row zero */
	u16 EndRow = AieDomain->Array.NumShimRows;

	for (u16 row = StartRow; row < EndRow; ++row) {
		for (u16 col = StartCol; col < EndCol; ++col) {
			u64 TileBaseAddress = AIE1_TILE_BADDR(AieDomain->Array.NocAddress, col, row);
			AieRMW64(TileBaseAddress + AIE_TILE_CLOCK_CONTROL_OFFSET,
				AIE_TILE_CLOCK_CONTROL_CLK_BUFF_EN_MASK,
				~AIE_TILE_CLOCK_CONTROL_CLK_BUFF_EN_MASK);
		}
	}
}

static XStatus AieCoreMemInit(const XPm_AieDomain *AieDomain)
{
	u16 StartCol = AieDomain->Array.StartCol;
	u16 EndCol = StartCol + AieDomain->Array.NumColsAdjusted;
	u16 StartRow = AieDomain->Array.StartRow;
	u16 EndRow = StartRow + AieDomain->Array.NumRowsAdjusted;
	XStatus Status;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	for (u16 col = StartCol; col < EndCol; col++) {
		for (u16 row = StartRow; row < EndRow; row++) {
			PmDbg("---------- (%d, %d)----------\r\n", col, row);
			Status = ProgramCore(AieDomain, col, row,
					     &ProgramMem[0], ARRAY_SIZE(ProgramMem));
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_PRGRM_CORE;
				goto done;
			}

			AieCoreEnable(AieDomain, col, row);
		}
	}

	/**
	 * NOTE: In future, if contents of ProgramMem[] are changed due to an
	 * updated AIE elf generated by latest tools, then the below check for
	 * core DONE may not work. Latest tools use events instead of DONE bit.
	 */
	Status = AieWaitForCoreDone(AieDomain, (u32)EndCol - 1U, (u32)EndRow - 1U);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_AIE_CORE_STATUS_TIMEOUT;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static void Aie2ClockInit(const XPm_AieDomain *AieDomain, u32 BaseAddress)
{
	u16 StartCol = AieDomain->Array.StartCol;
	u16 EndCol = StartCol + AieDomain->Array.NumColsAdjusted;

	/* Enable privileged write access */
	XPm_RMW32(BaseAddress + AIE2_NPI_ME_PROT_REG_CTRL_OFFSET,
			ME_PROT_REG_CTRL_PROTECTED_REG_EN_MASK,
			ME_PROT_REG_CTRL_PROTECTED_REG_EN_MASK);

	/* Enable all column clocks */
	for (u16 col = StartCol; col < EndCol; col++) {
		AieWrite64(AIE2_TILE_BADDR(AieDomain->Array.NocAddress, col, 0) +
				                AIE2_PL_MODULE_COLUMN_CLK_CTRL_OFFSET, 1U);
	}

	/* Disable privileged write access */
	XPm_RMW32(BaseAddress + AIE2_NPI_ME_PROT_REG_CTRL_OFFSET,
			ME_PROT_REG_CTRL_PROTECTED_REG_EN_MASK, 0U);
}

static void Aie2ClockGate(const XPm_AieDomain *AieDomain, u32 BaseAddress)
{
	u16 StartCol = AieDomain->Array.StartCol;
	u16 EndCol = StartCol + AieDomain->Array.NumColsAdjusted;

	/* Enable privileged write access */
	XPm_RMW32(BaseAddress + AIE2_NPI_ME_PROT_REG_CTRL_OFFSET,
			ME_PROT_REG_CTRL_PROTECTED_REG_EN_MASK,
			ME_PROT_REG_CTRL_PROTECTED_REG_EN_MASK);

	/* Disable all column clocks */
	for (u16 col = StartCol; col < EndCol; col++) {
		AieWrite64(AIE2_TILE_BADDR(AieDomain->Array.NocAddress, col, 0) +
				AIE2_PL_MODULE_COLUMN_CLK_CTRL_OFFSET, 0U);
	}

	/* Disable privileged write access */
	XPm_RMW32(BaseAddress + AIE2_NPI_ME_PROT_REG_CTRL_OFFSET,
			ME_PROT_REG_CTRL_PROTECTED_REG_EN_MASK, 0U);
}

/*
 * NOTE: This function is a workaround until tools supports pm_add_node for AIE
 * partition devices.
 * TODO: Remove once supported.
 */
XStatus AddAieDeviceNode(void)
{
	XStatus Status = XST_FAILURE;
	XPm_AieNode *AieNode = (XPm_AieNode *)XPmDevice_GetById(PM_DEV_AIE);
	XPm_AieDevice *AieDev;
	u32 NodeId = 0x18800000U;
	u32 NodeIdx = NODEINDEX(NodeId);
	u32 Args[2] = {0};

	/* Check for any existing AIE partition nodes */
	u32 Temp = NodeId;
	while ((u32)XPM_NODEIDX_DEV_AIE_MAX > NodeIdx) {
		AieDev = (XPm_AieDevice *)XPmDevice_GetById(Temp);
		if (NULL != AieDev) {
			/* Aie partition node already present so exit */
			Status = XST_SUCCESS;
			goto done;
		}
		Temp++;
		NodeIdx++;
	}

	/* AIE0 is default node ID so add this node and its parent only */
	Args[0] = NodeId;
	Args[1] = 0x18700000U;

	Status = XPm_AddNode(Args, 1U);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPm_AddNodeParent(Args, 2U);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	AieDev = (XPm_AieDevice *)XPmDevice_GetById(Args[0]);
	if (NULL == AieDev) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	/* Assign AIE device dependency */
	AieDev->BaseDev = AieNode;

done:
	return Status;
}

static XStatus AieInitStart(XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddress = 0U;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	XPm_AieDomain *AieDomain = (XPm_AieDomain *)PwrDomain;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	const XPm_Device * const AieDev = XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == AieDev) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	BaseAddress = AieDev->Node.BaseAddress;

	/* Use AIE NoC Address if available */
	if (2U <= NumOfArgs) {
		AieDomain->Array.NocAddress = ((u64)Args[1] << 32U) | (Args[0]);
		PmDbg("AIE: NoC Address: 0x%x%08x\r\n",
				(u32)(AieDomain->Array.NocAddress >> 32U),
				(u32)(AieDomain->Array.NocAddress));
	}

	/* Check for ME Power Status */
	if( (XPm_In32(BaseAddress + NPI_PCSR_STATUS_OFFSET) &
			 ME_NPI_REG_PCSR_STATUS_ME_PWR_SUPPLY_MASK) !=
			 ME_NPI_REG_PCSR_STATUS_ME_PWR_SUPPLY_MASK) {
		DbgErr = XPM_INT_ERR_POWER_SUPPLY;
		goto done;
	}

	/* Unlock ME PCSR */
	XPm_UnlockPcsr(BaseAddress);

	/* Relelase IPOR */
	Status = XPm_PcsrWrite(BaseAddress, ME_NPI_REG_PCSR_MASK_ME_IPOR_MASK, 0U);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_RST_RELEASE;
		goto fail;
	}

	/**
	 * Configure ME_TOP_ROW:
	 *	- ROW_OFFSET = 0
	 *	- ME_TOP_ROW = Total number of rows in the array
	 */
	PmOut32((BaseAddress + ME_NPI_ME_TOP_ROW_OFFSET), AieDomain->Array.NumRowsAdjusted);

	/*
	 * To maintain backwards compatibility, skip locking of AIE NPI space. NPI
	 * space shall remain unlocked for entire housecleaning sequence unless
	 * failure occurs.
	 */
	goto done;

fail:
	/* Lock ME PCSR */
	XPm_LockPcsr(BaseAddress);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus Aie2InitStart(XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddress = 0U;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	XPm_AieDomain *AieDomain = (XPm_AieDomain *)PwrDomain;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	const XPm_Device * const Aie2Dev = XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == Aie2Dev) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	BaseAddress = Aie2Dev->Node.BaseAddress;

	/* Use AIE NoC Address if available */
	if (2U <= NumOfArgs) {
		AieDomain->Array.NocAddress = ((u64)Args[1] << 32U) | (Args[0]);
		PmDbg("AIE: NoC Address: 0x%x%08x\r\n",
				(u32)(AieDomain->Array.NocAddress >> 32U),
				(u32)(AieDomain->Array.NocAddress));
	}

	/* Check for AIE2 Power Status */
	if ((XPm_In32(BaseAddress + NPI_PCSR_STATUS_OFFSET) &
			ME_NPI_REG_PCSR_STATUS_ME_PWR_SUPPLY_MASK) !=
			ME_NPI_REG_PCSR_STATUS_ME_PWR_SUPPLY_MASK) {
		DbgErr = XPM_INT_ERR_POWER_SUPPLY;
		goto done;
	}

	/* Unlock AIE PCSR */
	XPm_UnlockPcsr(BaseAddress);

	/* Release IPOR */
	Status = XPm_PcsrWrite(BaseAddress, ME_NPI_REG_PCSR_MASK_ME_IPOR_MASK, 0U);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_RST_RELEASE;
		goto fail;
	}

	/**
	 * Configure ME_TOP_ROW:
	 *	- ROW_OFFSET = 0
	 *	- ME_TOP_ROW = Total number of rows in the array
	 */
	PmOut32((BaseAddress + ME_NPI_ME_TOP_ROW_OFFSET), AieDomain->Array.NumRowsAdjusted);

	/* Change from AIE to AIE2. AIE handles in CDO */
	/* De-assert INIT_STATE */
	Status = XPm_PcsrWrite(BaseAddress, ME_NPI_REG_PCSR_MASK_INITSTATE_MASK, 0U);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_AIE_INITSTATE_RELEASE;
		goto fail;
	}

	/* Change from AIE to AIE2. AIE handles in CDO */
	/* De-assert AIE2 array reset */
	Status = XPm_PcsrWrite(BaseAddress, ME_NPI_REG_PCSR_MASK_ME_ARRAY_RESET_MASK, 0U);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_ARRAY_RESET_RELEASE;
		goto fail;
	}

	/*
	 * To maintain backwards compatibility, skip locking of AIE NPI space. NPI
	 * space shall remain unlocked for entire housecleaning sequence unless
	 * failure occurs.
	 */
	goto done;

fail:
	/* Lock AIE PCSR */
	XPm_LockPcsr(BaseAddress);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus AieInitFinish(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddress = 0U;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_AieDomain *AieDomain = (const XPm_AieDomain *)PwrDomain;
	u32 ClkDivider;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	XPm_AieNode *AieDev = (XPm_AieNode *)XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == AieDev) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	BaseAddress = AieDev->Device.Node.BaseAddress;

	/* Set PCOMPLETE bit */
	Status = XPm_PcsrWrite(BaseAddress, ME_NPI_REG_PCSR_MASK_PCOMPLETE_MASK,
				 ME_NPI_REG_PCSR_MASK_PCOMPLETE_MASK);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_AIE_PCOMPLETE;
		goto done;
	}

	/* Clock gate ME Array column-wise (except SHIM array) */
	AieClkGateByCol(AieDomain);

	/* Add AIE partition nodes. This is a temporary workaround until tools
	 * supports pm_add_node commands.
	 * TODO: Remove once supported
	 */
	Status = AddAieDeviceNode();
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Store initial clock devider value */
	/* TODO: Get clock address from clock topology */
	ClkDivider =  XPm_In32(BaseAddress + ME_CORE_REF_CTRL_OFFSET) & AIE_DIVISOR0_MASK;
	ClkDivider = ClkDivider >> AIE_DIVISOR0_SHIFT;

	AieDev->DefaultClockDiv = ClkDivider;

done:
	if (0U != BaseAddress) {
		/* Lock ME PCSR */
		XPm_LockPcsr(BaseAddress);
	}

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus Aie2InitFinish(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddress = 0U;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_AieDomain *AieDomain = (const XPm_AieDomain *)PwrDomain;
	u32 ClkDivider;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	XPm_AieNode *AieDev = (XPm_AieNode *)XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == AieDev) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	BaseAddress = AieDev->Device.Node.BaseAddress;

	/* Change from AIE to AIE2. */
	/* Clock gate for each column */
	Aie2ClockGate(AieDomain, BaseAddress);

	/* Change from AIE to AIE2. PCOMPLETE should be set at the end of the
	 * sequence */
	/* Set PCOMPLETE bit */
	Status = XPm_PcsrWrite(BaseAddress, ME_NPI_REG_PCSR_MASK_PCOMPLETE_MASK,
			ME_NPI_REG_PCSR_MASK_PCOMPLETE_MASK);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_AIE_PCOMPLETE;
	}

	/* Add AIE partition nodes. This is a temporary workaround until tools
	 * supports pm_add_node commands.
	 * TODO: Remove once supported.
	 */
	Status = AddAieDeviceNode();
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Store initial clock devider value */
	/* TODO: Get clock address from clock topology */
	ClkDivider =  XPm_In32(BaseAddress + ME_CORE_REF_CTRL_OFFSET) & AIE_DIVISOR0_MASK;
	ClkDivider = ClkDivider >> AIE_DIVISOR0_SHIFT;

	AieDev->DefaultClockDiv = ClkDivider;

done:
	if (0U != BaseAddress) {
		/* Lock AIE PCSR */
		XPm_LockPcsr(BaseAddress);
	}

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus AieScanClear(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddress = 0U;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_AieDomain *AieDomain = (const XPm_AieDomain *)PwrDomain;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	const XPm_Device * const AieDev = XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == AieDev) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	BaseAddress = AieDev->Node.BaseAddress;

	/* De-assert ODISABLE[1] */
	Status = XPm_PcsrWrite(BaseAddress, ME_NPI_REG_PCSR_MASK_ODISABLE_1_MASK, 0U);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_ODISABLE_1_RELEASE;
		goto fail;
	}

	if (PM_HOUSECLEAN_CHECK(AIE, SCAN)){
		PmInfo("Triggering ScanClear for power node 0x%x\r\n", PwrDomain->Power.Node.Id);

		/* Trigger Scan Clear */
		Status = XPm_PcsrWrite(BaseAddress, ME_NPI_REG_PCSR_MASK_SCAN_CLEAR_TRIGGER_MASK,
						ME_NPI_REG_PCSR_MASK_SCAN_CLEAR_TRIGGER_MASK);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_SCAN_CLEAR_TRIGGER;
			goto fail;
		}

		XPlmi_Printf(DEBUG_INFO, "INFO: %s : Wait for AIE Scan Clear complete...", __func__);

		/* Wait for Scan Clear DONE */
		Status = XPm_PollForMask(BaseAddress + NPI_PCSR_STATUS_OFFSET,
					 ME_NPI_REG_PCSR_STATUS_SCAN_CLEAR_DONE_MASK,
					 AIE_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_SCAN_CLEAR_TIMEOUT;
			XPlmi_Printf(DEBUG_INFO, "ERROR\r\n");
			goto fail;
		}
		else {
			XPlmi_Printf(DEBUG_INFO, "DONE\r\n");
		}

		/* Check Scan Clear PASS */
		if( (XPm_In32(BaseAddress + NPI_PCSR_STATUS_OFFSET) &
		     ME_NPI_REG_PCSR_STATUS_SCAN_CLEAR_PASS_MASK) !=
		    ME_NPI_REG_PCSR_STATUS_SCAN_CLEAR_PASS_MASK) {
			DbgErr = XPM_INT_ERR_SCAN_CLEAR_PASS;
			XPlmi_Printf(DEBUG_GENERAL, "ERROR: %s: AIE Scan Clear FAILED\r\n", __func__);
			Status = XST_FAILURE;
			goto fail;
		}
	} else {
		/* ScanClear is skipped */
		PmInfo("Skipping ScanClear for power node 0x%x\r\n", PwrDomain->Power.Node.Id);
	}

	/* De-assert ODISABLE[0] */
	Status = XPm_PcsrWrite(BaseAddress, ME_NPI_REG_PCSR_MASK_ODISABLE_0_MASK, 0U);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_ODISABLE_0_RELEASE;
		goto fail;
	}

	/* De-assert GATEREG */
	Status = XPm_PcsrWrite(BaseAddress, ME_NPI_REG_PCSR_MASK_GATEREG_MASK, 0U);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_GATEREG_UNSET;
		goto fail;
	}

	/**
	 * Call post scan clear hook for AIE power domain, if available.
	 * A couple things to note:
	 *	- NPI space must already be unlocked before calling the hook (which it is)
	 *	- If failure occurs within the hook, NPI space must be locked in caller
	 */
	if (NULL != AieDomain->Hooks.PostScanClearHook) {
		Status = AieDomain->Hooks.PostScanClearHook(AieDomain, BaseAddress);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_AIE_POST_SCAN_CLEAR_HOOK;
			goto fail;
		}
	}

	 /*
	  * To maintain backwards compatibility, skip locking of AIE NPI space. NPI
	  * space shall remain unlocked for entire housecleaning sequence unless
	  * failure occurs.
	  */
	goto done;

fail:
	/* Lock ME PCSR */
	XPm_LockPcsr(BaseAddress);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus AiePostScanClearHook(const XPm_AieDomain *AieDomain, u32 BaseAddress)
{
	XStatus Status = XST_FAILURE;

	(void)AieDomain;
	(void)BaseAddress;

	/* De-assert INIT_STATE */
	Status = XPm_PcsrWrite(BaseAddress, ME_NPI_REG_PCSR_MASK_INITSTATE_MASK, 0U);

	return Status;
}

static XStatus AiePreBisrHook(const XPm_AieDomain *AieDomain, u32 BaseAddress)
{
	XStatus Status = XST_FAILURE;

	(void)AieDomain;

	/* Config AIE SMID: ME_SMID_REG.ME_SMID[4:0]=0xf */
	PmOut32((BaseAddress + ME_NPI_ME_SMID_REG), 0xFU);

	/* Make AIE block non-secure: ME_SECURE_REG.ME_SECURE[0]=0x0 */
	PmOut32((BaseAddress + ME_NPI_ME_SECURE_REG), 0U);

	/* De-assert AIE array reset */
	Status = XPm_PcsrWrite(BaseAddress, ME_NPI_REG_PCSR_MASK_ME_ARRAY_RESET_MASK, 0U);

	return Status;
}

static XStatus AieBisr(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddress = 0U;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_AieDomain *AieDomain = (const XPm_AieDomain *)PwrDomain;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	const XPm_Device * const AieDev = XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == AieDev) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	BaseAddress = AieDev->Node.BaseAddress;

	/**
	 * Call pre bisr hook for AIE power domain, if available.
	 * A couple things to note:
	 *	- NPI space must already be unlocked before calling the hook (which it is)
	 *	- If failure occurs within the hook, NPI space must be locked in caller
	 */
	if (NULL != AieDomain->Hooks.PreBisrHook) {
		Status = AieDomain->Hooks.PreBisrHook(AieDomain, BaseAddress);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_AIE_PRE_BISR_HOOK;
			goto fail;
		}
	}

	/* Remove PMC-NoC domain isolation */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_SOC, FALSE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PMC_SOC_ISO;
		goto fail;
	}

	if (PM_HOUSECLEAN_CHECK(AIE, BISR)){
		PmInfo("Triggering BISR for power node 0x%x\r\n", PwrDomain->Power.Node.Id);

		Status = XPmBisr_Repair(MEA_TAG_ID);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MEA_BISR_REPAIR;
			goto fail;
		}

		Status = XPmBisr_Repair(MEB_TAG_ID);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MEB_BISR_REPAIR;
			goto fail;
		}

		Status = XPmBisr_Repair(MEC_TAG_ID);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MEC_BISR_REPAIR;
			goto fail;
		}
	} else {
		/* BISR is skipped */
		PmInfo("Skipping BISR for power node 0x%x\r\n", PwrDomain->Power.Node.Id);
	}

	/*
	 * To maintain backwards compatibility, skip locking of AIE NPI space. NPI
	 * space shall remain unlocked for entire housecleaning sequence unless
	 * failure occurs.
	 */
	goto done;

fail:
	/* Lock ME PCSR */
	XPm_LockPcsr(BaseAddress);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus Aie2PreBisrHook(const XPm_AieDomain *AieDomain, u32 BaseAddress)
{
	XStatus Status = XST_FAILURE;

	(void)AieDomain;

	/* Clear AIE2 Shim Reset */
	Status = XPm_PcsrWrite(BaseAddress, ME_NPI_REG_PCSR_MASK_ME_SHIM_RESET_MASK, 0U);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Config AIE SMID: ME_SMID_REG.ME_SMID[4:0]=0xf */
	PmOut32((BaseAddress + ME_NPI_ME_SMID_REG), 0xFU);

	/* Make AIE block non-secure: ME_SECURE_REG.ME_SECURE[0]=0x0 */
	PmOut32((BaseAddress + ME_NPI_ME_SECURE_REG), 0U);

done:
	return Status;
}

static XStatus Aie2Bisr(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddress = 0U;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_AieDomain *AieDomain = (const XPm_AieDomain *)PwrDomain;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	const XPm_Device * const AieDev = XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == AieDev) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	BaseAddress = AieDev->Node.BaseAddress;

	/**
	 * Call pre bisr hook for AIE power domain, if available.
	 * A couple things to note:
	 *	- NPI space must already be unlocked before calling the hook (which it is)
	 *	- If failure occurs within the hook, NPI space must be locked in caller
	 */
	if (NULL != AieDomain->Hooks.PreBisrHook) {
		Status = AieDomain->Hooks.PreBisrHook(AieDomain, BaseAddress);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_AIE_PRE_BISR_HOOK;
			goto fail;
		}
	}

	/* Remove PMC-NoC domain isolation */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_SOC, FALSE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PMC_SOC_ISO;
		goto fail;
	}

	/* Change from AIE to AIE2. AIE has clocks enabled by default whereas AIE2
	 * has then disabled by default. Clocks must be up from this point to
	 * continue the sequence */
	/* Enable all column clocks */
	Aie2ClockInit(AieDomain, BaseAddress);

	if (PM_HOUSECLEAN_CHECK(AIE, BISR)){
		PmInfo("Triggering BISR for power node 0x%x\r\n", PwrDomain->Power.Node.Id);

		Status = XPmBisr_Repair(MEA_TAG_ID);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MEA_BISR_REPAIR;
			goto fail;
		}

		Status = XPmBisr_Repair(MEB_TAG_ID);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MEB_BISR_REPAIR;
			goto fail;
		}

		Status = XPmBisr_Repair(MEC_TAG_ID);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MEC_BISR_REPAIR;
			goto fail;
		}
	} else {
		/* BISR is skipped */
		PmInfo("Skipping BISR for power node 0x%x\r\n", PwrDomain->Power.Node.Id);
	}

	/*
	 * To maintain backwards compatibility, skip locking of AIE NPI space. NPI
	 * space shall remain unlocked for entire housecleaning sequence unless
	 * failure occurs.
	 */
	goto done;

fail:
	/* Lock ME PCSR */
	XPm_LockPcsr(BaseAddress);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus TriggerMemClear(u32 BaseAddress, u16 *DbgErr)
{
	XStatus Status = XST_FAILURE;

	/* Clear MEM_CLEAR_EN_ALL to minimize power during mem clear */
	Status = XPm_PcsrWrite(BaseAddress, ME_NPI_REG_PCSR_MASK_MEM_CLEAR_EN_ALL_MASK, 0U);
	if (XST_SUCCESS != Status) {
		*DbgErr = XPM_INT_ERR_MEM_CLEAR_EN;
		goto done;
	}

	/* Set OD_MBIST_ASYNC_RESET_N bit */
	Status = XPm_PcsrWrite(BaseAddress, ME_NPI_REG_PCSR_MASK_OD_MBIST_ASYNC_RESET_N_MASK,
			      ME_NPI_REG_PCSR_MASK_OD_MBIST_ASYNC_RESET_N_MASK);
	if (XST_SUCCESS != Status) {
		*DbgErr = XPM_INT_ERR_MBIST_RESET;
		goto done;
	}

	/* Assert OD_BIST_SETUP_1 */
	Status = XPm_PcsrWrite(BaseAddress, ME_NPI_REG_PCSR_MASK_OD_BIST_SETUP_1_MASK,
			      ME_NPI_REG_PCSR_MASK_OD_BIST_SETUP_1_MASK);
	if (XST_SUCCESS != Status) {
		*DbgErr = XPM_INT_ERR_BIST_RESET;
		goto done;
	}

	/* Assert MEM_CLEAR_TRIGGER */
	Status = XPm_PcsrWrite(BaseAddress, ME_NPI_REG_PCSR_MASK_MEM_CLEAR_TRIGGER_MASK,
			      ME_NPI_REG_PCSR_MASK_MEM_CLEAR_TRIGGER_MASK);
	if (XST_SUCCESS != Status) {
		*DbgErr = XPM_INT_ERR_MEM_CLEAR_TRIGGER;
	}

done:
	return Status;
}

static XStatus IsMemClearDone(const u32 BaseAddress, u16 *DbgErr)
{
	XStatus Status = XST_FAILURE;

	/* Wait for Mem Clear DONE */
	Status = XPm_PollForMask(BaseAddress + NPI_PCSR_STATUS_OFFSET,
				 ME_NPI_REG_PCSR_STATUS_MEM_CLEAR_DONE_MASK,
				 AIE_POLL_TIMEOUT);
	if (Status != XST_SUCCESS) {
		*DbgErr = XPM_INT_ERR_MEM_CLEAR_DONE_TIMEOUT;
		XPlmi_Printf(DEBUG_INFO, "ERROR\r\n");
		goto done;
	}
	else {
		XPlmi_Printf(DEBUG_INFO, "DONE\r\n");
	}

	/* Check Mem Clear PASS */
	if ((XPm_In32(BaseAddress + NPI_PCSR_STATUS_OFFSET) &
		      ME_NPI_REG_PCSR_STATUS_MEM_CLEAR_PASS_MASK) !=
		      ME_NPI_REG_PCSR_STATUS_MEM_CLEAR_PASS_MASK) {
		XPlmi_Printf(DEBUG_GENERAL, "ERROR: %s: AIE Mem Clear FAILED\r\n", __func__);
		*DbgErr = XPM_INT_ERR_MEM_CLEAR_PASS;
		Status = XST_FAILURE;
	}

done:
	return Status;
}

static XStatus CleanupMemClear(u32 BaseAddress, u16 *DbgErr)
{
	XStatus Status = XST_FAILURE;

	/* Clear OD_MBIST_ASYNC_RESET_N bit */
	Status = XPm_PcsrWrite(BaseAddress, ME_NPI_REG_PCSR_MASK_OD_MBIST_ASYNC_RESET_N_MASK, 0U);
	if (XST_SUCCESS != Status) {
		*DbgErr = XPM_INT_ERR_MBIST_RESET_RELEASE;
		goto done;
	}

	/* De-assert OD_BIST_SETUP_1 */
	Status = XPm_PcsrWrite(BaseAddress, ME_NPI_REG_PCSR_MASK_OD_BIST_SETUP_1_MASK, 0U);
	if (XST_SUCCESS != Status) {
		*DbgErr = XPM_INT_ERR_BIST_RESET_RELEASE;
		goto done;
	}

	/* De-assert MEM_CLEAR_TRIGGER */
	Status = XPm_PcsrWrite(BaseAddress, ME_NPI_REG_PCSR_MASK_MEM_CLEAR_TRIGGER_MASK, 0U);
	if (XST_SUCCESS != Status) {
		*DbgErr = XPM_INT_ERR_MEM_CLEAR_TRIGGER_UNSET;
	}
done:
	return Status;
}

static XStatus AieMbistClear(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	volatile XStatus Status = XST_FAILURE;
	u32 BaseAddress = 0U;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	const XPm_Device * const AieDev = XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == AieDev) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	BaseAddress = AieDev->Node.BaseAddress;

	if (PM_HOUSECLEAN_CHECK(AIE, MBIST)) {
		PmInfo("Triggering MBIST for power node 0x%x\r\n", PwrDomain->Power.Node.Id);

		Status = TriggerMemClear(BaseAddress, &DbgErr);
		if (XST_SUCCESS != Status) {
			goto fail;
		}

		XPlmi_Printf(DEBUG_INFO, "INFO: %s : Wait for AIE Mem Clear complete...", __func__);

		Status = IsMemClearDone(BaseAddress, &DbgErr);
		if (XST_SUCCESS != Status) {
			goto fail;
		}

		Status = CleanupMemClear(BaseAddress, &DbgErr);
		if (XST_SUCCESS != Status) {
			goto fail;
		}
	} else {
		/* MBIST is skipped */
		PmInfo("Skipping MBIST for power node 0x%x\r\n", PwrDomain->Power.Node.Id);
		Status = XST_SUCCESS;
	}

	/*
	 * To maintain backwards compatibility, skip locking of AIE NPI space. NPI
	 * space shall remain unlocked for entire housecleaning sequence unless
	 * failure occurs.
	 */
	goto done;

fail:
	/* Lock ME PCSR */
	XPm_LockPcsr(BaseAddress);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus Aie2MbistClear(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	volatile XStatus Status = XST_FAILURE;
	u32 BaseAddress = 0U;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	const XPm_Device * const AieDev = XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == AieDev) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	BaseAddress = AieDev->Node.BaseAddress;

	if (PM_HOUSECLEAN_CHECK(AIE, MBIST)) {
		PmInfo("Triggering MBIST for power node 0x%x\r\n", PwrDomain->Power.Node.Id);
		/* Change from AIE to AIE2. */
		/* TODO: In AIE this is set to low power mode to avoid failures. Need
		 * confirmation that for AIE2 low power mode is not required. */
		/* Assert MEM_CLEAR_EN_ALL */
		Status = XPm_PcsrWrite(BaseAddress, ME_NPI_REG_PCSR_MASK_MEM_CLEAR_EN_ALL_MASK,
				ME_NPI_REG_PCSR_MASK_MEM_CLEAR_EN_ALL_MASK);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MEM_CLEAR_EN;
			goto fail;
		}

		/* Clear OD_MBIST_ASYNC_RESET_N bit */
		Status = XPm_PcsrWrite(BaseAddress, ME_NPI_REG_PCSR_MASK_OD_MBIST_ASYNC_RESET_N_MASK, 0U);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MBIST_RESET;
			goto fail;
		}

		/* Assert OD_BIST_SETUP_1 */
		Status = XPm_PcsrWrite(BaseAddress, ME_NPI_REG_PCSR_MASK_OD_BIST_SETUP_1_MASK,
				ME_NPI_REG_PCSR_MASK_OD_BIST_SETUP_1_MASK);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_BIST_RESET;
			goto fail;
		}

		/* Assert MEM_CLEAR_TRIGGER */
		Status = XPm_PcsrWrite(BaseAddress, ME_NPI_REG_PCSR_MASK_MEM_CLEAR_TRIGGER_MASK,
				ME_NPI_REG_PCSR_MASK_MEM_CLEAR_TRIGGER_MASK);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MEM_CLEAR_TRIGGER;
			goto fail;
		}

		XPlmi_Printf(DEBUG_INFO, "INFO: %s : Wait for AIE Mem Clear complete...", __func__);

		/* Wait for Mem Clear DONE */
		Status = XPm_PollForMask(BaseAddress + NPI_PCSR_STATUS_OFFSET,
				ME_NPI_REG_PCSR_STATUS_MEM_CLEAR_DONE_MASK,
				AIE_POLL_TIMEOUT);
		if (Status != XST_SUCCESS) {
			DbgErr = XPM_INT_ERR_MEM_CLEAR_DONE_TIMEOUT;
			XPlmi_Printf(DEBUG_INFO, "ERROR\r\n");
			goto fail;
		}
		else {
			XPlmi_Printf(DEBUG_INFO, "DONE\r\n");
		}

		/* Check Mem Clear PASS */
		if ((XPm_In32(BaseAddress + NPI_PCSR_STATUS_OFFSET) &
					ME_NPI_REG_PCSR_STATUS_MEM_CLEAR_PASS_MASK) !=
				ME_NPI_REG_PCSR_STATUS_MEM_CLEAR_PASS_MASK) {
			XPlmi_Printf(DEBUG_GENERAL, "ERROR: %s: AIE Mem Clear FAILED\r\n", __func__);
			DbgErr = XPM_INT_ERR_MEM_CLEAR_PASS;
			Status = XST_FAILURE;
			goto fail;
		}

		/* Assert OD_MBIST_ASYNC_RESET_N bit */
		Status = XPm_PcsrWrite(BaseAddress, ME_NPI_REG_PCSR_MASK_OD_MBIST_ASYNC_RESET_N_MASK,
				ME_NPI_REG_PCSR_MASK_OD_MBIST_ASYNC_RESET_N_MASK);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MBIST_RESET_RELEASE;
			goto fail;
		}

		/* De-assert OD_BIST_SETUP_1 */
		Status = XPm_PcsrWrite(BaseAddress, ME_NPI_REG_PCSR_MASK_OD_BIST_SETUP_1_MASK, 0U);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_BIST_RESET_RELEASE;
			goto fail;
		}

		/* De-assert MEM_CLEAR_TRIGGER */
		Status = XPm_PcsrWrite(BaseAddress, ME_NPI_REG_PCSR_MASK_MEM_CLEAR_TRIGGER_MASK, 0U);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MEM_CLEAR_TRIGGER_UNSET;
			goto fail;
		}
	} else {
		/* MBIST is skipped */
		PmInfo("Skipping MBIST for power node 0x%x\r\n", PwrDomain->Power.Node.Id);
		Status = XST_SUCCESS;
	}

	/*
	 * To maintain backwards compatibility, skip locking of AIE NPI space. NPI
	 * space shall remain unlocked for entire housecleaning sequence unless
	 * failure occurs.
	 */
	goto done;

fail:
	/* Lock AIE PCSR */
	XPm_LockPcsr(BaseAddress);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus AieMemInit(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddress = 0U;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_AieDomain *AieDomain = (const XPm_AieDomain *)PwrDomain;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	const XPm_Device * const AieDev = XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == AieDev) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	BaseAddress = AieDev->Node.BaseAddress;

	PmDbg("---------- START ----------\r\n");

	/* Enable scrub, Scrub ECC protected memories */
	TriggerEccScrub(AieDomain, ECC_SCRUB_ENABLE);
	/* Wait for scrubbing to finish (1ms)*/
	AieWait(1000U);

	/* Disable scrub, Scrub ECC protected memories */
	TriggerEccScrub(AieDomain, ECC_SCRUB_DISABLE);

	/* Reset Array */
	Status = ArrayReset(BaseAddress);
	if (XST_SUCCESS != Status) {
		PmErr("ERROR: Array reset failed\r\n");
	}
	/* Zeroize Data Memory */
	Status = AieCoreMemInit(AieDomain);
	if (Status != XST_SUCCESS) {
		PmInfo("ERROR: MemInit failed\r\n");
	}
	/* Reset Array */
	Status = ArrayReset(BaseAddress);
	if (XST_SUCCESS != Status) {
		PmErr("ERROR: Array reset failed\r\n");
		/* Lock ME PCSR */
		XPm_LockPcsr(BaseAddress);
		goto done;
	}
	PmDbg("---------- END ----------\r\n");

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus Aie2MemInit(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 BaseAddress;

	const XPm_AieArray *Array = &((const XPm_AieDomain *)PwrDomain)->Array;
	u16 StartCol = Array->StartCol;
	/* Adjust EndCol value as the StartCol will start from index 0 */
	u16 EndCol = (u16)(StartCol + Array->NumColsAdjusted - 1U);

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	const XPm_Device * const AieDev = XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == AieDev) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	BaseAddress = AieDev->Node.BaseAddress;
	Status = Aie2_Zeroization(AieDev, (u32)StartCol, (u32)EndCol);
	if (XST_SUCCESS != Status) {
		/* Lock ME PCSR */
		XPm_LockPcsr(BaseAddress);
		DbgErr = XPM_INT_ERR_AIE_MEMORY_ZEROISATION;
		goto done;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static struct XPm_PowerDomainOps AieOps[XPM_AIE_OPS_MAX] = {
	[XPM_AIE_OPS] = {
		.InitStart = AieInitStart,
		.InitFinish = AieInitFinish,
		.ScanClear = AieScanClear,
		.Bisr = AieBisr,
		.Mbist = AieMbistClear,
		.MemInit = AieMemInit,
		/* Mask to indicate which Ops are present */
		.InitMask = (BIT16(FUNC_INIT_START) |
				BIT16(FUNC_INIT_FINISH) |
				BIT16(FUNC_SCAN_CLEAR) |
				BIT16(FUNC_BISR) |
				BIT16(FUNC_MBIST_CLEAR) |
				BIT16(FUNC_MEM_INIT))
	},
	[XPM_AIE2_OPS] = {
		.InitStart = Aie2InitStart,
		.InitFinish = Aie2InitFinish,
		.ScanClear = AieScanClear,
		.Bisr = Aie2Bisr,
		.Mbist = Aie2MbistClear,
		.MemInit = Aie2MemInit,
		/* Mask to indicate which Ops are present */
		.InitMask = (BIT16(FUNC_INIT_START) |
				BIT16(FUNC_INIT_FINISH) |
				BIT16(FUNC_SCAN_CLEAR) |
				BIT16(FUNC_BISR) |
				BIT16(FUNC_MBIST_CLEAR) |
				BIT16(FUNC_MEM_INIT))
	},
};

XStatus XPmAieDomain_Init(XPm_AieDomain *AieDomain, u32 Id, u32 BaseAddress,
			  XPm_Power *Parent, const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u32 Platform = XPm_GetPlatform();
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u16 ArrayInfoPresent = 0U;
	const struct XPm_PowerDomainOps *Ops = NULL;
	XPm_AieArray *Array = &AieDomain->Array;
	XPm_AieDomainOpHooks *Hooks = &AieDomain->Hooks;

	(void)Args;
	(void)NumArgs;

	/* Set HC Ops based on AIE version */
	if (PM_POWER_ME == Id) {
		/* AIE1: Ops */
		Ops = &AieOps[XPM_AIE_OPS];
		/* AIE1 hooks for Ops */
		Hooks->PostScanClearHook = AiePostScanClearHook;
		Hooks->PreBisrHook = AiePreBisrHook;

		/* Non-Silicon defaults for SPP/EMU for AIE1 */
		if (Platform != PLATFORM_VERSION_SILICON) {
			Array->NumCols = 7U;
			Array->NumRows = 5U;
			Array->StartCol = 6U;
			Array->StartRow = 1U;
			Array->NumShimRows = 1U;
			Array->NumAieRows = Array->NumRows - Array->NumMemRows;
			Array->GenVersion = AIE_GENV1;
			Array->LColOffset = 0U;		/**< left col offset = 0 */
			Array->RColOffset = 0U;		/**< right col offset = 0 */
			Array->TRowOffset = 0U;		/**< top row offset = 0 */
			/* Skip this info from topology in this case */
			ArrayInfoPresent = 1U;
		}
	} else if (PM_POWER_ME2 == Id) {
		/* AIE2: Ops */
		Ops = &AieOps[XPM_AIE2_OPS];
		/* AIE2: Hooks for Ops */
		Hooks->PostScanClearHook = NULL;
		Hooks->PreBisrHook = Aie2PreBisrHook;
	} else {
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		Status = XPM_INVALID_PWRDOMAIN;
		goto done;
	}

	/**
	 * NOTE: This hardcoded AIE NoC Address will later be replaced;
	 * _if_ it is passed from pm_init_start for AIE PD command.
	 */
	Array->NocAddress = (u64)VIVADO_ME_BASEADDR;

	/* Read AIE array geometry info from topology if available */
	if ((3U <= NumArgs) && (1U != ArrayInfoPresent)) {
		Array->GenVersion = ARR_GENV(Args[0]);
		Array->NumRows = ARR_ROWS(Args[1]);
		Array->NumCols = ARR_COLS(Args[1]);
		Array->NumAieRows = ARR_AIEROWS(Args[2]);
		Array->NumMemRows = ARR_MEMROWS(Args[2]);
		Array->NumShimRows = ARR_SHMROWS(Args[2]);
		Array->StartCol = 0U;			/**< always start from first column */
		Array->StartRow = Array->NumShimRows;	/**< always start after shim row */

		/* TODO: Do this unconditionally when changes are available in topology */
		if (3U < NumArgs) {
			Array->LColOffset = ARR_LCOLOFF(Args[3]);
			Array->RColOffset = ARR_RCOLOFF(Args[3]);
			Array->TRowOffset = ARR_TROWOFF(Args[3]);
		} else {
			Array->LColOffset = 0U;		/**< left col offset = 0 */
			Array->RColOffset = 0U;		/**< right col offset = 0 */
			Array->TRowOffset = 0U;		/**< top row offset = 0 */
		}
	}

	/* Derive row and col ranges after offset adjustments */
	Array->StartCol += Array->LColOffset;
	Array->NumColsAdjusted = Array->NumCols - (u16)(Array->LColOffset + Array->RColOffset);
	Array->NumRowsAdjusted = Array->NumRows - Array->TRowOffset;

	/* NOP for HC on QEMU */
	if (Platform == PLATFORM_VERSION_QEMU) {
		Ops = NULL;
	}

	Status = XPmPowerDomain_Init(&AieDomain->Domain, Id, BaseAddress,
			Parent, Ops);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_POWER_DOMAIN_INIT;
	}

	/* Store the Node of Aie Domain */
	PmAieDomain = AieDomain;

	/* Clear AIE section of PMC RAM register reserved for houseclean disable */
	XPm_RMW32(PM_HOUSECLEAN_DISABLE_REG_2, PM_HOUSECLEAN_DISABLE_AIE_MASK, 0U);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static u8 Aie_TileType(const u32 Col, const u32 Row)
{
	u8 TileType;
	u8 RemVal;

	/* Check for the type of Tile based on (Col,Row) */
	if (0U == Row) {
		RemVal = (u8)((Col) % 4U);
		if((RemVal == 0U) || (RemVal == 1U)) {
			/* Shim-PL Tile */
			TileType = AIE_TILE_TYPE_SHIMPL;
		} else {
			/* Shim-NOC Tile */
			TileType = AIE_TILE_TYPE_SHIMNOC;
		}
	} else {
		TileType = AIE_TILE_TYPE_UNDEFINED;
	}

	return TileType;
}

static XStatus Aie1_ColRst(const XPm_Device *AieDev, const u32 ColStart, const u32 ColEnd)
{
	(void)AieDev;
	const XPm_AieDomain *AieDomain = PmAieDomain;
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u64 BaseAddress;
	u32 Col;

	for (Col = ColStart; Col <= ColEnd; Col++) {
		/* BaseAddress for AIE1 column */
		BaseAddress = AIE1_TILE_BADDR(NocAddress, Col, 0U);

		/* Gate columns */
		AieWrite64(BaseAddress + AIE_TILE_CLOCK_CONTROL_OFFSET, 0U);

		/* Set column Reset */
		AieWrite64(BaseAddress + AIE_PL_MODULE_COLUMN_RST_CTRL_OFFSET, 1U);

		/* Ungate columns */
		AieWrite64(BaseAddress + AIE_TILE_CLOCK_CONTROL_OFFSET, 3U);

		/* Un-Set column Reset */
		AieWrite64(BaseAddress + AIE_PL_MODULE_COLUMN_RST_CTRL_OFFSET, 0U);
	}

	return XST_SUCCESS;
}

static XStatus Aie1_ShimRst(const XPm_Device *AieDev, const u32 ColStart, const u32 ColEnd)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_AieDomain *AieDomain = PmAieDomain;
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u64 BaseAddress;
	u32 Col;

	/* Enable shim resets of columns */
	for (Col = ColStart; Col <= ColEnd; Col++) {
		/* BaseAddress for AIE1 column */
		BaseAddress = AIE1_TILE_BADDR(NocAddress, Col, 0U);

		/* Enable Shim Reset Per Column */
		AieWrite64(BaseAddress + AIE_TILE_COL_SHIM_RST_OFFSET, 1U);
	}

	/* Enable shim reset bit of AIE NPI PCSR register */
	Status = XPm_PcsrWrite(AieDev->Node.BaseAddress, ME_NPI_REG_PCSR_MASK_ME_SHIM_RESET_MASK,
			      ME_NPI_REG_PCSR_MASK_ME_SHIM_RESET_MASK);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_AIE_SHIM_RST_EN;
		goto done;
	}

	/* Disable shim reset bit of AIE NPI PCSR register */
	Status = XPm_PcsrWrite(AieDev->Node.BaseAddress, ME_NPI_REG_PCSR_MASK_ME_SHIM_RESET_MASK, 0U);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_AIE_SHIM_RST_DIS;
	}

done:
	/* Disable shim resets of columns */
	for (Col = ColStart; Col <= ColEnd; Col++) {
		/* BaseAddress for AIE1 column */
		BaseAddress = AIE1_TILE_BADDR(NocAddress, Col, 0U);

		/* Disable Shim Reset Per Column */
		AieWrite64(BaseAddress + AIE_TILE_COL_SHIM_RST_OFFSET, 1U);
	}

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus Aie1_EnbColClkBuff(const XPm_Device *AieDev, const u32 ColStart, const u32 ColEnd)
{
	(void)AieDev;
	const XPm_AieDomain *AieDomain = PmAieDomain;
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u64 BaseAddress;
	u32 Col;

	/* Enable column clock buffer */
	for (Col = ColStart; Col <= ColEnd; Col++) {
		/* BaseAddress for AIE1 column */
		BaseAddress = AIE1_TILE_BADDR(NocAddress, Col, 0U);

		/* Un-Gate columns */
		AieWrite64(BaseAddress + AIE_TILE_CLOCK_CONTROL_OFFSET, 3U);
	}

	return XST_SUCCESS;
}

static XStatus Aie1_DisColClkBuff(const XPm_Device *AieDev, const u32 ColStart, const u32 ColEnd)
{
	(void)AieDev;
	const XPm_AieDomain *AieDomain = PmAieDomain;
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u64 BaseAddress;
	u32 Col;

	/* Disable column clock buffer */
	for (Col = ColStart; Col <= ColEnd; Col++) {
		/* BaseAddress for AIE1 column */
		BaseAddress = AIE1_TILE_BADDR(NocAddress, Col, 0U);

		/* Gate columns */
		AieWrite64(BaseAddress + AIE_TILE_CLOCK_CONTROL_OFFSET, 0U);
	}

	return XST_SUCCESS;
}

static XStatus Aie1_EnbAxiMmErrEvent(const XPm_Device *AieDev, u32 ColStart, u32 ColEnd)
{
	const XPm_AieDomain *AieDomain = PmAieDomain;
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u32 NodeAddress = AieDev->Node.BaseAddress;
	u64 BaseAddress;
	u32 Col;

	/* Enable protect register */
	PmRmw32(NodeAddress + ME_NPI_ME_SPARE_CTRL_OFFSET,
		ME_NPI_ME_SPARE_CTRL_PROTECTED_REG_EN_MASK, 1U);

	for (Col = ColStart; Col <= ColEnd; Col++) {
		/* Skip if it's not an NOC Tile */
		if (AIE_TILE_TYPE_SHIMNOC != Aie_TileType(Col, 0U)) {
			continue;
		}

		/* BaseAddress for AIE1 column */
		BaseAddress = AIE1_TILE_BADDR(NocAddress, Col, 0U);

		/* Eanble AXI-MM decode and slave error events */
		XPm_RMW64(BaseAddress + AIE_NOC_MODULE_ME_AXIMM_CONFIG_OFFSET,
			  ME_AXIMM_CONFIG_DECERR_BLOCK_EN_MASK | ME_AXIMM_CONFIG_SLVERR_BLOCK_EN_MASK,
			  ME_AXIMM_CONFIG_DECERR_BLOCK_EN_MASK | ME_AXIMM_CONFIG_SLVERR_BLOCK_EN_MASK);
	}

	/* Disable protect register */
	PmRmw32(NodeAddress + ME_NPI_ME_SPARE_CTRL_OFFSET,
		ME_NPI_ME_SPARE_CTRL_PROTECTED_REG_EN_MASK, 0U);

	return XST_SUCCESS;
}

static XStatus Aie1_SetL2CtrlNpiIntr(const XPm_Device *AieDev, u32 ColStart, u32 ColEnd)
{
	const XPm_AieDomain *AieDomain = PmAieDomain;
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u32 NodeAddress = AieDev->Node.BaseAddress;
	u64 BaseAddress;
	u32 Col;

	/* Enable protect register */
	PmRmw32(NodeAddress + ME_NPI_ME_SPARE_CTRL_OFFSET,
		ME_NPI_ME_SPARE_CTRL_PROTECTED_REG_EN_MASK, 1U);

	for (Col = ColStart; Col <= ColEnd; Col++) {
		/* Skip if it's not an NOC Tile */
		if (AIE_TILE_TYPE_SHIMNOC != Aie_TileType(Col, 0U)) {
			continue;
		}

		/* BaseAddress for AIE1 column */
		BaseAddress = AIE1_TILE_BADDR(NocAddress, Col, 0U);

		/* Set L2 controller NPI INTR */
		AieWrite64(BaseAddress + AIE_NOC_MODULE_INTR_CTRL_L2_INTR_OFFSET, 1U);
	}

	/* Disable protect register */
	PmRmw32(NodeAddress + ME_NPI_ME_SPARE_CTRL_OFFSET,
		ME_NPI_ME_SPARE_CTRL_PROTECTED_REG_EN_MASK, 1U);

	return XST_SUCCESS;
}

static XStatus Aie1_Zeroization(const XPm_Device *AieDev, u32 ColStart, u32 ColEnd)
{
	(void)AieDev;
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_AieDomain *AieDomain = PmAieDomain;
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u64 BaseAddress;
	u64 Addr;
	u32 Col;

	/* write 0 to program memory 16KB from 0x20000 for every core module
	 * of every column of the partition */
	for (Col = ColStart; Col <= ColEnd; Col++) {
		/* BaseAddress for AIE1 column */
		BaseAddress = AIE1_TILE_BADDR(NocAddress, Col, 0U);
		/* Address of Program memory for AIE1 column */
		Addr = BaseAddress + AIE_PROGRAM_MEM_OFFSET;

		/* Initialize program memory with 0 */
		Status = XPlmi_EccInit(Addr, AIE1_PROG_MEM_SIZE);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_AIE_PROG_MEM_ZEROISATION;
			goto done;
		}
	}

	/* write 0 to data memory 32KB from 0x0 for every memory module of
	 * every column of the partition */
	for (Col = ColStart; Col <= ColEnd; Col++) {
		/* BaseAddress for AIE1 column */
		BaseAddress = AIE1_TILE_BADDR(NocAddress, Col, 0U);
		/* Address of Data memory for AIE1 column */
		Addr = BaseAddress + AIE_DATA_MEM_OFFSET;

		/* Initialize data memory with 0 */
		Status = XPlmi_EccInit(Addr, AIE1_DATA_MEM_SIZE);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_AIE_DATA_MEM_ZEROISATION;
			goto done;
		}
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus Aie2_ColRst(const XPm_Device *AieDev, const u32 ColStart, const u32 ColEnd)
{
	const XPm_AieDomain *AieDomain = PmAieDomain;
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u32 NodeAddress = AieDev->Node.BaseAddress;
	u64 BaseAddress;
	u32 RegVal = 0;
	u32 Col;

	/* Enable privileged write access */
	RegVal = (ME_PROT_REG_CTRL_PROTECTED_REG_EN_MASK |
		  ((ColStart & ME_PROT_REG_CTRL_WE_COL_ID_MASK) <<
		    ME_PROT_REG_CTRL_WE_COL_ID_FIRST_SHIFT) |
		  ((ColEnd & ME_PROT_REG_CTRL_WE_COL_ID_MASK) <<
		    ME_PROT_REG_CTRL_WE_COL_ID_LAST_SHIFT));
	XPm_Out32(NodeAddress + AIE2_NPI_ME_PROT_REG_CTRL_OFFSET, RegVal);

	for (Col = ColStart; Col <= ColEnd; Col++) {
		/* BaseAddress for AIE2 column */
		BaseAddress = AIE2_TILE_BADDR(NocAddress, Col, 0U);

		/* Gate columns */
		AieWrite64(BaseAddress + AIE2_PL_MODULE_COLUMN_CLK_CTRL_OFFSET, 0U);

		/* Set column Reset */
		AieWrite64(BaseAddress + AIE2_PL_MODULE_COLUMN_RST_CTRL_OFFSET, 1U);

		/* Ungate columns */
		AieWrite64(BaseAddress + AIE2_PL_MODULE_COLUMN_CLK_CTRL_OFFSET, 1U);

		/* Un-Set column Reset */
		AieWrite64(BaseAddress + AIE2_PL_MODULE_COLUMN_RST_CTRL_OFFSET, 0U);
	}

	/* Disable privileged write access */
	XPm_RMW32(NodeAddress + AIE2_NPI_ME_PROT_REG_CTRL_OFFSET,
		  ME_PROT_REG_CTRL_PROTECTED_REG_EN_MASK, 0U);

	return XST_SUCCESS;
}

static XStatus Aie2_ShimRst(const XPm_Device *AieDev, const u32 ColStart, const u32 ColEnd)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 NodeAddress = AieDev->Node.BaseAddress;
	u32 RegVal = 0;

	/* Enable privileged write access */
	RegVal = (ME_PROT_REG_CTRL_PROTECTED_REG_EN_MASK |
		  ((ColStart & ME_PROT_REG_CTRL_WE_COL_ID_MASK) <<
		    ME_PROT_REG_CTRL_WE_COL_ID_FIRST_SHIFT) |
		  ((ColEnd & ME_PROT_REG_CTRL_WE_COL_ID_MASK) <<
		    ME_PROT_REG_CTRL_WE_COL_ID_LAST_SHIFT));
	XPm_Out32(NodeAddress + AIE2_NPI_ME_PROT_REG_CTRL_OFFSET, RegVal);

	/* Enable SHIM reset */
	Status = XPm_PcsrWrite(NodeAddress, ME_NPI_REG_PCSR_MASK_ME_SHIM_RESET_MASK,
			      ME_NPI_REG_PCSR_MASK_ME_SHIM_RESET_MASK);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_AIE_SHIM_RST_EN;
	}

	/* Disable privileged write access */
	XPm_RMW32(NodeAddress + AIE2_NPI_ME_PROT_REG_CTRL_OFFSET,
		  ME_PROT_REG_CTRL_PROTECTED_REG_EN_MASK, 0U);

	/* Skip next pcsr write if previous one is failed */
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Disable SHIM reset */
	Status = XPm_PcsrWrite(NodeAddress, ME_NPI_REG_PCSR_MASK_ME_SHIM_RESET_MASK, 0U);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_AIE_SHIM_RST_DIS;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus Aie2_EnbColClkBuff(const XPm_Device *AieDev, u32 ColStart, u32 ColEnd)
{
	const XPm_AieDomain *AieDomain = PmAieDomain;
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u32 NodeAddress = AieDev->Node.BaseAddress;
	u64 BaseAddress;
	u32 RegVal = 0;
	u32 Col;

	/* Enable privileged write access */
	RegVal = (ME_PROT_REG_CTRL_PROTECTED_REG_EN_MASK |
		  ((ColStart & ME_PROT_REG_CTRL_WE_COL_ID_MASK) <<
		    ME_PROT_REG_CTRL_WE_COL_ID_FIRST_SHIFT) |
		  ((ColEnd & ME_PROT_REG_CTRL_WE_COL_ID_MASK) <<
		    ME_PROT_REG_CTRL_WE_COL_ID_LAST_SHIFT));
	XPm_Out32(NodeAddress + AIE2_NPI_ME_PROT_REG_CTRL_OFFSET, RegVal);

	for (Col = ColStart; Col <= ColEnd; Col++) {
		/* BaseAddress for AIE2 column */
		BaseAddress = AIE2_TILE_BADDR(NocAddress, Col, 0U);

		/* Ungate columns */
		AieWrite64(BaseAddress + AIE2_PL_MODULE_COLUMN_CLK_CTRL_OFFSET, 1U);
	}

	/* Disable privileged write access */
	XPm_RMW32(NodeAddress + AIE2_NPI_ME_PROT_REG_CTRL_OFFSET,
		  ME_PROT_REG_CTRL_PROTECTED_REG_EN_MASK, 0U);

	return XST_SUCCESS;
}

static XStatus Aie2_DisColClkBuff(const XPm_Device *AieDev, u32 ColStart, u32 ColEnd)
{
	const XPm_AieDomain *AieDomain = PmAieDomain;
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u32 NodeAddress = AieDev->Node.BaseAddress;
	u64 BaseAddress;
	u32 RegVal = 0;
	u32 Col;

	/* Enable privileged write access */
	RegVal = (ME_PROT_REG_CTRL_PROTECTED_REG_EN_MASK |
		  ((ColStart & ME_PROT_REG_CTRL_WE_COL_ID_MASK) <<
		    ME_PROT_REG_CTRL_WE_COL_ID_FIRST_SHIFT) |
		  ((ColEnd & ME_PROT_REG_CTRL_WE_COL_ID_MASK) <<
		    ME_PROT_REG_CTRL_WE_COL_ID_LAST_SHIFT));
	XPm_Out32(NodeAddress + AIE2_NPI_ME_PROT_REG_CTRL_OFFSET, RegVal);

	for (Col = ColStart; Col <= ColEnd; Col++) {
		/* BaseAddress for AIE2 column */
		BaseAddress = AIE2_TILE_BADDR(NocAddress, Col, 0U);

		/* Gate columns */
		AieWrite64(BaseAddress + AIE2_PL_MODULE_COLUMN_CLK_CTRL_OFFSET, 0U);
	}

	/* Disable privileged write access */
	XPm_RMW32(NodeAddress + AIE2_NPI_ME_PROT_REG_CTRL_OFFSET,
		  ME_PROT_REG_CTRL_PROTECTED_REG_EN_MASK, 0U);

	return XST_SUCCESS;
}

static XStatus Aie2_Zeroization(const XPm_Device *AieDev, u32 ColStart, u32 ColEnd)
{
	XStatus Status = XST_FAILURE;
	XStatus CoreZeroStatus = XST_FAILURE;
	XStatus MemZeroStatus = XST_FAILURE;
	XStatus MemTileZeroStatus = XST_FAILURE;
	const XPm_AieDomain *AieDomain = PmAieDomain;
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u32 NodeAddress = AieDev->Node.BaseAddress;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 RowStart = AieDomain->Array.StartRow;
	u32 RowEnd = RowStart + AieDomain->Array.NumRowsAdjusted - 1U;
	u32 StartTileRow = RowStart + AieDomain->Array.NumMemRows;
	u32 AieZeroizationTime = 0U;
	u32 RegVal = 0;
	u64 BaseAddress;
	u32 Col, Row, Mrow;

	/* Enable privileged write access */
	RegVal = (ME_PROT_REG_CTRL_PROTECTED_REG_EN_MASK |
		  ((ColStart & ME_PROT_REG_CTRL_WE_COL_ID_MASK) <<
		    ME_PROT_REG_CTRL_WE_COL_ID_FIRST_SHIFT) |
		  ((ColEnd & ME_PROT_REG_CTRL_WE_COL_ID_MASK) <<
		    ME_PROT_REG_CTRL_WE_COL_ID_LAST_SHIFT));
	XPm_Out32(NodeAddress + AIE2_NPI_ME_PROT_REG_CTRL_OFFSET, RegVal);

	/* Enable memory zeroization for AIE2 tiles.
	 * Enable for core and memory modules. */
	for (Col = ColStart; Col <= ColEnd; Col++) {
		for (Row = StartTileRow; Row <= RowEnd; Row++) {
			BaseAddress = AIE2_TILE_BADDR(NocAddress, Col, Row);

			AieWrite64(BaseAddress + AIE2_CORE_MODULE_MEM_CTRL_OFFSET,
				   AIE2_CORE_MODULE_MEM_CTRL_MEM_ZEROISATION_MASK);
			AieWrite64(BaseAddress + AIE2_MEM_MODULE_MEM_CTRL_OFFSET,
				   AIE2_MEM_MODULE_MEM_CTRL_MEM_ZEROISATION_MASK);
		}
	}

	/* Enable memory zeroization for mem tiles; stop before tile row begins */
	for (Col = ColStart; Col <= ColEnd; Col++) {
		for (Row = RowStart; Row < StartTileRow; Row++) {
			BaseAddress = AIE2_TILE_BADDR(NocAddress, Col, Row);

			AieRMW64(BaseAddress + AIE2_MEM_TILE_MODULE_MEM_CTRL_OFFSET,
				 AIE2_MEM_TILE_MODULE_MEM_CTRL_MEM_ZEROISATION_MASK,
				 AIE2_MEM_TILE_MODULE_MEM_CTRL_MEM_ZEROISATION_MASK);
		}
	}

	Col = ColEnd;
	Row = RowEnd;
	Mrow = (u32)(StartTileRow - 1U);

	/* Poll the last cell for each tile type for memory zeroization complete */
	while ((XST_SUCCESS != MemTileZeroStatus) ||
	       (XST_SUCCESS != CoreZeroStatus) ||
	       (XST_SUCCESS != MemZeroStatus)) {

		if (0U == (AIE2_MEM_TILE_MODULE_MEM_CTRL_MEM_ZEROISATION_MASK &
			   (AieRead64(AIE2_TILE_BADDR(NocAddress, Col, Mrow) + AIE2_MEM_TILE_MODULE_MEM_CTRL_OFFSET)))) {
			MemTileZeroStatus = XST_SUCCESS;
		}
		if (0U == (AIE2_CORE_MODULE_MEM_CTRL_MEM_ZEROISATION_MASK &
			   (AieRead64(AIE2_TILE_BADDR(NocAddress, Col, Row) + AIE2_CORE_MODULE_MEM_CTRL_OFFSET)))) {
			CoreZeroStatus = XST_SUCCESS;
		}
		if (0U == (AIE2_MEM_MODULE_MEM_CTRL_MEM_ZEROISATION_MASK &
			   (AieRead64(AIE2_TILE_BADDR(NocAddress, Col, Row) + AIE2_MEM_MODULE_MEM_CTRL_OFFSET)))) {
			MemZeroStatus = XST_SUCCESS;
		}

		AieZeroizationTime++;
		if (AieZeroizationTime > XPLMI_TIME_OUT_DEFAULT) {
			DbgErr = XPM_INT_ERR_AIE_MEMORY_ZEROISATION;
			Status = XPM_ERR_AIE_OPS_ZEROIZATION;
			goto done;
		}
	}

	Status = XST_SUCCESS;

done:
	/* Disable privileged write access */
	XPm_RMW32(NodeAddress + AIE2_NPI_ME_PROT_REG_CTRL_OFFSET,
		  ME_PROT_REG_CTRL_PROTECTED_REG_EN_MASK, 0U);

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus Aie2_EnbAxiMmErrEvent(const XPm_Device *AieDev, u32 ColStart, u32 ColEnd)
{
	const XPm_AieDomain *AieDomain = PmAieDomain;
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u32 NodeAddress = AieDev->Node.BaseAddress;
	u64 BaseAddress;
	u32 RegVal = 0;
	u32 Col;

	/* Enable privileged write access */
	RegVal = (ME_PROT_REG_CTRL_PROTECTED_REG_EN_MASK |
		  ((ColStart & ME_PROT_REG_CTRL_WE_COL_ID_MASK) <<
		    ME_PROT_REG_CTRL_WE_COL_ID_FIRST_SHIFT) |
		  ((ColEnd & ME_PROT_REG_CTRL_WE_COL_ID_MASK) <<
		    ME_PROT_REG_CTRL_WE_COL_ID_LAST_SHIFT));
	XPm_Out32(NodeAddress + AIE2_NPI_ME_PROT_REG_CTRL_OFFSET, RegVal);

	for (Col = ColStart; Col <= ColEnd; Col++) {
		/* Skip if it's not an NOC Tile */
		if (AIE_TILE_TYPE_SHIMNOC != Aie_TileType(Col, 0U)) {
			continue;
		}

		/* BaseAddress for AIE2 column */
		BaseAddress = AIE2_TILE_BADDR(NocAddress, Col, 0U);

		/* Eanble AXI-MM decode and slave error events */
		XPm_RMW64(BaseAddress + AIE2_NOC_MODULE_ME_AXIMM_CONFIG_OFFSET,
			  ME_AXIMM_CONFIG_DECERR_BLOCK_EN_MASK | ME_AXIMM_CONFIG_SLVERR_BLOCK_EN_MASK,
			  ME_AXIMM_CONFIG_DECERR_BLOCK_EN_MASK | ME_AXIMM_CONFIG_SLVERR_BLOCK_EN_MASK);
	}

	/* Disable privileged write access */
	XPm_RMW32(NodeAddress + AIE2_NPI_ME_PROT_REG_CTRL_OFFSET,
		  ME_PROT_REG_CTRL_PROTECTED_REG_EN_MASK, 0U);

	return XST_SUCCESS;
}

static XStatus Aie2_SetL2CtrlNpiIntr(const XPm_Device *AieDev, u32 ColStart, u32 ColEnd)
{
	const XPm_AieDomain *AieDomain = PmAieDomain;
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u32 NodeAddress = AieDev->Node.BaseAddress;
	u64 BaseAddress;
	u32 RegVal = 0;
	u32 Col;

	/* Enable privileged write access */
	RegVal = (ME_PROT_REG_CTRL_PROTECTED_REG_EN_MASK |
		  ((ColStart & ME_PROT_REG_CTRL_WE_COL_ID_MASK) <<
		    ME_PROT_REG_CTRL_WE_COL_ID_FIRST_SHIFT) |
		  ((ColEnd & ME_PROT_REG_CTRL_WE_COL_ID_MASK) <<
		    ME_PROT_REG_CTRL_WE_COL_ID_LAST_SHIFT));
	XPm_Out32(NodeAddress + AIE2_NPI_ME_PROT_REG_CTRL_OFFSET, RegVal);

	for (Col = ColStart; Col <= ColEnd; Col++) {
		/* Skip if it's not an NOC Tile */
		if (AIE_TILE_TYPE_SHIMNOC != Aie_TileType(Col, 0U)) {
			continue;
		}

		/* BaseAddress for AIE2 column */
		BaseAddress = AIE2_TILE_BADDR(NocAddress, Col, 0U);

		/* Set L2 controller NPI INTR */
		AieWrite64(BaseAddress + AIE2_NOC_MODULE_INTR_CTRL_L2_INTR_OFFSET, 1U);
	}

	/* Disable privileged write access */
	XPm_RMW32(NodeAddress + AIE2_NPI_ME_PROT_REG_CTRL_OFFSET,
		  ME_PROT_REG_CTRL_PROTECTED_REG_EN_MASK, 0U);

	return XST_SUCCESS;
}

static XStatus Aie1_Operation(const XPm_Device *AieDev, u32 Part, u32 Ops)
{
	XStatus Status = XST_FAILURE;
	const XPm_AieDomain *AieDomain = PmAieDomain;
	const XPm_AieArray *Array = &AieDomain->Array;
	u32 ColStart = (Part & AIE_START_COL_MASK);
	u32 NumCol = ((Part & AIE_NUM_COL_MASK) >> 16U);
	u32 ColEnd = ColStart + NumCol - 1U;
	u32 NodeAddress = AieDev->Node.BaseAddress;

	/* Check that column and operations are in range */
	if (((ColEnd) > ((u32)Array->NumCols + (u32)Array->StartCol - 1U)) ||
	    (ColStart < (u32)Array->StartCol) ||
	    (AIE_OPS_MAX < Ops)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Unlock AIE1 PCSR */
	XPm_UnlockPcsr(NodeAddress);

	/* Column Reset */
	if (0U != (AIE_OPS_COL_RST & Ops)) {
		Status = Aie1_ColRst(AieDev, ColStart, ColEnd);
		if (XST_SUCCESS != Status) {
			Status = XPM_ERR_AIE_OPS_COL_RST;
			goto done;
		}
	}

	/* Shim Reset */
	if (0U != (AIE_OPS_SHIM_RST & Ops)) {
		Status = Aie1_ShimRst(AieDev, ColStart, ColEnd);
		if (XST_SUCCESS != Status) {
			Status = XPM_ERR_AIE_OPS_SHIM_RST;
			goto done;
		}
	}

	/* Enable Column Clock Buffer */
	if (0U != (AIE_OPS_ENB_COL_CLK_BUFF & Ops)) {
		Status = Aie1_EnbColClkBuff(AieDev, ColStart, ColEnd);
		if (XST_SUCCESS != Status) {
			Status = XPM_ERR_AIE_OPS_ENB_COL_CLK_BUFF;
			goto done;
		}
	}

	/* Zeroization of Program and data memories */
	if (0U != (AIE_OPS_ZEROIZATION & Ops)) {
		Status = Aie1_Zeroization(AieDev, ColStart, ColEnd);
		if (XST_SUCCESS != Status) {
			Status = XPM_ERR_AIE_OPS_ZEROIZATION;
			goto done;
		}
	}

	/* Disable Column Clock Buffer */
	if (0U != (AIE_OPS_DIS_COL_CLK_BUFF & Ops)) {
		Status = Aie1_DisColClkBuff(AieDev, ColStart, ColEnd);
		if (XST_SUCCESS != Status) {
			Status = XPM_ERR_AIE_OPS_DIS_COL_CLK_BUFF;
			goto done;
		}
	}

	/* Enable AXI-MM error events */
	if (0U != (AIE_OPS_ENB_AXI_MM_ERR_EVENT & Ops)) {
		Status = Aie1_EnbAxiMmErrEvent(AieDev, ColStart, ColEnd);
		if (XST_SUCCESS != Status) {
			Status = XPM_ERR_AIE_OPS_ENB_AXI_MM_ERR_EVENT;
			goto done;
		}
	}

	/* Set L2 controller NPI INTR */
	if (0U != (AIE_OPS_SET_L2_CTRL_NPI_INTR & Ops)) {
		Status = Aie1_SetL2CtrlNpiIntr(AieDev, ColStart, ColEnd);
		if (XST_SUCCESS != Status) {
			Status = XPM_ERR_AIE_OPS_SET_L2_CTRL_NPI_INTR;
			goto done;
		}
	}

done:
	/* Lock AIE1 PCSR */
	XPm_LockPcsr(NodeAddress);

	return Status;
}
static XStatus Aie2_Operation(const XPm_Device *AieDev, u32 Part, u32 Ops)
{
	XStatus Status = XST_FAILURE;
	const XPm_AieDomain *AieDomain = PmAieDomain;
	const XPm_AieArray *Array = &AieDomain->Array;
	u32 ColStart = (Part & AIE_START_COL_MASK);
	u32 NumCol = ((Part & AIE_NUM_COL_MASK) >> 16U);
	u32 ColEnd = ColStart + NumCol - 1U;
	u32 NodeAddress = AieDev->Node.BaseAddress;

	/* Check that column and operations are in range */
	if (((ColEnd) > (u32)(Array->NumCols + Array->StartCol - 1U)) ||
	    (ColStart < (u32)Array->StartCol) ||
	    (AIE_OPS_MAX < Ops)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Unlock AIE2 PCSR */
	XPm_UnlockPcsr(NodeAddress);

	/* Column Reset */
	if (0U != (AIE_OPS_COL_RST & Ops)) {
		Status = Aie2_ColRst(AieDev, ColStart, ColEnd);
		if (XST_SUCCESS != Status) {
			Status = XPM_ERR_AIE_OPS_COL_RST;
			goto done;
		}
	}

	/* Shim Reset */
	if (0U != (AIE_OPS_SHIM_RST & Ops)) {
		Status = Aie2_ShimRst(AieDev, ColStart, ColEnd);
		if (XST_SUCCESS != Status) {
			Status = XPM_ERR_AIE_OPS_SHIM_RST;
			goto done;
		}
	}

	/* Enable Column Clock Buffer */
	if (0U != (AIE_OPS_ENB_COL_CLK_BUFF & Ops)) {
		Status = Aie2_EnbColClkBuff(AieDev, ColStart, ColEnd);
		if (XST_SUCCESS != Status) {
			Status = XPM_ERR_AIE_OPS_ENB_COL_CLK_BUFF;
			goto done;
		}
	}

	/* Zeroization of Program and data memories */
	if (0U != (AIE_OPS_ZEROIZATION & Ops)) {
		Status = Aie2_Zeroization(AieDev, ColStart, ColEnd);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/* Disable Column Clock Buffer */
	if (0U != (AIE_OPS_DIS_COL_CLK_BUFF & Ops)) {
		Status = Aie2_DisColClkBuff(AieDev, ColStart, ColEnd);
		if (XST_SUCCESS != Status) {
			Status = XPM_ERR_AIE_OPS_DIS_COL_CLK_BUFF;
			goto done;
		}
	}

	/* Enable AXI-MM error events */
	if (0U != (AIE_OPS_ENB_AXI_MM_ERR_EVENT & Ops)) {
		Status = Aie2_EnbAxiMmErrEvent(AieDev, ColStart, ColEnd);
		if (XST_SUCCESS != Status) {
			Status = XPM_ERR_AIE_OPS_ENB_AXI_MM_ERR_EVENT;
			goto done;
		}
	}

	/* Set L2 controller NPI INTR */
	if (0U != (AIE_OPS_SET_L2_CTRL_NPI_INTR & Ops)) {
		Status = Aie2_SetL2CtrlNpiIntr(AieDev, ColStart, ColEnd);
		if (XST_SUCCESS != Status) {
			Status = XPM_ERR_AIE_OPS_SET_L2_CTRL_NPI_INTR;
			goto done;
		}
	}

done:
	/* Lock AIE2 PCSR */
	XPm_LockPcsr(NodeAddress);

	return Status;
}

XStatus Aie_Operations(u32 Part, u32 Ops)
{
	XStatus Status = XST_FAILURE;
	const XPm_AieDomain *AieDomain = PmAieDomain;
	const XPm_AieArray *Array = &AieDomain->Array;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	const XPm_Device * const AieDev = XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == AieDev) {
		DbgErr = XPM_INT_ERR_DEV_AIE;
		goto done;
	}

	if (AIE_GENV2 == Array->GenVersion) {
		/* AIE2 Operations */
		Status = Aie2_Operation(AieDev, Part, Ops);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_AIE2_OPS;
		}
	} else {
		/* AIE1 Operations */
		Status = Aie1_Operation(AieDev, Part, Ops);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_AIE1_OPS;
		}
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
