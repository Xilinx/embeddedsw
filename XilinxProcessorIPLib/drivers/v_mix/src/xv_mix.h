// ==============================================================
// Copyright (c) 1986 - 2022 Xilinx Inc. All rights reserved.
// Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifndef XV_MIX_H
#define XV_MIX_H

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
#include "xv_mix_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#else

#define XV_MIX_MAX_MEMORY_LAYERS    (16)

/**
* This typedef contains Alpha feature enable flag per memory layer
*/
typedef struct {
  u8  Layer1AlphaEn;     /**< Layer Alpha support indicator flag  */
  u8  Layer2AlphaEn;     /**< Layer Alpha support indicator flag  */
  u8  Layer3AlphaEn;     /**< Layer Alpha support indicator flag  */
  u8  Layer4AlphaEn;     /**< Layer Alpha support indicator flag  */
  u8  Layer5AlphaEn;     /**< Layer Alpha support indicator flag  */
  u8  Layer6AlphaEn;     /**< Layer Alpha support indicator flag  */
  u8  Layer7AlphaEn;     /**< Layer Alpha support indicator flag  */
  u8  Layer8AlphaEn;     /**< Layer Alpha support indicator flag  */
  u8  Layer9AlphaEn;     /**< Layer Alpha support indicator flag  */
  u8  Layer10AlphaEn;     /**< Layer Alpha support indicator flag  */
  u8  Layer11AlphaEn;     /**< Layer Alpha support indicator flag  */
  u8  Layer12AlphaEn;     /**< Layer Alpha support indicator flag  */
  u8  Layer13AlphaEn;     /**< Layer Alpha support indicator flag  */
  u8  Layer14AlphaEn;     /**< Layer Alpha support indicator flag  */
  u8  Layer15AlphaEn;     /**< Layer Alpha support indicator flag  */
  u8  Layer16AlphaEn;     /**< Layer Alpha support indicator flag  */
}XVMix_AlphaFlag;

/**
* This typedef contains Scaling feature enable flag per memory layer
*/
typedef struct {
  u8  Layer1ScalingEn;  /**< Layer scaling support indicator flag  */
  u8  Layer2ScalingEn;  /**< Layer scaling support indicator flag  */
  u8  Layer3ScalingEn;  /**< Layer scaling support indicator flag  */
  u8  Layer4ScalingEn;  /**< Layer scaling support indicator flag  */
  u8  Layer5ScalingEn;  /**< Layer scaling support indicator flag  */
  u8  Layer6ScalingEn;  /**< Layer scaling support indicator flag  */
  u8  Layer7ScalingEn;  /**< Layer scaling support indicator flag  */
  u8  Layer8ScalingEn;  /**< Layer scaling support indicator flag  */
  u8  Layer9ScalingEn;  /**< Layer scaling support indicator flag  */
  u8  Layer10ScalingEn;  /**< Layer scaling support indicator flag  */
  u8  Layer11ScalingEn;  /**< Layer scaling support indicator flag  */
  u8  Layer12ScalingEn;  /**< Layer scaling support indicator flag  */
  u8  Layer13ScalingEn;  /**< Layer scaling support indicator flag  */
  u8  Layer14ScalingEn;  /**< Layer scaling support indicator flag  */
  u8  Layer15ScalingEn;  /**< Layer scaling support indicator flag  */
  u8  Layer16ScalingEn;  /**< Layer scaling support indicator flag  */
}XVMix_ScaleFlag;

