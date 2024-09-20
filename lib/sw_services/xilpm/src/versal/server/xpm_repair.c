/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
*****************************************************************************/
#include "xpm_repair.h"
#include "xpm_pldomain.h"
#include "xpm_device.h"
#include "xplmi_ssit.h"

#define VDU_NPI_CACHE_DATA_REGISTER_OFFSET		0x104

/* VDU Repair */
#define VDU_PCSR_BISR_TRIGGER_MASK			0x02000000
#define VDU_PCSR_STATUS_BISR_DONE_MASK			0x00010000
#define VDU_PCSR_STATUS_BISR_PASS_MASK			0x00020000

/* BFR Repair */
#define BFR_NPI_CACHE_DATA_REGISTER_OFFSET		0x00000010U
#define BFR_PCSR_MASK_REGISTER_OFFSET			0x00000000U
#define BFR_PCSR_CONTROL_REGISTER_OFFSET		0x00000004U
#define BFR_PCSR_STATUS_REGISTER_OFFSET			0x00000008U
#define BFR_PCSR_STATUS_BISR_DONE_MASK			0x00800000U
#define BFR_PCSR_STATUS_BISR_PASS_MASK			0x01000000U
#define BFR_PCSR_STATUS_POWER_STATE_BIT_ZERO_MASK	0x00080000U
#define BFR_PCSR_CONTROL_BISR_TRIGGER_MASK		0x01000000U
#define BFR_PCSR_CONTROL_INITSTATE_MASK			0x00000040U
#define BFR_PCSR_MASK_BISR_TRIGGER_MASK			0x01000000U
#define BFR_PCSR_MASK_PWRDN_MASK			0x00004000U
#define BFR_PCSR_MASK_INITSTATE_MASK			0x00000040U

/* DDRMC5 repair */
#define DDRMC5_NPI_CACHE_STATUS_MAIN_REGISTER_OFFSET    (0x00000254U)
#define DDRMC5_NPI_CACHE_DATA_MAIN_REGISTER_OFFSET      (0x00000244U)
#define DDRMC5_NPI_PCSR_CONTROL_REGISTER_OFFSET         (0x00000004U)
#define DDRMC5_NPI_PCSR_BISR_MAIN_TRIGGER_MASK          (0x02000000U)
#define DDRMC5_NPI_CACHE_STATUS_BISR_DONE_MASK          (0x00000001U)
#define DDRMC5_NPI_CACHE_STATUS_BISR_PASS_MASK          (0x00000002U)
#define DDRMC5_NPI_CLK_GATE_REGISTER_OFFSET             (0x00000238U)
#define DDRMC5_NPI_CLK_GATE_MAIN_BISREN_MASK            (0x00000040U)
#define DDRMC5_NPI_CLK_GATE_CRYPTO_BISREN_MASK          (0x00000080U)
#define DDRMC5_NPI_CACHE_STATUS_CRYPTO_REGISTER_OFFSET  (0x00000414U)
#define DDRMC5_NPI_CACHE_DATA_CRYPTO_REGISTER_OFFSET    (0x00000404U)
#define DDRMC5_NPI_PCSR_BISR_CRYPTO_TRIGGER_MASK        (0x04000000U)
#define DDRMC_NPI_PCSR_MASK_REGISTER_OFFSET             (0x00000000U)

