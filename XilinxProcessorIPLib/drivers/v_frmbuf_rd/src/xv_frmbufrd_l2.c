/******************************************************************************
* Copyright (C) 2017-2023 Xilinx, Inc. All Rights Reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_frmbufrd_l2.c
* @addtogroup v_frmbuf_rd Overview
* @{
*
* Frame Buffer Read Layer-2 Driver. The functions in this file provides an
* abstraction from the register peek/poke methodology by implementing most
* common use-case provided by the sub-core. See xv_frmbufrd_l2.h for a detailed
* description of the layer-2 driver
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  vyc   04/05/17   Initial Release
* 2.00  vyc   10/04/17   Add second buffer pointer for semi-planar formats
*                        Add memory formats RGBA8, YUVA8, BGRA8, BGRX8, UYVY8
* 3.00  vyc   04/04/18   Add interlaced support
                         Add new memory format BGR8
                         Add interrupt handler for ap_ready
* 4.00  pv    11/10/18   Added flushing feature support in driver.
*			 flush bit should be set and held (until reset) by
*			 software to flush pending transactions.IP is expecting
*			 a hard reset, when flushing is done.(There is a flush
*			 status bit and is asserted when the flush is done).
* 4.10  vv    02/05/19   Added new pixel formats with 12 and 16 bpc.
* 4.50  pg    01/07/21   Added new registers to support fid_out interlace solution.
*						Interrupt count support for throughput measurement.
* 4.70  pg    05/23/23   Added new 3 planar video format Y_U_V8_420.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>
#include "xv_frmbufrd_l2.h"
#include "sleep.h"

/************************** Constant Definitions *****************************/
#define XVFRMBUFRD_MIN_STRM_WIDTH            (64u)
#define XVFRMBUFRD_MIN_STRM_HEIGHT           (64u)
#define XVFRMBUFRD_IDLE_TIMEOUT              (1000000)
#define XV_WAIT_FOR_FLUSH_DONE		         (25)
#define XV_WAIT_FOR_FLUSH_DELAY		         (2000)
/************************** Function Prototypes ******************************/
static void SetPowerOnDefaultState(XV_FrmbufRd_l2 *InstancePtr);
XVidC_ColorFormat RdMemory2Live(XVidC_ColorFormat MemFmt);

/*****************************************************************************/
/**
* This function maps the memory video formats to the live/stream video formats
*
* @param  MemFmt is the video format read from memory
*
* @return Live/Stream Video Format associated with that memory format
*
******************************************************************************/
XVidC_ColorFormat RdMemory2Live(XVidC_ColorFormat MemFmt)
{
	XVidC_ColorFormat StrmFmt;

	switch(MemFmt) {
		case XVIDC_CSF_MEM_RGBX8 :
			StrmFmt = XVIDC_CSF_RGB;
			break;
		case XVIDC_CSF_MEM_YUVX8 :
			StrmFmt = XVIDC_CSF_YCRCB_444;
			break;
		case XVIDC_CSF_MEM_YUYV8 :
			StrmFmt = XVIDC_CSF_YCRCB_422;
			break;
		case XVIDC_CSF_MEM_RGBA8 :
			StrmFmt = XVIDC_CSF_RGBA;
			break;
		case XVIDC_CSF_MEM_YUVA8 :
			StrmFmt = XVIDC_CSF_YCRCBA_444;
			break;
		case XVIDC_CSF_MEM_RGBX10 :
			StrmFmt = XVIDC_CSF_RGB;
			break;
		case XVIDC_CSF_MEM_YUVX10 :
			StrmFmt = XVIDC_CSF_YCRCB_444;
			break;
		case XVIDC_CSF_MEM_Y_UV8 :
			StrmFmt = XVIDC_CSF_YCRCB_422;
			break;
		case XVIDC_CSF_MEM_Y_UV8_420 :
			StrmFmt = XVIDC_CSF_YCRCB_420;
			break;
		case XVIDC_CSF_MEM_RGB8 :
			StrmFmt = XVIDC_CSF_RGB;
			break;
		case XVIDC_CSF_MEM_YUV8 :
			StrmFmt = XVIDC_CSF_YCRCB_444;
			break;
		case XVIDC_CSF_MEM_Y_UV10 :
			StrmFmt = XVIDC_CSF_YCRCB_422;
			break;
		case XVIDC_CSF_MEM_Y_UV10_420 :
			StrmFmt = XVIDC_CSF_YCRCB_420;
			break;
		case XVIDC_CSF_MEM_Y8 :
			StrmFmt = XVIDC_CSF_YONLY;
			break;
		case XVIDC_CSF_MEM_Y10 :
			StrmFmt = XVIDC_CSF_YONLY;
			break;
		case XVIDC_CSF_MEM_BGRA8 :
			StrmFmt = XVIDC_CSF_RGBA;
			break;
		case XVIDC_CSF_MEM_BGRX8 :
			StrmFmt = XVIDC_CSF_RGB;
			break;
		case XVIDC_CSF_MEM_UYVY8 :
			StrmFmt = XVIDC_CSF_YCRCB_422;
			break;
		case XVIDC_CSF_MEM_BGR8 :
			StrmFmt = XVIDC_CSF_RGB;
			break;
		case XVIDC_CSF_MEM_RGBX12 :
			StrmFmt = XVIDC_CSF_RGB;
			break;
		case XVIDC_CSF_MEM_YUVX12 :
			StrmFmt = XVIDC_CSF_YCRCB_444;
			break;
		case XVIDC_CSF_MEM_Y_UV12 :
			StrmFmt = XVIDC_CSF_YCRCB_422;
			break;
		case XVIDC_CSF_MEM_Y_UV12_420 :
			StrmFmt = XVIDC_CSF_YCRCB_420;
			break;
		case XVIDC_CSF_MEM_Y12 :
			StrmFmt = XVIDC_CSF_YONLY;
			break;
		case XVIDC_CSF_MEM_RGB16 :
			StrmFmt = XVIDC_CSF_RGB;
			break;
		case XVIDC_CSF_MEM_YUV16 :
			StrmFmt = XVIDC_CSF_YCRCB_444;
			break;
		case XVIDC_CSF_MEM_Y_UV16 :
			StrmFmt = XVIDC_CSF_YCRCB_422;
			break;
		case XVIDC_CSF_MEM_Y_UV16_420 :
			StrmFmt = XVIDC_CSF_YCRCB_420;
			break;
		case XVIDC_CSF_MEM_Y16 :
			StrmFmt = XVIDC_CSF_YONLY;
			break;
        case XVIDC_CSF_MEM_Y_U_V8 :
            StrmFmt = XVIDC_CSF_YCRCB_444;
            break;
		case XVIDC_CSF_MEM_Y_U_V10 :
            StrmFmt = XVIDC_CSF_YCRCB_444;
            break;
        case XVIDC_CSF_MEM_Y_U_V8_420 :
            StrmFmt = XVIDC_CSF_YCRCB_420;
            break;
		case XVIDC_CSF_MEM_Y_U_V12 :
            StrmFmt = XVIDC_CSF_YCRCB_444;
            break;
		default:
			StrmFmt = (XVidC_ColorFormat)~0;
			break;
	}
	return(StrmFmt);
}

