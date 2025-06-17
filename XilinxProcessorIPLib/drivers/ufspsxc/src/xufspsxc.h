/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xufspsxc.h
* @addtogroup ufspsxc Overview
* @{
* @details
*
* This section contains information about the driver structures, user
* API prototypes and all the defines required for the user to interact
* with the driver. ufspsxc driver supports the UFS controller in Versal 2VE and
* 2VM SoC platforms.
*
* <b>Initialization & Configuration</b>
*
*
* <b>Supported features</b>
*
* This driver supports following features
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------.
* 1.0   sk  01/16/24 First release
* 1.1   sk  01/13/25 Update the example to enable the LU before
*                    configuring the Boot LUN ID.
* 1.2   an  06/09/25 Configure RMMI and M-PHY registers for HS mode
*
* </pre>
*
******************************************************************************/
#ifndef XUFSPSXC_H_		/**< prevent circular inclusions */
#define XUFSPSXC_H_		/**< by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xufspsxc_hw.h"
#include "xil_cache.h"
#include "xil_util.h"

/**************************** Type Definitions *******************************/

#ifdef VERSAL_PLM
#define XUFSPSXC_PRDT_ENTRIES	1U
#else
#define XUFSPSXC_PRDT_ENTRIES	4096U
#endif

/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
	char *Name;		/**< Compatible string of the device */
	UINTPTR BaseAddress;	/**< Base address of the device */
	u32 InputClockHz;	/**< Input clock frequency */
	u32 CfgClkFreqHz;	/**< M-PHY Config clock frequency (external clock) */
	u32 RefPadClk;		/**< UFS Reference Pad clock */
	u8 IsCacheCoherent;	/**< If UFS is Cache Coherent or not */
	u16 IntrId;             /**< Bits[11:0] Interrupt-id Bits[15:12]
	                        * trigger type and level flags */
	UINTPTR IntrParent;     /**< Bit[0] Interrupt parent type Bit[64/32:1]
	                        * Parent base address */
} XUfsPsxc_Config;

typedef struct {
	u8 TransactionType;
	u8 Flags;
	u8 LUN;
	u8 TaskTag;
	u8 IID_Cmd_Type;
	u8 Query_Task_Mang_Fn;
	u8 Response;
	u8 Status;
	u8 TotalEHSLen;
	u8 DeviceInfo;
	u16 DataSegmentLen;	/**< Big Endian format */
} __attribute__((__packed__))XUfsPsxc_UpiuHeader;

/**
 * Command UPIU Structure
 */
typedef struct {
	u32 ExpDataXferLen;
	u8 scsi_cdb[16U];
} __attribute__((__packed__))XUfsPsxc_CmdUpiu;

/**
 * Transaction Specific Fields Structure
 */
typedef struct {
	u8 Opcode;
	u8 DescId;
	u8 Index;
	u8 Selector;
	u16 Reserved0;
	u16 Length;	/**< Big Endian format */
	u32 Value;	/**< Big Endian format */
	u32 Reserved1;
} __attribute__((__packed__))XUfsPsxc_TransSpecFlds;

/**
 * Query UPIU Structure
 */
typedef struct {
	XUfsPsxc_TransSpecFlds Tsf;
	u32 Reserved;
	u8 Data[256U];
} __attribute__((__packed__))XUfsPsxc_QueryUpiu;

/**
 * NOP IN/OUT UPIU Structure
 */
typedef struct {
	u8 Reserved[20];
} __attribute__((__packed__))XUfsPsxc_NopUpiu;

/**
 * UTP Transfer Request Descriptor Structure
 */
typedef struct {
	XUfsPsxc_UpiuHeader UpiuHeader;
	union {
		XUfsPsxc_CmdUpiu CmdUpiu;
		XUfsPsxc_QueryUpiu QueryReqUpiu;
		XUfsPsxc_NopUpiu NopOutUpiu;
	};
} __attribute__((__packed__))XUfsPsxc_Xfer_ReqUpiu;

/**
 * Response UPIU Structure
 */
typedef struct {
	u32 ResidualTransCnt;	/**< Big Endian format */
	u32 Reserved[4];
	u16 SenseDataLen;	/**< Big Endian format */
	u8 SenseData[20];
} __attribute__((__packed__))XUfsPsxc_RespUpiu;

/**
 * UTP Transfer Response Descriptor Structure - 8-byte aligned
 */
typedef struct {
	XUfsPsxc_UpiuHeader UpiuHeader;
	union {
		XUfsPsxc_RespUpiu RespUpiu;
		XUfsPsxc_QueryUpiu QueryRespUpiu;
		XUfsPsxc_NopUpiu NopInUpiu;
	};
} __attribute__((__packed__))XUfsPsxc_Xfer_RespUpiu;

