/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_aie.h"
#include "xpm_runtime_aie.h"
#include "xpm_device.h"
#include "xpm_power.h"
#include "xpm_debug.h"
#include "xpm_regs.h"
#define AIE2PS_COL_SHIFT 25U
#define AIE2PS_ROW_SHIFT 20U
#define AIE2PS_TILE_BADDR(NocAddr, col, row)	\
	(((u64)(NocAddr)) + ((u64)(col) << AIE2PS_COL_SHIFT) + ((u64)(row) << AIE2PS_ROW_SHIFT))

#define AieWrite64(addr, val) swea(addr, val)
#define AieRead64(addr) lwea(addr)

/* Maximum buffer size in bytes for AIE Operations */
#define AIE_OPS_MAX_BUF_SIZE		(200U)

struct XPmAieOpsHandlers {
	enum XPmAieOperations Ops;
	XStatus (*Handler)(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer);
};

static inline void AieRMW64(u64 addr, u32 Mask, u32 Value)
{
	u32 l_val;
	l_val = AieRead64(addr);
	l_val = (l_val & (~Mask)) | (Mask & Value);
	AieWrite64(addr, l_val);
}

static XStatus Aie2ps_ColRst(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer);
static XStatus Aie2ps_ShimRst(const XPm_Device *AieDev, const u32 StartCol, const u32 EndCol, const void* Buffer);
static XStatus Aie2ps_EnbColClkBuff(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer);
static XStatus Aie2ps_UcZeroization(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer);
static XStatus Aie2ps_HandShake(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer);
static XStatus Aie2ps_IntcHwSts(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer);
static XStatus Aie2ps_Zeroization(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer);
static XStatus Aie2ps_EnbAxiMmIsolation(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer);
static XStatus Aie2ps_NmuConfig(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer);
static XStatus Aie2ps_DisMemPriv(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer);
static XStatus Aie2ps_DisMemInterleave(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer);
static XStatus Aie2ps_EnbUcDmaPause(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer);
static XStatus Aie2ps_EnbNocDmaPause(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer);
static XStatus Aie2ps_SetEccScrub(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer);
static XStatus Aie2ps_DisColClkBuff(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer);
static XStatus Aie2ps_HwErrInt(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer);
static XStatus Aie2ps_HwErrMask(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer);
static XStatus Aie2ps_EnbMemPriv(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer);
static XStatus Aie2ps_EnbAxiMmErrEvent(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer);
static XStatus Aie2ps_SetL2CtrlNpiIntr(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer);
static XStatus Aie2ps_StartNumCol(u32 *StartCol, u32 *EndCol, const void *Buffer);

static struct XPmAieOpsHandlers AieOpsHandlers[] = {
	{AIE_OPS_COL_RST,		Aie2ps_ColRst},
	{AIE_OPS_SHIM_RST,		Aie2ps_ShimRst},
	{AIE_OPS_UC_ZEROIZATION,	Aie2ps_UcZeroization},
	{AIE_OPS_ENB_COL_CLK_BUFF,	Aie2ps_EnbColClkBuff},
	{AIE_OPS_HANDSHAKE,		Aie2ps_HandShake},
	{AIE_OPS_CLR_HW_ERR_STS,	Aie2ps_IntcHwSts},
	{AIE_OPS_ALL_MEM_ZEROIZATION,	Aie2ps_Zeroization},
	{AIE_OPS_AXIMM_ISOLATION,	Aie2ps_EnbAxiMmIsolation},
	{AIE_OPS_NMU_CONFIG,		Aie2ps_NmuConfig},
	{AIE_OPS_DIS_MEM_PRIV,		Aie2ps_DisMemPriv},
	{AIE_OPS_DIS_MEM_INTERLEAVE,	Aie2ps_DisMemInterleave},
	{AIE_OPS_ENB_UC_DMA_PAUSE,	Aie2ps_EnbUcDmaPause},
	{AIE_OPS_ENB_NOC_DMA_PAUSE,	Aie2ps_EnbNocDmaPause},
	{AIE_OPS_SET_ECC_SCRUB_PERIOD,	Aie2ps_SetEccScrub},
	{AIE_OPS_DIS_COL_CLK_BUFF,	Aie2ps_DisColClkBuff},
	{AIE_OPS_HW_ERR_INT,		Aie2ps_HwErrInt},
	{AIE_OPS_HW_ERR_MASK,		Aie2ps_HwErrMask},
	{AIE_OPS_ENB_MEM_PRIV,		Aie2ps_EnbMemPriv},
	{AIE_OPS_ENB_AXI_MM_ERR_EVENT,	Aie2ps_EnbAxiMmErrEvent},
	{AIE_OPS_SET_L2_CTRL_NPI_INTR,	Aie2ps_SetL2CtrlNpiIntr},
	{AIE_OPS_PROG_MEM_ZEROIZATION,	Aie2ps_Zeroization},
	{AIE_OPS_DATA_MEM_ZEROIZATION,	Aie2ps_Zeroization},
	{AIE_OPS_MEM_TILE_ZEROIZATION,	Aie2ps_Zeroization},
};

