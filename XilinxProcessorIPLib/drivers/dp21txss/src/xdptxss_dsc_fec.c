/******************************************************************************
* Copyright 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
 * @file xdptxss_dsc_fec.c
 * @brief DP TX Subsystem DSC/FEC Support Implementation
 *
 * Implementation using static state and standard driver APIs.
 */

/***************************** Include Files *********************************/
#include "xdptxss_dsc_fec.h"

#ifdef XDPTXSS_USE_DSC

#include "xdp21txss_hw.h"
#include "xdp21_dsc.h"
#include "xdp21_dsc_tu.h"
#include "xdp.h"
#include "xdp_hw.h"
#include "xdebug.h"

#define DSC_PPS_SIZE 128
#define DPCD_DSC_SUPPORT_ADDR        0x00060U
#define DPCD_DSC_ENABLE_ADDR         0x00160U
#define DPCD_FEC_CAPABILITY_ADDR     0x00090U
#define DPCD_FEC_ENABLE_ADDR         0x00120U

static u32 XDpTxSs_GetCurrentPpc(XDpTxSs *InstancePtr, u8 *Ppc)
{
    u32 RegVal;

    if ((InstancePtr == NULL) || (InstancePtr->DpPtr == NULL) || (Ppc == NULL)) {
        return XST_INVALID_PARAM;
    }

    RegVal = XDp_ReadReg(InstancePtr->DpPtr->Config.BaseAddr, XDP_TX_USER_PIXEL_WIDTH);

    /*
     * HW is expected to expose 1/2/4 PPC. Fall back to 1 to avoid divide-by-zero
     * on platforms that do not initialize this register before DSC is configured.
     */
    if ((RegVal != 1U) && (RegVal != 2U) && (RegVal != 4U)) {
        *Ppc = 1U;
    } else {
        *Ppc = (u8)RegVal;
    }

    return XST_SUCCESS;
}

static u32 XDpTxSs_ValidatePpsForSstDsc(const u8 *PpsData, u8 Ppc)
{
    XDp21_DscPps PpsParsed;
    u32 NumSlices;
    u32 Numerator;
    u32 Denominator;
    int Status;

    if ((PpsData == NULL) || (Ppc == 0U)) {
        return XST_INVALID_PARAM;
    }

    Status = XDp21_DscParsePps(PpsData, &PpsParsed);
    if (Status != XST_SUCCESS) {
        xdbg_printf(XDBG_DEBUG_GENERAL, "TX DSC: PPS parse failed\r\n");
        return XST_FAILURE;
    }

    if ((PpsParsed.pic_width == 0U) || (PpsParsed.slice_width == 0U) ||
        (PpsParsed.chunk_size == 0U)) {
        xdbg_printf(XDBG_DEBUG_GENERAL,
                    "TX DSC: Invalid PPS dimensions for DSC\r\n");
        return XST_FAILURE;
    }

    if ((PpsParsed.pic_width % PpsParsed.slice_width) != 0U) {
        xdbg_printf(XDBG_DEBUG_GENERAL,
                    "TX DSC: Unsupported PPS, pic_width not divisible by "
                    "slice_width\r\n");
        return XST_FAILURE;
    }

    NumSlices = (u32)PpsParsed.pic_width / (u32)PpsParsed.slice_width;
    Numerator = (u32)PpsParsed.chunk_size * NumSlices;
    Denominator = 3U * (u32)Ppc;

    /*
     * (chunk_size * num_of_slices) / (3 * PPC) not being an integer
     * ("unaligned with PPC") is no longer treated as fatal: the TX
     * fabric IP already carries DSC_NUM_OF_SLICES-based dummy/EOC byte
     * padding (see TxDscNumSlices in XDpTxSs_WriteDscPps() below, and
     * XDp_TxCalcDscMsaTu()'s NumDummy/NumEoc in xdp21_dsc_tu.c) that
     * absorbs the remainder, so this duplicate gate only needs to log
     * it, matching XDp_TxValidateDscPps() in xdp21_dsc_tu.c.
     */
    if (Denominator == 0U) {
        xdbg_printf(XDBG_DEBUG_GENERAL,
                    "TX DSC: Invalid PPC for compressed HACTIVE check\r\n");
        return XST_FAILURE;
    }
    if ((Numerator % Denominator) != 0U) {
        xdbg_printf(XDBG_DEBUG_GENERAL,
                    "TX DSC: non-integer compressed HACTIVE "
                    "(chunk_size*num_slices)/(3*PPC) -- accepted, "
                    "TX dummy/EOC padding covers the remainder\r\n");
    }

    return XST_SUCCESS;
}