/**
 * Physical Region Descriptor Table - 8-byte aligned
 */
typedef struct {
	u32 BufAddr_Lower;		/* physical address of data block, 4-byte aligned */
	u32 BufAddr_Upper;		/* physical address of data block, 4-byte aligned */
	u32 reserved;
	u32 DataByteCount;	/* [17:0]-Byte Count of data blocks([1:0]-3(fixed, DWORD granularity)) */
} __attribute__((__packed__))XUfsPsxc_Xfer_Prdt;

/**
 * UTP Command request Descriptor Structure - 128-byte aligned
 */
typedef struct {
	XUfsPsxc_Xfer_ReqUpiu ReqUpiu;		/**< Transfer Request UPIU */
	XUfsPsxc_Xfer_RespUpiu RespUpiu __attribute__ ((aligned(8)));	/**< Transfer Response UPIU */
	XUfsPsxc_Xfer_Prdt Prdt[XUFSPSXC_PRDT_ENTRIES] __attribute__ ((aligned(8)));	/**< PRDT - Number of entries restricted to 1 for ROM/PLM use case */

} __attribute__((__packed__))XUfsPsxc_Xfer_CmdDesc;

/**
 * UTP Transfer request Descriptor Structure - 1KB aligned
 */
typedef struct {
	u32 DW0_Config;		/**< [7:0]-CCI, [23]-CE, [24]-I, [26:25]-DD, [31:28]-CT */
	u32 DW1_Dunl;		/**< Data Unit Number Lower 32 bits */
	u32 DW2_Ocs;		/**< [7:0]-Overall Command Status */
	u32 DW3_Dunu;		/**< Data Unit Number Upper 32 bits */
	u32 DW4_UTPCmdDesc_BaseAddressLow;		/**< [31:7]-UTP Command Descriptor BaseAddress */
	u32 DW5_UTPCmdDesc_BaseAddressUpp;		/**< [31:0]-UTP Command Descriptor BaseAddress Upper 32-bits*/
	u32 DW6_RespUpiuInfo;		/**< [31:16]-Resp UPIU Offset, [15:0]-Resp UPIU Length */
	u32 DW7_PrdtInfo;		/**< [31:16]-PRDT Offset, [15:0]-PRDT Length */
} __attribute__((__packed__))XUfsPsxc_Xfer_ReqDesc;

/**
 * UIC Command Structure
 */
typedef struct {
	u8 Command;
	u8 AttrSetType;
	u8 ResetLevel;
	u8 ResultCode;
	u32 MibValue;
	u16 GenSelIndex;
	u16 MibAttribute;
} XUfsPsxc_UicCmd;

/**
 * BootLU information structure
 */
typedef struct {
	u32 LunID;
	u32 BootLunID;
	u32 BlockSize;
	u32 MemoryType;
	u32 NumAllocUnits;
	u64 LUNSize;	/**< Size is in MB */
} XUfsPsxc_BLUNInfo;

/**
 * The XUfsPsxc driver instance data. The user is required to allocate a
 * variable of this type for every ufspsxc device in the system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 */
typedef struct {
	u32 BootLunEn;
	u32 NumOfLuns;
	u32 TestUnitRdyLun;
	u32 AllocUnitSize;
	u32 SegmentSize;
	volatile u32 IsReady;
	u32 CapAdjFactor[7];
	u32 UD0BaseOffset;
	u32 UDLength;
	u32 RxATTCompValL0;
	u32 RxATTCompValL1;
	u32 RxCTLECompValL0;
	u32 RxCTLECompValL1;
	u32 ErrorCode;
	u32 DevBootEn;
	u32 BLunALunId;
	u32 BLunBLunId;
	u32 PowerMode;
	u8 Config_Desc_Data[1024];
	XUfsPsxc_BLUNInfo LUNInfo[32] __attribute__ ((aligned(64)));
	XUfsPsxc_Config Config __attribute__ ((aligned(64)));
	XUfsPsxc_Xfer_ReqDesc req_desc_baseaddr __attribute__ ((aligned(1024)));
	XUfsPsxc_Xfer_CmdDesc CmdDesc __attribute__ ((aligned(128)));
} XUfsPsxc;

/************************** Variable Definitions *****************************/
extern XUfsPsxc_Config XUfsPsxc_ConfigTable[];

/***************** Macros (Inline Functions) Definitions *********************/

/* Error Codes */
/* Bits				[31:24]					[23:16]					[15:0]				*/
/* UIC transfers ->  <UIC_ERROR>				<UIC_CFG/CTRL/PHY/...>	<UIC CFG/CTRL>	*/
/* UPIU Transfers -> <SCSI/QRY/NOP/OCS/...>	<Response/General/OCS>  <ERROR CODES>		*/

