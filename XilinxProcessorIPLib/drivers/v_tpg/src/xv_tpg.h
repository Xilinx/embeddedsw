// ==============================================================
// Copyright (c) 2015 - 2021 Xilinx Inc. All rights reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_tpg.h
 * @addtogroup v_tpg Overview
 */

#ifndef XV_TPG_H
#define XV_TPG_H

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
#include <stdbool.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>
#endif
#include "xv_tpg_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#else

/**
 * @brief Enumeration of test patterns supported by the TPG.
 *
 * This enumeration defines all the test pattern types that can be generated
 * by the Test Pattern Generator.
 */
typedef enum
{
  XTPG_BKGND_H_RAMP = 1,          /**< Horizontal ramp pattern */
  XTPG_BKGND_V_RAMP,              /**< Vertical ramp pattern */
  XTPG_BKGND_TEMPORAL_RAMP,       /**< Temporal ramp pattern */
  XTPG_BKGND_SOLID_RED,           /**< Solid red color */
  XTPG_BKGND_SOLID_GREEN,         /**< Solid green color */
  XTPG_BKGND_SOLID_BLUE,          /**< Solid blue color */
  XTPG_BKGND_SOLID_BLACK,         /**< Solid black color */
  XTPG_BKGND_SOLID_WHITE,         /**< Solid white color */
  XTPG_BKGND_COLOR_BARS,          /**< Color bars pattern */
  XTPG_BKGND_ZONE_PLATE,          /**< Zone plate pattern */
  XTPG_BKGND_TARTAN_COLOR_BARS,   /**< Tartan color bars pattern */
  XTPG_BKGND_CROSS_HATCH,         /**< Cross hatch pattern */
  XTPG_BKGND_RAINBOW_COLOR,       /**< Rainbow color pattern */
  XTPG_BKGND_HV_RAMP,             /**< Horizontal and vertical ramp pattern */
  XTPG_BKGND_CHECKER_BOARD,       /**< Checker board pattern */
  XTPG_BKGND_PBRS,                /**< Pseudo-random binary sequence pattern */
  XTPG_BKGND_DP_COLOR_RAMP,       /**< DisplayPort color ramp pattern */
  XTPG_BKGND_DP_BW_VERTICAL_LINE, /**< DisplayPort black/white vertical line pattern */
  XTPG_BKGND_DP_COLOR_SQUARE,     /**< DisplayPort color square pattern */
  XTPG_BKGND_LAST                 /**< Last pattern ID (used for validation) */
}XTpg_PatternId;

/**
 * @brief Callback function pointer type for TPG events.
 *
 * @param  InstancePtr is a pointer to the XV_tpg instance.
 */
typedef void (*XVTpg_Callback)(void *InstancePtr);

/************************** Constant Definitions *****************************/
/** Interrupt mask for frame processing done event */
#define XVTPG_IRQ_DONE_MASK            (0x01)
/** Interrupt mask for ready for next frame event */
#define XVTPG_IRQ_READY_MASK           (0x02)

/**
 * @brief Enumeration of interrupt handler types.
 *
 * This enumeration defines the types of interrupt handlers that can be
 * registered for TPG events.
 */
typedef enum {
  XVTPG_HANDLER_DONE = 1,  /**< Handler for ap_done */
  XVTPG_HANDLER_READY      /**< Handler for ap_ready */
} XVTPG_HandlerType;

/**
* This typedef contains configuration information for the tpg core
* Each core instance should have a configuration structure associated.
*/
typedef struct {
#ifndef SDT
    u16 DeviceId;          /**< Unique ID  of device */
#else
    char *Name;
#endif
    UINTPTR BaseAddress;   /**< The base address of the core instance. */
    u16 HasAxi4sSlave;     /**< Axi4s Slave capability indicator */
    u16 PixPerClk;         /**< Samples Per Clock supported by core instance */
    u16 NumVidComponents;  /**< Number of Video Components */
    u16 MaxWidth;          /**< Maximum columns supported by core instance */
    u16 MaxHeight;         /**< Maximum rows supported by core instance */
    u16 MaxDataWidth;      /**< Maximum Data width of each channel */
	u16 SolidColorEnable;  /**< Axi4s Slave capability indicator */
	u16 RampEnable;        /**< Axi4s Slave capability indicator */
	u16 ColorBarEnable;    /**< Axi4s Slave capability indicator */
	u16 DisplayPortEnable; /**< Axi4s Slave capability indicator */
	u16 ColorSweepEnable;  /**< Axi4s Slave capability indicator */
	u16 ZoneplateEnable;   /**< Axi4s Slave capability indicator */
	u16 ForegroundEnable;  /**< Axi4s Slave capability indicator */
#ifdef SDT
    u16 IntrId; 		    /**< Interrupt ID */
    UINTPTR IntrParent; 	/**< Bit[0] Interrupt parent type Bit[64/32:1] Parent base address */
#endif
} XV_tpg_Config;
#endif