static XStatus Aie2ps_StartNumCol(u32 *StartCol, u32 *EndCol, const void *Buffer) {
	XStatus Status = XST_FAILURE;
	const XPm_AieDomain *AieDomain = (XPm_AieDomain*)XPmPower_GetById(PM_POWER_ME2);
	const XPm_AieArray *Array;

	if (NULL == AieDomain) {
		goto done;
	}

	Array = &AieDomain->Array;

	*StartCol = ((struct XPm_AieOpStartNumCol *)Buffer)->StartCol;
	*EndCol = ((struct XPm_AieOpStartNumCol *)Buffer)->NumCol + *StartCol - 1U;

	/* Check that column and operations are in range */
	if ((*EndCol > ((u32)Array->NumCols + (u32)Array->StartCol - 1U)) ||
            (*StartCol < (u32)Array->StartCol)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

/*****************************************************************************/
/**
 * @brief This function zeroizes AIE2 data and/or program memory.
 *
 * @param AieDev 	AIE Device
 * @param StartCol 	Column start index for zeroization
 * @param EndCol 	Column end index for zeroization
 * @param Buffer	Ops buffer used to get OPS type
 *
 * @return XST_SUCCESS if zeroization successful, error code otherwise.
 *****************************************************************************/
static XStatus Aie2ps_Zeroization(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer)
{
	XStatus Status = XST_FAILURE;
	XStatus CoreZeroStatus = XST_FAILURE;
	XStatus MemZeroStatus = XST_FAILURE;
	XStatus MemTileZeroStatus = XST_FAILURE;
	const XPm_AieDomain *AieDomain = (XPm_AieDomain *)XPmPower_GetById(AieDev->Node.Id);
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 RowStart = AieDomain->Array.StartRow;
	u32 RowEnd = RowStart + AieDomain->Array.NumRowsAdjusted - 1U;
	u32 StartTileRow = RowStart + AieDomain->Array.NumMemRows;
	u32 AieZeroizationTime = 0U;
	u64 BaseAddress;
	u32 Col, Row, Mrow;
	u32 Ops = ((struct XPm_AieTypeLen *)Buffer)->Type;

	/* Unused parameter */
	(void)AieDev;

	/* Enable zeroization for program memory of core module. */
	if (0U != ((AIE_OPS_ALL_MEM_ZEROIZATION | AIE_OPS_PROG_MEM_ZEROIZATION)	& Ops)) {
		for (Col = StartCol; Col <= EndCol; Col++) {
			for (Row = StartTileRow; Row <= RowEnd; Row++) {
				BaseAddress = AIE2PS_TILE_BADDR(NocAddress, Col, Row);

				AieWrite64(BaseAddress + AIE2PS_CORE_MODULE_MEM_CTRL_OFFSET,
					   AIE2PS_CORE_MODULE_MEM_CTRL_MEM_ZEROISATION_MASK);
			}
		}
	} else {
		/* Wait for zeroization status only if corresponding memory is selected */
		CoreZeroStatus = XST_SUCCESS;
	}

	/* Enable zeroization for data memory of core module. */
	if (0U != ((AIE_OPS_ALL_MEM_ZEROIZATION | AIE_OPS_DATA_MEM_ZEROIZATION)	& Ops)) {
		for (Col = StartCol; Col <= EndCol; Col++) {
			for (Row = StartTileRow; Row <= RowEnd; Row++) {
				BaseAddress = AIE2PS_TILE_BADDR(NocAddress, Col, Row);

				AieWrite64(BaseAddress + AIE2PS_MEM_MODULE_MEM_CTRL_OFFSET,
					   AIE2PS_MEM_MODULE_MEM_CTRL_MEM_ZEROISATION_MASK);
			}
		}
	} else {
		/* Wait for zeroization status only if corresponding memory is selected */
		MemZeroStatus = XST_SUCCESS;
	}

	/* Enable memory zeroization for mem tiles; stop before tile row begins */
	if (0U != ((AIE_OPS_ALL_MEM_ZEROIZATION | AIE_OPS_MEM_TILE_ZEROIZATION)	& Ops)) {
		for (Col = StartCol; Col <= EndCol; Col++) {
			for (Row = RowStart; Row < StartTileRow; Row++) {
				BaseAddress = AIE2PS_TILE_BADDR(NocAddress, Col, Row);

				AieRMW64(BaseAddress + AIE2PS_MEM_TILE_MODULE_MEM_CTRL_OFFSET,
					 AIE2PS_MEM_TILE_MODULE_MEM_CTRL_MEM_ZEROISATION_MASK,
					 AIE2PS_MEM_TILE_MODULE_MEM_CTRL_MEM_ZEROISATION_MASK);
			}
		}
	} else {
		/* Wait for zeroization status only if corresponding memory is selected */
		MemTileZeroStatus = XST_SUCCESS;
	}

	Col = EndCol;
	Row = RowEnd;
	Mrow = (u32)(StartTileRow - 1U);

	/* Poll the last cell for each tile type for memory zeroization complete */
	while ((XST_SUCCESS != MemTileZeroStatus) ||
	       (XST_SUCCESS != CoreZeroStatus) ||
	       (XST_SUCCESS != MemZeroStatus)) {

		if ((XST_SUCCESS != MemTileZeroStatus) &&
		    (0U == (AIE2PS_MEM_TILE_MODULE_MEM_CTRL_MEM_ZEROISATION_MASK &
			    (AieRead64(AIE2PS_TILE_BADDR(NocAddress, Col, Mrow) +
				       AIE2PS_MEM_TILE_MODULE_MEM_CTRL_OFFSET))))) {
			MemTileZeroStatus = XST_SUCCESS;
		}
		if ((XST_SUCCESS != CoreZeroStatus) &&
		    (0U == (AIE2PS_CORE_MODULE_MEM_CTRL_MEM_ZEROISATION_MASK &
			   (AieRead64(AIE2PS_TILE_BADDR(NocAddress, Col, Row) +
			    AIE2PS_CORE_MODULE_MEM_CTRL_OFFSET))))) {
			CoreZeroStatus = XST_SUCCESS;
		}
		if ((XST_SUCCESS != MemZeroStatus) &&
		    (0U == (AIE2PS_MEM_MODULE_MEM_CTRL_MEM_ZEROISATION_MASK &
			    (AieRead64(AIE2PS_TILE_BADDR(NocAddress, Col, Row) +
				       AIE2PS_MEM_MODULE_MEM_CTRL_OFFSET))))) {
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
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus Aie2ps_UcZeroization(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer)
{
	XStatus Status = XST_FAILURE;
	const XPm_AieDomain *AieDomain = (XPm_AieDomain*)XPmPower_GetById(PM_POWER_ME2);
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u64 BaseAddress;
	u32 Col;
	u32 AieZeroizationTime = 0U;
	u32 Value = ((struct XPm_AieOpUcZeroisation *)Buffer)->Flag;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	XStatus UcZeroStatus = XST_FAILURE;

	/* Unused arguments */
	(void)AieDev;

	for (Col = StartCol; Col <= EndCol; Col++) {
		BaseAddress = AIE2PS_TILE_BADDR(NocAddress, Col, 0U);

		AieWrite64(BaseAddress + AIE2PS_UC_MODULE_MEM_ZERO_OFFSET, Value);
	}

	/* Poll the last cell for uc zeroization complete */
	while(XST_SUCCESS != UcZeroStatus) {
		if (0U == ((AieRead64(AIE2PS_TILE_BADDR(NocAddress, EndCol, 0U) +
				      AIE2PS_UC_MODULE_MEM_ZERO_OFFSET)) & Value)) {
			UcZeroStatus = XST_SUCCESS;
		}

		AieZeroizationTime++;
		if (AieZeroizationTime > XPLMI_TIME_OUT_DEFAULT) {
			DbgErr = XPM_INT_ERR_AIE_UC_ZEROISATION;
			Status = XPM_ERR_AIE_OPS_UC_ZEROIZATION;
			goto done;
		}
	}

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus Aie2ps_ColRst(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer)
{
	const XPm_AieDomain *AieDomain = (XPm_AieDomain*)XPmPower_GetById(PM_POWER_ME2);
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u64 BaseAddress;
	u32 Col;

	/* Unused arguments */
	(void)AieDev;
	(void)Buffer;

	for (Col = StartCol; Col <= EndCol; Col++) {
		/* BaseAddress for AIE2 column */
		BaseAddress = AIE2PS_TILE_BADDR(NocAddress, Col, 0U);

		/* Gate columns */
		AieWrite64(BaseAddress + AIE2PS_PL_MODULE_COLUMN_CLK_CTRL_OFFSET, 0U);

		/* Set column Reset */
		AieWrite64(BaseAddress + AIE2PS_PL_MODULE_COLUMN_RST_CTRL_OFFSET, 1U);

		/* Ungate columns */
		AieWrite64(BaseAddress + AIE2PS_PL_MODULE_COLUMN_CLK_CTRL_OFFSET, 1U);

		/* Un-Set column Reset */
		AieWrite64(BaseAddress + AIE2PS_PL_MODULE_COLUMN_RST_CTRL_OFFSET, 0U);
	}

	return XST_SUCCESS;
}

static XStatus Aie2ps_ShimRst(const XPm_Device *AieDev, const u32 StartCol, const u32 EndCol, const void* Buf)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 NodeAddress = AieDev->Node.BaseAddress;

	/* Unused arguments */
	(void)StartCol;
	(void)EndCol;
	(void)Buf;

	/* Unlock PCSR */
	XPm_UnlockPcsr(NodeAddress);

	/* Enable privileged write access */
	XPm_RMW32(NodeAddress + AIE2PS_NPI_ME_PROT_REG_CTRL_OFFSET,
		  ME_PROT_REG_CTRL_PROTECTED_REG_EN_MASK, 1U);

	/* Enable SHIM reset */
	Status = XPm_PcsrWrite(NodeAddress, ME_NPI_REG_PCSR_MASK_ME_SHIM_RESET_MASK,
			      ME_NPI_REG_PCSR_MASK_ME_SHIM_RESET_MASK);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_AIE_SHIM_RST_EN;
	}

	/* Disable privileged write access */
	XPm_RMW32(NodeAddress + AIE2PS_NPI_ME_PROT_REG_CTRL_OFFSET,
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
	/* Lock Pcsr */
	XPm_LockPcsr(NodeAddress);

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus Aie2ps_EnbColClkBuff(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer)
{
	const XPm_AieDomain *AieDomain = (XPm_AieDomain*)XPmPower_GetById(PM_POWER_ME2);
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u64 BaseAddress;
	u32 Col;

	/* Unused arguments */
	(void)AieDev;
	(void)Buffer;

	for (Col = StartCol; Col <= EndCol; Col++) {
		/* BaseAddress for AIE2 column */
		BaseAddress = AIE2PS_TILE_BADDR(NocAddress, Col, 0U);

		/* Ungate columns */
		AieWrite64(BaseAddress + AIE2PS_PL_MODULE_COLUMN_CLK_CTRL_OFFSET, 1U);
	}

	return XST_SUCCESS;
}

static XStatus Aie2ps_DisColClkBuff(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer)
{
	const XPm_AieDomain *AieDomain = (XPm_AieDomain*)XPmPower_GetById(PM_POWER_ME2);
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u64 BaseAddress;
	u32 Col;

	/* Unused arguments */
	(void)AieDev;
	(void)Buffer;

	for (Col = StartCol; Col <= EndCol; Col++) {
		/* BaseAddress for AIE2 column */
		BaseAddress = AIE2PS_TILE_BADDR(NocAddress, Col, 0U);

		/* Gate columns */
		AieWrite64(BaseAddress + AIE2PS_PL_MODULE_COLUMN_CLK_CTRL_OFFSET, 0U);
	}

	return XST_SUCCESS;
}

static XStatus Aie2ps_EnbAxiMmErrEvent(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer)
{
	const XPm_AieDomain *AieDomain = (XPm_AieDomain*)XPmPower_GetById(PM_POWER_ME2);
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u64 BaseAddress;
	u32 Col;

	/* Unused arguments */
	(void)AieDev;
	(void)Buffer;

	for (Col = StartCol; Col <= EndCol; Col++) {
		/* BaseAddress for AIE2 column */
		BaseAddress = AIE2PS_TILE_BADDR(NocAddress, Col, 0U);

		/* Eanble AXI-MM decode and slave error events */
		AieRMW64(BaseAddress + AIE2PS_NOC_MODULE_ME_AXIMM_CONFIG_OFFSET,
			 ME_AXIMM_CONFIG_DECERR_BLOCK_EN_MASK | ME_AXIMM_CONFIG_SLVERR_BLOCK_EN_MASK,
			 ME_AXIMM_CONFIG_DECERR_BLOCK_EN_MASK | ME_AXIMM_CONFIG_SLVERR_BLOCK_EN_MASK);
	}

	return XST_SUCCESS;
}

static XStatus Aie2ps_EnbAxiMmIsolation(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer)
{
	XStatus Status = XST_FAILURE;
	const XPm_AieDomain *AieDomain = (XPm_AieDomain*)XPmPower_GetById(PM_POWER_ME2);
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u64 BaseAddress;
	u32 Col, Mask = 0U;
	u32 Value = ((struct XPm_AieOpAximmIso *)Buffer)->Traffic;

	/* Unused arguments */
	(void)AieDev;

	/*
	 * If the value passed from the driver is not one of the following
	 * scenarios it is considered invalid:
	 * - Bit 3 is set to enable isolation from the east
	 * - Bit 1 is set to enable isolation from the west
	 * - Bits 1 and 3 are set to enable isolations from both east and west
	 * - A value of 0 indicating isolations should be removed
	 */
	if ((TILE_CTRL_AXI_MM_ISOLATE_FROM_EAST_MASK != Value) &&
	    (TILE_CTRL_AXI_MM_ISOLATE_FROM_WEST_MASK != Value) &&
	    ((TILE_CTRL_AXI_MM_ISOLATE_FROM_EAST_MASK |
	      TILE_CTRL_AXI_MM_ISOLATE_FROM_WEST_MASK) != Value) &&
	    (0U != Value)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Set EAST/WEST isolation depending on value passed from driver */
	if (TILE_CTRL_AXI_MM_ISOLATE_FROM_EAST_MASK & Value) {
		Mask |= TILE_CTRL_AXI_MM_ISOLATE_FROM_EAST_MASK;
	}
	if (TILE_CTRL_AXI_MM_ISOLATE_FROM_WEST_MASK & Value) {
		Mask |= TILE_CTRL_AXI_MM_ISOLATE_FROM_WEST_MASK;
	}

	for (Col = StartCol; Col <= EndCol; Col++) {
		/* BaseAddress for AIE2 column */
		BaseAddress = AIE2PS_TILE_BADDR(NocAddress, Col, 0U);

		/* Enable isolations for east and west partition boundaries */
		AieWrite64(BaseAddress + AIE2PS_PL_MODULE_TILE_CTRL_AXI_MM_OFFSET, Mask);
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus Aie2ps_SetL2CtrlNpiIntr(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer)
{
	const XPm_AieDomain *AieDomain = (XPm_AieDomain*)XPmPower_GetById(PM_POWER_ME2);
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u64 BaseAddress;
	u32 Col;
	u32 Value = ((struct XPm_AieOpL2CtrlIrq *)Buffer)->Irq;

	/* Unused arguments */
	(void)AieDev;

	for (Col = StartCol; Col <= EndCol; Col++) {
		/* BaseAddress for AIE2 column */
		BaseAddress = AIE2PS_TILE_BADDR(NocAddress, Col, 0U);

		/* Set L2 controller NPI INTR */
		AieWrite64(BaseAddress + AIE2PS_NOC_MODULE_INTR_CTRL_L2_INTR_OFFSET, Value);
	}

	return XST_SUCCESS;
}

static XStatus Aie2ps_NmuConfig(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer)
{
	XStatus Status = XST_FAILURE;
	const XPm_AieDomain *AieDomain = (XPm_AieDomain*)XPmPower_GetById(PM_POWER_ME2);
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u64 BaseAddress;
	u32 Col;
	u32 ColRoute0 = ((struct XPm_AieOpNmuSwitch *)Buffer)->C0Route;
	u32 ColRoute1 = ((struct XPm_AieOpNmuSwitch *)Buffer)->C1Route;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* Unused arguments */
	(void)AieDev;

	for (Col = StartCol; Col <= EndCol; Col++) {
		/* BaseAddress for AIE2 column */
		BaseAddress = AIE2PS_TILE_BADDR(NocAddress, Col, 0U);

		if (0U == Col) {
			/* ColRout0 value should be writting to column 0 shim tile only  */
			AieWrite64(BaseAddress + AIE2PS_PL_MODULE_NMU_SWITCHES_CONFIG_OFFSET, ColRoute0);
		} else if (1U == Col) {
			/* ColRout1 value should be writting to column 0 shim tile only  */
			AieWrite64(BaseAddress + AIE2PS_PL_MODULE_NMU_SWITCHES_CONFIG_OFFSET, ColRoute1);
		} else {
			/* Operation is expected to only modify values for column0 and column1 */
			DbgErr = XPM_INT_ERR_AIE_NMU_CONFIG;
			Status = XPM_ERR_AIE_OPS_NMU_CONFIG;
			goto done;
		}
	}

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus Aie2ps_HwErrInt(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer)
{
	const XPm_AieDomain *AieDomain = (XPm_AieDomain*)XPmPower_GetById(PM_POWER_ME2);
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u64 BaseAddress;
	u32 Col;
	u32 Value = ((struct XPm_AieOpHwErrSts *)Buffer)->Val;

	/* Unused arguments */
	(void)AieDev;

	for (Col = StartCol; Col <= EndCol; Col++) {
		/* BaseAddress for AIE2 column */
		BaseAddress = AIE2PS_TILE_BADDR(NocAddress, Col, 0U);

		/* Configure hardware errors interrupt out signal */
		AieWrite64(BaseAddress + AIE2PS_PL_MODULE_INT_CTRL_HW_ERR_INT_OFFSET, Value);
	}

	return XST_SUCCESS;
}

static XStatus Aie2ps_HwErrMask(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer)
{
	const XPm_AieDomain *AieDomain = (XPm_AieDomain*)XPmPower_GetById(PM_POWER_ME2);
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u64 BaseAddress;
	u32 Col;
	u32 Value = ((struct XPm_AieOpHwErrSts *)Buffer)->Val;

	/* Unused arguments */
	(void)AieDev;

	for (Col = StartCol; Col <= EndCol; Col++) {
		/* BaseAddress for AIE2 column */
		BaseAddress = AIE2PS_TILE_BADDR(NocAddress, Col, 0U);

		/* Mask error interrupts */
		AieWrite64(BaseAddress + AIE2PS_PL_MODULE_INT_CTRL_HW_ERR_MASK_OFFSET, Value);
	}

	return XST_SUCCESS;
}

static XStatus Aie2ps_SetEccScrub(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer)
{
	const XPm_AieDomain *AieDomain = (XPm_AieDomain*)XPmPower_GetById(PM_POWER_ME2);
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u64 BaseAddress;
	u32 Col;
	u32 Value = ((struct XPm_AieOpEccScrubPeriod *)Buffer)->ScrubPeriod;

	/* Unused arguments */
	(void)AieDev;

	for (Col = StartCol; Col <= EndCol; Col++) {
		/* BaseAddress for AIE2 column */
		BaseAddress = AIE2PS_TILE_BADDR(NocAddress, Col, 0U);

		/* Set ECC scrub period */
		AieRMW64(BaseAddress + AIE2PS_UC_MODULE_MEM_DM_ECC_SCRUB_PERIOD_OFFSET,
			  MEM_DM_ECC_SCRUB_PERIOD_SCRUBBING_PERIOD_MASK, Value);
	}

	return XST_SUCCESS;
}

static XStatus Aie2ps_EnbMemPriv(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer)
{
	const XPm_AieDomain *AieDomain = (XPm_AieDomain*)XPmPower_GetById(PM_POWER_ME2);
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u64 BaseAddress;
	u32 Col;

	/* Unused arguments */
	(void)AieDev;
	(void)Buffer;

	for (Col = StartCol; Col <= EndCol; Col++) {
		/* BaseAddress for AIE2 column */
		BaseAddress = AIE2PS_TILE_BADDR(NocAddress, Col, 0U);

		/* Eanble privileged memory */
		AieWrite64(BaseAddress + AIE2PS_UC_MODULE_MEMORY_PRIVILEGED_OFFSET, 1U);
	}

	return XST_SUCCESS;
}

static XStatus Aie2ps_DisMemPriv(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer)
{
	const XPm_AieDomain *AieDomain = (XPm_AieDomain*)XPmPower_GetById(PM_POWER_ME2);
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u64 BaseAddress;
	u32 Col;

	/* Unused arguments */
	(void)AieDev;
	(void)Buffer;

	for (Col = StartCol; Col <= EndCol; Col++) {
		/* BaseAddress for AIE2 column */
		BaseAddress = AIE2PS_TILE_BADDR(NocAddress, Col, 0U);

		/* Disable privileged memory */
		AieWrite64(BaseAddress + AIE2PS_UC_MODULE_MEMORY_PRIVILEGED_OFFSET, 0U);
	}

	return XST_SUCCESS;
}

static XStatus Aie2ps_EnbNocDmaPause(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer)
{
	const XPm_AieDomain *AieDomain = (XPm_AieDomain*)XPmPower_GetById(PM_POWER_ME2);
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u64 BaseAddress;
	u32 Col;

	/* Unused arguments */
	(void)AieDev;
	(void)Buffer;

	for (Col = StartCol; Col <= EndCol; Col++) {
		/* BaseAddress for AIE2 column */
		BaseAddress = AIE2PS_TILE_BADDR(NocAddress, Col, 0U);

		/* Pause shim DMA transaction */
		AieWrite64(BaseAddress + AIE2PS_NOC_MODULE_DMA_PAUSE_OFFSET,
			   AIE2PS_NOC_MODULE_DMA_PAUSE_MASK);
	}

	return XST_SUCCESS;
}

static XStatus Aie2ps_EnbUcDmaPause(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer)
{
	const XPm_AieDomain *AieDomain = (XPm_AieDomain*)XPmPower_GetById(PM_POWER_ME2);
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u64 BaseAddress;
	u32 Col;

	/* Unused arguments */
	(void)AieDev;
	(void)Buffer;

	for (Col = StartCol; Col <= EndCol; Col++) {
		/* BaseAddress for AIE2 column */
		BaseAddress = AIE2PS_TILE_BADDR(NocAddress, Col, 0U);

		/* Pause UC DMA transaction */
		AieWrite64(BaseAddress + AIE2PS_UC_MODULE_DMA_PAUSE_OFFSET, AIE2PS_UC_MODULE_DMA_PAUSE_MASK);
	}

	return XST_SUCCESS;
}

static XStatus Aie2ps_IntcHwSts(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer)
{
	const XPm_AieDomain *AieDomain = (XPm_AieDomain*)XPmPower_GetById(PM_POWER_ME2);
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u64 BaseAddress;
	u32 Col;
	u32 Value = ((struct XPm_AieOpHwErrSts *)Buffer)->Val;

	/* Unused arguments */
	(void)AieDev;

	for (Col = StartCol; Col <= EndCol; Col++) {
		/* BaseAddress for AIE2 column */
		BaseAddress = AIE2PS_TILE_BADDR(NocAddress, Col, 0U);

		/* De-assert the raised interrupt */
		AieRMW64(BaseAddress + AIE2PS_PL_MODULE_INT_CTRL_HW_ERR_STATUS_OFFSET, Value, Value);
	}

	return XST_SUCCESS;
}

static XStatus Aie2ps_DisMemInterleave(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer)
{
	const XPm_AieDomain *AieDomain = (XPm_AieDomain*)XPmPower_GetById(PM_POWER_ME2);
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u64 BaseAddress;
	u32 Col, Row;
	u32 RowStart = AieDomain->Array.StartRow;
	u32 RowEnd = RowStart + AieDomain->Array.NumMemRows;

	/* Unused arguments */
	(void)AieDev;
	(void)Buffer;

	for (Col = StartCol; Col <= EndCol; Col++) {
		for (Row = RowStart; Row < RowEnd; Row++) {
			BaseAddress = AIE2PS_TILE_BADDR(NocAddress, Col, Row);

			/* Disable memory interleaving */
			AieRMW64(BaseAddress + AIE2PS_MEM_TILE_MODULE_MEM_CTRL_OFFSET,
				  AIE2PS_MEM_TILE_MODULE_MEM_CTRL_MEM_INTERLEAVING_MASK, 0U);
		}
	}

	return XST_SUCCESS;
}

static XStatus Aie2ps_HandShake(const XPm_Device *AieDev, u32 StartCol, u32 EndCol, const void *Buffer)
{
	XStatus Status = XST_FAILURE;
	const XPm_AieDomain *AieDomain = (XPm_AieDomain*)XPmPower_GetById(PM_POWER_ME2);
	const u64 NocAddress = AieDomain->Array.NocAddress;
	u64 DestAddr, BaseAddress, DdrAddr;
	u32 Len = ((struct XPm_AieOpHandShake *)Buffer)->Len;
	u32 HighAddr = ((struct XPm_AieOpHandShake *)Buffer)->HighAddr;
	u32 LowAddr = ((struct XPm_AieOpHandShake *)Buffer)->LowAddr;
	u32 Offset = ((struct XPm_AieOpHandShake *)Buffer)->Offset;
	u32 Col;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	(void)AieDev;

	DdrAddr = LowAddr | (u64)HighAddr << 32U;
	Len = (Len - sizeof(struct XPm_AieOpHandShake));

	/* Len + Offset should not exceed 16KB */
	if (AIE2PS_UC_PRIVATE_DM_MAX_SIZE < (Len + Offset)) {
		DbgErr = XPM_INT_ERR_UC_PRIVATE_DM_MAX_SIZE;
		Status = XPM_ERR_AIE_OPS_HANDSHAKE;
		goto done;
	}

	for (Col = StartCol; Col <= EndCol; Col++) {
		BaseAddress = AIE2PS_TILE_BADDR(NocAddress, Col, 0U);
		DestAddr = BaseAddress + AIE2PS_UC_MODULE_CORE_PRIVATE_DATA_MEM + Offset;

		Status = XPlmi_DmaXfr(DdrAddr, DestAddr, Len/4U, XPLMI_PMCDMA_0);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/******************************************************/
/**
 * @brief  Perform AIE2PS operations.
 *
 * @param  Size		Buffer size
 * @param  HighAddr	Upper 32 bits of the buffer
 * @param  LowAddr	Lower 32 bits of the buffer
 *
 * @return XST_SUCCESS if successful else error code
 *
 ******************************************************/
static XStatus Aie2ps_Operation(u32 Size, u32 HighAddr, u32 LowAddr)
{
        XStatus Status = XST_FAILURE;
        u64 DdrAddr =  LowAddr | (u64)HighAddr << 32U;
	u8 Buffer[AIE_OPS_MAX_BUF_SIZE];
	XPm_Device *AieDevice;
	u32 DestAddr = (u32)&Buffer;
	u32 Index = 0U;
	u32 StartCol = 0U;
	u32 EndCol = 0U;
        char *Buf = (char *)Buffer;
        char *End = Buf + Size;
        struct XPm_AieTypeLen *OpTypeLen;
        u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* @TODO: Check if AIE domain initialized from CDO */
	AieDevice = (XPm_Device *)XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == AieDevice) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	if ((AIE_OPS_MAX_BUF_SIZE < Size) || (0U == Size)) {
		Status = XST_INVALID_PARAM;
		DbgErr = XPM_INT_ERR_INVALID_PARAM;
		goto done;
	}

	/* @TODO Validate address for DDR memory range */
	if ((0U == LowAddr) && (0U == HighAddr)) {
		Status = XST_INVALID_PARAM;
		DbgErr = XPM_INT_ERR_INVALID_PARAM;
		goto done;
	}

	/* Copy the buffer shared by Linux driver from DDR to RAM */
        Status = XPlmi_DmaXfr(DdrAddr, DestAddr, Size/4U, XPLMI_PMCDMA_0);
        if (XST_SUCCESS != Status) {
                goto done;
        }

	OpTypeLen = (struct XPm_AieTypeLen *)Buf;
	/* Buffer must start with AIE_OPS_START_NUM_COL operation */
	if (AIE_OPS_START_NUM_COL != OpTypeLen->Type) {
		DbgErr = XPM_INT_ERR_INVALID_PARAM;
		goto done;
	}

	while (Buf < End) {
		/* Initialize for each Aie operation */
		Status = XST_FAILURE;

		if (AIE_OPS_START_NUM_COL == OpTypeLen->Type) {
			Status = Aie2ps_StartNumCol(&StartCol, &EndCol, Buf);
			if (XST_SUCCESS != Status) {
				goto done;
			}

			Buf += OpTypeLen->Len;
		} else {
			for (Index = 0U; Index < sizeof(AieOpsHandlers)/sizeof(struct XPmAieOpsHandlers); Index++) {
				if (AieOpsHandlers[Index].Ops == OpTypeLen->Type) {
					if (NULL != AieOpsHandlers[Index].Handler) {
						/* Perform AIE operation */
						Status = AieOpsHandlers[Index].Handler(AieDevice, StartCol,
										       EndCol, Buf);
					}
				}
			}

			if (XST_SUCCESS != Status) {
				goto done;
			} else if (AIE_OPS_HANDSHAKE == OpTypeLen->Type) {
				/* Len from the XPm_AieOpHandShake structure is total
				 * length of structure + handshake operation
				 * so only consider structure length for traversing buffer */
				Buf += sizeof(struct XPm_AieOpHandShake);
			} else {
				/* Move to next operation in buffer */
				Buf += OpTypeLen->Len;
			}
		}

		OpTypeLen = (struct XPm_AieTypeLen *)Buf;
	}

        Status = XST_SUCCESS;

done:
        XPm_PrintDbgErr(Status, DbgErr);
        return Status;
}

XStatus XPmAie_Operations(u32 Size, u32 HighAddr, u32 LowAddr)
{
	XStatus Status = XST_FAILURE;

	/* @TODO: Check if AIE device is added and initialized */
	Status = Aie2ps_Operation(Size, HighAddr, LowAddr);

	return Status;
}