/**
* This typedef contains Interface Type per layer
*/
typedef struct {
  u8  Layer1IntfType;  /**< Layer Interface type (Memory/Stream)  */
  u8  Layer2IntfType;  /**< Layer Interface type (Memory/Stream)  */
  u8  Layer3IntfType;  /**< Layer Interface type (Memory/Stream)  */
  u8  Layer4IntfType;  /**< Layer Interface type (Memory/Stream)  */
  u8  Layer5IntfType;  /**< Layer Interface type (Memory/Stream)  */
  u8  Layer6IntfType;  /**< Layer Interface type (Memory/Stream)  */
  u8  Layer7IntfType;  /**< Layer Interface type (Memory/Stream)  */
  u8  Layer8IntfType;  /**< Layer Interface type (Memory/Stream)  */
  u8  Layer9IntfType;  /**< Layer Interface type (Memory/Stream)  */
  u8  Layer10IntfType;  /**< Layer Interface type (Memory/Stream)  */
  u8  Layer11IntfType;  /**< Layer Interface type (Memory/Stream)  */
  u8  Layer12IntfType;  /**< Layer Interface type (Memory/Stream)  */
  u8  Layer13IntfType;  /**< Layer Interface type (Memory/Stream)  */
  u8  Layer14IntfType;  /**< Layer Interface type (Memory/Stream)  */
  u8  Layer15IntfType;  /**< Layer Interface type (Memory/Stream)  */
  u8  Layer16IntfType;  /**< Layer Interface type (Memory/Stream)  */
}XVMix_LayerIntfType;

/**
* This typedef contains color format per memory layer
*/
typedef struct {
  u8  Layer1ColorFmt;  /**< Layer Video Color Format  */
  u8  Layer2ColorFmt;  /**< Layer Video Color Format  */
  u8  Layer3ColorFmt;  /**< Layer Video Color Format  */
  u8  Layer4ColorFmt;  /**< Layer Video Color Format  */
  u8  Layer5ColorFmt;  /**< Layer Video Color Format  */
  u8  Layer6ColorFmt;  /**< Layer Video Color Format  */
  u8  Layer7ColorFmt;  /**< Layer Video Color Format  */
  u8  Layer8ColorFmt;  /**< Layer Video Color Format  */
  u8  Layer9ColorFmt;  /**< Layer Video Color Format  */
  u8  Layer10ColorFmt;  /**< Layer Video Color Format  */
  u8  Layer11ColorFmt;  /**< Layer Video Color Format  */
  u8  Layer12ColorFmt;  /**< Layer Video Color Format  */
  u8  Layer13ColorFmt;  /**< Layer Video Color Format  */
  u8  Layer14ColorFmt;  /**< Layer Video Color Format  */
  u8  Layer15ColorFmt;  /**< Layer Video Color Format  */
  u8  Layer16ColorFmt;  /**< Layer Video Color Format  */
}XVMix_LayerColorFormat;

/**
* This typedef contains maximum width per memory layer
*/
typedef struct {
  u16 Layer1MaxWidth;    /**< Layer maximum column width  */
  u16 Layer2MaxWidth;    /**< Layer maximum column width  */
  u16 Layer3MaxWidth;    /**< Layer maximum column width  */
  u16 Layer4MaxWidth;    /**< Layer maximum column width  */
  u16 Layer5MaxWidth;    /**< Layer maximum column width  */
  u16 Layer6MaxWidth;    /**< Layer maximum column width  */
  u16 Layer7MaxWidth;    /**< Layer maximum column width  */
  u16 Layer8MaxWidth;    /**< Layer maximum column width  */
  u16 Layer9MaxWidth;    /**< Layer maximum column width  */
  u16 Layer10MaxWidth;    /**< Layer maximum column width  */
  u16 Layer11MaxWidth;    /**< Layer maximum column width  */
  u16 Layer12MaxWidth;    /**< Layer maximum column width  */
  u16 Layer13MaxWidth;    /**< Layer maximum column width  */
  u16 Layer14MaxWidth;    /**< Layer maximum column width  */
  u16 Layer15MaxWidth;    /**< Layer maximum column width  */
  u16 Layer16MaxWidth;    /**< Layer maximum column width  */
}XVMix_LayerMaxWidth;

