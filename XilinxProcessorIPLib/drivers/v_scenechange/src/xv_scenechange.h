// ==============================================================
// Copyright (c) 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_scenechange.h
 * @addtogroup v_scenechange Overview
 */

#ifndef XV_SCENECHANGE_H
#define XV_SCENECHANGE_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#ifndef __linux__
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_io.h"
#else
#include <stdint.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>
#endif
#include "xv_scenechange_hw.h"
#include "sleep.h"

/************************** Constant Definitions *****************************/

/**
 * Layer offset for scene change detection IP
 */
#define XV_SCD_LAYER_OFFSET		(0x100)

/**
 * Maximum number of streams supported by scene change IP
 */
#define XV_SCD_IP_MAX_STREAMS		8

/**
 * Wait count for flush done operation
 */
#define XV_SCD_WAIT_FOR_FLUSH_DONE	(25)

/**
 * Delay in microseconds for flush operation
 */
#define XV_SCD_WAIT_FOR_FLUSH_DELAY	(2000)

/**
 * Idle timeout in microseconds for scene change IP
 */
#define XV_SCD_IDLE_TIMEOUT		(1000000)

/**
 * Memory based mode for scene change detection
 */
#define XV_SCD_MEMORY_MODE		1

/**
 * Stream mode for scene change detection
 */
#define XV_SCD_STREAM_MODE		0

/**************************** Type Definitions ******************************/

/* Please check Xilinx SceneChange PG*/

/**
 * @typedef XVScdClrFmt
 * @brief Enumeration for video color format support in scene change detection.
 *
 * Defines the supported color formats for the scene change detection IP core.
 * These values correspond to the hardware register bits that indicate format capabilities.
 */
typedef enum {
    XV_SCD_HAS_Y8  = 24,  /**< Y8 (8-bit grayscale) format support enabled */
    XV_SCD_HAS_Y10 = 25   /**< Y10 (10-bit grayscale) format support enabled */
} XVScdClrFmt;

#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#else
/**
 * @struct XV_scenechange_Config
 * @brief Configuration structure for the Scene Change Detection IP core.
 *
 * This structure contains the configuration parameters for the Xilinx Scene Change
 * Detection video processing IP. It includes device identification, memory addressing,
 * format support settings, and resource constraints.
 *
 * @note The structure uses conditional compilation (SDT - Scoped Device Tree) to support
 *       both legacy and device tree based configurations.
 */
typedef struct {
#ifndef SDT
    u16 DeviceId;		/**< Unique ID of device */
#else
    char *Name;			/**< Unique Name of device */
#endif
    UINTPTR Ctrl_BaseAddress;	/**< Base address of the control registers */
    u8	MemoryBased;		/**< Memory-based mode flag (1=memory, 0=stream) */
    u8	NumStreams;		/**< Number of streams supported by the IP */
    u32	HistogramBits;		/**< Number of histogram bits */
    u8  EnableY8;		/**< Y8 format support enabled flag */
    u8  EnableY10;		/**< Y10 format support enabled flag */
    u32 Cols;			/**< Maximum number of columns supported */
    u32	Rows;			/**< Maximum number of rows supported */
#ifdef SDT
    u16 IntrId; 		    /**< Interrupt ID */
    UINTPTR IntrParent; 	    /**< Bit[0] Interrupt parent type Bit[64/32:1] Parent base address */
#endif
} XV_scenechange_Config;
#endif



/**
 * @typedef XVSceneChange_Callback
 * @brief Callback function pointer type for scene change detection events.
 *
 * This typedef defines the signature for callback functions that are invoked
 * when a scene change is detected by the XVSceneChange driver.
 *
 * @param InstancePtr Pointer to the XVSceneChange instance that triggered the callback.
 *                    This allows the callback handler to identify which scene change
 *                    detector instance generated the event.
 *
 * @return void
 *
 * @note The callback function should not perform time-consuming operations as it
 *       may be called from an interrupt context.
 */
typedef void (*XVSceneChange_Callback)(void *InstancePtr);