/**
* Driver instance data. An instance must be allocated for each core in use.
*/
typedef struct {
    XV_tpg_Config Config;  /**< Hardware Configuration */
    u32 IsReady;           /**< Device is initialized and ready */
    XVTpg_Callback FrameDoneCallback;  /**< Callback for frame done event */
    void *CallbackDoneRef;     /**< To be passed to the connect interrupt
                                callback */
    XVTpg_Callback FrameReadyCallback; /**< Callback for frame ready event */
    void *CallbackReadyRef;     /**< To be passed to the connect interrupt
                                callback */
} XV_tpg;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
/** Write a value to a register */
#define XV_tpg_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
/** Read a value from a register */
#define XV_tpg_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
/** Write a value to a register (Linux) */
#define XV_tpg_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
/** Read a value from a register (Linux) */
#define XV_tpg_ReadReg(BaseAddress, RegOffset) \
    *(volatile u32*)((BaseAddress) + (RegOffset))

/** Assertion macro for void functions */
#define Xil_AssertVoid(expr)    assert(expr)
/** Assertion macro for non-void functions */
#define Xil_AssertNonvoid(expr) assert(expr)

/** Success status code */
#define XST_SUCCESS             0
/** Device not found status code */
#define XST_DEVICE_NOT_FOUND    2
/** Failed to open device status code */
#define XST_OPEN_DEVICE_FAILED  3
/** Component is ready flag */
#define XIL_COMPONENT_IS_READY  1
#endif

/************************** Function Prototypes *****************************/
#ifndef __linux__
#ifndef SDT
int XV_tpg_Initialize(XV_tpg *InstancePtr, u16 DeviceId);
XV_tpg_Config* XV_tpg_LookupConfig(u16 DeviceId);
#else
int XV_tpg_Initialize(XV_tpg *InstancePtr, UINTPTR BaseAddress);
XV_tpg_Config* XV_tpg_LookupConfig(UINTPTR BaseAddress);
#endif
int XV_tpg_CfgInitialize(XV_tpg *InstancePtr,
                         XV_tpg_Config *ConfigPtr,
                         UINTPTR EffectiveAddr);
#else
int XV_tpg_Initialize(XV_tpg *InstancePtr, const char* InstanceName);
int XV_tpg_Release(XV_tpg *InstancePtr);
#endif

void XV_tpg_Start(XV_tpg *InstancePtr);
u32 XV_tpg_IsDone(XV_tpg *InstancePtr);
u32 XV_tpg_IsIdle(XV_tpg *InstancePtr);
u32 XV_tpg_IsReady(XV_tpg *InstancePtr);
void XV_tpg_EnableAutoRestart(XV_tpg *InstancePtr);
void XV_tpg_DisableAutoRestart(XV_tpg *InstancePtr);