/*****************************************************************************/
int XDpTxSs_SetDscStreamConfig(XDpTxSs *InstancePtr, u8 Enable, const u8 *PpsData)
{
	int Status;

	if (InstancePtr == NULL || InstancePtr->DpPtr == NULL) {
		return XST_INVALID_PARAM;
	}

	if (Enable == 0U) {
		XDp_TxSetDscStreamEnable(InstancePtr->DpPtr, 0U);
		return XST_SUCCESS;
	}

	if (PpsData == NULL) {
		return XST_INVALID_PARAM;
	}

	Status = XDp_TxSetDscStreamPps(InstancePtr->DpPtr, PpsData);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	XDp_TxSetDscStreamEnable(InstancePtr->DpPtr, 1U);
	return XST_SUCCESS;
}

/*****************************************************************************/
int XDpTxSs_RefreshDscMsaTu(XDpTxSs *InstancePtr, u8 Stream)
{
	if (InstancePtr == NULL || InstancePtr->DpPtr == NULL) {
		return XST_INVALID_PARAM;
	}

	XDp_TxCfgMsaRecalculate(InstancePtr->DpPtr, Stream);
	//xil_printf("L3 = 0x%x\r\n", InstancePtr->DpPtr->TxInstance.MsaConfig[Stream - 1]);
	XDp_TxSetMsaValues(InstancePtr->DpPtr, Stream);
	return XST_SUCCESS;
}