/** < 4-bits [15:12] */
enum Error_Bits_15_12 {
	XUFSPSXC_NOP_OUT_ERROR = 1U,
	XUFSPSXC_SCSI_CMD_WRITE_ERROR,
	XUFSPSXC_SCSI_CMD_READ_ERROR,
	XUFSPSXC_QRY_SET_FLAG_ERROR,
	XUFSPSXC_QRY_READ_FLAG_ERROR,
	XUFSPSXC_QRY_READ_CFG_DESC_ERROR,
	XUFSPSXC_QRY_READ_GEOMETRY_DESC_ERROR,
	XUFSPSXC_QRY_READ_DEVICE_DESC_ERROR,
	XUFSPSXC_QRY_WRITE_CFG_DESC_ERROR,
	XUFSPSXC_CMD_TEST_UNIT_RDY_ERROR,
	XUFSPSXC_QRY_READ_ATTR_ERROR1,
	XUFSPSXC_QRY_READ_ATTR_ERROR2,
	XUFSPSXC_QRY_WRITE_ATTR_ERROR,
	XUFSPSXC_UIC_ERROR
};

/** < 4-bits [11:8] */
enum Error_Bits_11_8 {
	XUFSPSXC_UIC_GET_CMD_ERROR = 1U,
	XUFSPSXC_UIC_SET_CMD_ERROR,
	XUFSPSXC_OCS_ERROR,
	XUFSPSXC_GENERAL_ERROR,
	XUFSPSXC_RESPONSE_ERROR,
	XUFSPSXC_RESPONSE_TAG_ERROR,
	XUFSPSXC_STATUS_ERROR,
	XUFSPSXC_DEVICE_CFG_ERROR,
	XUFSPSXC_UIC_LINK_STARTUP_CMD_ERROR
};

/** < UPIU(SCSI) 8-bits [7:0] */
#define XUFSPSXC_SCSI_GOOD					0x0U
#define XUFSPSXC_SCSI_CHK_CONDITION			0x2U
#define XUFSPSXC_SCSI_BUSY					0x8U
#define XUFSPSXC_SCSI_RESV_CONFLICT			0x18U
#define XUFSPSXC_SCSI_TASK_SET_FULL			0x28U

/** < UPIU(QUERY) 8-bits [7:0] */
enum Qry_Error_Bits_7_0 {
	XUFSPSXC_QRY_SUCCESS = 0U,
	XUFSPSXC_QRY_PARAM_NOT_READABLE = 0xF6U,
	XUFSPSXC_QRY_PARAM_NOT_WRITABLE,
	XUFSPSXC_QRY_PARAM_ALRDY_WRITTEN,
	XUFSPSXC_QRY_PARAM_INVALID_LEN,
	XUFSPSXC_QRY_PARAM_INVALID_VAL,
	XUFSPSXC_QRY_PARAM_INVALID_SELECTOR,
	XUFSPSXC_QRY_PARAM_INVALID_INDEX,
	XUFSPSXC_QRY_PARAM_INVALID_IDN,
	XUFSPSXC_QRY_PARAM_INVALID_OPCODE,
	XUFSPSXC_QRY_PARAM_GEN_FAILURE
};

/** < UPIU(OCS) 8-bits [7:0] */
enum Ocs {
	XUFSPSXC_OCS_SUCCESS = 0U,
	XUFSPSXC_OCS_INVLD_CMD_TBL_ATTR,
	XUFSPSXC_OCS_INVLD_PRDT_ATTR,
	XUFSPSXC_OCS_MISMATCH_DATA_BUFF_SZ,
	XUFSPSXC_OCS_MISMATCH_RESP_UPIU_SZ,
	XUFSPSXC_OCS_COMMUNICATION_FAILURE,
	XUFSPSXC_OCS_ABORTED,
	XUFSPSXC_OCS_HOST_FATAL_ERROR,
	XUFSPSXC_OCS_DEV_FATAL_ERROR,
	XUFSPSXC_OCS_INVLD_CRYPTO_CFG,
	XUFSPSXC_OCS_GENERAL_CRYPTO_ERROR,
	XUFSPSXC_OCS_INVLD_OCS_VAL = 0xFU
};

/** < UIC CFG 8-bits [7:0] */
enum Uic_Cfg_Codes {
	XUFSPSXC_INVALID_MIB_ATTRIBUTE = 1U,
	XUFSPSXC_INVALID_MIB_ATTRIBUTE_VALUE,
	XUFSPSXC_READ_ONLY_MIB_ATTRIBUTE,
	XUFSPSXC_WRITE_ONLY_MIB_ATTRIBUTE,
	XUFSPSXC_BAD_INDEX,
	XUFSPSXC_LOCKED_MIB_ATTRIBUTE,
	XUFSPSXC_BAD_TEST_FEATURE_INDEX,
	XUFSPSXC_PEER_COMMUNICATION_FAILURE,
	XUFSPSXC_BUSY,
	XUFSPSXC_DME_FAILURE
};

