/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file versal_net/xplmi_update.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bm   01/30/2022 Initial release
*       bm   07/06/2022 Refactor versal and versal_net code
*       bm   07/13/2022 Added compatibility check for In-Place PLM Update
* 1.01  ng   11/11/2022 Fixed doxygen file name error
*       dd   03/28/2023 Updated doxygen comments
* 1.02  vns  07/06/2023 Added EXPORT_OCP_DS
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

#ifndef XPLMI_UPDATE_H
#define XPLMI_UPDATE_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xplmi_cmd.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/*
 * Data Structure Version Info is used in DsHdr and also
 * used in creating a list of data structure versions which
 * is stored in .struct_versions section of elf
 */
typedef union {
	struct {
		u8 ModuleId; /**< PLM module ID */
		u8 DsId; /**< Data structure ID */
		u8 Version; /**< Current data structure version */
		u8 LowestCompVer; /**< Lowest compatible version */
	};
	u32 HdrVal; /**< Header value */
} XPlmi_DsVer;

/*
 * Data Structure Header used in DsEntry and also
 * as the header of each data structure in DB
 */
typedef struct {
	XPlmi_DsVer Ver; /**< Version Information of the Data Structure */
	u32 Len; /**< Length of Data Structure in Bytes */
} XPlmi_DsHdr;

/*
 * Data Structure Entry is used to export data
 * structure information into the .struct_entries
 * section of the elf
 */
typedef struct {
	XPlmi_DsHdr DsHdr; /**< Data Structure Header */
	u32 Addr; /**< Address of the Data Structure */
	int (*Handler)(u32 Op, u64 Addr, void *Data); /**< Handler for the Data Structure operations */
} XPlmi_DsEntry;

/*
 * Database Header is header of the database of structures
 * which are stored during the update
 */
typedef struct {
	u8 HdrVersion; /**< Database Header Version */
	u8 HdrSize; /**< Database Header Size */
	u16 Reserved; /**< Reserved Field */
	u32 DbSize;/**< Database Size in Words */
} XPlmi_DbHdr;

typedef int (*XPlmi_CompatibilityCheck_t)(u32 PdiAddr);

/***************** Macros (Inline Functions) Definitions *********************/

/* Data structure handler operations */
#define XPLMI_STORE_DATABASE	(0x1U) /**< Store database */
#define XPLMI_RESTORE_DATABASE	(0x2U)  /**< Restore database */

#define DSVER_ATTR __attribute__ ((section(".struct_versions")))\
				__attribute__((used)) /**< Data structure versions section attribute */

#define DSENTRY_ATTR __attribute__ ((section(".struct_entries")))\
				__attribute__((used)) /**< Data structure entries section attribute */

#define XPLMI_INIT_DS_VER_HDR(MId, DId, Ver, LCVer) {\
			.ModuleId = ((MId) & 0xFFU),\
			.DsId = ((DId) & 0xFFU),\
			.Version = ((Ver) & 0xFFU),\
			.LowestCompVer = ((LCVer) & 0xFFU),\
			} /**< Data structure version header initialization */

/* Macro to export Data structure with custom handler */
/* The below macro has following arguments:
 * Name - Name of the Data Structure
 * Mid - Module Id of the Data Structure
 * Did - Data Structure ID
 * Ver - Current Data Structure version
 * LCVer - Lowest compatible version
 * Size - Size of the Data Structure
 * Address - Address of the Data Structure
 * handler - custom handler of the data structure
 */
#define EXPORT_DS_W_HANDLER(Name, Mid, Did, ver, LCVer, Size, Address, handler) \
	static XPlmi_DsVer Name##_DsInfo DSVER_ATTR = \
			XPLMI_INIT_DS_VER_HDR(Mid, Did, ver, LCVer);\
	static XPlmi_DsEntry Name##_DsEntry DSENTRY_ATTR = {\
		.DsHdr.Ver = XPLMI_INIT_DS_VER_HDR(Mid, Did, ver, LCVer),\
		.DsHdr.Len = Size,\
		.Addr = Address,\
		.Handler = handler} /**< Data structure with custom write handler */

/* Macro to export Data structure with default handler */
/* The below macro has following arguments:
 * Name - Name of the Data Structure
 * Mid - Module Id of the Data Structure
 * Did - Data Structure ID
 * Ver - Current Data Structure version
 * LCVer - Lowest compatible version
 * Size - Size of the Data Structure
 * Address - Address of the Data Structure
 */
#define EXPORT_DS(...) 	EXPORT_DS_W_HANDLER(__VA_ARGS__, XPlmi_DsOps) /**< Data structure with default write handler */

/* Macros to export Data structure of different modules with default handler */
/* Each of the macro has following arguments:
 * Name - Name of the Data Structure
 * Did - Data Structure ID
 * Ver - Current Data Structure version
 * LCVer - Lowest compatible version
 * Size - Size of the Data Structure
 * Address - Address of the Data Structure
 */
#define EXPORT_GENERIC_DS(Name, ...) 	EXPORT_DS(Name, XPLMI_MODULE_GENERIC_ID, __VA_ARGS__) /**< Generic data structure */
#define EXPORT_LOADER_DS(Name, ...) 	EXPORT_DS(Name, XPLMI_MODULE_LOADER_ID, __VA_ARGS__) /**< Loader data structure */
#define EXPORT_SEM_DS(Name, ...) 	EXPORT_DS(Name, XPLMI_MODULE_SEM_ID, __VA_ARGS__) /**< Semaphore data structure */
#define EXPORT_XILSECURE_DS(Name, ...) 	EXPORT_DS(Name, XPLMI_MODULE_XILSECURE_ID, __VA_ARGS__) /**< XilSecure data structure */
#define EXPORT_XILPSM_DS(Name, ...) 	EXPORT_DS(Name, XPLMI_MODULE_XILPSM_ID, __VA_ARGS__) /**< XilPSM data structure */
#define EXPORT_ERROR_DS(Name, ...) 	EXPORT_DS(Name, XPLMI_MODULE_ERROR_ID, __VA_ARGS__) /**< Error data structure */
#define EXPORT_STL_DS(Name, ...) 	EXPORT_DS(Name, XPLMI_MODULE_STL_ID, __VA_ARGS__) /**< STL data structure */
#define EXPORT_OCP_DS(Name, ...) 	EXPORT_DS(Name, XPLMI_MODULE_XILOCP_ID, __VA_ARGS__) /**< OCP data structure */


/************************** Function Prototypes ******************************/
int XPlmi_PlmUpdate(XPlmi_Cmd *Cmd);
int XPlmi_RestoreDataBackup(void);
int XPlmi_DsOps(u32 Op, u64 Addr, void *Data);
int XPlmi_UpdateInit(XPlmi_CompatibilityCheck_t CompatibilityHandler);
XPlmi_DsEntry* XPlmi_GetDsEntry(XPlmi_DsEntry *DsList, u32 DsCnt, XPlmi_DsVer *DsVer);

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_UPDATE_H */