/*Scene Change Detection Layer Configuration Structure*/
typedef struct {
    u64 BufferAddr;           /**< Frame buffer address for scene change detection */
    u32 SAD;                  /**< Sum of Absolute Differences result */
    u32 Threshold;            /**< Scene change detection threshold value */
    u32 Width;                /**< Frame width in pixels */
    u32 Height;               /**< Frame height in pixels */
    u32 Stride;               /**< Frame stride/line width in bytes */
    u32 SubSample;            /**< Subsampling factor for processing */
    u8  LayerId;              /**< Layer identifier (0-7) */
    u8  StreamEnable;         /**< Stream enable flag */
    XVScdClrFmt VFormat;      /**< Video format (Y8 or Y10) */
} XVScdLayerConfig;

/*Main structure for scene change detection IP driver instance*/
typedef struct {
    UINTPTR Ctrl_BaseAddress;              /**< Base address of the control registers */
    u32 IsReady;                           /**< Initialization status flag */
    u32 ScdDetLayerId;                     /**< Current detected scene change layer ID */
    u32 ScdLayerDetSAD;                    /**< Sum of Absolute Differences for detected layer */
    void *CallbackRef;                     /**< User-defined callback reference pointer */
    XV_scenechange_Config *ScdConfig;      /**< Pointer to device configuration structure */
    XVScdLayerConfig LayerConfig[XV_SCD_IP_MAX_STREAMS];  /**< Layer configurations for all streams */
    XVSceneChange_Callback FrameDoneCallback;  /**< Callback function invoked on frame processing completion */
} XV_scenechange;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
/**
 * Write to register at specified offset
 */
#define XV_scenechange_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))

/**
 * Read from register at specified offset
 */
#define XV_scenechange_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
/**
 * Write to register at specified offset (Linux)
 */
#define XV_scenechange_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)

/**
 * Read from register at specified offset (Linux)
 */
#define XV_scenechange_ReadReg(BaseAddress, RegOffset) \
    *(volatile u32*)((BaseAddress) + (RegOffset))

/**
 * Assert void expression
 */
#define Xil_AssertVoid(expr)    assert(expr)

/**
 * Assert non-void expression
 */
#define Xil_AssertNonvoid(expr) assert(expr)

/**
 * Success status code
 */
#define XST_SUCCESS             0

/**
 * Device not found error code
 */
#define XST_DEVICE_NOT_FOUND    2

/**
 * Open device failed error code
 */
#define XST_OPEN_DEVICE_FAILED  3

/**
 * Component is ready status
 */
#define XIL_COMPONENT_IS_READY  1
#endif

/************************** Function Prototypes *****************************/
#ifndef __linux__
#ifndef SDT
int XV_scenechange_Initialize(XV_scenechange *InstancePtr, u16 DeviceId);
XV_scenechange_Config* XV_scenechange_LookupConfig(u16 DeviceId);
#else
int XV_scenechange_Initialize(XV_scenechange *InstancePtr, UINTPTR BaseAddress);
XV_scenechange_Config* XV_scenechange_LookupConfig(UINTPTR BaseAddress);
#endif
int XV_scenechange_CfgInitialize(XV_scenechange *InstancePtr, XV_scenechange_Config *ConfigPtr);
#else
int XV_scenechange_Initialize(XV_scenechange *InstancePtr, const char* InstanceName);
int XV_scenechange_Release(XV_scenechange *InstancePtr);
#endif

void XV_scenechange_Start(XV_scenechange *InstancePtr);
u32 XV_scenechange_IsDone(XV_scenechange *InstancePtr);
u32 XV_scenechange_IsIdle(XV_scenechange *InstancePtr);
u32 XV_scenechange_IsReady(XV_scenechange *InstancePtr);
void XV_scenechange_EnableAutoRestart(XV_scenechange *InstancePtr);
void XV_scenechange_DisableAutoRestart(XV_scenechange *InstancePtr);