/**
 * Initializes the XV_FrmbufRd_l2 instance.
 *
 * This function sets up the XV_FrmbufRd_l2 instance structure, zeroes its memory,
 * and initializes the underlying XV_frmbufrd hardware or driver instance.
 * Depending on the build configuration, it uses either a DeviceId or a BaseAddress
 * to perform the initialization.
 *
 * @param InstancePtr Pointer to the XV_FrmbufRd_l2 instance to be initialized.
 * @param DeviceId Device ID of the XV_frmbufrd hardware (used if SDT is not defined).
 *
 * @return
 *   - XST_SUCCESS if initialization was successful.
 *   - Error code otherwise.
 *
 * @note
 *   - The function asserts that InstancePtr is not NULL.
 *   - After successful initialization, the instance is set to its power-on default state.
 */
#ifndef SDT
int XVFrmbufRd_Initialize(XV_FrmbufRd_l2 *InstancePtr, u16 DeviceId)
#else
int XVFrmbufRd_Initialize(XV_FrmbufRd_l2 *InstancePtr, UINTPTR BaseAddress)
#endif
{
	int Status;
	Xil_AssertNonvoid(InstancePtr != NULL);

	/*Initialising frmbufrd structure to 0*/
	memset(InstancePtr, 0, sizeof(XV_FrmbufRd_l2));
	/* Setup the instance */
#ifndef SDT
	Status = XV_frmbufrd_Initialize(&InstancePtr->FrmbufRd, DeviceId);
#else
	Status = XV_frmbufrd_Initialize(&InstancePtr->FrmbufRd, BaseAddress);
#endif
	/*Programming frmbufrd structure with default values*/
	if (Status == XST_SUCCESS) {
		SetPowerOnDefaultState(InstancePtr);
	}
	return(Status);
}

/**
 * SetPowerOnDefaultState - Initializes the frame buffer reader instance to its default power-on state.
 *
 * This function configures the XV_FrmbufRd_l2 instance with default video stream parameters,
 * including resolution, color format, frame rate, color depth, and pixels per clock. It sets
 * the hardware registers for frame width, height, stride, and memory video format. If interlaced
 * mode is enabled, it also sets the field ID register. The function disables the DONE and READY
 * interrupts to set up polling mode.
 *
 * @param InstancePtr Pointer to the XV_FrmbufRd_l2 instance to be initialized.
 *
 * @return None
 */
static void SetPowerOnDefaultState(XV_FrmbufRd_l2 *InstancePtr)
{
	XVidC_VideoStream VidStrm;
	XVidC_VideoTiming const *ResTiming;

	u32 IrqMask = 0;

	memset(&VidStrm, 0, sizeof(XVidC_VideoStream));

	/* Set Default Stream Out */
	VidStrm.VmId          = XVIDC_VM_1920x1080_60_P;
	VidStrm.ColorFormatId = XVIDC_CSF_RGB;
	VidStrm.FrameRate     = XVIDC_FR_60HZ;
	VidStrm.IsInterlaced  = FALSE;
	VidStrm.ColorDepth    = (XVidC_ColorDepth)InstancePtr->FrmbufRd.Config.MaxDataWidth;
	VidStrm.PixPerClk     = (XVidC_PixelsPerClock)InstancePtr->FrmbufRd.Config.PixPerClk;

	ResTiming = XVidC_GetTimingInfo(VidStrm.VmId);

	VidStrm.Timing = *ResTiming;

	/* Set frame width, height, stride and memory video format */
	XV_frmbufrd_Set_HwReg_width(&InstancePtr->FrmbufRd,  VidStrm.Timing.HActive);
	XV_frmbufrd_Set_HwReg_height(&InstancePtr->FrmbufRd, VidStrm.Timing.VActive);
	XV_frmbufrd_Set_HwReg_stride(&InstancePtr->FrmbufRd, 7680);
	XV_frmbufrd_Set_HwReg_video_format(&InstancePtr->FrmbufRd, XVIDC_CSF_MEM_RGBX8);
	if (XVFrmbufRd_InterlacedEnabled(InstancePtr)) {
		XV_frmbufrd_Set_HwReg_field_id(&InstancePtr->FrmbufRd, 0);
	}
	InstancePtr->Stream = VidStrm;

	/* Setup polling mode */
	IrqMask = XVFRMBUFRD_IRQ_DONE_MASK | XVFRMBUFRD_IRQ_READY_MASK;
	XVFrmbufRd_InterruptDisable(InstancePtr, IrqMask);
}

