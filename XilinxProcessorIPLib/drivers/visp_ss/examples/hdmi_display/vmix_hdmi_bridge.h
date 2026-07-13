/******************************************************************************
 * @file vmix_hdmi_bridge.h
 * @brief VMix + HDMI TX Bridge for ISP Pipeline Output
 *
 * This module bridges the ISP (VISP SS) video output to an HDMI TX display
 * through a Video Mixer (VMix) IP. It provides:
 *   - VMix initialization, layer management, and stream configuration
 *   - HDMI TX 2.1 Subsystem initialization and stream startup
 *   - Per-frame buffer address update for ISP DMA and FBWR paths
 *   - Layer mapping between ISP instances/paths and VMix layers
 *
 * Data flow:
 *   Sensor -> ISP -> FBWR (DDR triple-buffer) -> VMix Layer -> HDMI TX
 *                 -> ISP DMA (DDR buffer)      -> VMix Layer -^
 *
 * This module uses the latest HDMI 2.1 SS APIs (XV_HdmiTxSs1, XHdmiphy1)
 * and the VMix L2 driver (XV_Mix_l2 / xv_mix_l2.h).
 *
 * Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 ******************************************************************************/

#ifndef VMIX_HDMI_BRIDGE_H
#define VMIX_HDMI_BRIDGE_H