#ifdef XCVP1902
/* Laguna Repair */
#define VP1902_LAGUNA_FUSE_REDUNDANT_Y_LSB	   (0U)
#define VP1902_LAGUNA_FUSE_REDUNDANT_Y_BITS	   (10U)
#define VP1902_LAGUNA_FUSE_DEFECT_Y_LSB		   (VP1902_LAGUNA_FUSE_REDUNDANT_Y_LSB + VP1902_LAGUNA_FUSE_REDUNDANT_Y_BITS)
#define VP1902_LAGUNA_FUSE_DEFECT_Y_BITS	   (10U)
#define VP1902_LAGUNA_FUSE_DEFECT_X_LSB		   (VP1902_LAGUNA_FUSE_DEFECT_Y_LSB + VP1902_LAGUNA_FUSE_DEFECT_Y_BITS)
#define VP1902_LAGUNA_FUSE_DEFECT_X_BITS	   (9U)
#define VP1902_LAGUNA_FUSE_DRIVER_LSB		   (VP1902_LAGUNA_FUSE_DEFECT_X_LSB + VP1902_LAGUNA_FUSE_DEFECT_X_BITS)
#define VP1902_LAGUNA_FUSE_DRIVER_BITS		   (3U)
#define HALF_TILE_NUM_PER_FRAME			(48U)
#define FDRO_BASEADDR				(0xF12C2000U)
#define FSR_TILE_END				(95U)
#define HALF_FSR_START				(48U)
#define FRAME_BLOCK_TYPE_6			(6U)
#define CFRAME0_REG_FAR_BLOCKTYPE_SHIFT		(20U)
#define EFUSE_CACHE_TBITS1_BISR_RSVD_OFFSET	(0x00000400U)
#define EFUSE_CACHE_TBITS2_BISR_RSVD_OFFSET	(0x00000800U)
#define CFRAME_REG_CMD_OFFSET			(0x00000060U)
#define CFRAME_REG_CMD_DLPARK			(0x00000005U)
#define CFRAME_BCAST_REG_BASEADDR		(0xF12EE000U)
#define CFU_APB_CFU_ROW_RANGE_OFFSET		(0x0000006CU)
#define CFU_APB_CFU_ROW_RANGE_HALF_FSR_MASK	(0x00000020U)
#define CFRAME1_REG_BASEADDR			(0xF12D2000U)
#define CFRAME1_REG_CFRAME_FAR_TOP_OFFSET	(0x00000210U)
#define CFRAME_REG_FAR_OFFSET			(0x00000010U)
#define CFRAME_REG_FRCNT_OFFSET			(0x00000050U)
#define VP1902_H_INTER_SLR_WIDTH		   (20U)
#define VP1902_LAGUNA_ROWS			   (576U)
#define VP1902_LAGUNA_WIDTH			   (117U)
#define VP1902_BLUE_REGION_X_EW			   (VP1902_LAGUNA_WIDTH - VP1902_H_INTER_SLR_WIDTH)
#define VP1902_BLUE_REGION_Y_NS			   (VP1902_LAGUNA_ROWS - 63U)
#define VP1902_BLUE_REGION_Y_NS_IN_ROW		   (VP1902_BLUE_REGION_Y_NS % 96U)
#define FRAME_BUFFER_SIZE			(100U)