/*****************************************************************************/
int XDpTxSs_CalcDscTuValues(const XDp21_DscPps *Pps,
			    u64 UncompPixelClockHz,
			    u8 LinkRate,
			    u8 LaneCount,
			    XDpTxSs_DscTuValues *Out)
{
	XDp_TxMainStreamAttributes MsaTmp;
	u32 FracRaw;
	int Status;

	if ((Pps == NULL) || (Out == NULL) || (LaneCount == 0U) ||
	    (UncompPixelClockHz == 0U)) {
		return XST_INVALID_PARAM;
	}

	MsaTmp.PixelClockHz = UncompPixelClockHz;
	MsaTmp.Misc1 = 0U;
	Status = XDp_TxCalcDscMsaTu(&MsaTmp, Pps, LinkRate, LaneCount);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Out->UserDataCountPerLane = MsaTmp.DataPerLane;
	Out->AvgBytesPerTu = MsaTmp.AvgBytesPerTU;
	Out->InitWait = MsaTmp.InitWait;
	Out->MinBytesPerTu = MsaTmp.AvgBytesPerTU / 1000U;
	FracRaw = MsaTmp.AvgBytesPerTU % 1000U;
	Out->FracBytesPerTu = (FracRaw * 1024U) / 1000U;

	{
		u32 lanes = (u32)LaneCount;
		u32 dummy = ((Pps->chunk_size % lanes) == 0U) ? 0U :
			      (lanes - (Pps->chunk_size % lanes));
		u32 slices = ((u32)Pps->pic_width + (u32)Pps->slice_width - 1U) /
			     (u32)Pps->slice_width;

		Out->CompBytesPerLine = (Pps->chunk_size + dummy + lanes) * slices;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
int XDpTxSs_ApplyDscTuValues(XDpTxSs *InstancePtr,
			     const XDpTxSs_DscTuValues *Values,
			     u8 Stream)
{
	XDp *DpPtr;
	UINTPTR BaseAddr;
	XDp_Tx *TxInst;
	XDp_TxMainStreamAttributes *MsaConfig;
	u32 MiscOneRd;
	u32 StreamOffset[4] = { 0U, XDP_TX_STREAM2_MSA_START_OFFSET,
				XDP_TX_STREAM3_MSA_START_OFFSET,
				XDP_TX_STREAM4_MSA_START_OFFSET };

	if ((InstancePtr == NULL) || (InstancePtr->DpPtr == NULL) ||
	    (Values == NULL) || (Stream == 0U) || (Stream > 4U)) {
		return XST_INVALID_PARAM;
	}

	DpPtr = InstancePtr->DpPtr;
	TxInst = &DpPtr->TxInstance;
	MsaConfig = &TxInst->MsaConfig[Stream - 1U];
	BaseAddr = DpPtr->Config.BaseAddr;

	MsaConfig->DataPerLane = Values->UserDataCountPerLane;
	MsaConfig->AvgBytesPerTU = Values->AvgBytesPerTu;
	MsaConfig->InitWait = Values->InitWait;
	MsaConfig->TransferUnitSize = XDP_TX_MAX_NUM_OF_TIMESLOTS;

	XDp_WriteReg(BaseAddr,
		     XDP_TX_USER_DATA_COUNT_PER_LANE + StreamOffset[Stream - 1U],
		     Values->UserDataCountPerLane);
	XDp_WriteReg(BaseAddr,
		     XDP_TX_MIN_BYTES_PER_TU + StreamOffset[Stream - 1U],
		     Values->MinBytesPerTu);
	XDp_WriteReg(BaseAddr,
		     XDP_TX_FRAC_BYTES_PER_TU + StreamOffset[Stream - 1U],
		     Values->FracBytesPerTu);
	XDp_WriteReg(BaseAddr,
		     XDP_TX_INIT_WAIT + StreamOffset[Stream - 1U],
		     Values->InitWait);
	XDp_WriteReg(BaseAddr,
		     XDP_TX_TU_SIZE + StreamOffset[Stream - 1U],
		     XDP_TX_MAX_NUM_OF_TIMESLOTS);

	/* MISC1 bit 6 = compressed stream marker (DP 2.1). */
	MiscOneRd = XDp_ReadReg(BaseAddr,
				XDP_TX_MAIN_STREAM_MISC1 + StreamOffset[Stream - 1U]);
	XDp_WriteReg(BaseAddr,
		     XDP_TX_MAIN_STREAM_MISC1 + StreamOffset[Stream - 1U],
		     MiscOneRd | (1U << 6));

	return XST_SUCCESS;
}

/************************** Static Variables *********************************/
static u8 TxDscEnabled = 0;
static u8 TxFecEnabled = 0;
static u8 TxDscPassthrough = 0;
static u32 TxDscEnableOffsetInUse = XDP21_TX_DSC_ENABLE_OFFSET;
static u8 TxDscDivisionFactor = 1U; /* DSC_ENABLE[3:1] at 0x0094 per RAL */
static u8 TxDscNumSlices = 0U;     /* DSC_NUM_OF_SLICES[12:8] at 0x0094 = ceil(pic_width/slice_width) */
/* -1 unknown, 0 unsupported, 1 supported */
static int TxPpsCtrlSupport = -1;
static u8 TxPpsCtrlFallbackMsgPrinted = 0U;

/************************** Function Definitions *****************************/

/**
 * @brief Initialize DSC/FEC support for TX Subsystem
 *
 * Probes sink DSC/FEC capability and auto-enables:
 *   - FEC enabled if sink supports it (DPCD 0x090)
 *   - DSC passthrough set ready
 *   - Actual DSC enable happens when VB-ID[6] compressed stream is detected
 *     and PPS is available; otherwise normal stream is forwarded
 *
 * If sink is not connected, AUX reads fail gracefully and enablement
 * is deferred to runtime (ConfigureTx path).
 */
int XDpTxSs_DscFecInit(XDpTxSs *InstancePtr)
{
    u8 FecCap = 0;
    u8 DscCap = 0;
    u32 Status;

    if (InstancePtr == NULL || InstancePtr->DpPtr == NULL) {
        return XST_FAILURE;
    }

    TxDscEnabled = 0;
    TxFecEnabled = 0;
    TxDscPassthrough = 0;
    TxDscDivisionFactor = 1U;
    TxDscNumSlices = 0U;
    TxPpsCtrlSupport = -1;
    TxPpsCtrlFallbackMsgPrinted = 0U;

    /* Probe sink FEC capability (DPCD 0x090) — non-fatal if sink not connected */
    Status = XDp_TxAuxRead(InstancePtr->DpPtr, DPCD_FEC_CAPABILITY_ADDR, 1, &FecCap);
    if (Status == XST_SUCCESS && (FecCap & 0x01)) {
        u8 FecEnable = 0x01;
        (void)XDp_TxAuxWrite(InstancePtr->DpPtr, DPCD_FEC_ENABLE_ADDR, 1, &FecEnable);
        TxFecEnabled = 1;
    }

    /* Probe sink DSC capability (DPCD 0x060) — non-fatal if sink not connected */
    Status = XDp_TxAuxRead(InstancePtr->DpPtr, DPCD_DSC_SUPPORT_ADDR, 1, &DscCap);
    (void)Status;

    TxDscPassthrough = 1;

#ifdef DEBUG
    xil_printf("TX DSC/FEC: Init done (FEC=%s, sink_DSC=%s, passthrough=ready)\r\n",
               TxFecEnabled ? "ON" : "OFF",
               (DscCap & 0x01) ? "YES" : "NO");
#endif

    return XST_SUCCESS;
}

/**
 * @brief Enable/Disable FEC on TX
 */
int XDpTxSs_SetFecEnable(XDpTxSs *InstancePtr, u8 Enable)
{
    u8 FecCap = 0;
    u32 Status;

    if (InstancePtr == NULL || InstancePtr->DpPtr == NULL) {
        return XST_FAILURE;
    }

    /* Read sink FEC capability (DPCD 0x090) */
    Status = XDp_TxAuxRead(InstancePtr->DpPtr, DPCD_FEC_CAPABILITY_ADDR, 1, &FecCap);
    if (Status != XST_SUCCESS) {
        xdbg_printf(XDBG_DEBUG_GENERAL,
                    "TX FEC: Failed to read sink capability\r\n");
        return XST_FAILURE;
    }

    if (Enable && (FecCap & 0x01)) {
        /* 1. Enable FEC in sink (DPCD 0x120) */
        u8 FecEnable = 0x01;
        Status = XDp_TxAuxWrite(InstancePtr->DpPtr, DPCD_FEC_ENABLE_ADDR, 1, &FecEnable);
        if (Status != XST_SUCCESS) {
            xdbg_printf(XDBG_DEBUG_GENERAL,
                        "TX FEC: Failed to enable in sink\r\n");
            return XST_FAILURE;
        }

        /*
         * 2. Enable FEC in TX AXI-Lite register 0x008 (XDP_TX_ENHANCED_FRAME_EN).
         *    bit[0] = enhanced framing (required for all DP links)
         *    bit[8] = FEC enable (DP 2.1 TX IP extension — must be set so the TX
         *             actually generates FEC-protected link symbols on the wire;
         *             without this the sink reports FEC=Disabled and CRC fails).
         *
         * NOTE: toggling bit[8] can cause the TX IP to re-arm its internal
         * pipeline, which may clear XDP_TX_ENABLE (0x080) and
         * XDP_TX_ENABLE_MAIN_STREAM (0x084).  Re-assert both after the FEC
         * write to ensure the main link video output is not interrupted.
         */
        UINTPTR TxBase = InstancePtr->DpPtr->Config.BaseAddr;
        XDp_WriteReg(TxBase, XDP_TX_ENHANCED_FRAME_EN,
                     1U | (1U << 8));   /* 0x101: enhanced_framing=1 + fec_enable=1 */

        /* Re-assert main link enables in case the FEC write cleared them */
        XDp_WriteReg(TxBase, XDP_TX_ENABLE, 1U);
        XDp_WriteReg(TxBase, XDP_TX_ENABLE_MAIN_STREAM, 1U);

        TxFecEnabled = 1;

#ifdef DEBUG
        xil_printf("TX FEC: ENABLED\r\n");
#endif

    } else if (!Enable) {
        TxFecEnabled = 0;
        /*
         * Disable FEC in TX AXI: clear bit[8], keep enhanced framing (bit[0]).
         * DPCD 0x120 write is skipped here because the sink resets itself on
         * link loss; we just make sure the TX IP stops generating FEC.
         */
        UINTPTR TxBase = InstancePtr->DpPtr->Config.BaseAddr;
        XDp_WriteReg(TxBase, XDP_TX_ENHANCED_FRAME_EN, 1U); /* 0x001: fec_enable=0 */

#ifdef DEBUG
        xil_printf("TX FEC: DISABLED\r\n");
#endif
    }

    return XST_SUCCESS;
}

/**
 * @brief Negotiate DSC capabilities with sink
 */
int XDpTxSs_DscNegotiateCapabilities(XDpTxSs *InstancePtr)
{
    u8 DscSupport = 0;
    u32 Status;

    if (InstancePtr == NULL || InstancePtr->DpPtr == NULL) {
        return XST_FAILURE;
    }

    /* Read DSC support (DPCD 0x060) */
    Status = XDp_TxAuxRead(InstancePtr->DpPtr, DPCD_DSC_SUPPORT_ADDR, 1, &DscSupport);
    if (Status != XST_SUCCESS || !(DscSupport & 0x01)) {
        xdbg_printf(XDBG_DEBUG_GENERAL,
                    "TX DSC: Sink does not support DSC\r\n");
        return XST_FAILURE;
    }
#ifdef DEBUG
    xil_printf("TX DSC: Sink supports DSC\r\n");
#endif
    return XST_SUCCESS;
}

/**
 * @brief Enable DSC pass-through on TX
 */
int XDpTxSs_EnableDscPassthrough(XDpTxSs *InstancePtr, u8 Enable)
{
    UINTPTR BaseAddr;
    u32 PpsCtrl;

    if (InstancePtr == NULL || InstancePtr->DpPtr == NULL) {
        return XST_FAILURE;
    }

    BaseAddr = InstancePtr->DpPtr->Config.BaseAddr;
    if (TxPpsCtrlSupport == 0) {
        TxDscPassthrough = Enable ? 1U : 0U;
        return XST_SUCCESS;
    }
    PpsCtrl = XDp_ReadReg(BaseAddr, XDP21_TX_PPS_SDP_CTRL_OFFSET);

    if (Enable) {
        PpsCtrl |= XDP21_TX_DSC_PASSTHROUGH;
    } else {
        PpsCtrl &= ~((u32)XDP21_TX_DSC_PASSTHROUGH);
    }
    XDp_WriteReg(BaseAddr, XDP21_TX_PPS_SDP_CTRL_OFFSET, PpsCtrl);
    PpsCtrl = XDp_ReadReg(BaseAddr, XDP21_TX_PPS_SDP_CTRL_OFFSET);

    TxDscPassthrough = Enable ? 1U : 0U;
    return XST_SUCCESS;
}

/**
 * @brief Write PPS data to TX hardware
 *
 * Writes the 128-byte DSC PPS to DSC_PPS_DATA registers (0x0710-0x078C)
 * per DP TX PPS RAL. DSC_ENABLE is programmed separately in XDpTxSs_SetDscEnable().
 * Byte order: BYTE_0 [7:0], BYTE_1 [15:8], BYTE_2 [23:16], BYTE_3 [31:24].
 */
int XDpTxSs_WriteDscPps(XDpTxSs *InstancePtr, u8 *PpsData)
{
    UINTPTR BaseAddr;
    u32 RegValue;
    u32 RegOffset;
    u8 Ppc;
    u32 Status;
    int i;

    if (InstancePtr == NULL || InstancePtr->DpPtr == NULL || PpsData == NULL) {
        return XST_FAILURE;
    }

    Status = XDpTxSs_GetCurrentPpc(InstancePtr, &Ppc);
    if (Status != XST_SUCCESS) {
        xdbg_printf(XDBG_DEBUG_GENERAL,
                    "TX DSC: Failed to read PPC configuration\r\n");
        return XST_FAILURE;
    }

    Status = XDpTxSs_ValidatePpsForSstDsc(PpsData, Ppc);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    TxDscDivisionFactor = 1U;

    /*
     * Compute DSC_NUM_OF_SLICES = ceil(pic_width / slice_width) for 0x094[12:8].
     * pic_width  = {PPS8[7:0], PPS9[7:0]}   (big-endian in PPS packet)
     * slice_width = {PPS12[7:0], PPS13[7:0]}
     */
    {
        u16 pps_pic_width   = ((u16)PpsData[8] << 8) | (u16)PpsData[9];
        u16 pps_slice_width = ((u16)PpsData[12] << 8) | (u16)PpsData[13];

        if (pps_slice_width != 0U) {
            TxDscNumSlices = (u8)((pps_pic_width + pps_slice_width - 1U) /
                                  pps_slice_width);
        } else {
            TxDscNumSlices = 0U;
        }
#ifdef DEBUG
        xil_printf("TX DSC: DSC_NUM_OF_SLICES=%u\r\n", (u32)TxDscNumSlices);
#endif
    }

    BaseAddr = InstancePtr->DpPtr->Config.BaseAddr;

    /* Write 128-byte PPS to DSC_PPS_DATA (0x0710-0x078C)
     * Per RAL: DSC_PPS_DATA[31:0] at 0x0710, [63:32] at 0x0714, ... [1023:992] at 0x078C
     * Byte order: BYTE_0 [7:0], BYTE_1 [15:8], BYTE_2 [23:16], BYTE_3 [31:24]
     */
    for (i = 0; i < DSC_PPS_SIZE; i += 4) {
        RegOffset = XDP21_TX_DSC_PPS_DATA_BASE_OFFSET + (u32)i;
        if (RegOffset > XDP21_TX_DSC_PPS_DATA_LAST_OFFSET) {
            xdbg_printf(XDBG_DEBUG_GENERAL,
                        "TX DSC: PPS register offset overflow (0x%03X)\r\n",
                        (unsigned int)RegOffset);
            return XST_FAILURE;
        }

        RegValue = (u32)PpsData[i + 0] |
                  ((u32)PpsData[i + 1] << 8) |
                  ((u32)PpsData[i + 2] << 16) |
                  ((u32)PpsData[i + 3] << 24);
        XDp_WriteReg(BaseAddr, RegOffset, RegValue);
    }

#ifdef DEBUG
    xil_printf("TX DSC: PPS written\r\n");
#endif
    return XST_SUCCESS;
}

/**
 * @brief Enable/Disable DSC using guideline programming order
 */
int XDpTxSs_SetDscEnable(XDpTxSs *InstancePtr, u8 Enable)
{
    /*
     * DPCD 0x160 DSC_ENABLE register bit fields:
     *   bit[0] = DSC_ENABLE            (1 = enable DSC at sink)
     *   bit[1] = DSC_PASS_THROUGH_ENABLE (1 = DP repeater passes DSC stream
     *            through without decoding — required for passthrough topology)
     *
     * For a DP DSC passthrough repeater: write 0x03 (both bits set).
     * 0x01 alone only enables DSC decode at the sink but does not tell the
     * downstream device that this is a passthrough path; some sinks and
     * analyzers (e.g. UCD500) expect bit[1]=1 to recognise the stream as
     * a DSC pass-through and report the PPS correctly.
     */
    u8 DscEnable = Enable ? 0x03U : 0x00U;
    u8 SinkDscEnableRd = 0U;
    u32 ActiveDscOffset = TxDscEnableOffsetInUse;
    u32 DscEnableRegVal;
    u32 Status;
    UINTPTR BaseAddr;
    u32 TxDscEnableRd;
    u32 TxDscEnableAltRd;
    u32 PpsCtrlRd;
    u32 PpsSendRd;

    if ((InstancePtr == NULL) || (InstancePtr->DpPtr == NULL)) {
        return XST_FAILURE;
    }

    BaseAddr = InstancePtr->DpPtr->Config.BaseAddr;

    if (Enable) {
        /* Safety check before setting sink DSC_ENABLE */
        Status = XDpTxSs_DscNegotiateCapabilities(InstancePtr);
        if (Status != XST_SUCCESS) {
            xdbg_printf(XDBG_DEBUG_GENERAL,
                        "TX DSC: Sink capability check failed before DSC "
                        "enable\r\n");
            return XST_FAILURE;
        }

        /*
         * Guideline programming order:
         * 1) Program DSC_NUM_OF_SLICES in 0x0094[12:8] (before DSC_EN)
         * 2) Set DSC_ENABLE in sink from TX through AUX (DPCD 0x160)
         * 3) Set DSC_ENABLE of TX AXI-Lite programmable registers
         */

        /*
         * Step 1: Pre-program DSC_NUM_OF_SLICES into 0x0094[12:8].
         *
         *   DSC_NUM_OF_SLICES = ceil(pic_width / slice_width)
         *     pic_width   from PPS {PPS8[7:0], PPS9[7:0]}
         *     slice_width from PPS {PPS12[7:0], PPS13[7:0]}
         *
         * Write ONLY the num_slices + div_factor fields; DSC_EN bit[0] is
         * NOT set yet — the IP must see the slice configuration before
         * DSC is activated.
         */
        {
            u32 NumSlicesPreReg = ((u32)TxDscNumSlices << XDP21_TX_DSC_NUM_SLICES_SHIFT) &
                                  XDP21_TX_DSC_NUM_SLICES_MASK;
            NumSlicesPreReg |= ((u32)TxDscDivisionFactor << 1);
            XDp_WriteReg(BaseAddr, XDP21_TX_DSC_ENABLE_OFFSET_ALT, NumSlicesPreReg);
#ifdef DEBUG
            xil_printf("TX DSC: 0x094 pre-programmed (NUM_SLICES=%u)\r\n", (u32)TxDscNumSlices);
#endif
        }

        /*
         * Step 2: Set DSC_ENABLE in sink from TX through AUX (DPCD 0x160).
         */
        Status = XDp_TxAuxWrite(InstancePtr->DpPtr, DPCD_DSC_ENABLE_ADDR, 1, &DscEnable);
        if (Status != XST_SUCCESS) {
            xdbg_printf(XDBG_DEBUG_GENERAL,
                        "TX DSC: Failed to set sink DSC_ENABLE (0x160)\r\n");
            return XST_FAILURE;
        }

        /*
         * Step 3: Set DSC_ENABLE of TX AXI-Lite programmable registers.
         *
         * Ensure PPS SDP packets are emitted on TX main link.
         * Program PPS SDP control before asserting TX DSC enable.
         */
        if (TxPpsCtrlSupport != 0) {
            XDp_WriteReg(BaseAddr, XDP21_TX_PPS_SDP_CTRL_OFFSET,
                         XDP21_TX_PPS_SDP_ENABLE | XDP21_TX_DSC_PASSTHROUGH);
            XDp_WriteReg(BaseAddr, XDP21_TX_PPS_SDP_SEND_OFFSET, 0x1U);
            XDp_WriteReg(BaseAddr, XDP21_TX_PPS_SDP_CTRL_OFFSET,
                         XDP21_TX_PPS_SDP_ENABLE | XDP21_TX_PPS_SDP_TRIGGER |
                         XDP21_TX_DSC_PASSTHROUGH);
            XDp_WriteReg(BaseAddr, XDP21_TX_PPS_SDP_CTRL_OFFSET,
                         XDP21_TX_PPS_SDP_ENABLE | XDP21_TX_DSC_PASSTHROUGH);
        }

        /*
         * Write DSC_ENABLE to both register locations:
         *
         * - 0x0790: bit[0]=DSC_EN, bit[2:1]=DIV_FACTOR, bit[2]=PASSTHROUGH
         * - 0x0094: bit[0]=DSC_EN, bit[2:1]=DIV_FACTOR, bit[12:8]=NUM_SLICES
         */
        DscEnableRegVal = (u32)0x1U | ((u32)TxDscDivisionFactor << 1);
        XDp_WriteReg(BaseAddr, XDP21_TX_DSC_ENABLE_OFFSET,
                     DscEnableRegVal | (u32)XDP21_TX_DSC_PASSTHROUGH);
        DscEnableRegVal |= ((u32)TxDscNumSlices << XDP21_TX_DSC_NUM_SLICES_SHIFT) &
                           XDP21_TX_DSC_NUM_SLICES_MASK;
        XDp_WriteReg(BaseAddr, XDP21_TX_DSC_ENABLE_OFFSET_ALT, DscEnableRegVal);

        /*
         * DSC_EN_ACTION (0x0794): write-to-trigger register that arms the TX IP
         * to start emitting PPS SDP packets from the stored PPS data (0x0710-0x078C)
         * on every subsequent VBlank.
         *
         * In this RTL the PPS SDP control path via 0x0790 / 0x0804-0x0808 is not
         * implemented (both always read back 0).  0x0794 is the only register in
         * the DSC block that triggers PPS SDP output onto the DP main link.
         * PPS registers MUST be valid before this write (guaranteed by call order
         * in DscFecPt_ConfigureTx: WriteDscPps → EnableDscPassthrough → SetDscEnable).
         */
        XDp_WriteReg(BaseAddr, XDP21_TX_DSC_EN_ACTION_OFFSET, 0x1U);

        TxDscEnabled = 1U;
        (void)XDp_TxAuxRead(InstancePtr->DpPtr, DPCD_DSC_ENABLE_ADDR, 1, &SinkDscEnableRd);
        TxDscEnableRd = XDp_ReadReg(BaseAddr, XDP21_TX_DSC_ENABLE_OFFSET);
        TxDscEnableAltRd = XDp_ReadReg(BaseAddr, XDP21_TX_DSC_ENABLE_OFFSET_ALT);
        PpsCtrlRd = XDp_ReadReg(BaseAddr, XDP21_TX_PPS_SDP_CTRL_OFFSET);
        PpsSendRd = XDp_ReadReg(BaseAddr, XDP21_TX_PPS_SDP_SEND_OFFSET);
        if ((TxPpsCtrlSupport == -1) && (PpsCtrlRd == 0U) && (PpsSendRd == 0U)) {
            TxPpsCtrlSupport = 0;

#ifdef DEBUG
            xil_printf("TX DSC: PPS_SDP_CTRL/SEND not implemented at 0x804/0x808 (0x800-0x8FF payload table map)\r\n");
#endif
            if (TxPpsCtrlFallbackMsgPrinted == 0U) {
#ifdef DEBUG
                xil_printf("TX DSC: Using fallback signaling path: sink DPCD 0x160 + TX AXI-Lite DSC_ENABLE (0x094/0x790 auto-detect)\r\n");
#endif
                TxPpsCtrlFallbackMsgPrinted = 1U;
            }
        } else if (TxPpsCtrlSupport == -1) {
            TxPpsCtrlSupport = 1;
        }

        /*
         * Auto-detect which TX DSC enable register is implemented in current RTL.
         */
        if ((TxDscEnableRd & 0x1U) != 0U) {
            ActiveDscOffset = (u32)XDP21_TX_DSC_ENABLE_OFFSET;
        } else if ((TxDscEnableAltRd & 0x1U) != 0U) {
            ActiveDscOffset = (u32)XDP21_TX_DSC_ENABLE_OFFSET_ALT;
        }
        TxDscEnableOffsetInUse = ActiveDscOffset;
#ifdef DEBUG
        xil_printf("TX DSC: ENABLED\r\n");
#endif
    } else {
        /* Disable in reverse-safe order: TX first, then sink */
        XDp_WriteReg(BaseAddr, XDP21_TX_DSC_ENABLE_OFFSET, 0x00U);
        XDp_WriteReg(BaseAddr, XDP21_TX_DSC_ENABLE_OFFSET_ALT, 0x00U);
        if (TxPpsCtrlSupport != 0) {
            XDp_WriteReg(BaseAddr, XDP21_TX_PPS_SDP_SEND_OFFSET, 0x0U);
            XDp_WriteReg(BaseAddr, XDP21_TX_PPS_SDP_CTRL_OFFSET, 0x0U);
        }
        (void)XDp_TxAuxWrite(InstancePtr->DpPtr, DPCD_DSC_ENABLE_ADDR, 1, &DscEnable);
        TxDscEnabled = 0U;
        (void)XDp_TxAuxRead(InstancePtr->DpPtr, DPCD_DSC_ENABLE_ADDR, 1, &SinkDscEnableRd);

#ifdef DEBUG
        xil_printf("TX DSC: DISABLED\r\n");
#endif
    }

    return XST_SUCCESS;
}

#endif /* XDPTXSS_USE_DSC */