/**
* This typedef contains configuration information for the mixer core
* Each core instance should have a configuration structure associated.
*/
typedef struct {
#ifndef SDT
  u16 DeviceId;          /**< Unique ID  of device */
#else
  char *Name;
#endif
  UINTPTR BaseAddress;   /**< The base address of the core instance. */
  u16 PixPerClk;         /**< Samples Per Clock supported by core instance */
  u16 MaxWidth;          /**< Maximum columns supported by core instance */
  u16 MaxHeight;         /**< Maximum rows supported by core instance */
  u16 MaxDataWidth;      /**< Maximum Data width of each channel */
  u16 ColorFormat;       /**< Master layer color format */
  u8  NumLayers;         /**< Number of layers supported */
  u8  LogoEn;            /**< Logo layer support indicator flag  */
  u16 MaxLogoWidth;      /**< Maximum columns supported by log layer */
  u16 MaxLogoHeight;     /**< Maximum rows supported by log layer */
  u16 LogoColorKeyEn;    /**< Logo layer color key feature indicator flag */
  u16 LogoPixAlphaEn;    /**< Logo layer per pixel alpha feature indicator flag */
  u16 CscCoeffsRegsEn;    /**< Logo layer per pixel alpha feature indicator flag */
  union {                /**< Alpha feature enable flag per memory layer */
	  XVMix_AlphaFlag AlphaFlag;
	  u8 AlphaEn[XV_MIX_MAX_MEMORY_LAYERS];
  };
  union {                /**< Scaling feature enable flag per memory layer */
	  XVMix_ScaleFlag ScaleFlag;
	  u8 ScalingEn[XV_MIX_MAX_MEMORY_LAYERS];
  };
  union {                /**< Maximum width per memory layer */
	  XVMix_LayerMaxWidth LyrMaxWidth;
	  u16 LayerMaxWidth[XV_MIX_MAX_MEMORY_LAYERS];
  };
  union {                /**< Layer Interface Type */
	  XVMix_LayerIntfType LyrIntfType;
	  u8 LayerIntrfType[XV_MIX_MAX_MEMORY_LAYERS];
  };
  union {                /**< Layer Interface Type */
	  XVMix_LayerColorFormat LyrColorFmt;
	  u8 LayerColorFmt[XV_MIX_MAX_MEMORY_LAYERS];
  };
#ifdef SDT
  u16 IntrId; 		    /**< Interrupt ID */
  UINTPTR IntrParent;	/**< Bit[0] Interrupt parent type Bit[64/32:1] Parent base address */
#endif
} XV_mix_Config;
#endif

/**
* Driver instance data. An instance must be allocated for each core in use.
*/
typedef struct {
  XV_mix_Config Config;  /**< Hardware Configuration */
  u32 IsReady;           /**< Device is initialized and ready */
} XV_mix;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XV_mix_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XV_mix_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XV_mix_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XV_mix_ReadReg(BaseAddress, RegOffset) \
    *(volatile u32*)((BaseAddress) + (RegOffset))

#define Xil_AssertVoid(expr)    assert(expr)
#define Xil_AssertNonvoid(expr) assert(expr)

#define XST_SUCCESS             0
#define XST_DEVICE_NOT_FOUND    2
#define XST_OPEN_DEVICE_FAILED  3
#define XIL_COMPONENT_IS_READY  1
#endif

/************************** Function Prototypes *****************************/
#ifndef __linux__
#ifndef SDT
int XV_mix_Initialize(XV_mix *InstancePtr, u16 DeviceId);
XV_mix_Config* XV_mix_LookupConfig(u16 DeviceId);
#else
int XV_mix_Initialize(XV_mix *InstancePtr, UINTPTR BaseAddress);
XV_mix_Config* XV_mix_LookupConfig(UINTPTR BaseAddress);
#endif
int XV_mix_CfgInitialize(XV_mix *InstancePtr,
		                 XV_mix_Config *ConfigPtr,
						 UINTPTR EffectiveAddr);
#else
int XV_mix_Initialize(XV_mix *InstancePtr, const char* InstanceName);
int XV_mix_Release(XV_mix *InstancePtr);
#endif

void XV_mix_Start(XV_mix *InstancePtr);
u32 XV_mix_IsDone(XV_mix *InstancePtr);
u32 XV_mix_IsIdle(XV_mix *InstancePtr);
u32 XV_mix_IsReady(XV_mix *InstancePtr);
void XV_mix_EnableAutoRestart(XV_mix *InstancePtr);
void XV_mix_DisableAutoRestart(XV_mix *InstancePtr);