/** < UIC CTRL 8-bits [7:0] */
enum Uic_Ctrl_Codes {
	XUFSPSXC_UIC_CTRL_SUCCESS = 0U,
	XUFSPSXC_UIC_CTRL_FAILURE
};

/* General Error Codes [7:0] */
enum General_Error_Codes {
	XUFSPSXC_SUCCESS = 0U,
	XUFSPSXC_BOOT_NOT_ENABLED,
	XUFSPSXC_INVALID_BLUN_ENABLED,
	XUFSPSXC_DP_ERROR,
	XUFSPSXC_HCE_ERROR,
	XUFSPSXC_UTRLRDY_ERROR,
	XUFSPSXC_UTRLDBR_ERROR,
	XUFSPSXC_UTRLRSR_ERROR,
	XUFSPSXC_UCRDY_ERROR,
	XUFSPSXC_TASKTAG_NOT_MATCHED,
	XUFSPSXC_INVALID_LUN_CFG,
	XUFSPSXC_MISMATCH_BLUNEN_BLUNID,
	XUFSPSXC_BLUNID1_NOT_CONFIGURED,
	XUFSPSXC_BLUNID2_NOT_CONFIGURED,
	XUFSPSXC_BLUNEN_SET_ERROR,
	XUFSPSXC_UTRLDBR_TIMEOUT,
	XUFSPSXC_TRANSFER_TIMEOUT,
	XUFSPSXC_SRAM_INIT_TIMEOUT,
	XUFSPSXC_UIC_IS_ERROR,
	XUFSPSXC_INVALID_BLKCNT,
	XUFSPSXC_CCS_TIMEOUT,
	XUFSPSXC_CFGRDY_TIMEOUT,
	XUFSPSXC_TX_FSM_TIMEOUT,
	XUFSPSXC_RX_FSM_TIMEOUT,
	XUFSPSXC_INVALID_GEAR_CFG,
	XUFSPSXC_POWER_MODE_TIMEOUT,
	XUFSPSXC_POWER_MODE_FAILURE,
	XUFSPSXC_INVALID_BLUN,
	XUFSPSXC_PHY_INIT_FAILURE,
	XUFSPSXC_LINKUP_ERROR,
	XUFSPSXC_PWMG1_SET_ERROR,
	XUFSPSXC_INVALID_CAPADJ_FACTOR,
	XUFSPSXC_INVALID_UD_PARAMS,
	XUFSPSXC_INVALID_LUNID,
	XUFSPSXC_INVALID_MEMTYPE,
	XUFSPSXC_INVALID_TRANS_CODE,
	XUFSPSXC_PHY_NOT_CALIBRATED,
	XUFSPSXC_FAILURE = 0xFFU
};

enum Hce {
	XUFSPSXC_HCE_DISABLE = 0U,
	XUFSPSXC_HCE_ENABLE
};

enum Card_Status {
	XUFSPSXC_HCS_CARD_INSERTED = 0U,
	XUFSPSXC_HCS_CARD_REMOVED
};

enum memory_types {
	XUFSPSXC_NORM_MEM = 0U,
	XUFSPSXC_SYS_CODE_MEM,
	XUFSPSXC_NON_PERSISTANT_MEM,
	XUFSPSXC_ENHMEM_TYPE1,
	XUFSPSXC_ENHMEM_TYPE2,
	XUFSPSXC_ENHMEM_TYPE3,
	XUFSPSXC_ENHMEM_TYPE4,
};

#define XUFSPSXC_DME_GET_OPCODE				0x1U
#define XUFSPSXC_DME_SET_OPCODE				0x2U
#define XUFSPSXC_UIC_CFG_CMD_MAX_OPCODE		0xFU
#define XUFSPSXC_DME_LINKSTARTUP_OPCODE		0x16U

/* Task Tags */
enum Task_Tag {
	XUFSPSXC_NOP_TASK_TAG = 1U,
	XUFSPSXC_CMD_WRITE_TASK_TAG,
	XUFSPSXC_CMD_READ_TASK_TAG,
	XUFSPSXC_QRY_SET_FLAG_TASK_TAG,
	XUFSPSXC_QRY_READ_FLAG_TASK_TAG,
	XUFSPSXC_QRY_READ_DESC_TASK_TAG,
	XUFSPSXC_QRY_WRITE_DESC_TASK_TAG,
	XUFSPSXC_CMD_TEST_UNIT_RDY_TASK_TAG,
	XUFSPSXC_QRY_READ_ATTR_TASK_TAG,
	XUFSPSXC_QRY_WRITE_ATTR_TASK_TAG
};

