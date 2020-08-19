/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xbir_sys.h
*
* This file contains list of SoM syste API defination provided by xbir_sys.c
* file
*
******************************************************************************/

#ifndef XBIR_SYS_H
#define XBIR_SYS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"

/************************** Constant Definitions *****************************/
#define XBIR_SYS_IMG_A_BOOTABLE_MASK	(0x08U)
#define XBIR_SYS_IMG_B_BOOTABLE_MASK	(0x04U)
#define XBIR_SYS_BOOTABLE_IMG_MASK	(0x02U)
#define XBIR_SYS_LAST_BOOTABLE_IMG_MASK	(0x01U)
#define XBIR_SYS_IMG_A_BOOTABLE_BIT_NO	(0x03U)
#define XBIR_SYS_IMG_B_BOOTABLE_BIT_NO	(0x02U)
#define XBIR_SYS_BOOTABLE_IMG_BIT_NO	(0x01U)
#define XBIR_BOOT_IMG_INFO_HDR_SIZE_IN_WORDS	(4U)

#define XBIR_SYS_MAX_BRD_NAME_LEN	(25U)
#define XBIR_SYS_MAX_REV_NO_VAL_LEN	(8U)
#define XBIR_SYS_MAX_SERIAL_NO_VAL_LEN	(20U)
#define XBIR_SYS_MAX_STATE_NAME_VAL_LEN	(8U)
#define XBIR_SYS_MAX_PART_NO_VAL_LEN	(20U)
#define XBIR_SYS_MAX_UUID_VAL_LEN	(20U)

/**************************** Type Definitions *******************************/
typedef struct {
	u8 Version;
	u8 IntUsrArea;
	u8 ChassInfoArea;
	u8 BoardArea;
	u8 ProdInfoArea;
	u8 MultiRecArea;
	u8 Pad;
	u8 ChkSum;
} Xbir_SysHeaderInfo;

typedef struct {
	u8 Version;
	u8 Length;
	u8 LanguageCode;
	u8 ManufacDate[3U];
	u8 BoardMaufacType;
	u8 BoardManufacturer[6U];
	u8 BoardPrdNameType;
	u8 BoardPrdName[16U];
	u8 BoardSerialType;
	u8 BoardSerialNumber[16U];
	u8 BoardPartNumType;
	u8 BoardPartNum[9U];
	u8 FRUFileIdTYpe;
	u8 FRUFileId;
	u8 RevType;
	u8 RevNum[8U];
	u8 PcieInfoType;
	u8 PcieInfo[8U];
	u8 UUIDType;
	u8 UUID[16U];
	u8 Eof;
	u8 ChkSum;
} Xbir_SysBoardInfo;

typedef struct {
	u8 LastBootedImg;
	u8 RequestedBootImg;
	u8 ImgBBootable;
	u8 ImgABootable;
} Xbir_SysPersistentState;

typedef struct {
	u8 IdStr[4U];
	u32 Ver;
	u32 Len;
	u32 Checksum;
	Xbir_SysPersistentState PersistentState;
	u32 BootImgAOffset;
	u32 BootImgBOffset;
	u32 RecoveryImgOffset;
} Xbir_SysBootImgInfo;

typedef enum {
	XBIR_SYS_BOOT_IMG_A_ID = 0,
	XBIR_SYS_BOOT_IMG_B_ID
} Xbir_SysBootImgId;

typedef enum {
	XBIR_SYS_PARTIAL_DATA_CHUNK = 0,
	XBIR_SYS_LAST_DATA_CHUNK
} Xbir_ImgDataStatus;


/************************** Function Prototypes ******************************/
int Xbir_SysInit (void);
const Xbir_SysBoardInfo* Xbir_SysGetSomInfo (void);
const Xbir_SysBoardInfo* Xbir_SysGetCcInfo (void);
const Xbir_SysPersistentState * Xbir_SysGetBootImgStatus (void);
int Xbir_SysUpdateBootImgStatus (u8 ImgABootable, u8 ImgBBootable,
	u8 ReqBootImg);
int Xbir_SysGetBootImgOffset (Xbir_SysBootImgId BootImgId, u32 *Offset);
int Xbir_SysWriteFlash (u32 Offset, u8 *Data, u32 Size,
	Xbir_ImgDataStatus IsLast);
int Xbir_SysEraseBootImg (Xbir_SysBootImgId BootImgId);
int Xbir_SysValidateCrc (Xbir_SysBootImgId BootImgId, u32 Size, u32 InCrc);

#ifdef __cplusplus
}
#endif

#endif