static void LagunaRmwOneFrame_vp1902(u32 RowIndex,
	u32 FrameAddr, u32 LowerTile, u32 UpperTile,
	u32 LagunaDriverIndex, u32 LagunaXAdj, u32 CfuApbBaseAddr)
{
	u32 CFrameAddr;
	u32 FrameData[FRAME_BUFFER_SIZE];
	u32 FdriAddr;
	u32 CurrentFdro;
	u32 i;
	u32 Idx = 0U;
	u32 XlrBitPosistion;
	u32 CFrameTopRow;
	const XPm_PlDomain *Pld;

	Pld = (XPm_PlDomain *)XPmPower_GetById(PM_POWER_PLD);
	if (NULL == Pld) {
		goto done;
	}
	CFrameTopRow = (XPm_In32(CfuApbBaseAddr + CFU_APB_CFU_ROW_RANGE_OFFSET)
		& (u32)CFU_APB_CFU_ROW_RANGE_NUM_MASK) - 1U;
	/* Get CFRAME Address */
	CFrameAddr = Pld->Cframe0RegBaseAddr + (XCFRAME_FRAME_OFFSET * (u32)RowIndex);

	/* Enable CFRAME Row */
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0x0U, CFRAME_REG_CMD_ROWON);
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0x4U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0x8U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0xCU, 0x0U);

	/* nop delay */
	XPm_Wait(300U);

	/* Enable read configuration data */
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0x0U, CFRAME_REG_CMD_RCFG);
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0x4U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0x8U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0xCU, 0x0U);

	/* nop delay */
	XPm_Wait(200U);

	/* Set Frame address register */
	XPm_Out32((CFrameAddr + CFRAME_REG_FAR_OFFSET) + 0x0U, FrameAddr);
	XPm_Out32((CFrameAddr + CFRAME_REG_FAR_OFFSET) + 0x4U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_FAR_OFFSET) + 0x8U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_FAR_OFFSET) + 0xCU, 0x0U);

	/* Set read count to one frame. One frame is 25 quadwords */
	XPm_Out32((CFrameAddr + CFRAME_REG_FRCNT_OFFSET) + 0x0U, 0x19U);
	XPm_Out32((CFrameAddr + CFRAME_REG_FRCNT_OFFSET) + 0x4U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_FRCNT_OFFSET) + 0x8U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_FRCNT_OFFSET) + 0xCU, 0x0U);

	/* Read 25 quadwords of Frame data. FDRO address is 128 bits wide so read
	 * 4 words of data per iteration.
	 */

	for (i = 0U; i < 25U; i++) {
		CurrentFdro = FDRO_BASEADDR;

		FrameData[Idx] = XPm_In32(CurrentFdro);
		CurrentFdro += 0x4U;
		++Idx;
		FrameData[Idx] = XPm_In32(CurrentFdro);
		CurrentFdro += 0x4U;
		++Idx;
		FrameData[Idx] = XPm_In32(CurrentFdro);
		CurrentFdro += 0x4U;
		++Idx;
		FrameData[Idx] = XPm_In32(CurrentFdro);
		++Idx;
	}

	/* Bit to be modified*/
	XlrBitPosistion = (LagunaDriverIndex << 2U) + 6U;
	if (((XPlmi_GetSlrIndex() == 1U) || (XPlmi_GetSlrIndex() == 2U)) &&
		(RowIndex == CFrameTopRow) &&
		((LagunaXAdj < VP1902_BLUE_REGION_X_EW) && (UpperTile > VP1902_BLUE_REGION_Y_NS_IN_ROW))){
		for( Idx = UpperTile; Idx <= FSR_TILE_END; Idx++) {
			if(Idx < HALF_TILE_NUM_PER_FRAME) {
				FrameData[Idx] &= (0xFFFFFFFFU ^ (1U << XlrBitPosistion));
			}
			else{
				FrameData[Idx + 4U] &= (0xFFFFFFFFU ^ (1U << XlrBitPosistion));
			}
		}
	}
	else{
		for( Idx = LowerTile; Idx <= UpperTile; Idx++){
			if(Idx < HALF_TILE_NUM_PER_FRAME){
				FrameData[Idx] |= ((1U << XlrBitPosistion));
			}
			else{
				FrameData[Idx + 4U] |= ((1U << XlrBitPosistion));
			}
		}
	}

	/* Enable write configuration data */
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0x0U, CFRAME_REG_CMD_WCFG);
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0x4U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0x8U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0xCU, 0x0U);

	/* nop delay */
	XPm_Wait(200U);

	/* Set Frame address register */
	XPm_Out32((CFrameAddr + CFRAME_REG_FAR_OFFSET) + 0x0U, FrameAddr);
	XPm_Out32((CFrameAddr + CFRAME_REG_FAR_OFFSET) + 0x4U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_FAR_OFFSET) + 0x8U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_FAR_OFFSET) + 0xCU, 0x0U);

	FdriAddr = CFRAME0_FDRI_BASEADDR + (XCFRAME_FRAME_OFFSET * (u32)RowIndex);

	/* Write 100 words of Frame data */
	for (i = 0U; i < 100U; i++) {
		XPm_Out32(FdriAddr, FrameData[i]);
		FdriAddr += 0x4U;
	}

	/* Enable write configuration data */
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0x0U, CFRAME_REG_CMD_DLPARK);
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0x4U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0x8U, 0x0U);
	XPm_Out32((CFrameAddr + CFRAME_REG_CMD_OFFSET) + 0xCU, 0x0U);
done:
	return;
}
#endif