/* Transaction Codes */
#define XUFSPSXC_NOP_UPIU_TRANS_CODE		0x0U
#define XUFSPSXC_CMD_UPIU_TRANS_CODE		0x1U
#define XUFSPSXC_QRY_UPIU_TRANS_CODE		0x16U

/* UPIU Flags */
#define XUFSPSXC_UPIU_FLAGS_WRITE		0x20U
#define XUFSPSXC_UPIU_FLAGS_READ		0x40U

/* QUERY function */
#define XUFSPSXC_QRY_READ		0x1U
#define XUFSPSXC_QRY_WRITE		0x81U

/* SCSI Commands */
#define XUFSPSXC_SCSI_TEST_UNIT_RDY_CMD		0x0U
#define XUFSPSXC_SCSI_READ10_CMD			0x28U
#define XUFSPSXC_SCSI_WRITE10_CMD			0x2AU
#define XUFSPSXC_SCSI_READ16_CMD			0x88U
#define XUFSPSXC_SCSI_WRITE16_CMD			0x8AU

/* QUERY Commands */
#define XUFSPSXC_QRY_READ_DESC_CMD			0x1U
#define XUFSPSXC_QRY_WRITE_DESC_CMD			0x2U
#define XUFSPSXC_QRY_READ_ATTR_CMD			0x3U
#define XUFSPSXC_QRY_WRITE_ATTR_CMD			0x4U
#define XUFSPSXC_QRY_READ_FLAG_CMD			0x5U
#define XUFSPSXC_QRY_SET_FLAG_CMD			0x6U

/* Descriptor IDN */
#define XUFSPSXC_DEVICE_DESC_IDN			0x0U
#define XUFSPSXC_CONFIG_DESC_IDN			0x1U
#define XUFSPSXC_UNIT_DESC_IDN				0x2U
#define XUFSPSXC_GEOMETRY_DESC_IDN			0x7U

#define XUFSPSXC_DEVICE_DESC_BOOTEN_LEN		0x9U
#define XUFSPSXC_DEVICE_DESC_REQ_LEN		0x1CU
#define XUFSPSXC_DEVICE_BOOTEN_OFFSET		0x8U
#define XUFSPSXC_UD0_BASE_OFFSET			0x1AU
#define XUFSPSXC_UD_LEN_OFFSET				0x1BU

#define XUFSPSXC_GEOMETRY_DESC_LEN			0x57U

#define XUFSPSXC_CFG_DESC_LEN(InstancePtr)						(((InstancePtr)->UDLength * 8U) + (InstancePtr)->UD0BaseOffset)
#define XUFSPSXC_CFG_DESC_HEADER_LEN(InstancePtr)				(InstancePtr)->UD0BaseOffset
#define XUFSPSXC_CFG_DESC_UNITDESC_LEN(InstancePtr)				(InstancePtr)->UDLength
#define XUFSPSXC_CFG_DESC_CONT_OFFSET							0x2U
#define XUFSPSXC_CFG_DESC_OFFSET(InstancePtr, Index)			((Index) * XUFSPSXC_CFG_DESC_LEN(InstancePtr))
#define XUFSPSXC_LU_ENABLE_OFFSET(InstancePtr, LUIndex)		(XUFSPSXC_CFG_DESC_HEADER_LEN(InstancePtr) + ((LUIndex) * XUFSPSXC_CFG_DESC_UNITDESC_LEN(InstancePtr)))
#define XUFSPSXC_BLUNEN_OFFSET(InstancePtr, LUIndex)		(XUFSPSXC_CFG_DESC_HEADER_LEN(InstancePtr) + ((LUIndex) * XUFSPSXC_CFG_DESC_UNITDESC_LEN(InstancePtr)) + 1U)
#define XUFSPSXC_BLKSZ_OFFSET(InstancePtr, LUIndex)			(XUFSPSXC_CFG_DESC_HEADER_LEN(InstancePtr) + ((LUIndex) * XUFSPSXC_CFG_DESC_UNITDESC_LEN(InstancePtr)) + 9U)
#define XUFSPSXC_MEMTYPE_OFFSET(InstancePtr, LUIndex)		(XUFSPSXC_CFG_DESC_HEADER_LEN(InstancePtr) + ((LUIndex) * XUFSPSXC_CFG_DESC_UNITDESC_LEN(InstancePtr)) + 3U)
#define XUFSPSXC_NUM_ALLOC_OFFSET(InstancePtr, LUIndex)		(XUFSPSXC_CFG_DESC_HEADER_LEN(InstancePtr) + ((LUIndex) * XUFSPSXC_CFG_DESC_UNITDESC_LEN(InstancePtr)) + 4U)