void XV_tpg_Set_height(XV_tpg *InstancePtr, u32 Data);
u32 XV_tpg_Get_height(XV_tpg *InstancePtr);
void XV_tpg_Set_width(XV_tpg *InstancePtr, u32 Data);
u32 XV_tpg_Get_width(XV_tpg *InstancePtr);
void XV_tpg_Set_bckgndId(XV_tpg *InstancePtr, u32 Data);
u32 XV_tpg_Get_bckgndId(XV_tpg *InstancePtr);
void XV_tpg_Set_motionEn(XV_tpg *InstancePtr, u32 Data);
u32 XV_tpg_Get_motionEnStatus(XV_tpg *InstancePtr);
void XV_tpg_Set_ovrlayId(XV_tpg *InstancePtr, u32 Data);
u32 XV_tpg_Get_ovrlayId(XV_tpg *InstancePtr);
void XV_tpg_Set_maskId(XV_tpg *InstancePtr, u32 Data);
u32 XV_tpg_Get_maskId(XV_tpg *InstancePtr);
void XV_tpg_Set_motionSpeed(XV_tpg *InstancePtr, u32 Data);
u32 XV_tpg_Get_motionSpeed(XV_tpg *InstancePtr);
void XV_tpg_Set_colorFormat(XV_tpg *InstancePtr, u32 Data);
u32 XV_tpg_Get_colorFormat(XV_tpg *InstancePtr);
void XV_tpg_Set_crossHairX(XV_tpg *InstancePtr, u32 Data);
u32 XV_tpg_Get_crossHairX(XV_tpg *InstancePtr);
void XV_tpg_Set_crossHairY(XV_tpg *InstancePtr, u32 Data);
u32 XV_tpg_Get_crossHairY(XV_tpg *InstancePtr);
void XV_tpg_Set_ZplateHorContStart(XV_tpg *InstancePtr, u32 Data);
u32 XV_tpg_Get_ZplateHorContStart(XV_tpg *InstancePtr);
void XV_tpg_Set_ZplateHorContDelta(XV_tpg *InstancePtr, u32 Data);
u32 XV_tpg_Get_ZplateHorContDelta(XV_tpg *InstancePtr);
void XV_tpg_Set_ZplateVerContStart(XV_tpg *InstancePtr, u32 Data);
u32 XV_tpg_Get_ZplateVerContStart(XV_tpg *InstancePtr);
void XV_tpg_Set_ZplateVerContDelta(XV_tpg *InstancePtr, u32 Data);
u32 XV_tpg_Get_ZplateVerContDelta(XV_tpg *InstancePtr);
void XV_tpg_Set_boxSize(XV_tpg *InstancePtr, u32 Data);
u32 XV_tpg_Get_boxSize(XV_tpg *InstancePtr);
void XV_tpg_Set_boxColorR(XV_tpg *InstancePtr, u32 Data);
u32 XV_tpg_Get_boxColorR(XV_tpg *InstancePtr);
void XV_tpg_Set_boxColorG(XV_tpg *InstancePtr, u32 Data);
u32 XV_tpg_Get_boxColorG(XV_tpg *InstancePtr);
void XV_tpg_Set_boxColorB(XV_tpg *InstancePtr, u32 Data);
u32 XV_tpg_Get_boxColorB(XV_tpg *InstancePtr);
void XV_tpg_Set_enableInput(XV_tpg *InstancePtr, u32 Data);
u32 XV_tpg_Get_enableInput(XV_tpg *InstancePtr);
void XV_tpg_Set_passthruStartX(XV_tpg *InstancePtr, u32 Data);
u32 XV_tpg_Get_passthruStartX(XV_tpg *InstancePtr);
void XV_tpg_Set_passthruStartY(XV_tpg *InstancePtr, u32 Data);
u32 XV_tpg_Get_passthruStartY(XV_tpg *InstancePtr);
void XV_tpg_Set_passthruEndX(XV_tpg *InstancePtr, u32 Data);
u32 XV_tpg_Get_passthruEndX(XV_tpg *InstancePtr);
void XV_tpg_Set_passthruEndY(XV_tpg *InstancePtr, u32 Data);
u32 XV_tpg_Get_passthruEndY(XV_tpg *InstancePtr);
void XV_tpg_Set_dpDynamicRange(XV_tpg *InstancePtr, u32 Data);
u32 XV_tpg_Get_dpDynamicRange(XV_tpg *InstancePtr);
void XV_tpg_Set_dpYUVCoef(XV_tpg *InstancePtr, u32 Data);
u32 XV_tpg_Get_dpYUVCoef(XV_tpg *InstancePtr);
void XV_tpg_Set_Interlaced(XV_tpg *InstancePtr, _Bool Data);
void XV_tpg_Set_Polarity(XV_tpg *InstancePtr, _Bool Data);
u32 XV_tpg_Get_fieldId(XV_tpg *InstancePtr);

void XV_tpg_InterruptGlobalEnable(XV_tpg *InstancePtr);
void XV_tpg_InterruptGlobalDisable(XV_tpg *InstancePtr);
void XV_tpg_InterruptEnable(XV_tpg *InstancePtr, u32 Mask);
void XV_tpg_InterruptDisable(XV_tpg *InstancePtr, u32 Mask);
void XV_tpg_InterruptClear(XV_tpg *InstancePtr, u32 Mask);
u32 XV_tpg_InterruptGetEnabled(XV_tpg *InstancePtr);
u32 XV_tpg_InterruptGetStatus(XV_tpg *InstancePtr);

void XVTpg_SetCallback(XV_tpg *InstancePtr, u32 HandlerType,
		void *CallbackFunc, void *CallbackRef);
void XVTpg_InterruptHandler(XV_tpg *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
