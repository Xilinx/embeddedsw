/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
* @file xplat_init.c
*
* Programs the Versal Gen2 (Versal_2VE_2VM) EL3 secure registers before main():
* LPD secure SLCR, peripheral AxProt/TrustZone, SMMU global-bypass shareability
* and MMI AxProt.
*
* These registers are XPPU-protected (PMC_XPPU / LPD_XPPU). With XPPU enabled the
* APU is not a permitted master, so a direct write faults. When XPM_SUPPORT is
* defined each write is routed through the PLM via XilPM IOCTL_MASK_WRITE_REG
* (XPm_DevIoctl2); the PLM is a permitted master, so the writes are XPPU-safe.
* Without XPM_SUPPORT the same registers are written directly - valid only when
* XPPU is disabled.
*
* The PLM resolves each target as BaseAddress(RegNodeId) + Offset and permits it
* only if the topology (CDO) created the RegNode (PM_ADD_NODE) and granted a
* write aperture (PM_SET_NODE_ACCESS); the RegNode IDs below must match the CDO.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.0   tni  10/05/26 Initial version. Moves the Versal Gen2 EL3 secure-register
*                     programming out of xil-crt0.S into a pre-main constructor,
*                     routed through the PLM when XPM_SUPPORT is defined so it is
*                     XPPU-safe, with a direct-write fallback otherwise.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "bspconfig.h"
#include "xil_io.h"

#if (EL3 == 1)

/* Register addresses/bits, shared by the PLM and direct-write paths below. */
/* LPD secure SLCR */
#define MMUSECSID_CTRL_TBU		0xEB510B28U
#define LPD_SECURE_SLCR_WPROT0		0xEB510000U
#define IOU_AXIPROT_ADMA0		0xEB510080U
#define IOU_AXIPROT_ADMA1		0xEB510084U
#define IOU_AXIPROT_ADMA2		0xEB510088U
#define IOU_AXIPROT_ADMA3		0xEB51008CU
#define IOU_AXIPROT_ADMA4		0xEB510090U
#define IOU_AXIPROT_ADMA5		0xEB510094U
#define IOU_AXIPROT_ADMA6		0xEB510098U
#define IOU_AXIPROT_ADMA7		0xEB51009CU

/* PMC IOU secure AxProt */
#define IOU_AXIWPROT_SD0		0xF1070000U
#define IOU_AXIRPROT_SD0		0xF1070004U
#define IOU_AXIWPROT_EMMC0		0xF1070010U
#define IOU_AXIRPROT_EMMC0		0xF1070014U
#define IOU_AXIWPROT_QSPI		0xF1070020U
#define IOU_AXIWPROT_OSPI		0xF1070030U
#define IOU_AXIWPROT_UFS		0xF10700D0U
#define IOU_AXIRPROT_UFS		0xF10700D4U

/* GEM / USB AxProt */
#define IOU_AXIWPROT_GEM0		0xF1A40000U
#define IOU_AXIRPROT_GEM0		0xF1A40004U
#define IOU_AXIWPROT_GEM1		0xF1A40010U
#define IOU_AXIRPROT_GEM1		0xF1A40014U
#define IOU_AXIPROT_USB0		0xF1A40020U
#define IOU_AXIPROT_USB1		0xF1A40030U

/* MMI 10G AxProt */
#define MMI_AXIPROT_10G			0xEDC3019CU

/* SMMU global bypass */
#define SMMU_GBPA			0xEC000044U
#define SMMU_GBPA_SHCFG_SHIFT		12U
#define SMMU_GBPA_SHCFG_MASK		(0x3U << SMMU_GBPA_SHCFG_SHIFT)	/* bits 13:12 */
#define SMMU_GBPA_SHCFG_OSH		(0x2U << SMMU_GBPA_SHCFG_SHIFT)	/* 0b10 << 12 */
#define SMMU_GBPA_UPDATE		(0x1U << 31)			/* bit 31: Update (WO) */

#if defined(XPM_SUPPORT)
#include "xparameters.h"
#include "xipipsu.h"
#include "pm_api_sys.h"
#include "xpm_defs.h"