#define XUFSPSXC_MAX_CDB_LEN				16U

#define XUFSPSXC_FDEVINIT_FLAG_IDN			0x1U

/* Geometry Descriptor offsets */
#define XUFSPSXC_MAXNUMLU_OFFSET			0xCU
#define XUFSPSXC_SEGSZ_OFFSET				0xDU
#define XUFSPSXC_ALLOCSZ_OFFSET				0x11U
#define XUFSPSXC_SYSCODE_CAPADJ_OFFSET		0x24U
#define XUFSPSXC_NONPERS_CAPADJ_OFFSET		0x2AU
#define XUFSPSXC_ENH1_CAPADJ_OFFSET			0x30U
#define XUFSPSXC_ENH2_CAPADJ_OFFSET			0x36U
#define XUFSPSXC_ENH3_CAPADJ_OFFSET			0x3CU
#define XUFSPSXC_ENH4_CAPADJ_OFFSET			0x42U

#define XUFSPSXC_BLUN_A			0x1U
#define XUFSPSXC_BLUN_B			0x2U
#define XUFSPSXC_LU_ENABLE		0x1U
#define XUFSPSXC_LU_BLKSZ_4K	4096U
#define XUFSPSXC_BLUNEN_ATTRID	0x0U
#define XUFSPSXC_WRITE			0x0U
#define XUFSPSXC_READ			0x1U

/* UTP Transfer Request Descriptor definitions */
#define XUFSPSXC_CT_UFS_STORAGE_MASK		0x10000000U
#define XUFSPSXC_INTERRUPT_CMD_MASK			0x1000000U
#define XUFSPSXC_DD_DEV_TO_MEM_MASK			0x4000000U
#define XUFSPSXC_DD_MEM_TO_DEV_MASK			0x2000000U

#define XUFSPSXC_LBA_OVER_32BIT			0x100000000U
#define XUFSPSXC_TL_OVER_16BIT			0x10000U
#define XUFSPSXC_256KB					0x40000U

#define XUFSPSXC_INVALID_LUN_ID				0xFFU

#define XUFSPSXC_BLUN_ID		0xB0U

#define XUFSPSXC_PWM_G1		0x2201U
#define XUFSPSXC_PWM_G2		0x2202U
#define XUFSPSXC_PWM_G3		0x2203U
#define XUFSPSXC_PWM_G4		0x2204U
/* RATE-A */
#define XUFSPSXC_HS_G1		0x11101U
#define XUFSPSXC_HS_G2		0x11102U
#define XUFSPSXC_HS_G3		0x11103U
#define XUFSPSXC_HS_G4		0x11104U

/* RATE-B */
#define XUFSPSXC_HS_G1_B	0x21101U
#define XUFSPSXC_HS_G2_B	0x21102U
#define XUFSPSXC_HS_G3_B	0x21103U
#define XUFSPSXC_HS_G4_B	0x21104U

#define XUFSPSXC_TX_RX_FAST		0x11U
#define XUFSPSXC_TX_RX_SLOW		0x22U

#define XUFSPSXC_HSSERIES_A		1U
#define XUFSPSXC_HSSERIES_B		2U

#define XUFSPSXC_GEAR4			4U

#define XUFSPSXC_PWR_OK			0U
#define XUFSPSXC_PWR_LOCAL		1U
#define XUFSPSXC_PWR_ERR_CAP	4U
#define XUFSPSXC_PWR_FATAL_ERR	5U

#define XUFSPSXC_PWR_MODE_VAL	0x100U

#define XUFSPSXC_CLK_SEL_19P2	19200000U
#define XUFSPSXC_CLK_SEL_26		26000000U
#define XUFSPSXC_CLK_SEL_38P4	38400000U
#define XUFSPSXC_CLK_SEL_52		52000000U

static INLINE void XUfsPsxc_FillUpiuHeader(XUfsPsxc_Xfer_CmdDesc *CmdDescPtr, u8 TransType, u32 Upiu_Dw0, u8 QueryTaskMangFn, u16 DataSegmentLen)
{
	CmdDescPtr->ReqUpiu.UpiuHeader.TransactionType = TransType;
	CmdDescPtr->ReqUpiu.UpiuHeader.Flags = (u8)(Upiu_Dw0 >> 8U);
	CmdDescPtr->ReqUpiu.UpiuHeader.TaskTag = (u8)(Upiu_Dw0 >> 24U);
	CmdDescPtr->ReqUpiu.UpiuHeader.Query_Task_Mang_Fn = QueryTaskMangFn;
	CmdDescPtr->ReqUpiu.UpiuHeader.DataSegmentLen = Xil_EndianSwap16(DataSegmentLen);
}