XStatus XPmRepair_Vdu(u32 EfuseTagAddr, u32 TagSize,
			u32 TagOptional, u32 *TagDataAddr)
{
	XStatus Status = XST_FAILURE;
	u32 RegValue;
	u64 BisrDataDestAddr;
	u32 BaseAddr;

	BaseAddr = NPI_FIXED_BASEADDR + (TagOptional << NPI_EFUSE_ENDPOINT_SHIFT);
	BisrDataDestAddr = BaseAddr + (u64)VDU_NPI_CACHE_DATA_REGISTER_OFFSET;

	/* Unlock PCSR */
	XPm_UnlockPcsr(BaseAddr);

	/* Copy repair data */
	*TagDataAddr = XPmBisr_CopyStandard(EfuseTagAddr, TagSize, BisrDataDestAddr);

	/* Trigger BISR */
	XPm_Out32(BaseAddr + NPI_PCSR_MASK_OFFSET, VDU_PCSR_BISR_TRIGGER_MASK);
	XPm_Out32(BaseAddr + NPI_PCSR_CONTROL_OFFSET, VDU_PCSR_BISR_TRIGGER_MASK);

	/* Wait for BISR to finish */
	Status = XPm_PollForMask(BaseAddr + NPI_PCSR_STATUS_OFFSET, VDU_PCSR_STATUS_BISR_DONE_MASK, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Check for BISR PASS */
	RegValue = XPm_In32(BaseAddr + NPI_PCSR_STATUS_OFFSET);
	if ((RegValue & (u32)VDU_PCSR_STATUS_BISR_PASS_MASK) != (u32)VDU_PCSR_STATUS_BISR_PASS_MASK) {
		Status = XST_FAILURE;
		goto done;
	}

done:
	/* Lock PCSR */
	XPm_LockPcsr(BaseAddr);
	return Status;
}

XStatus XPmRepair_Bfrb(u32 EfuseTagAddr, u32 TagSize,
			u32 TagOptional, u32 *TagDataAddr)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddr = NPI_FIXED_BASEADDR + (TagOptional << NPI_EFUSE_ENDPOINT_SHIFT);
	u64 BisrDataDestAddr = BaseAddr + (u64)BFR_NPI_CACHE_DATA_REGISTER_OFFSET;

	/* Unlock PCSR */
	XPm_UnlockPcsr(BaseAddr);

	/*Deassert PWRDN*/
	XPm_Out32(BaseAddr + BFR_PCSR_MASK_REGISTER_OFFSET, BFR_PCSR_MASK_PWRDN_MASK);
	XPm_Out32(BaseAddr + BFR_PCSR_CONTROL_REGISTER_OFFSET, 0U);

	Status = XPm_PollForZero(BaseAddr + BFR_PCSR_STATUS_REGISTER_OFFSET,
			BFR_PCSR_STATUS_POWER_STATE_BIT_ZERO_MASK, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	XPm_Out32(BaseAddr + BFR_PCSR_MASK_REGISTER_OFFSET, BFR_PCSR_MASK_INITSTATE_MASK);
	XPm_Out32(BaseAddr + BFR_PCSR_CONTROL_REGISTER_OFFSET, BFR_PCSR_CONTROL_INITSTATE_MASK);

	XPm_Out32(BaseAddr + BFR_PCSR_MASK_REGISTER_OFFSET, BFR_PCSR_MASK_INITSTATE_MASK);
	XPm_Out32(BaseAddr + BFR_PCSR_CONTROL_REGISTER_OFFSET, 0U);

	/* Copy repair data */
	*TagDataAddr = XPmBisr_CopyStandard(EfuseTagAddr, TagSize, BisrDataDestAddr);

	/* Trigger BISR */
	XPm_Out32(BaseAddr + BFR_PCSR_MASK_REGISTER_OFFSET, BFR_PCSR_MASK_BISR_TRIGGER_MASK);
	XPm_Out32(BaseAddr + BFR_PCSR_CONTROL_REGISTER_OFFSET, BFR_PCSR_CONTROL_BISR_TRIGGER_MASK);

	/* Wait for BISR to finish */
	Status = XPm_PollForMask(BaseAddr + BFR_PCSR_STATUS_REGISTER_OFFSET,
			BFR_PCSR_STATUS_BISR_DONE_MASK, BFR_PCSR_STATUS_BISR_DONE_MASK);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Wait for BISR to finish */
	Status = XPm_PollForMask(BaseAddr + BFR_PCSR_STATUS_REGISTER_OFFSET,
			BFR_PCSR_STATUS_BISR_PASS_MASK, BFR_PCSR_STATUS_BISR_PASS_MASK);

done:
	/* Lock PCSR */
	XPm_LockPcsr(BaseAddr);
	return Status;
}

#ifdef XCVP1902
u32 XPmRepair_Laguna_vp1902(u32 EfuseTagAddr, u32 TagSize)
{
	u32 TagRow = 0U;
	u32 TagData;
	u32 TagDataAddr;
	u32 LagunaX;
	u32 LagunaXAdj;
	u32 Temp;
	u32 HalfFsr;
	u32 NumberOfRows;
	u32 Row0;
	u32 Row1;
	u32 Tile0;
	u32 Tile1;
	u8 LowerTile;
	u8 UpperTile;
	u32 RowIndex;
	u32 FrameAddr;
	u32 EfuseCacheBaseAddr;
	u32 EfuseTagBitS1Addr;
	u32 EfuseTagBitS2Addr;
	u32 Row0XOffset;
	u32 LagunaDriverIndex;
	u32 CfuApbBaseAddr;
	const XPm_PlDomain *Pld;
	const XPm_Device *EfuseCache = XPmDevice_GetById(PM_DEV_EFUSE_CACHE);
	if (NULL == EfuseCache) {
		TagDataAddr = ~0U;
		/* Return negative address so error can be identified by caller */
		goto done;
	}

	Pld = (XPm_PlDomain *)XPmPower_GetById(PM_POWER_PLD);
	if (NULL == Pld) {
		TagDataAddr = ~0U;
		/* Return negative address so error can be identified by caller*/
		goto done;
	}

	EfuseCacheBaseAddr = EfuseCache->Node.BaseAddress;
	EfuseTagBitS1Addr = EfuseCacheBaseAddr + EFUSE_CACHE_TBITS1_BISR_RSVD_OFFSET;
	EfuseTagBitS2Addr = EfuseCacheBaseAddr + EFUSE_CACHE_TBITS2_BISR_RSVD_OFFSET;
	TagDataAddr = EfuseTagAddr + 4U;

	/* Broadcast row on command */
	XPm_Out32((CFRAME_BCAST_REG_BASEADDR + CFRAME_REG_CMD_OFFSET) + 0x0U, CFRAME_REG_CMD_ROWON);
	XPm_Out32((CFRAME_BCAST_REG_BASEADDR + CFRAME_REG_CMD_OFFSET) + 0x4U, 0x0U);
	XPm_Out32((CFRAME_BCAST_REG_BASEADDR + CFRAME_REG_CMD_OFFSET) + 0x8U, 0x0U);
	XPm_Out32((CFRAME_BCAST_REG_BASEADDR + CFRAME_REG_CMD_OFFSET) + 0xCU, 0x0U);

	/* Get device row info */
	CfuApbBaseAddr = Pld->CfuApbBaseAddr;
	HalfFsr = (XPm_In32(CfuApbBaseAddr + CFU_APB_CFU_ROW_RANGE_OFFSET) &
	    (u32)CFU_APB_CFU_ROW_RANGE_HALF_FSR_MASK) >> CFU_APB_CFU_ROW_RANGE_HALF_FSR_SHIFT;
	NumberOfRows = XPm_In32(CfuApbBaseAddr + CFU_APB_CFU_ROW_RANGE_OFFSET)
		& (u32)CFU_APB_CFU_ROW_RANGE_NUM_MASK;

	/* read 128bits CFRAME1_REG_CFRAME_FAR_TOP. type6 FAR is at bit 59:40*/
	Temp = XPm_In32(CFRAME1_REG_BASEADDR + CFRAME1_REG_CFRAME_FAR_TOP_OFFSET + 0x0U);
	Row0XOffset = (XPm_In32(CFRAME1_REG_BASEADDR + CFRAME1_REG_CFRAME_FAR_TOP_OFFSET + 0x4U) & 0x0000ff00U) >> 8U;
	Temp = XPm_In32(CFRAME1_REG_BASEADDR + CFRAME1_REG_CFRAME_FAR_TOP_OFFSET + 0x8U);
	Temp = XPm_In32(CFRAME1_REG_BASEADDR + CFRAME1_REG_CFRAME_FAR_TOP_OFFSET + 0xCU);

	while (TagRow < TagSize) {
		if ((TagDataAddr == EfuseTagBitS1Addr) || (TagDataAddr == EfuseTagBitS2Addr)) {
			TagDataAddr += 4U;
		}

		TagData = XPm_In32(TagDataAddr);
		TagRow++;
		TagDataAddr += 4U;

		LagunaDriverIndex = (TagData >> VP1902_LAGUNA_FUSE_DRIVER_LSB) & ((1U << VP1902_LAGUNA_FUSE_DRIVER_BITS) - 1U);
		if (LagunaDriverIndex > 5U){
			TagDataAddr = ~0U;
			goto done;
		}
		/* Laguna pointer coordinate for Y0 */
		Temp = (TagData >> VP1902_LAGUNA_FUSE_REDUNDANT_Y_LSB) & (((u32)1U << VP1902_LAGUNA_FUSE_REDUNDANT_Y_BITS) - 1U);
		/* Row index of Y0 and Tile index within Y0. 96 tiles per FSR row */
		Row0 = Temp / 96U;
		Tile0 = Temp % 96U;

		/* Laguna pointer coordinate for Y1 */
		Temp = (TagData >> VP1902_LAGUNA_FUSE_DEFECT_Y_LSB) & (((u32)1U << VP1902_LAGUNA_FUSE_DEFECT_Y_BITS) - 1U);
		/* Row index of Y0 and Tile index within Y1. 96 tiles per FSR row */
		Row1 = Temp / 96U;
		Tile1 = Temp % 96U;

		/* Laguna pointer coordinate for X1 */
		LagunaX = (TagData >> VP1902_LAGUNA_FUSE_DEFECT_X_LSB) & (((u32)1U << VP1902_LAGUNA_FUSE_DEFECT_X_BITS) - 1U);

		/* Walk through each FSR row that requires repair */
		for (RowIndex = Row0; RowIndex <= Row1; RowIndex++) {
			LowerTile = 0U;
			UpperTile = FSR_TILE_END;

			/* Bottom row requires repair */
			if (RowIndex == Row0) {
				LowerTile = (u8)Tile0;
			}

			/* Top row requires repair */
			if (RowIndex == Row1) {
				UpperTile = (u8)Tile1;
			}

			/* Top row is Half FSR */
			if ((RowIndex == (NumberOfRows - 1U)) && (HalfFsr == 1U)) {
				LowerTile += HALF_FSR_START;
				UpperTile += HALF_FSR_START;
			}

			/* Construct block type-6 frame address */
			LagunaXAdj = LagunaX;
			if (RowIndex == 0U){
				LagunaXAdj -=Row0XOffset;
			}
			FrameAddr = ((u32)FRAME_BLOCK_TYPE_6 << CFRAME0_REG_FAR_BLOCKTYPE_SHIFT) | LagunaXAdj;

			LagunaRmwOneFrame_vp1902(RowIndex, FrameAddr, LowerTile,
				UpperTile, LagunaDriverIndex, LagunaXAdj, CfuApbBaseAddr);
		}
	}

done:
    return TagDataAddr;
}
#endif

XStatus XPmRepair_Ddrmc5_Main(u32 EfuseTagAddr, u32 TagSize,
			      u32 TagOptional, u32 *TagDataAddr)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddr = NPI_FIXED_BASEADDR | (TagOptional << NPI_EFUSE_ENDPOINT_SHIFT);
	u64 BisrDataDestAddr = BaseAddr | DDRMC5_NPI_CACHE_DATA_MAIN_REGISTER_OFFSET;

	/* Copy repair data */
	*TagDataAddr = XPmBisr_CopyStandard((u32)EfuseTagAddr, TagSize, BisrDataDestAddr);

	/* Enable BISR clock */
	XPm_RMW32(BaseAddr | DDRMC5_NPI_CLK_GATE_REGISTER_OFFSET, DDRMC5_NPI_CLK_GATE_MAIN_BISREN_MASK, DDRMC5_NPI_CLK_GATE_MAIN_BISREN_MASK);

	/* Trigger BISR */
	XPm_RMW32(BaseAddr | DDRMC_NPI_PCSR_MASK_REGISTER_OFFSET, DDRMC5_NPI_PCSR_BISR_MAIN_TRIGGER_MASK, DDRMC5_NPI_PCSR_BISR_MAIN_TRIGGER_MASK);
	XPm_RMW32(BaseAddr | DDRMC5_NPI_PCSR_CONTROL_REGISTER_OFFSET, DDRMC5_NPI_PCSR_BISR_MAIN_TRIGGER_MASK, DDRMC5_NPI_PCSR_BISR_MAIN_TRIGGER_MASK);

	/* Wait for BISR to be done and check status */
	Status = XPm_PollForMask(BaseAddr | DDRMC5_NPI_CACHE_STATUS_MAIN_REGISTER_OFFSET, DDRMC5_NPI_CACHE_STATUS_BISR_DONE_MASK, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status){
		goto done;
	}

	Status = XPm_PollForMask(BaseAddr | DDRMC5_NPI_CACHE_STATUS_MAIN_REGISTER_OFFSET, DDRMC5_NPI_CACHE_STATUS_BISR_PASS_MASK, XPM_POLL_TIMEOUT);

done:
	/* Disable BISR clock */
	XPm_RMW32(BaseAddr | DDRMC5_NPI_CLK_GATE_REGISTER_OFFSET, DDRMC5_NPI_CLK_GATE_MAIN_BISREN_MASK, ~DDRMC5_NPI_CLK_GATE_MAIN_BISREN_MASK);

	return Status;
}