/************************** Constant Definitions *****************************/
/* IPI channel used to reach the PLM (same selection scheme as xpm_init.c). */
#ifndef SDT
#define XPLAT_IPI_CHANNEL_ID		XPAR_XIPIPSU_0_DEVICE_ID
#else
#define XPLAT_IPI_CHANNEL_ID		XPAR_XIPIPSU_0_BASEADDR
#endif

/* Full 32-bit mask: makes the PLM do a plain write instead of an RMW. */
#define XPLAT_FULL_MASK			0xFFFFFFFFU

/*
 * RegNode IDs - must match the design topology (CDO).
 *
 *   Macro                         Node-id      Base         Registers
 *   ---------------------------   ----------   ----------   -------------------------
 *   XPLAT_REGNODE_PMC_IOU_SEC     0x30000009   0xF1070000   SD0/EMMC/QSPI/OSPI/UFS AxProt
 *   XPLAT_REGNODE_LPD_IOU_SEC     0x3000000A   0xEB510000   WPROT0, ADMA0..7, TBU
 *   XPLAT_REGNODE_PMC_IOU_GEMUSB  0x3000000B   0xF1A40000   GEM0/1 + USB0/1 AxProt
 *   XPLAT_REGNODE_SMMU            0x3000001A   0xEC000000   SMMU_GBPA shareability
 *   XPLAT_REGNODE_MMI             0x3000001B   0xEDC30000   MMI 10G AxProt
 */
#ifndef XPLAT_REGNODE_PMC_IOU_SEC
#define XPLAT_REGNODE_PMC_IOU_SEC	0x30000009U	/* PM_REG_PMC_IOU_SECURE_SLCR */
#endif
#ifndef XPLAT_REGNODE_LPD_IOU_SEC
#define XPLAT_REGNODE_LPD_IOU_SEC	0x3000000AU	/* PM_REG_LPD_SLCR_SECURE */
#endif
#ifndef XPLAT_REGNODE_PMC_IOU_GEMUSB
#define XPLAT_REGNODE_PMC_IOU_GEMUSB	0x3000000BU	/* PM_REG_LPD_IOU_SECURE_SLCR (GEM0/1 + USB0/1) */
#endif
#ifndef XPLAT_REGNODE_SMMU
#define XPLAT_REGNODE_SMMU		0x3000001AU	/* SMMU_GBPA (base 0xEC000000) */
#endif
#ifndef XPLAT_REGNODE_MMI
#define XPLAT_REGNODE_MMI		0x3000001BU	/* MMI 10G AxProt (base 0xEDC30000) */
#endif

/* RegNode base addresses (must equal the BaseAddress used in PM_ADD_NODE). */
#define XPLAT_PMC_IOU_SEC_BASE		0xF1070000U
#define XPLAT_PMC_IOU_GEMUSB_BASE	0xF1A40000U
#define XPLAT_LPD_IOU_SEC_BASE		0xEB510000U
#define XPLAT_SMMU_BASE			0xEC000000U
#define XPLAT_MMI_BASE			0xEDC30000U

/* Offset of a register within its owning RegNode base. */
#define XPLAT_OFF(addr, base)		((u32)((addr) - (base)))

/**************************** Variable Definitions ***************************/
static XIpiPsu IpiInst;

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
* @brief	Bring up the IPI instance and XilPM so XPm_DevIoctl* can reach
*		the PLM.
*
* @return	XST_SUCCESS on success, error code otherwise.
*
******************************************************************************/
static XStatus XPlat_XilpmInit(void)
{
	XStatus Status;
	XIpiPsu_Config *IpiCfgPtr;

	IpiCfgPtr = XIpiPsu_LookupConfig(XPLAT_IPI_CHANNEL_ID);
	if (NULL == IpiCfgPtr) {
		return XST_FAILURE;
	}

	Status = XIpiPsu_CfgInitialize(&IpiInst, IpiCfgPtr, IpiCfgPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		return Status;
	}

	return XPm_InitXilpm(&IpiInst);
}