void XV_mix_Set_HwReg_width(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_width(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_height(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_height(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_video_format(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_video_format(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_background_Y_R(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_background_Y_R(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_background_U_G(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_background_U_G(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_background_V_B(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_background_V_B(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerEnable(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerEnable(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerAlpha_0(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerAlpha_0(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartX_0(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartX_0(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartY_0(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartY_0(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerWidth_0(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerWidth_0(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStride_0(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStride_0(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerHeight_0(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerHeight_0(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerScaleFactor_0(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerScaleFactor_0(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerVideoFormat_0(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerVideoFormat_0(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerAlpha_1(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerAlpha_1(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartX_1(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartX_1(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartY_1(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartY_1(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerWidth_1(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerWidth_1(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStride_1(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStride_1(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerHeight_1(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerHeight_1(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerScaleFactor_1(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerScaleFactor_1(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerVideoFormat_1(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerVideoFormat_1(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer1_buf1_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer1_buf1_V(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer1_buf2_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer1_buf2_V(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerAlpha_2(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerAlpha_2(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartX_2(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartX_2(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartY_2(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartY_2(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerWidth_2(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerWidth_2(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStride_2(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStride_2(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerHeight_2(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerHeight_2(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerScaleFactor_2(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerScaleFactor_2(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerVideoFormat_2(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerVideoFormat_2(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer2_buf1_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer2_buf1_V(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer2_buf2_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer2_buf2_V(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerAlpha_3(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerAlpha_3(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartX_3(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartX_3(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartY_3(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartY_3(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerWidth_3(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerWidth_3(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStride_3(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStride_3(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerHeight_3(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerHeight_3(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerScaleFactor_3(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerScaleFactor_3(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerVideoFormat_3(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerVideoFormat_3(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer3_buf1_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer3_buf1_V(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer3_buf2_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer3_buf2_V(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerAlpha_4(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerAlpha_4(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartX_4(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartX_4(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartY_4(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartY_4(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerWidth_4(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerWidth_4(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStride_4(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStride_4(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerHeight_4(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerHeight_4(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerScaleFactor_4(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerScaleFactor_4(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerVideoFormat_4(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerVideoFormat_4(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer4_buf1_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer4_buf1_V(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer4_buf2_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer4_buf2_V(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerAlpha_5(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerAlpha_5(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartX_5(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartX_5(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartY_5(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartY_5(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerWidth_5(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerWidth_5(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStride_5(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStride_5(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerHeight_5(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerHeight_5(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerScaleFactor_5(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerScaleFactor_5(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerVideoFormat_5(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerVideoFormat_5(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer5_buf1_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer5_buf1_V(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer5_buf2_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer5_buf2_V(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerAlpha_6(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerAlpha_6(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartX_6(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartX_6(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartY_6(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartY_6(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerWidth_6(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerWidth_6(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStride_6(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStride_6(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerHeight_6(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerHeight_6(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerScaleFactor_6(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerScaleFactor_6(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerVideoFormat_6(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerVideoFormat_6(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer6_buf1_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer6_buf1_V(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer6_buf2_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer6_buf2_V(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerAlpha_7(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerAlpha_7(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartX_7(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartX_7(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartY_7(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartY_7(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerWidth_7(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerWidth_7(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStride_7(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStride_7(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerHeight_7(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerHeight_7(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerScaleFactor_7(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerScaleFactor_7(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerVideoFormat_7(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerVideoFormat_7(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer7_buf1_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer7_buf1_V(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer7_buf2_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer7_buf2_V(XV_mix *InstancePtr);

void XV_mix_Set_HwReg_layerAlpha_8(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerAlpha_8(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartX_8(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartX_8(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartY_8(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartY_8(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerWidth_8(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerWidth_8(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStride_8(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStride_8(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerHeight_8(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerHeight_8(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerScaleFactor_8(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerScaleFactor_8(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerVideoFormat_8(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerVideoFormat_8(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer8_buf1_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer8_buf1_V(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer8_buf2_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer8_buf2_V(XV_mix *InstancePtr);

void XV_mix_Set_HwReg_layerAlpha_9(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerAlpha_9(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartX_9(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartX_9(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartY_9(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartY_9(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerWidth_9(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerWidth_9(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStride_9(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStride_9(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerHeight_9(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerHeight_9(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerScaleFactor_9(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerScaleFactor_9(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerVideoFormat_9(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerVideoFormat_9(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer9_buf1_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer9_buf1_V(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer9_buf2_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer9_buf2_V(XV_mix *InstancePtr);

void XV_mix_Set_HwReg_layerAlpha_10(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerAlpha_10(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartX_10(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartX_10(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartY_10(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartY_10(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerWidth_10(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerWidth_10(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStride_10(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStride_10(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerHeight_10(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerHeight_10(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerScaleFactor_10(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerScaleFactor_10(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerVideoFormat_10(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerVideoFormat_10(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer10_buf1_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer10_buf1_V(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer10_buf2_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer10_buf2_V(XV_mix *InstancePtr);

void XV_mix_Set_HwReg_layerAlpha_11(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerAlpha_11(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartX_11(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartX_11(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartY_11(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartY_11(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerWidth_11(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerWidth_11(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStride_11(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStride_11(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerHeight_11(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerHeight_11(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerScaleFactor_11(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerScaleFactor_11(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerVideoFormat_11(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerVideoFormat_11(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer11_buf1_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer11_buf1_V(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer11_buf2_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer11_buf2_V(XV_mix *InstancePtr);

void XV_mix_Set_HwReg_layerAlpha_12(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerAlpha_12(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartX_12(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartX_12(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartY_12(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartY_12(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerWidth_12(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerWidth_12(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStride_12(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStride_12(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerHeight_12(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerHeight_12(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerScaleFactor_12(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerScaleFactor_12(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerVideoFormat_12(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerVideoFormat_12(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer12_buf1_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer12_buf1_V(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer12_buf2_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer12_buf2_V(XV_mix *InstancePtr);

void XV_mix_Set_HwReg_layerAlpha_13(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerAlpha_13(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartX_13(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartX_13(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartY_13(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartY_13(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerWidth_13(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerWidth_13(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStride_13(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStride_13(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerHeight_13(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerHeight_13(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerScaleFactor_13(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerScaleFactor_13(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerVideoFormat_13(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerVideoFormat_13(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer13_buf1_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer13_buf1_V(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer13_buf2_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer13_buf2_V(XV_mix *InstancePtr);

void XV_mix_Set_HwReg_layerAlpha_14(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerAlpha_14(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartX_14(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartX_14(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartY_14(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartY_14(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerWidth_14(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerWidth_14(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStride_14(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStride_14(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerHeight_14(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerHeight_14(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerScaleFactor_14(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerScaleFactor_14(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerVideoFormat_14(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerVideoFormat_14(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer14_buf1_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer14_buf1_V(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer14_buf2_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer14_buf2_V(XV_mix *InstancePtr);

void XV_mix_Set_HwReg_layerAlpha_15(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerAlpha_15(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartX_15(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartX_15(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartY_15(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartY_15(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerWidth_15(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerWidth_15(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStride_15(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStride_15(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerHeight_15(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerHeight_15(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerScaleFactor_15(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerScaleFactor_15(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerVideoFormat_15(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerVideoFormat_15(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer15_buf1_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer15_buf1_V(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer15_buf2_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer15_buf2_V(XV_mix *InstancePtr);

void XV_mix_Set_HwReg_layerAlpha_16(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerAlpha_16(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartX_16(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartX_16(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartY_16(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartY_16(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerWidth_16(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerWidth_16(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStride_16(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStride_16(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerHeight_16(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerHeight_16(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerScaleFactor_16(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerScaleFactor_16(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerVideoFormat_16(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerVideoFormat_16(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer16_buf1_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer16_buf1_V(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer16_buf2_V(XV_mix *InstancePtr, u64 Data);
u64 XV_mix_Get_HwReg_layer16_buf2_V(XV_mix *InstancePtr);

void XV_mix_Set_HwReg_reserve(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_reserve(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_logoStartX(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_logoStartX(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_logoStartY(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_logoStartY(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_logoWidth(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_logoWidth(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_logoHeight(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_logoHeight(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_logoScaleFactor(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_logoScaleFactor(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_logoAlpha(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_logoAlpha(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_logoClrKeyMin_R(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_logoClrKeyMin_R(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_logoClrKeyMin_G(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_logoClrKeyMin_G(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_logoClrKeyMin_B(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_logoClrKeyMin_B(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_logoClrKeyMax_R(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_logoClrKeyMax_R(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_logoClrKeyMax_G(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_logoClrKeyMax_G(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_logoClrKeyMax_B(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_logoClrKeyMax_B(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoR_V_BaseAddress(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoR_V_HighAddress(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoR_V_TotalBytes(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoR_V_BitWidth(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoR_V_Depth(XV_mix *InstancePtr);
u32 XV_mix_Write_HwReg_logoR_V_Words(XV_mix *InstancePtr, int offset, int *data, int length);
u32 XV_mix_Read_HwReg_logoR_V_Words(XV_mix *InstancePtr, int offset, int *data, int length);
u32 XV_mix_Write_HwReg_logoR_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length);
u32 XV_mix_Read_HwReg_logoR_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length);
u32 XV_mix_Get_HwReg_logoG_V_BaseAddress(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoG_V_HighAddress(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoG_V_TotalBytes(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoG_V_BitWidth(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoG_V_Depth(XV_mix *InstancePtr);
u32 XV_mix_Write_HwReg_logoG_V_Words(XV_mix *InstancePtr, int offset, int *data, int length);
u32 XV_mix_Read_HwReg_logoG_V_Words(XV_mix *InstancePtr, int offset, int *data, int length);
u32 XV_mix_Write_HwReg_logoG_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length);
u32 XV_mix_Read_HwReg_logoG_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length);
u32 XV_mix_Get_HwReg_logoB_V_BaseAddress(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoB_V_HighAddress(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoB_V_TotalBytes(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoB_V_BitWidth(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoB_V_Depth(XV_mix *InstancePtr);
u32 XV_mix_Write_HwReg_logoB_V_Words(XV_mix *InstancePtr, int offset, int *data, int length);
u32 XV_mix_Read_HwReg_logoB_V_Words(XV_mix *InstancePtr, int offset, int *data, int length);
u32 XV_mix_Write_HwReg_logoB_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length);
u32 XV_mix_Read_HwReg_logoB_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length);
u32 XV_mix_Get_HwReg_logoA_V_BaseAddress(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoA_V_HighAddress(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoA_V_TotalBytes(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoA_V_BitWidth(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoA_V_Depth(XV_mix *InstancePtr);
u32 XV_mix_Write_HwReg_logoA_V_Words(XV_mix *InstancePtr, int offset, int *data, int length);
u32 XV_mix_Read_HwReg_logoA_V_Words(XV_mix *InstancePtr, int offset, int *data, int length);
u32 XV_mix_Write_HwReg_logoA_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length);
u32 XV_mix_Read_HwReg_logoA_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length);

void XV_mix_InterruptGlobalEnable(XV_mix *InstancePtr);
void XV_mix_InterruptGlobalDisable(XV_mix *InstancePtr);
void XV_mix_InterruptEnable(XV_mix *InstancePtr, u32 Mask);
void XV_mix_InterruptDisable(XV_mix *InstancePtr, u32 Mask);
void XV_mix_InterruptClear(XV_mix *InstancePtr, u32 Mask);
u32 XV_mix_InterruptGetEnabled(XV_mix *InstancePtr);
u32 XV_mix_InterruptGetStatus(XV_mix *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