/**
 * Enables specific interrupts for the Frame Buffer Read core and globally enables interrupts.
 *
 * This function enables the interrupts specified by the IrqMask for the Frame Buffer Read
 * instance pointed to by InstancePtr. It also globally enables interrupts for the core and
 * disables the autostart feature to ensure manual control of frame buffer operations.
 *
 * @param InstancePtr Pointer to the XV_FrmbufRd_l2 instance.
 * @param IrqMask     is the interrupt mask the driver interrupt handler
 * 	      			  passes to the callback function.
 * @return None.
 *
 * @note The InstancePtr must not be NULL.
 */
void XVFrmbufRd_InterruptEnable(XV_FrmbufRd_l2 *InstancePtr, u32 IrqMask)
{
	Xil_AssertVoid(InstancePtr != NULL);

	/* Enable Interrupts */
	XV_frmbufrd_InterruptEnable(&InstancePtr->FrmbufRd, IrqMask);
	XV_frmbufrd_InterruptGlobalEnable(&InstancePtr->FrmbufRd);

	/* Clear autostart bit */
	XV_frmbufrd_DisableAutoRestart(&InstancePtr->FrmbufRd);
}

/**
 * XVFrmbufRd_InterruptDisable - Disables specific interrupts and enables auto-restart.
 *
 * This function disables the interrupts specified by the IrqMask for the given
 * Frame Buffer Read (FrmbufRd) instance. It also globally disables interrupts
 * for the instance and sets the auto-restart bit to ensure the hardware continues
 * operation without further software intervention.
 *
 * @param  InstancePtr Pointer to the XV_FrmbufRd_l2 instance.
 * @param  IrqMask is the interrupt mask the driver interrupt handler
 * 	       passes to the callback function.
 * @return None.
 *
 * @note The InstancePtr must not be NULL.
 */
void XVFrmbufRd_InterruptDisable(XV_FrmbufRd_l2 *InstancePtr, u32 IrqMask)
{
	Xil_AssertVoid(InstancePtr != NULL);

	/* Disable Interrupts */
	XV_frmbufrd_InterruptDisable(&InstancePtr->FrmbufRd, IrqMask);
	XV_frmbufrd_InterruptGlobalDisable(&InstancePtr->FrmbufRd);

	/* Set autostart bit */
	XV_frmbufrd_EnableAutoRestart(&InstancePtr->FrmbufRd);
}

/**
 * This function starts the Frame Buffer Read core.
 *
 * This function initiates the frame buffer read operation by calling the
 * lower-level start function for the frame buffer read hardware. It asserts
 * that the provided instance pointer is not NULL before proceeding.
 *
 * @param InstancePtr Pointer to the XV_FrmbufRd_l2 instance.
 *
 * @return none
 */
void XVFrmbufRd_Start(XV_FrmbufRd_l2 *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XV_frmbufrd_Start(&InstancePtr->FrmbufRd);
}

/**
 * XVFrmbufRd_Stop - Stops the Frame Buffer Read core.
 *
 * This function disables the auto-restart feature, initiates a flush operation,
 * and waits for the flush to complete or until a timeout occurs. It returns
 * XST_SUCCESS if the flush completes successfully, or XST_FAILURE if the flush
 * does not complete within the allowed number of attempts.
 *
 * @param	InstancePtr is a pointer to the XV_FrmbufRd_l2 instance.
 *
 * @return
 *		- XST_SUCCESS if the flush operation completes successfully.
 *		- XST_FAILURE if the flush operation times out.
 *
 * @note	None.
 */
int XVFrmbufRd_Stop(XV_FrmbufRd_l2 *InstancePtr)
{
	int Status = XST_SUCCESS;
	u32 cnt = 0;
	u32 Data = 0;
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Clear autostart bit */
	XV_frmbufrd_DisableAutoRestart(&InstancePtr->FrmbufRd);

	/* Flush the core bit */
	XV_frmbufrd_SetFlushbit(&InstancePtr->FrmbufRd);

	do {
		Data = XV_frmbufrd_Get_FlushDone(&InstancePtr->FrmbufRd);
		usleep(XV_WAIT_FOR_FLUSH_DELAY);
		cnt++;
	} while((Data == 0) && (cnt < XV_WAIT_FOR_FLUSH_DONE));

	if (Data == 0)
		Status = XST_FAILURE;

	return Status;
}

/**
 * XVFrmbufRd_WaitForIdle - Waits until the Frame Buffer Read core is idle or a timeout occurs.
 *
 * @param InstancePtr: Pointer to the XV_FrmbufRd_l2 instance.
 *
 * This function polls the Frame Buffer Read core's status until it becomes idle
 * or until a predefined timeout (XVFRMBUFRD_IDLE_TIMEOUT) is reached. It uses
 * the XV_frmbufrd_IsIdle function to check the idle status. If the core becomes
 * idle within the timeout period, the function returns XST_SUCCESS. Otherwise,
 * it returns XST_FAILURE.
 *
 * @return XST_SUCCESS if the core is in idle state
 *         XST_FAILURE if the core is not in idle state
 */