#ifdef __cplusplus
extern "C" {
#endif

/************************** Include Files ************************************/
#include "xil_types.h"
#include "xvidc.h"
#include "cam_device_common.h"
#include "cam_device_buffer_api.h"

/*
 * Conditional includes for VMix and HDMI subsystems.
 * These are guarded by XPAR defines so the code compiles even when
 * the HW design doesn't include these IPs.
 */
#ifdef XPAR_XV_MIX_NUM_INSTANCES
#include "xv_mix_l2.h"
#endif

/*
 * HDMI TX 2.1 SS types (XV_HdmiTxSs1, XHdmiphy1, XV_Tx, XHdmi_Exdes)
 * are only used internally in vmix_hdmi_bridge.c and are included there.
 */

/************************** Constant Definitions *****************************/

/** Maximum number of ISP instances that can be mapped to VMix layers */
#define VMIX_BRIDGE_MAX_INSTANCES    6

/** Maximum number of output paths per ISP instance (MP, SP1, SP2, RDMA) */
#define VMIX_BRIDGE_MAX_OUTPUT_PATHS 4

/** Maximum number of VMix layers (fallback if HW define not available) */
#ifndef XPAR_XV_MIX_NUM_INSTANCES
#define VMIX_BRIDGE_MAX_LAYERS       17
#else
#define VMIX_BRIDGE_MAX_LAYERS       17 /* updated at runtime from HW config */
#endif

/** Number of supported display resolutions */
#define VMIX_BRIDGE_NUM_RESOLUTIONS  18

/** VMix auto-start mode (no interrupt, free-running) */
#define VMIX_AUTOSTART

/************************** Type Definitions *********************************/

/**
 * @brief Simple structure for passing buffer addresses to VMix layer update
 */
typedef struct VmixHdmiBridge_InputBuffer {
	u64 baseAddress;
} VmixHdmiBridge_InputBuffer;

/**
 * @brief Per-layer tracking structure
 */
typedef struct {
	u8  LayerID;             /**< VMix layer index */
	u8  LayerCfmt;           /**< XVidC_ColorFormat for this layer */
	u8  IsEnabled;           /**< Layer currently enabled in VMix HW */
	u8  IsMapped;            /**< Layer mapped to an ISP instance+path */
	s16 MappedInstanceId;    /**< ISP instance ID mapped to this layer (-1 = none) */
	u32 Stride;              /**< Memory stride in bytes for this layer */
} VmixHdmiBridge_LayerPool;

/**
 * @brief Supported display resolution entry
 */
typedef struct {
	u32 Width;
	u32 Height;
} VmixHdmiBridge_Resolution;

/**
 * @brief Display coordinate for multi-view layout
 */
typedef struct {
	int StartX;
	int StartY;
} VmixHdmiBridge_Coordinate;

/************************** Function Prototypes ******************************/

/**
 * @brief Initialize the VMix + HDMI TX bridge
 *
 * This function performs the complete initialization:
 *  1. Initializes the VMix IP (XV_Mix_l2)
 *  2. Initializes the HDMI TX 2.1 SS (XV_HdmiTxSs1) + HDMI PHY
 *  3. Configures VMix master stream for the target display resolution
 *  4. Starts the VMix core
 *  5. Configures and starts HDMI TX output
 *
 * @return 0 on success, negative on failure
 */
int VmixHdmiBridge_Init(void);

/**
 * @brief Configure the HDMI TX subsystem and start output
 *
 * Sets up HDMI TX 2.1 SS with the specified video mode and starts
 * the transmitter. Uses the latest XV_HdmiTxSs1 / XV_Tx APIs.
 *
 * @param VideoMode - Target video mode (e.g., XVIDC_VM_3840x2160_30_P)
 * @return XST_SUCCESS on success
 */
int VmixHdmiBridge_ConfigHdmiTx(XVidC_VideoMode VideoMode);

/**
 * @brief Map a VMix layer to a specific ISP instance + output path
 *
 * Finds a free VMix layer matching the required color format and
 * assigns it to the given ISP instance/path combination.
 *
 * @param outFormat  - ISP output format (resolution, pixel format, bit depth)
 * @param InstanceID - ISP instance ID (0-5)
 * @param hpId       - Hardware pipeline ID
 * @param bufIo      - Buffer chain ID (MP/SP1)
 * @param dataBits   - Output data bit depth (8 or 10)
 * @param outputType - 0=MO (memory output), 1=LO (live output via FBWR)
 * @return 1 on success, 0 if layer already mapped, -1 on failure
 */
int VmixHdmiBridge_MapLayer(CamDevicePipeOutFmt_t outFormat, u8 InstanceID,
			    u8 hpId, CamDeviceBufChainId_t bufIo,
			    int dataBits, int outputType);

/**
 * @brief Update VMix layer buffer address with latest ISP output frame
 *
 * Called per-frame from VsiVvdeviceShowBuffer() to push a new DMA buffer
 * address to the VMix layer assigned to this ISP instance/path.
 *
 * @param baseAddress - Physical address of the ISP output buffer
 * @param Vmix_format - Format/resolution of the output buffer
 * @param InstanceID  - ISP instance ID
 * @param bufferIO    - Buffer chain ID (MP/SP1)
 * @return 1 on success, 0 if unsupported resolution, -1 on failure
 */
int VmixHdmiBridge_UpdateBufferAddr(u64 baseAddress,
				    CamDevicePipeOutFmt_t Vmix_format,
				    u8 InstanceID,
				    CamDeviceBufChainId_t bufferIO);

/**
 * @brief Update VMix layer from FBWR callback (live output path)
 *
 * Called from FBWR done interrupt callbacks to update the VMix layer
 * with the latest frame buffer writer read-pointer address.
 *
 * @param fbwr_id     - Frame buffer writer instance ID (0-3)
 * @param readAddr    - Physical address of the latest completed frame
 * @param chromaAddr  - Physical address of chroma plane (0 for packed formats)
 */
#ifdef XPAR_XV_FRMBUF_WR_NUM_INSTANCES
void VmixHdmiBridge_UpdateFbwrLayer(int fbwr_id, u64 readAddr, u64 chromaAddr);
#endif

/**
 * @brief Clean up all VMix layers and reset mapping tables
 *
 * Disables all mapped VMix layers, resets layer tracking structures,
 * and clears instance-to-layer mapping tables.
 */
void VmixHdmiBridge_Cleanup(void);

/**
 * @brief Configure VMix master video stream and start the mixer
 *
 * Sets the VMix master stream resolution/format, enumerates all
 * layer color formats into the layer pool, and starts the mixer.
 *
 * @param StreamPtr - Video stream parameters for the master output
 * @return 0 on success
 */
int VmixHdmiBridge_ConfigureMixer(XVidC_VideoStream *StreamPtr);

/**
 * @brief Get the FBWR-to-VMix-layer mapping for a specific FBWR
 *
 * @param fbwr_id - Frame buffer writer ID (0-3)
 * @return VMix layer ID mapped to this FBWR, or -1 if not mapped
 */
#ifdef XPAR_XV_FRMBUF_WR_NUM_INSTANCES
int VmixHdmiBridge_GetFbwrLayerId(int fbwr_id);
#endif

/**
 * @brief Poll HDMI TX state machine in the main loop
 *
 * Must be called periodically from the main while loop to process
 * the TX state machine events and start the transmitter when ready.
 */
void VmixHdmiBridge_HdmiPoll(void);

#ifdef __cplusplus
}
#endif

#endif /* VMIX_HDMI_BRIDGE_H */