void XV_scenechange_Set_HwReg_width_0(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_width_0(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_height_0(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_height_0(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_stride_0(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_stride_0(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_video_format_0(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_video_format_0(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_subsample_0(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_subsample_0(XV_scenechange *InstancePtr);
u32 XV_scenechange_Get_HwReg_sad0(XV_scenechange *InstancePtr);
u32 XV_scenechange_Get_HwReg_sad0_vld(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_frm_buffer0_V(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_frm_buffer0_V(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_width_1(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_width_1(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_height_1(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_height_1(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_stride_1(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_stride_1(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_video_format_1(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_video_format_1(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_subsample_1(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_subsample_1(XV_scenechange *InstancePtr);
u32 XV_scenechange_Get_HwReg_sad1(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_frm_buffer1_V(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_frm_buffer1_V(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_width_2(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_width_2(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_height_2(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_height_2(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_stride_2(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_stride_2(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_video_format_2(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_video_format_2(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_subsample_2(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_subsample_2(XV_scenechange *InstancePtr);
u32 XV_scenechange_Get_HwReg_sad2(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_frm_buffer2_V(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_frm_buffer2_V(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_width_3(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_width_3(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_height_3(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_height_3(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_stride_3(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_stride_3(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_video_format_3(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_video_format_3(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_subsample_3(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_subsample_3(XV_scenechange *InstancePtr);
u32 XV_scenechange_Get_HwReg_sad3(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_frm_buffer3_V(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_frm_buffer3_V(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_width_4(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_width_4(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_height_4(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_height_4(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_stride_4(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_stride_4(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_video_format_4(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_video_format_4(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_subsample_4(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_subsample_4(XV_scenechange *InstancePtr);
u32 XV_scenechange_Get_HwReg_sad4(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_frm_buffer4_V(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_frm_buffer4_V(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_width_5(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_width_5(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_height_5(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_height_5(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_stride_5(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_stride_5(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_video_format_5(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_video_format_5(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_subsample_5(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_subsample_5(XV_scenechange *InstancePtr);
u32 XV_scenechange_Get_HwReg_sad5(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_frm_buffer5_V(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_frm_buffer5_V(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_width_6(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_width_6(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_height_6(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_height_6(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_stride_6(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_stride_6(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_video_format_6(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_video_format_6(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_subsample_6(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_subsample_6(XV_scenechange *InstancePtr);
u32 XV_scenechange_Get_HwReg_sad6(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_frm_buffer6_V(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_frm_buffer6_V(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_width_7(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_width_7(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_height_7(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_height_7(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_stride_7(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_stride_7(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_video_format_7(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_video_format_7(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_subsample_7(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_subsample_7(XV_scenechange *InstancePtr);
u32 XV_scenechange_Get_HwReg_sad7(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_frm_buffer7_V(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_frm_buffer7_V(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_stream_enable(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_stream_enable(XV_scenechange *InstancePtr);

void XV_scenechange_InterruptGlobalEnable(XV_scenechange *InstancePtr);
void XV_scenechange_InterruptGlobalDisable(XV_scenechange *InstancePtr);
void XV_scenechange_InterruptEnable(XV_scenechange *InstancePtr, u32 Mask);
void XV_scenechange_InterruptDisable(XV_scenechange *InstancePtr, u32 Mask);
void XV_scenechange_InterruptClear(XV_scenechange *InstancePtr, u32 Mask);
u32 XV_scenechange_InterruptGetEnabled(XV_scenechange *InstancePtr);
u32 XV_scenechange_InterruptGetStatus(XV_scenechange *InstancePtr);
void XV_scenechange_InterruptHandler(void *InstancePtr);
void XV_scenechange_EnableInterrupts(void *InstancePtr);
void XV_scenechange_SetCallback(XV_scenechange *InstancePtr, void *CallbackFunc,
			       void *CallbackRef);
int XV_scenechange_Layer_config(XV_scenechange *InstancePtr, u8 layerid);
void XV_scenechange_Layer_stream_enable(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Stop(XV_scenechange *InstancePtr);
u32 XV_scenechange_WaitForIdle(XV_scenechange *InstancePtr);
#ifdef __cplusplus
}
#endif

#endif