/*****************************************************************************/
/**
* @brief	Masked register write through the PLM (XPPU-safe).
*
* @param	NodeId	RegNode whose base + Offset == target register.
* @param	Offset	Byte offset within the RegNode base.
* @param	Mask	Bits to modify (XPLAT_FULL_MASK for a plain write).
* @param	Value	Value to write under Mask.
*
* @return	XST_SUCCESS on success, error code otherwise.
*
******************************************************************************/
static XStatus XPlat_MaskWrite(u32 NodeId, u32 Offset, u32 Mask, u32 Value)
{
	u32 Payload[3];
	u32 Response = 0U;

	Payload[0] = Offset;
	Payload[1] = Mask;
	Payload[2] = Value;

	return XPm_DevIoctl2(NodeId, IOCTL_MASK_WRITE_REG, Payload,
			     3U, &Response, 1U);
}

/*****************************************************************************/
/**
* @brief	Program the Versal Gen2 EL3 secure registers via the PLM
*		(XPPU-safe), before main().
*
* @return	None.
*
* @note	Runs before main() via .init_array. xil-crt0.S emits a symbol
*		reference so this (otherwise unreferenced) archive object is
*		linked; without it the constructor is dropped.
*
******************************************************************************/
void __attribute__((constructor)) Xil_PlatformInit(void)
{
	XStatus Status;

	Status = XPlat_XilpmInit();
	if (XST_SUCCESS != Status) {
		return;
	}

	/* ---- LPD secure SLCR group ---- */
	/* Remove write protection */
	(void)XPlat_MaskWrite(XPLAT_REGNODE_LPD_IOU_SEC,
			      XPLAT_OFF(LPD_SECURE_SLCR_WPROT0, XPLAT_LPD_IOU_SEC_BASE),
			      XPLAT_FULL_MASK, 0U);
	/* Clear TBU default configuration */
	(void)XPlat_MaskWrite(XPLAT_REGNODE_LPD_IOU_SEC,
			      XPLAT_OFF(MMUSECSID_CTRL_TBU, XPLAT_LPD_IOU_SEC_BASE),
			      XPLAT_FULL_MASK, 0U);
	/* Re-apply write protection */
	(void)XPlat_MaskWrite(XPLAT_REGNODE_LPD_IOU_SEC,
			      XPLAT_OFF(LPD_SECURE_SLCR_WPROT0, XPLAT_LPD_IOU_SEC_BASE),
			      XPLAT_FULL_MASK, 1U);

	/* ADMA TrustZone */
	(void)XPlat_MaskWrite(XPLAT_REGNODE_LPD_IOU_SEC, XPLAT_OFF(IOU_AXIPROT_ADMA0, XPLAT_LPD_IOU_SEC_BASE), XPLAT_FULL_MASK, 0U);
	(void)XPlat_MaskWrite(XPLAT_REGNODE_LPD_IOU_SEC, XPLAT_OFF(IOU_AXIPROT_ADMA1, XPLAT_LPD_IOU_SEC_BASE), XPLAT_FULL_MASK, 0U);
	(void)XPlat_MaskWrite(XPLAT_REGNODE_LPD_IOU_SEC, XPLAT_OFF(IOU_AXIPROT_ADMA2, XPLAT_LPD_IOU_SEC_BASE), XPLAT_FULL_MASK, 0U);
	(void)XPlat_MaskWrite(XPLAT_REGNODE_LPD_IOU_SEC, XPLAT_OFF(IOU_AXIPROT_ADMA3, XPLAT_LPD_IOU_SEC_BASE), XPLAT_FULL_MASK, 0U);
	(void)XPlat_MaskWrite(XPLAT_REGNODE_LPD_IOU_SEC, XPLAT_OFF(IOU_AXIPROT_ADMA4, XPLAT_LPD_IOU_SEC_BASE), XPLAT_FULL_MASK, 0U);
	(void)XPlat_MaskWrite(XPLAT_REGNODE_LPD_IOU_SEC, XPLAT_OFF(IOU_AXIPROT_ADMA5, XPLAT_LPD_IOU_SEC_BASE), XPLAT_FULL_MASK, 0U);
	(void)XPlat_MaskWrite(XPLAT_REGNODE_LPD_IOU_SEC, XPLAT_OFF(IOU_AXIPROT_ADMA6, XPLAT_LPD_IOU_SEC_BASE), XPLAT_FULL_MASK, 0U);
	(void)XPlat_MaskWrite(XPLAT_REGNODE_LPD_IOU_SEC, XPLAT_OFF(IOU_AXIPROT_ADMA7, XPLAT_LPD_IOU_SEC_BASE), XPLAT_FULL_MASK, 0U);

	/* ---- PMC IOU secure AxProt group (SD/EMMC/QSPI/OSPI/UFS) ---- */
	(void)XPlat_MaskWrite(XPLAT_REGNODE_PMC_IOU_SEC, XPLAT_OFF(IOU_AXIWPROT_SD0, XPLAT_PMC_IOU_SEC_BASE), XPLAT_FULL_MASK, 0U);
	(void)XPlat_MaskWrite(XPLAT_REGNODE_PMC_IOU_SEC, XPLAT_OFF(IOU_AXIRPROT_SD0, XPLAT_PMC_IOU_SEC_BASE), XPLAT_FULL_MASK, 0U);
	(void)XPlat_MaskWrite(XPLAT_REGNODE_PMC_IOU_SEC, XPLAT_OFF(IOU_AXIWPROT_EMMC0, XPLAT_PMC_IOU_SEC_BASE), XPLAT_FULL_MASK, 0U);
	(void)XPlat_MaskWrite(XPLAT_REGNODE_PMC_IOU_SEC, XPLAT_OFF(IOU_AXIRPROT_EMMC0, XPLAT_PMC_IOU_SEC_BASE), XPLAT_FULL_MASK, 0U);
	(void)XPlat_MaskWrite(XPLAT_REGNODE_PMC_IOU_SEC, XPLAT_OFF(IOU_AXIWPROT_QSPI, XPLAT_PMC_IOU_SEC_BASE), XPLAT_FULL_MASK, 0U);
	(void)XPlat_MaskWrite(XPLAT_REGNODE_PMC_IOU_SEC, XPLAT_OFF(IOU_AXIWPROT_OSPI, XPLAT_PMC_IOU_SEC_BASE), XPLAT_FULL_MASK, 0U);
	(void)XPlat_MaskWrite(XPLAT_REGNODE_PMC_IOU_SEC, XPLAT_OFF(IOU_AXIWPROT_UFS, XPLAT_PMC_IOU_SEC_BASE), XPLAT_FULL_MASK, 0U);
	(void)XPlat_MaskWrite(XPLAT_REGNODE_PMC_IOU_SEC, XPLAT_OFF(IOU_AXIRPROT_UFS, XPLAT_PMC_IOU_SEC_BASE), XPLAT_FULL_MASK, 0U);

	/* ---- GEM / USB AxProt (node 0x3000000B) ---- */
	(void)XPlat_MaskWrite(XPLAT_REGNODE_PMC_IOU_GEMUSB, XPLAT_OFF(IOU_AXIWPROT_GEM0, XPLAT_PMC_IOU_GEMUSB_BASE), XPLAT_FULL_MASK, 0U);
	(void)XPlat_MaskWrite(XPLAT_REGNODE_PMC_IOU_GEMUSB, XPLAT_OFF(IOU_AXIRPROT_GEM0, XPLAT_PMC_IOU_GEMUSB_BASE), XPLAT_FULL_MASK, 0U);
	(void)XPlat_MaskWrite(XPLAT_REGNODE_PMC_IOU_GEMUSB, XPLAT_OFF(IOU_AXIWPROT_GEM1, XPLAT_PMC_IOU_GEMUSB_BASE), XPLAT_FULL_MASK, 0U);
	(void)XPlat_MaskWrite(XPLAT_REGNODE_PMC_IOU_GEMUSB, XPLAT_OFF(IOU_AXIRPROT_GEM1, XPLAT_PMC_IOU_GEMUSB_BASE), XPLAT_FULL_MASK, 0U);
	(void)XPlat_MaskWrite(XPLAT_REGNODE_PMC_IOU_GEMUSB, XPLAT_OFF(IOU_AXIPROT_USB0, XPLAT_PMC_IOU_GEMUSB_BASE), XPLAT_FULL_MASK, 0U);
	(void)XPlat_MaskWrite(XPLAT_REGNODE_PMC_IOU_GEMUSB, XPLAT_OFF(IOU_AXIPROT_USB1, XPLAT_PMC_IOU_GEMUSB_BASE), XPLAT_FULL_MASK, 0U);

	/*
	 * SMMU global-bypass shareability (node 0x3000001A). Bit[31]=Update must
	 * be set for bits[13:12] to latch, so include it in mask and value.
	 */
	(void)XPlat_MaskWrite(XPLAT_REGNODE_SMMU, XPLAT_OFF(SMMU_GBPA, XPLAT_SMMU_BASE),
			      SMMU_GBPA_UPDATE | SMMU_GBPA_SHCFG_MASK,
			      SMMU_GBPA_UPDATE | SMMU_GBPA_SHCFG_OSH);

	/* MMI 10G AxProt (node 0x3000001B) */
	(void)XPlat_MaskWrite(XPLAT_REGNODE_MMI, XPLAT_OFF(MMI_AXIPROT_10G, XPLAT_MMI_BASE),
			      XPLAT_FULL_MASK, 0U);
}