static INLINE void XUfsPsxc_FillCmdUpiu(const XUfsPsxc *InstancePtr, XUfsPsxc_Xfer_CmdDesc *CmdDescPtr, u32 BlkCnt, u8 Cmd, u64 Address)
{
	u32 BigEndianAddr;
	u32 BigEndianBlkCnt;
	u32 LunId;

	if (InstancePtr->CmdDesc.ReqUpiu.UpiuHeader.LUN == XUFSPSXC_BLUN_ID) {
		if (InstancePtr->BootLunEn == XUFSPSXC_BLUN_A) {
			LunId = InstancePtr->BLunALunId;
		} else {
			LunId = InstancePtr->BLunBLunId;
		}
	} else {
		LunId = InstancePtr->CmdDesc.ReqUpiu.UpiuHeader.LUN;
	}

	CmdDescPtr->ReqUpiu.CmdUpiu.ExpDataXferLen = Xil_EndianSwap32(InstancePtr->LUNInfo[LunId].BlockSize * BlkCnt);

	if (BlkCnt != 0U) {	/* Data Read Command */
		if ((Address >= XUFSPSXC_LBA_OVER_32BIT) || (BlkCnt >= XUFSPSXC_TL_OVER_16BIT)) {	/* READ(16)/WRITE(16) Command */
			CmdDescPtr->ReqUpiu.CmdUpiu.scsi_cdb[0] = Cmd;
			BigEndianAddr =  Xil_EndianSwap32((u32)(Address >> 32));
			CmdDescPtr->ReqUpiu.CmdUpiu.scsi_cdb[2] = (u8)BigEndianAddr;
			CmdDescPtr->ReqUpiu.CmdUpiu.scsi_cdb[3] = (u8)(BigEndianAddr >> 8);
			CmdDescPtr->ReqUpiu.CmdUpiu.scsi_cdb[4] = (u8)(BigEndianAddr >> 16);
			CmdDescPtr->ReqUpiu.CmdUpiu.scsi_cdb[5] = (u8)(BigEndianAddr >> 24);
			BigEndianAddr =  Xil_EndianSwap32((u32)Address);
			CmdDescPtr->ReqUpiu.CmdUpiu.scsi_cdb[6] = (u8)BigEndianAddr;
			CmdDescPtr->ReqUpiu.CmdUpiu.scsi_cdb[7] = (u8)(BigEndianAddr >> 8);
			CmdDescPtr->ReqUpiu.CmdUpiu.scsi_cdb[8] = (u8)(BigEndianAddr >> 16);
			CmdDescPtr->ReqUpiu.CmdUpiu.scsi_cdb[9] = (u8)(BigEndianAddr >> 24);
			BigEndianBlkCnt = Xil_EndianSwap32(BlkCnt);
			CmdDescPtr->ReqUpiu.CmdUpiu.scsi_cdb[10] = (u8)BigEndianBlkCnt;
			CmdDescPtr->ReqUpiu.CmdUpiu.scsi_cdb[11] = (u8)(BigEndianBlkCnt >> 8);
			CmdDescPtr->ReqUpiu.CmdUpiu.scsi_cdb[12] = (u8)(BigEndianBlkCnt >> 16);
			CmdDescPtr->ReqUpiu.CmdUpiu.scsi_cdb[13] = (u8)(BigEndianBlkCnt >> 24);
		} else {	/* READ(10)/WRITE(10) Command */
			CmdDescPtr->ReqUpiu.CmdUpiu.scsi_cdb[0] = Cmd;
			BigEndianAddr = Xil_EndianSwap32((u32)Address);
			CmdDescPtr->ReqUpiu.CmdUpiu.scsi_cdb[2] = (u8)BigEndianAddr;
			CmdDescPtr->ReqUpiu.CmdUpiu.scsi_cdb[3] = (u8)(BigEndianAddr >> 8);
			CmdDescPtr->ReqUpiu.CmdUpiu.scsi_cdb[4] = (u8)(BigEndianAddr >> 16);
			CmdDescPtr->ReqUpiu.CmdUpiu.scsi_cdb[5] = (u8)(BigEndianAddr >> 24);
			BigEndianBlkCnt = Xil_EndianSwap16((u16)BlkCnt);
			CmdDescPtr->ReqUpiu.CmdUpiu.scsi_cdb[7] = (u8)BigEndianBlkCnt;
			CmdDescPtr->ReqUpiu.CmdUpiu.scsi_cdb[8] = (u8)(BigEndianBlkCnt >> 8);
		}
	} else {	/* TEST UNIT READY command */
		CmdDescPtr->ReqUpiu.CmdUpiu.scsi_cdb[0] = Cmd;
	}
}