XStatus XPmRepair_Ddrmc5_Crypto(u32 EfuseTagAddr, u32 TagSize,
				u32 TagOptional, u32 *TagDataAddr)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddr = NPI_FIXED_BASEADDR | (TagOptional << NPI_EFUSE_ENDPOINT_SHIFT);
	u64 BisrDataDestAddr = BaseAddr | DDRMC5_NPI_CACHE_DATA_CRYPTO_REGISTER_OFFSET;

	/* Copy repair data */
	*TagDataAddr = XPmBisr_CopyStandard((u32)EfuseTagAddr, TagSize, BisrDataDestAddr);

	/* Enable BISR clock */
	XPm_RMW32(BaseAddr | DDRMC5_NPI_CLK_GATE_REGISTER_OFFSET, DDRMC5_NPI_CLK_GATE_CRYPTO_BISREN_MASK, DDRMC5_NPI_CLK_GATE_CRYPTO_BISREN_MASK);

	/* Trigger BISR */
	XPm_RMW32(BaseAddr | DDRMC_NPI_PCSR_MASK_REGISTER_OFFSET,DDRMC5_NPI_PCSR_BISR_CRYPTO_TRIGGER_MASK, DDRMC5_NPI_PCSR_BISR_CRYPTO_TRIGGER_MASK);
	XPm_RMW32(BaseAddr | DDRMC5_NPI_PCSR_CONTROL_REGISTER_OFFSET, DDRMC5_NPI_PCSR_BISR_CRYPTO_TRIGGER_MASK, DDRMC5_NPI_PCSR_BISR_CRYPTO_TRIGGER_MASK);

	/* Wait for BISR to be done and check status */
	Status = XPm_PollForMask(BaseAddr | DDRMC5_NPI_CACHE_STATUS_CRYPTO_REGISTER_OFFSET, DDRMC5_NPI_CACHE_STATUS_BISR_DONE_MASK, XPM_POLL_TIMEOUT);
	if (XST_SUCCESS != Status){
		goto done;
	}

	Status = XPm_PollForMask(BaseAddr | DDRMC5_NPI_CACHE_STATUS_CRYPTO_REGISTER_OFFSET, DDRMC5_NPI_CACHE_STATUS_BISR_PASS_MASK, XPM_POLL_TIMEOUT);

done:
	/* Disable BISR clock */
	XPm_RMW32(BaseAddr | DDRMC5_NPI_CLK_GATE_REGISTER_OFFSET, DDRMC5_NPI_CLK_GATE_CRYPTO_BISREN_MASK, ~DDRMC5_NPI_CLK_GATE_CRYPTO_BISREN_MASK);

	return Status;
}