#else /* !XPM_SUPPORT */

/*****************************************************************************/
/**
* @brief	Program the Versal Gen2 EL3 secure registers with direct writes,
*		before main(). Fallback used when XilPM is not built in.
*
* @return	None.
*
* @note	Valid only when XPPU is disabled - direct writes fault under XPPU.
*		XPPU-protected systems must build with XPM_SUPPORT.
*
******************************************************************************/
void __attribute__((constructor)) Xil_PlatformInit(void)
{
	u32 RegVal;

	/* LPD secure SLCR: drop write-protect, clear TBU, restore protect */
	Xil_Out32(LPD_SECURE_SLCR_WPROT0, 0U);
	Xil_Out32(MMUSECSID_CTRL_TBU,     0U);
	Xil_Out32(LPD_SECURE_SLCR_WPROT0, 1U);

	/* SMMU_GBPA: set SHCFG[13:12] = Outer Shareable (bit 31 Update to latch) */
	RegVal  = Xil_In32(SMMU_GBPA);
	RegVal &= ~SMMU_GBPA_SHCFG_MASK;
	RegVal |=  SMMU_GBPA_SHCFG_OSH;
	RegVal |=  SMMU_GBPA_UPDATE;
	Xil_Out32(SMMU_GBPA, RegVal);

	/* GEM / USB AxProt */
	Xil_Out32(IOU_AXIWPROT_GEM0, 0U);
	Xil_Out32(IOU_AXIRPROT_GEM0, 0U);
	Xil_Out32(IOU_AXIWPROT_GEM1, 0U);
	Xil_Out32(IOU_AXIRPROT_GEM1, 0U);
	Xil_Out32(IOU_AXIPROT_USB0,  0U);
	Xil_Out32(IOU_AXIPROT_USB1,  0U);

	/* PMC IOU secure AxProt (SD/EMMC/QSPI/OSPI/UFS) */
	Xil_Out32(IOU_AXIWPROT_SD0,   0U);
	Xil_Out32(IOU_AXIRPROT_SD0,   0U);
	Xil_Out32(IOU_AXIWPROT_EMMC0, 0U);
	Xil_Out32(IOU_AXIRPROT_EMMC0, 0U);
	Xil_Out32(IOU_AXIWPROT_QSPI,  0U);
	Xil_Out32(IOU_AXIWPROT_OSPI,  0U);
	Xil_Out32(IOU_AXIWPROT_UFS,   0U);
	Xil_Out32(IOU_AXIRPROT_UFS,   0U);

	/* LPD IOU secure AxProt (ADMA0..7) */
	Xil_Out32(IOU_AXIPROT_ADMA0, 0U);
	Xil_Out32(IOU_AXIPROT_ADMA1, 0U);
	Xil_Out32(IOU_AXIPROT_ADMA2, 0U);
	Xil_Out32(IOU_AXIPROT_ADMA3, 0U);
	Xil_Out32(IOU_AXIPROT_ADMA4, 0U);
	Xil_Out32(IOU_AXIPROT_ADMA5, 0U);
	Xil_Out32(IOU_AXIPROT_ADMA6, 0U);
	Xil_Out32(IOU_AXIPROT_ADMA7, 0U);

	/* MMI 10G AxProt */
	Xil_Out32(MMI_AXIPROT_10G, 0U);
}

#endif /* XPM_SUPPORT */

#endif /* EL3 == 1 */