static INLINE void XUfsPsxc_FillQryReqUpiu(XUfsPsxc_Xfer_CmdDesc *CmdDescPtr, u8 Cmd, u32 Tsf_Dw0, u32 Value, u16 Length)
{
	CmdDescPtr->ReqUpiu.QueryReqUpiu.Tsf.Opcode = Cmd;
	CmdDescPtr->ReqUpiu.QueryReqUpiu.Tsf.DescId = (u8)(Tsf_Dw0 >> 8U);
	CmdDescPtr->ReqUpiu.QueryReqUpiu.Tsf.Index = (u8)(Tsf_Dw0 >> 16U);
	CmdDescPtr->ReqUpiu.QueryReqUpiu.Tsf.Selector = (u8)(Tsf_Dw0 >> 24U);
	CmdDescPtr->ReqUpiu.QueryReqUpiu.Tsf.Value = Xil_EndianSwap32(Value);
	CmdDescPtr->ReqUpiu.QueryReqUpiu.Tsf.Length = Xil_EndianSwap16(Length);
}

static INLINE void XUfsPsxc_FillUTPTransReqDesc(XUfsPsxc *InstancePtr, const XUfsPsxc_Xfer_CmdDesc *CmdDescPtr, u32 DataDirection, u32 RespUpiuLen, u32 PrdtLen)
{
	u32 Dw0;
	u32 PrdtOffset;
	u32 RespUpiuOffset;
	u32 RespUpiuInfo;
	u32 PrdtInfo;

	Dw0 = (XUFSPSXC_CT_UFS_STORAGE_MASK | DataDirection | XUFSPSXC_INTERRUPT_CMD_MASK);
	RespUpiuOffset = (u32)((UINTPTR)&CmdDescPtr->RespUpiu - (UINTPTR)CmdDescPtr);
	RespUpiuInfo = (u32)((RespUpiuOffset >> 2U) << 16U) | (u32)((sizeof(CmdDescPtr->RespUpiu.UpiuHeader) + RespUpiuLen) >> 2U);
	if (PrdtLen != 0U) {
		PrdtOffset = (u32)((UINTPTR)&CmdDescPtr->Prdt - (UINTPTR)CmdDescPtr);
		PrdtInfo = (u32)((PrdtOffset >> 2U) << 16U) | (u32)PrdtLen;
	} else {
		PrdtInfo = 0U;
	}

	InstancePtr->req_desc_baseaddr.DW0_Config = Dw0;
	InstancePtr->req_desc_baseaddr.DW1_Dunl = 0U;
	InstancePtr->req_desc_baseaddr.DW3_Dunu = 0U;
	InstancePtr->req_desc_baseaddr.DW2_Ocs = 0xFU;	/* 0xF - Invalid OCS */
	InstancePtr->req_desc_baseaddr.DW4_UTPCmdDesc_BaseAddressLow = (u32)(UINTPTR)CmdDescPtr;
	InstancePtr->req_desc_baseaddr.DW5_UTPCmdDesc_BaseAddressUpp = 0U;
	InstancePtr->req_desc_baseaddr.DW6_RespUpiuInfo = RespUpiuInfo;
	InstancePtr->req_desc_baseaddr.DW7_PrdtInfo = PrdtInfo;
}

static INLINE void XUfsPsxc_FillUICCmd(XUfsPsxc_UicCmd *UicCmdPtr, u32 MIBAttr_GenSel, u32 MIBVal, u32 AttrSetType, u32 Cmd)
{
	UicCmdPtr->Command = (u8)Cmd;
	UicCmdPtr->MibAttribute = (u16)(MIBAttr_GenSel >> 16U);
	UicCmdPtr->GenSelIndex = (u16)MIBAttr_GenSel;
	UicCmdPtr->AttrSetType = (u8)AttrSetType;
	UicCmdPtr->MibValue = MIBVal;
}

/************************** Function Prototypes ******************************/

XUfsPsxc_Config *XUfsPsxc_LookupConfig(UINTPTR BaseAddress);
void XUfsPsxc_CfgInitialize(XUfsPsxc *InstancePtr,
							const XUfsPsxc_Config *ConfigPtr);
u32 XUfsPsxc_Initialize(XUfsPsxc *InstancePtr);
u32 XUfsPsxc_ReadPolled(XUfsPsxc *InstancePtr, u32 Lun, u64 Address, u32 BlkCnt,
							const u8 *Buff);
u32 XUfsPsxc_WritePolled(XUfsPsxc *InstancePtr, u32 Lun, u64 Address, u32 BlkCnt,
							const u8 *Buff);
u32 XUfsPsxc_CheckBootReq(XUfsPsxc *InstancePtr);
u32 XUfsPsxc_ConfigureSpeedGear(XUfsPsxc *InstancePtr, u32 SpeedGear);
u32 XUfsPsxc_SwitchBootLUN(XUfsPsxc *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* XUFSPSXC_H_ */
/** @} */