int XVFrmbufRd_WaitForIdle(XV_FrmbufRd_l2 *InstancePtr)
{
	int Status = XST_FAILURE;
	u32 isIdle = 0;
	u32 cnt = 0;

	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Wait for idle */
	do {
		isIdle = XV_frmbufrd_IsIdle(&InstancePtr->FrmbufRd);
		cnt++;
	} while((isIdle!=1) && (cnt < XVFRMBUFRD_IDLE_TIMEOUT));

	if (isIdle == 1 ) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/**
 * XVFrmbufRd_SetMemFormat - Configure the memory format and stream parameters for the frame buffer reader.
 *
 * @param  InstancePtr:    Pointer to the XV_FrmbufRd_l2 instance.
 * @param  StrideInBytes:  Stride of the frame buffer in bytes. Must be aligned to 2 * PixPerClk * 4 bytes.
 * @param  MemFmt:         Memory color format (XVidC_ColorFormat) to be used for reading frames.
 * @param  StrmOut:        Pointer to the output video stream structure (XVidC_VideoStream).
 *
 * This function validates the provided memory format, stride, and stream parameters,
 * ensuring they are compatible with the hardware and the selected memory format.
 * It checks for alignment, format support, and matching between memory and streaming formats.
 * If all checks pass, it configures the hardware registers for the frame buffer reader
 * with the specified parameters.
 *
 * @return
 *   - XST_SUCCESS if the configuration is successful.
 *   - XVFRMBUFRD_ERR_FRAME_SIZE_INVALID if the frame size is invalid for the selected format.
 *   - XVFRMBUFRD_ERR_STRIDE_MISALIGNED if the stride is not properly aligned.
 *   - XVFRMBUFRD_ERR_VIDEO_FORMAT_MISMATCH if the memory and stream formats do not match.
 *   - XVFRMBUFRD_ERR_DISABLED_IN_HW if the selected memory format is not supported in hardware.
 *   - XST_FAILURE for other failures.
 */
int XVFrmbufRd_SetMemFormat(XV_FrmbufRd_l2 *InstancePtr,
		u32 StrideInBytes,
		XVidC_ColorFormat MemFmt,
		const XVidC_VideoStream *StrmOut)
{
	int Status = XST_FAILURE;
	u32 FmtValid = FALSE;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(StrmOut != NULL);

	/* copy stream data */
	InstancePtr->Stream = *StrmOut;
	/* Width must be multiple of Samples per Clock */
	Xil_AssertNonvoid((StrmOut->Timing.HActive  % \
				InstancePtr->FrmbufRd.Config.PixPerClk) == 0);

	/* For 4:2:2 and 4:2:0, columns must be in pairs */
	if (((RdMemory2Live(MemFmt) == XVIDC_CSF_YCRCB_422) || \
				(RdMemory2Live(MemFmt) == XVIDC_CSF_YCRCB_420)) && \
			((StrmOut->Timing.HActive  % 2) != 0)) {
		Status = XVFRMBUFRD_ERR_FRAME_SIZE_INVALID;

		/* For 4:2:0, rows must be in pairs */
	} else if ((RdMemory2Live(MemFmt) == XVIDC_CSF_YCRCB_420) && \
			((StrmOut->Timing.VActive  % 2) != 0)) {
		Status = XVFRMBUFRD_ERR_FRAME_SIZE_INVALID;

		/* Check if stride is aligned to aximm width (2*PPC*32-bits) */
	} else if ((StrideInBytes % (2 * InstancePtr->FrmbufRd.Config.PixPerClk * 4)) != 0) {
		Status   = XVFRMBUFRD_ERR_STRIDE_MISALIGNED;

		/* Streaming Video Format must match Memory Video Format */
	} else if (StrmOut->ColorFormatId != RdMemory2Live(MemFmt)) {
		Status = XVFRMBUFRD_ERR_VIDEO_FORMAT_MISMATCH;

		/* Memory Video Format must be supported in hardware */
	} else {
		Status = XVFRMBUFRD_ERR_DISABLED_IN_HW;
		FmtValid = FALSE;
		switch(MemFmt) {
			case XVIDC_CSF_MEM_RGBX8 :
				if (XVFrmbufRd_IsRGBX8Enabled(InstancePtr)) {
                  FmtValid = TRUE;
                }
                break;
			case XVIDC_CSF_MEM_YUVX8 :
				if (XVFrmbufRd_IsYUVX8Enabled(InstancePtr)) {
					FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_YUYV8 :
				if (XVFrmbufRd_IsYUYV8Enabled(InstancePtr)) {
					FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_RGBA8 :
				if (XVFrmbufRd_IsRGBA8Enabled(InstancePtr)) {
					FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_YUVA8 :
				if (XVFrmbufRd_IsYUVA8Enabled(InstancePtr)) {
					FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_RGBX10 :
				if (XVFrmbufRd_IsRGBX10Enabled(InstancePtr)) {
					FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_YUVX10 :
				if (XVFrmbufRd_IsYUVX10Enabled(InstancePtr)) {
					FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_Y_UV8 :
				if (XVFrmbufRd_IsY_UV8Enabled(InstancePtr)) {
					FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_Y_UV8_420 :
				if (XVFrmbufRd_IsY_UV8_420Enabled(InstancePtr)) {
					FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_RGB8 :
				if (XVFrmbufRd_IsRGB8Enabled(InstancePtr)) {
					FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_YUV8 :
				if (XVFrmbufRd_IsYUV8Enabled(InstancePtr)) {
					FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_Y_UV10 :
				if (XVFrmbufRd_IsY_UV10Enabled(InstancePtr)) {
					FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_Y_UV10_420 :
				if (XVFrmbufRd_IsY_UV10_420Enabled(InstancePtr)) {
					FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_Y8 :
				if (XVFrmbufRd_IsY8Enabled(InstancePtr)) {
					FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_Y10 :
				if (XVFrmbufRd_IsY10Enabled(InstancePtr)) {
					FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_BGRA8 :
				if (XVFrmbufRd_IsBGRA8Enabled(InstancePtr)) {
					FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_BGRX8 :
				if (XVFrmbufRd_IsBGRX8Enabled(InstancePtr)) {
					FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_UYVY8 :
				if (XVFrmbufRd_IsUYVY8Enabled(InstancePtr)) {
					FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_BGR8 :
				if (XVFrmbufRd_IsBGR8Enabled(InstancePtr)) {
					FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_RGBX12 :
				if (XVFrmbufRd_IsRGBX12Enabled(InstancePtr)) {
					FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_RGB16 :
				if (XVFrmbufRd_IsRGB16Enabled(InstancePtr)) {
					FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_YUVX12 :
				if (XVFrmbufRd_IsYUVX12Enabled(InstancePtr)) {
					FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_YUV16 :
				if (XVFrmbufRd_IsYUV16Enabled(InstancePtr)) {
					FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_Y_UV12 :
				if (XVFrmbufRd_IsY_UV12Enabled(InstancePtr)) {
					FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_Y_UV16 :
				if (XVFrmbufRd_IsY_UV16Enabled(InstancePtr)) {
					FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_Y_UV12_420 :
				if (XVFrmbufRd_IsY_UV12_420Enabled(InstancePtr)) {
					FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_Y_UV16_420 :
				if (XVFrmbufRd_IsY_UV16_420Enabled(InstancePtr)) {
					FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_Y12 :
				if (XVFrmbufRd_IsY12Enabled(InstancePtr)) {
					FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_Y16 :
				if (XVFrmbufRd_IsY16Enabled(InstancePtr)) {
					FmtValid = TRUE;
				}
				break;
            case XVIDC_CSF_MEM_Y_U_V8 :
                if (XVFrmbufRd_IsY_U_V8Enabled(InstancePtr)) {
				   FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_Y_U_V10 :
                if (XVFrmbufRd_IsY_U_V10Enabled(InstancePtr)) {
				   FmtValid = TRUE;
				}
				break;
            case XVIDC_CSF_MEM_Y_U_V8_420 :
                if (XVFrmbufRd_IsY_U_V8_420Enabled(InstancePtr)) {
				   FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_Y_U_V12 :
                if (XVFrmbufRd_IsY_U_V12Enabled(InstancePtr)) {
				   FmtValid = TRUE;
				}
				break;
			default :
				FmtValid = FALSE;
				break;
		}

		if (FmtValid == TRUE) {
			/* configure frame buffer read */
			XV_frmbufrd_Set_HwReg_width(&InstancePtr->FrmbufRd,
					StrmOut->Timing.HActive);
			XV_frmbufrd_Set_HwReg_height(&InstancePtr->FrmbufRd,
					StrmOut->Timing.VActive);
			XV_frmbufrd_Set_HwReg_stride(&InstancePtr->FrmbufRd, StrideInBytes);
			XV_frmbufrd_Set_HwReg_video_format(&InstancePtr->FrmbufRd, MemFmt);
			Status = XST_SUCCESS;
		}
	}

	return(Status);
}

/**
 * Retrieves a pointer to the video stream associated with the given Frame Buffer Read instance.
 *
 * @param	InstancePtr	Pointer to the XV_FrmbufRd_l2 instance.
 *
 * @return	Pointer to the XVidC_VideoStream structure associated with the instance.
 */
XVidC_VideoStream *XVFrmbufRd_GetVideoStream(XV_FrmbufRd_l2 *InstancePtr)
{
	return(&InstancePtr->Stream);
}

/**
 * XVFrmbufRd_SetBufferAddr - Set the frame buffer address for the Frame Buffer Read core.
 *
 * @param  InstancePtr: Pointer to the XV_FrmbufRd_l2 instance.
 * @param  Addr:        Physical address of the frame buffer to be set.
 *
 * This function sets the frame buffer address for the Frame Buffer Read core.
 * It checks if the provided address is aligned to the required AXI memory width,
 * which is calculated as 2 * PixelsPerClock * 4 bytes. If the address is not
 * properly aligned, the function returns an error code indicating misalignment.
 * If the address is valid and aligned, it writes the address to the hardware register.
 *
 * @return XST_SUCCESS if the address is set successfully.
 *         XVFRMBUFRD_ERR_MEM_ADDR_MISALIGNED if the address is misaligned.
 *         XST_FAILURE for other failures.
 */
int XVFrmbufRd_SetBufferAddr(XV_FrmbufRd_l2 *InstancePtr,
		UINTPTR Addr)
{
	UINTPTR Align;
	u32 AddrValid = FALSE;
	int Status = XST_FAILURE;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Addr != 0);

	/* Check if addr is aligned to aximm width (2*PPC*32-bits (4Bytes)) */
	Align = 2 * InstancePtr->FrmbufRd.Config.PixPerClk * 4;
	if ((Addr % Align) != 0) {
		AddrValid = FALSE;
		Status   = XVFRMBUFRD_ERR_MEM_ADDR_MISALIGNED;
	} else {
		AddrValid = TRUE;
	}

	if (AddrValid) {
		XV_frmbufrd_Set_HwReg_frm_buffer_V(&InstancePtr->FrmbufRd, Addr);
		Status = XST_SUCCESS;
	}
	return(Status);
}

/*****************************************************************************/
/**
 * This function reads the buffer address
 *
 * @param  InstancePtr is a pointer to core instance to be worked upon
 *
 * @return Address of buffer in memory
 *
 ******************************************************************************/
UINTPTR XVFrmbufRd_GetBufferAddr(XV_FrmbufRd_l2 *InstancePtr)
{
	UINTPTR ReadVal = 0;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ReadVal = XV_frmbufrd_Get_HwReg_frm_buffer_V(&InstancePtr->FrmbufRd);
	return(ReadVal);
}

/*****************************************************************************/
/**
* This function sets the buffer address for the V plane for 3 planar formats
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  Addr is the absolute address of buffer in memory
*
* @return XST_SUCCESS or XST_FAILURE
*
******************************************************************************/
int XVFrmbufRd_SetVChromaBufferAddr(XV_FrmbufRd_l2 *InstancePtr,
		UINTPTR Addr)
{
	UINTPTR Align;
	u32 AddrValid = FALSE;
	int Status = XST_FAILURE;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Addr != 0);

	/* Check if addr is aligned to aximm width (2*PPC*32-bits (4Bytes)) */
	Align = 2 * InstancePtr->FrmbufRd.Config.PixPerClk * 4;
	if ((Addr % Align) != 0) {
		AddrValid = FALSE;
		Status = XVFRMBUFRD_ERR_MEM_ADDR_MISALIGNED;
	} else {
		AddrValid = TRUE;
	}
	if (AddrValid) {
		XV_frmbufrd_Set_HwReg_frm_buffer3_V(&InstancePtr->FrmbufRd, Addr);
		Status = XST_SUCCESS;
	}
	return(Status);
}

/*****************************************************************************/
/**
* This function reads the buffer address for the V plane for 3 planar formats
*
* @param  InstancePtr is a pointer to core instance to be worked upon
*
* @return Address of buffer in memory
*
******************************************************************************/
UINTPTR XVFrmbufRd_GetVChromaBufferAddr(XV_FrmbufRd_l2 *InstancePtr)
{
	UINTPTR ReadVal = 0;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ReadVal = XV_frmbufrd_Get_HwReg_frm_buffer3_V(&InstancePtr->FrmbufRd);
	return(ReadVal);
}

/*****************************************************************************/
/**
* This function sets the buffer address for the UV plane for semi-planar formats
* or Only U Plane for 3 planar formats
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  Addr is the absolute address of buffer in memory
*
* @return XST_SUCCESS or XST_FAILURE
*
******************************************************************************/
int XVFrmbufRd_SetChromaBufferAddr(XV_FrmbufRd_l2 *InstancePtr,
		UINTPTR Addr)
{
	UINTPTR Align;
	u32 AddrValid = FALSE;
	int Status = XST_FAILURE;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Addr != 0);

	/* Check if addr is aligned to aximm width (2*PPC*32-bits (4Bytes)) */
	Align = 2 * InstancePtr->FrmbufRd.Config.PixPerClk * 4;
	if ((Addr % Align) != 0) {
		AddrValid = FALSE;
		Status = XVFRMBUFRD_ERR_MEM_ADDR_MISALIGNED;
	} else {
		AddrValid = TRUE;
	}

	if (AddrValid) {
		XV_frmbufrd_Set_HwReg_frm_buffer2_V(&InstancePtr->FrmbufRd, Addr);
		Status = XST_SUCCESS;
	}
	return(Status);
}

/*****************************************************************************/
/**
 * This function reads the buffer address for the UV plane for semi-planar formats
 * or Only U plane for 3 planar formats
 *
 * @param  InstancePtr is a pointer to core instance to be worked upon
 *
 * @return Address of buffer in memory
 *
 ******************************************************************************/
UINTPTR XVFrmbufRd_GetChromaBufferAddr(XV_FrmbufRd_l2 *InstancePtr)
{
	UINTPTR ReadVal = 0;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ReadVal = XV_frmbufrd_Get_HwReg_frm_buffer2_V(&InstancePtr->FrmbufRd);
	return(ReadVal);
}

/**
 * XVFrmbufRd_SetFieldID - Sets the Field ID for the Frame Buffer Read hardware.
 *
 * @param InstancePtr Pointer to the XV_FrmbufRd_l2 instance.
 * @param FieldID     Field ID value to set (typically 0 or 1 for interlaced operation).
 *
 * This function writes the specified Field ID value to the hardware register
 * controlling the field selection for interlaced video modes. It asserts that
 * the instance pointer is valid and that the hardware is configured for interlaced mode.
 *
 * @return XST_SUCCESS if the Field ID is set successfully, otherwise XST_FAILURE.
 */
int XVFrmbufRd_SetFieldID(XV_FrmbufRd_l2 *InstancePtr,
		u32 FieldID)
{
	int Status = XST_FAILURE;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->FrmbufRd.Config.Interlaced);

	XV_frmbufrd_Set_HwReg_field_id(&InstancePtr->FrmbufRd, FieldID);
	Status = XST_SUCCESS;

	return(Status);
}

/**
 * XVFrmbufRd_GetFieldID - Retrieves the current Field ID from the Frame Buffer Read hardware.
 *
 * @param	InstancePtr: Pointer to the XV_FrmbufRd_l2 instance.
 *
 * @return	The current Field ID value as read from the hardware register.
 *
 * @note	This function asserts that the InstancePtr is not NULL and that
 *          the Frame Buffer Read instance is configured for interlaced mode.
 */
u32 XVFrmbufRd_GetFieldID(XV_FrmbufRd_l2 *InstancePtr)
{
	u32 ReadVal;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->FrmbufRd.Config.Interlaced);

	ReadVal = XV_frmbufrd_Get_HwReg_field_id(&InstancePtr->FrmbufRd);
	return(ReadVal);
}

/**
 * XVFrmbufRd_Set_FidOutMode - Sets the Field ID Output Mode for the Frame Buffer Read core.
 *
 * @param  InstancePtr Pointer to the XV_FrmbufRd_l2 instance.
 * @param  fidmodeval  Field ID output mode value (must be 0, 1, or 2).
 *
 * This function configures the Field ID Output Mode register of the Frame Buffer Read
 * hardware. It asserts that the instance pointer is valid, the fidmodeval is within
 * the allowed range, and that the core is configured for interlaced operation.
 *
 * @return XST_SUCCESS if the operation is successful, otherwise XST_FAILURE.
 */
int XVFrmbufRd_Set_FidOutMode(XV_FrmbufRd_l2 *InstancePtr, u32 fidmodeval)
{
	int Status = XST_FAILURE;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(fidmodeval <= 2);
	Xil_AssertNonvoid(InstancePtr->FrmbufRd.Config.Interlaced);

	XV_frmbufrd_Set_HwReg_fidOutMode(&InstancePtr->FrmbufRd, \
		fidmodeval & XV_FRMBUFRD_CTRL_BITS_HWREG_FIDOUTMODE_DATA);
	Status = XST_SUCCESS;

	return(Status);
}

/**
 * Retrieves the Field ID Output Mode (FidOutMode) from the Frame Buffer Read core instance.
 *
 * This function reads the current FidOutMode hardware register value from the specified
 * XV_FrmbufRd_l2 instance. It asserts that the instance pointer is not NULL and that the
 * instance is configured for interlaced mode before accessing the register.
 *
 * @param	InstancePtr	Pointer to the XV_FrmbufRd_l2 instance.
 *
 * @return	The value of the FidOutMode hardware register.
 */
u32 XVFrmbufRd_Get_FidOutMode(XV_FrmbufRd_l2 *InstancePtr)
{
	u32 ReadVal;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->FrmbufRd.Config.Interlaced);

	ReadVal = XV_frmbufrd_Get_HwReg_fidOutMode(&InstancePtr->FrmbufRd) & \
				XV_FRMBUFRD_CTRL_BITS_HWREG_FIDOUTMODE_DATA;
	return(ReadVal);
}

/**
 * XVFrmbufRd_Get_FidErrorOut - Retrieves the Field ID (FID) error status from the Frame Buffer Read core.
 *
 * @param	InstancePtr: Pointer to the XV_FrmbufRd_l2 instance.
 *
 * @return	The value of the FID error output register. Non-zero indicates an FID error.
 *
 * @note	This function asserts that the InstancePtr is not NULL and that the
 *          Frame Buffer Read core is configured for interlaced mode.
 */
u32 XVFrmbufRd_Get_FidErrorOut(XV_FrmbufRd_l2 *InstancePtr)
{
	u32 ReadVal;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->FrmbufRd.Config.Interlaced);

	ReadVal = XV_frmbufrd_Get_HwReg_fid_error(&InstancePtr->FrmbufRd) & \
				XV_FRMBUFRD_CTRL_BITS_HWREG_FID_ERROR_DATA;
	return(ReadVal);
}

/**
 * XVFrmbufRd_Get_FidErrorCount - Retrieves the Field ID (FID) error count from the Frame Buffer Read core.
 *
 * @param	InstancePtr: Pointer to the XV_FrmbufRd_l2 instance.
 *
 * @return	The number of FID errors detected by the hardware.
 *
 * @note	This function asserts that the InstancePtr is not NULL and that the
 *          Frame Buffer Read core is configured for interlaced mode.
 */
u32 XVFrmbufRd_Get_FidErrorCount(XV_FrmbufRd_l2 *InstancePtr)
{
	u32 ReadVal;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->FrmbufRd.Config.Interlaced);

	ReadVal = XV_frmbufrd_Get_HwReg_fid_error(&InstancePtr->FrmbufRd) >> \
				XV_FRMBUFRD_CTRL_BITS_HWREG_FID_ERROR_DATA;
	return(ReadVal);
}

/**
 * Retrieves the current field output value from the Frame Buffer Read hardware.
 *
 * This function reads the hardware register associated with the field output
 * and returns its value, masked with the appropriate control bits. It asserts
 * that the provided instance pointer is not NULL and that the instance is
 * configured for interlaced mode.
 *
 * @param	InstancePtr is a pointer to the XV_FrmbufRd_l2 instance.
 *
 * @return	The value of the field output as a 32-bit unsigned integer.
 *
 */
u32 XVFrmbufRd_Get_FieldOut(XV_FrmbufRd_l2 *InstancePtr)
{
	u32 ReadVal;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->FrmbufRd.Config.Interlaced);

	ReadVal = XV_frmbufrd_Get_HwReg_field_out(&InstancePtr->FrmbufRd) & \
				XV_FRMBUFRD_CTRL_BITS_HWREG_FIELD_OUT_DATA;
	return(ReadVal);
}

/**
 * XVFrmbufRd_DbgReportStatus - Prints the current status and configuration of the Frame Buffer Read core.
 *
 * @param  InstancePtr Pointer to the XV_FrmbufRd_l2 instance.
 *
 * This function outputs a detailed debug report of the Frame Buffer Read (FrmbufRd) core's
 * configuration and runtime status to the console using xil_printf. The report includes
 * information such as pixel format support, color depth, AXI-MM data width, enabled formats,
 * control register value, frame dimensions, stride, video format, and buffer addresses.
 *
 * This function is intended for debugging and diagnostic purposes.
 *
 * Preconditions:
 * - InstancePtr must not be NULL.
 *
 * @return none
 */
void XVFrmbufRd_DbgReportStatus(XV_FrmbufRd_l2 *InstancePtr)
{
	u32 ctrl;

	Xil_AssertVoid(InstancePtr != NULL);

	xil_printf("\r\n\r\n----->FRAME BUFFER READ STATUS<----\r\n");

	ctrl  = XV_frmbufrd_ReadReg(InstancePtr->FrmbufRd.Config.BaseAddress,
			XV_FRMBUFRD_CTRL_ADDR_AP_CTRL);

	xil_printf("Pixels Per Clock:           %d\r\n", InstancePtr->FrmbufRd.Config.PixPerClk);
	xil_printf("Color Depth:                %d\r\n", InstancePtr->FrmbufRd.Config.MaxDataWidth);
	xil_printf("AXI-MM Data Width:          %d\r\n", InstancePtr->FrmbufRd.Config.AXIMMDataWidth);
	xil_printf("RGBX8 Enabled:              %d\r\n", InstancePtr->FrmbufRd.Config.RGBX8En);
	xil_printf("YUVX8 Enabled:              %d\r\n", InstancePtr->FrmbufRd.Config.YUVX8En);
	xil_printf("YUYV8 Enabled:              %d\r\n", InstancePtr->FrmbufRd.Config.YUYV8En);
	xil_printf("RGBA8 Enabled:              %d\r\n", InstancePtr->FrmbufRd.Config.RGBA8En);
	xil_printf("YUVA8 Enabled:              %d\r\n", InstancePtr->FrmbufRd.Config.YUVA8En);
	xil_printf("RGBX10 Enabled:             %d\r\n", InstancePtr->FrmbufRd.Config.RGBX10En);
	xil_printf("YUVX10 Enabled:             %d\r\n", InstancePtr->FrmbufRd.Config.YUVX10En);
	xil_printf("Y_UV8 Enabled:              %d\r\n", InstancePtr->FrmbufRd.Config.Y_UV8En);
	xil_printf("Y_UV8_420 Enabled:          %d\r\n", InstancePtr->FrmbufRd.Config.Y_UV8_420En);
	xil_printf("RGB8 Enabled:               %d\r\n", InstancePtr->FrmbufRd.Config.RGB8En);
	xil_printf("YUV8 Enabled:               %d\r\n", InstancePtr->FrmbufRd.Config.YUV8En);
	xil_printf("Y_UV10 Enabled:             %d\r\n", InstancePtr->FrmbufRd.Config.Y_UV10En);
	xil_printf("Y_UV10_420 Enabled:         %d\r\n", InstancePtr->FrmbufRd.Config.Y_UV10_420En);
	xil_printf("Y8 Enabled:                 %d\r\n", InstancePtr->FrmbufRd.Config.Y8En);
	xil_printf("Y10 Enabled:                %d\r\n", InstancePtr->FrmbufRd.Config.Y10En);
	xil_printf("BGRA8 Enabled:              %d\r\n", InstancePtr->FrmbufRd.Config.BGRA8En);
	xil_printf("BGRX8 Enabled:              %d\r\n", InstancePtr->FrmbufRd.Config.BGRX8En);
	xil_printf("BGR8 Enabled:               %d\r\n", InstancePtr->FrmbufRd.Config.BGR8En);
	xil_printf("UYVY8 Enabled:              %d\r\n", InstancePtr->FrmbufRd.Config.UYVY8En);
	xil_printf("RGBX12 Enabled:             %d\r\n", InstancePtr->FrmbufRd.Config.RGBX12En);
	xil_printf("RGB16 Enabled:              %d\r\n", InstancePtr->FrmbufRd.Config.RGB16En);
	xil_printf("YUVX12 Enabled:             %d\r\n", InstancePtr->FrmbufRd.Config.YUVX12En);
	xil_printf("YUV16 Enabled:              %d\r\n", InstancePtr->FrmbufRd.Config.YUV16En);
	xil_printf("Y_UV12 Enabled:             %d\r\n", InstancePtr->FrmbufRd.Config.Y_UV12En);
	xil_printf("Y_UV16 Enabled:             %d\r\n", InstancePtr->FrmbufRd.Config.Y_UV16En);
	xil_printf("Y_UV12_420 Enabled:         %d\r\n", InstancePtr->FrmbufRd.Config.Y_UV12_420En);
	xil_printf("Y_UV16_420 Enabled:         %d\r\n", InstancePtr->FrmbufRd.Config.Y_UV16_420En);
	xil_printf("Y12 Enabled:                %d\r\n", InstancePtr->FrmbufRd.Config.Y12En);
	xil_printf("Y16 Enabled:                %d\r\n", InstancePtr->FrmbufRd.Config.Y16En);
	xil_printf("Y_U_V8 Enabled:             %d\r\n", InstancePtr->FrmbufRd.Config.Y_U_V8En);
	xil_printf("Y_U_V10 Enabled:            %d\r\n", InstancePtr->FrmbufRd.Config.Y_U_V10En);
	xil_printf("Y_U_V8_420 Enabled:         %d\r\n", InstancePtr->FrmbufRd.Config.Y_U_V8_420En);
	xil_printf("Y_U_V12 Enabled:            %d\r\n", InstancePtr->FrmbufRd.Config.Y_U_V12En);
	xil_printf("Interlaced Enabled:         %d\r\n", InstancePtr->FrmbufRd.Config.Interlaced);
	xil_printf("Tile format Enabled:        %d\r\n", InstancePtr->FrmbufRd.Config.IsTileFormat);

	xil_printf("Control Reg:                0x%x\r\n", ctrl);
	xil_printf("Width:                      %d\r\n",   XV_frmbufrd_Get_HwReg_width(&InstancePtr->FrmbufRd));
	xil_printf("Height:                     %d\r\n",   XV_frmbufrd_Get_HwReg_height(&InstancePtr->FrmbufRd));
	xil_printf("Stride (in bytes):          %d\r\n",   XV_frmbufrd_Get_HwReg_stride(&InstancePtr->FrmbufRd));
	xil_printf("Video Format:               %d\r\n",   XV_frmbufrd_Get_HwReg_video_format(&InstancePtr->FrmbufRd));
	xil_printf("Buffer Address:             0x%x\r\n", XV_frmbufrd_Get_HwReg_frm_buffer_V(&InstancePtr->FrmbufRd));
	xil_printf("Chroma Buffer Address:      0x%x\r\n", XV_frmbufrd_Get_HwReg_frm_buffer2_V(&InstancePtr->FrmbufRd));
	xil_printf("VChroma Buffer Address:     0x%x\r\n", XV_frmbufrd_Get_HwReg_frm_buffer3_V(&InstancePtr->FrmbufRd));
}

/** @} */
