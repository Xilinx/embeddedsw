/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XUFSPSXC_H_ /* prevent circular inclusions */
#define XUFSPSXC_H_

/*****************************************************************************/
/**
*
* @file xufspsxc.h
* @addtogroup ufspsxc Overview
* @{
* @brief Defines the UFSPSXC driver software API.
*
* This section contains information about the driver structures, user
* API prototypes and all the defines required to interact
* with the driver. ufspsxc driver supports the UFS controller in Versal 2VE and
* 2VM SoC platforms.
*
* <b>Initialization & Configuration</b>
*
* <b>Supported features</b>
*
* This driver supports following features
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
* 1.3   an  04/09/26 Fix doxygen warnings, Fix stale PHY adapt-type
*                    Add bRefClkFreq attribute programming
*
* 1.3   tb  09/16/26 Align Doxygen comments with AMD standards.
* </pre>
*
******************************************************************************/

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
/** Number of PRDT entries in the command descriptor. */
#define XUFSPSXC_PRDT_ENTRIES	1U
#else
/** Number of PRDT entries in the command descriptor. */
#define XUFSPSXC_PRDT_ENTRIES	4096U
#endif

/** Maximum number of logical units. */
#define XUFSPSXC_MAX_LUNS		32U

/**
 * @brief Holds UFSPSXC device configuration data.
 *
 * This typedef contains configuration information for the device.
 */
typedef struct {
	char *Name;		/**< Compatible string of the device. */
	UINTPTR BaseAddress;	/**< Base address of the device. */
	u32 InputClockHz;	/**< Input clock frequency. */
	/** M-PHY Config clock frequency (external clock). */
	u32 CfgClkFreqHz;
	u32 RefPadClk;		/**< UFS Reference Pad clock. */
	u8 IsCacheCoherent;	/**< If UFS is Cache Coherent or not. */
	/**
	 * Bits[11:0] interrupt ID. Bits[15:12] trigger type and level flags.
	 */
	u16 IntrId;
	/** Bit[0] interrupt parent type. Bit[64/32:1] parent base address. */
	UINTPTR IntrParent;
} XUfsPsxc_Config;

/**
 * @brief Holds a UPIU header.
 */
typedef struct {
	u8 TransactionType;	/**< UPIU type (CMD/QUERY/NOP/etc.). */
	u8 Flags;		/**< Read/Write direction flags. */
	u8 LUN;			/**< Target Logical Unit Number. */
	u8 TaskTag;		/**< Tag for request/response matching. */
	u8 IID_Cmd_Type;	/**< Initiator ID and command type. */
	u8 Query_Task_Mang_Fn;	/**< Query or task management function. */
	u8 Response;		/**< Device response code. */
	u8 Status;		/**< Command completion status. */
	u8 TotalEHSLen;		/**< Extended Header Segment length. */
	u8 DeviceInfo;		/**< Device-specific information. */
	u16 DataSegmentLen;	/**< Data segment length (Big Endian). */
} __attribute__((__packed__))XUfsPsxc_UpiuHeader;

/**
 * @brief Holds a SCSI command UPIU payload.
 *
 * Command UPIU Structure
 */
typedef struct {
	/** Expected data transfer length (bytes). */
	u32 ExpDataXferLen;
	u8 scsi_cdb[16U];	/**< SCSI Command Descriptor Block. */
} __attribute__((__packed__))XUfsPsxc_CmdUpiu;

/**
 * @brief Holds query transaction-specific fields.
 *
 * Transaction Specific Fields Structure
 */
typedef struct {
	u8 Opcode;	/**< Query operation opcode. */
	u8 DescId;	/**< Descriptor identification value. */
	u8 Index;	/**< Descriptor or attribute index. */
	u8 Selector;	/**< Descriptor or attribute selector. */
	u16 Reserved0;	/**< Reserved, must be zero. */
	u16 Length;	/**< Data transfer length (Big Endian). */
	u32 Value;	/**< Attribute value (Big Endian). */
	u32 Reserved1;	/**< Reserved, must be zero. */
} __attribute__((__packed__))XUfsPsxc_TransSpecFlds;

/**
 * @brief Holds a Query UPIU payload.
 *
 * Query UPIU Structure
 */
typedef struct {
	XUfsPsxc_TransSpecFlds Tsf; /**< Query transaction-specific fields. */
	u32 Reserved;	/**< Reserved, must be zero. */
	u8 Data[256U];	/**< Descriptor or attribute data payload. */
} __attribute__((__packed__))XUfsPsxc_QueryUpiu;

/**
 * @brief Holds a NOP IN or NOP OUT UPIU payload.
 *
 * NOP IN/OUT UPIU Structure
 */
typedef struct {
	u8 Reserved[20];	/**< Reserved, must be zero. */
} __attribute__((__packed__))XUfsPsxc_NopUpiu;

/**
 * @brief Holds a UTP transfer request UPIU.
 *
 * UTP Transfer Request Descriptor Structure
 */
typedef struct {
	XUfsPsxc_UpiuHeader UpiuHeader;	/**< Common UPIU header. */
	/** Request UPIU payload (one of CMD/Query/NOP) */
	union {
		XUfsPsxc_CmdUpiu CmdUpiu;	/**< SCSI command payload. */
		/** Query request payload. */
		XUfsPsxc_QueryUpiu QueryReqUpiu;
		XUfsPsxc_NopUpiu NopOutUpiu;	/**< NOP OUT payload. */
	};
} __attribute__((__packed__))XUfsPsxc_Xfer_ReqUpiu;

/**
 * @brief Holds a SCSI command response UPIU.
 *
 * Response UPIU Structure
 */
typedef struct {
	/** Residual transfer count (Big Endian). */
	u32 ResidualTransCnt;
	u32 Reserved[4];	/**< Reserved, must be zero. */
	u16 SenseDataLen;	/**< Sense data length (Big Endian). */
	u8 SenseData[20];	/**< Sense data from device. */
} __attribute__((__packed__))XUfsPsxc_RespUpiu;

/**
 * @brief Holds a UTP transfer response UPIU.
 *
 * UTP Transfer Response Descriptor Structure - 8-byte aligned
 */
typedef struct {
	/** Common UPIU header. */
	XUfsPsxc_UpiuHeader UpiuHeader;
	/** Response UPIU payload (one of Resp/Query/NOP) */
	union {
		/** SCSI command response. */
		XUfsPsxc_RespUpiu RespUpiu;
		/** Query response payload. */
		XUfsPsxc_QueryUpiu QueryRespUpiu;
		XUfsPsxc_NopUpiu NopInUpiu;	/**< NOP IN payload. */
	};
} __attribute__((__packed__))XUfsPsxc_Xfer_RespUpiu;

/**
 * @brief Holds a physical region descriptor table entry.
 *
 * Physical Region Descriptor Table - 8-byte aligned
 */
typedef struct {
	/** Data buffer physical address lower 32 bits. */
	u32 BufAddr_Lower;
	/** Data buffer physical address upper 32 bits. */
	u32 BufAddr_Upper;
	u32 reserved;		/**< Reserved. */
	u32 DataByteCount;	/**< [17:0] Byte count, DWORD granularity. */
} __attribute__((__packed__))XUfsPsxc_Xfer_Prdt;

/**
 * @brief Holds a UTP command request descriptor.
 *
 * UTP Command request Descriptor Structure - 128-byte aligned
 */
typedef struct {
	/** Transfer Request UPIU. */
	XUfsPsxc_Xfer_ReqUpiu ReqUpiu;
	/** Transfer Response UPIU. */
	XUfsPsxc_Xfer_RespUpiu RespUpiu __attribute__ ((aligned(8)));
	XUfsPsxc_Xfer_Prdt Prdt[XUFSPSXC_PRDT_ENTRIES] __attribute__ ((aligned(8)));	/**< PRDT - Number of entries restricted to 1 for ROM/PLM use case */

} __attribute__((__packed__))XUfsPsxc_Xfer_CmdDesc;

/**
 * @brief Holds a UTP transfer request descriptor.
 *
 * UTP Transfer request Descriptor Structure - 1KB aligned
 */
typedef struct {
	/** [7:0]-CCI, [23]-CE, [24]-I, [26:25]-DD, [31:28]-CT. */
	u32 DW0_Config;
	u32 DW1_Dunl;		/**< Data Unit Number Lower 32 bits. */
	u32 DW2_Ocs;		/**< [7:0]-Overall Command Status. */
	u32 DW3_Dunu;		/**< Data Unit Number Upper 32 bits. */
	/** [31:7]-UTP Command Descriptor BaseAddress. */
	u32 DW4_UTPCmdDesc_BaseAddressLow;
	/** [31:0]-UTP Command Descriptor BaseAddress Upper 32-bits. */
	u32 DW5_UTPCmdDesc_BaseAddressUpp;
	/** [31:16]-Resp UPIU Offset, [15:0]-Resp UPIU Length. */
	u32 DW6_RespUpiuInfo;
	/** [31:16]-PRDT Offset, [15:0]-PRDT Length. */
	u32 DW7_PrdtInfo;
} __attribute__((__packed__))XUfsPsxc_Xfer_ReqDesc;

/**
 * @brief Holds a UIC command.
 *
 * UIC Command Structure
 */
typedef struct {
	/** UIC command opcode (DME_GET, DME_SET, DME_LINKSTARTUP, etc.). */
	u8 Command;
	/** Attribute set type for DME operations (Local/Peer/etc.). */
	u8 AttrSetType;
	u8 ResetLevel;	/**< Reset level for reset operations. */
	/** Result code returned after command execution. */
	u8 ResultCode;
	u32 MibValue;	/**< MIB attribute value for GET/SET operations. */
	/** Generic selector index for addressing specific instances. */
	u16 GenSelIndex;
	/** MIB attribute identifier being accessed. */
	u16 MibAttribute;
} XUfsPsxc_UicCmd;

/**
 * @brief Holds boot logical-unit information.
 *
 * BootLU information structure
 */
typedef struct {
	u32 LunID;	/**< Logical Unit Number identifier (0-31). */
	u32 BootLunID;	/**< Boot LUN identifier (BLUN_A=1, BLUN_B=2). */
	/** Block size in bytes (typically 4096 for UFS). */
	u32 BlockSize;
	/** Memory type (Normal, System Code, Enhanced, etc.). */
	u32 MemoryType;
	/** Number of allocation units assigned to this LU. */
	u32 NumAllocUnits;
	u64 LUNSize;	/**< Total size of the Logical Unit in MB. */
} XUfsPsxc_BLUNInfo;

/**
 * @brief Holds instance data for a UFSPSXC device.
 *
 * Allocate a variable of this type for every UFSPSXC device in the
 * system. Pass a pointer to that variable to the driver API functions.
 */
typedef struct {
	/** Boot LUN enable status (BLUN_A=1, BLUN_B=2, disabled=0). */
	u32 BootLunEn;
	/** Total number of Logical Units supported by the device. */
	u32 NumOfLuns;
	/** LUN ID used for Test Unit Ready command during initialization. */
	u32 TestUnitRdyLun;
	/** Allocation unit size in bytes (from Geometry descriptor). */
	u32 AllocUnitSize;
	/** Segment size in bytes (from Geometry descriptor). */
	u32 SegmentSize;
	/** Total raw device capacity in MiB (from Geometry descriptor). */
	u64 DeviceCapacityMiB;
	/** Device ready status flag (1=ready, 0=not ready). */
	volatile u32 IsReady;
	/**
	 * Capacity adjustment factors per memory type. [0]=Normal,
	 * [1]=System Code, [2]=Non-Persistent. [3-6]=Enhanced Types 1-4.
	 */
	u32 CapAdjFactor[7];
	/** Base offset for Unit Descriptor 0 in Configuration descriptor. */
	u32 UD0BaseOffset;
	u32 UDLength;	/**< Length of each Unit Descriptor in bytes. */
	/** RX ATT compensation value for Lane 0 (from calibration). */
	u32 RxATTCompValL0;
	/** RX ATT compensation value for Lane 1 (from calibration). */
	u32 RxATTCompValL1;
	/** RX CTLE compensation value for Lane 0 (from calibration). */
	u32 RxCTLECompValL0;
	/** RX CTLE compensation value for Lane 1 (from calibration). */
	u32 RxCTLECompValL1;
	/** Last error code encountered during operations. */
	u32 ErrorCode;
	/** Device boot enable flag from Device descriptor. */
	u32 DevBootEn;
	u32 BLunALunId;	/**< LUN ID mapped to Boot LUN A. */
	u32 BLunBLunId;	/**< LUN ID mapped to Boot LUN B. */
	/** Current power mode setting (PWM/HS with gear). */
	u32 PowerMode;
	/** Configuration descriptor data buffer. */
	u8 Config_Desc_Data[1024];
	XUfsPsxc_BLUNInfo LUNInfo[XUFSPSXC_MAX_LUNS] __attribute__ ((aligned(64)));	/**< Information for all supported LUNs */
	/** Device configuration structure. */
	XUfsPsxc_Config Config __attribute__ ((aligned(64)));
	/** Transfer request descriptor. */
	XUfsPsxc_Xfer_ReqDesc req_desc_baseaddr __attribute__ ((aligned(1024)));
	/** Command descriptor for transfers. */
	XUfsPsxc_Xfer_CmdDesc CmdDesc __attribute__ ((aligned(128)));
} XUfsPsxc;

/************************** Variable Definitions *****************************/
extern XUfsPsxc_Config XUfsPsxc_ConfigTable[];

/***************** Macros (Inline Functions) Definitions *********************/

/* Error Codes */
/* Bits [31:24] | [23:16] | [15:0]. */
/* UIC transfers: <UIC_ERROR> | <UIC_CFG/CTRL/PHY/...> | <UIC CFG/CTRL>. */
/* UPIU transfers: <SCSI/QRY/NOP/OCS/...> | <Response/General/OCS> | */
/* <ERROR CODES>. */

/**
 * @brief Lists the error codes in bits [15:12] of the driver status.
 */
enum Error_Bits_15_12 {
	XUFSPSXC_NOP_OUT_ERROR = 1U, /**< NOP OUT UPIU transaction failed. */
	XUFSPSXC_SCSI_CMD_WRITE_ERROR, /**< SCSI WRITE command failed. */
	XUFSPSXC_SCSI_CMD_READ_ERROR, /**< SCSI READ command failed. */
	XUFSPSXC_QRY_SET_FLAG_ERROR, /**< Query SET FLAG command failed. */
	XUFSPSXC_QRY_READ_FLAG_ERROR, /**< Query READ FLAG command failed. */
	/** Query read of the configuration descriptor failed. */
	XUFSPSXC_QRY_READ_CFG_DESC_ERROR,
	/** Query read of the geometry descriptor failed. */
	XUFSPSXC_QRY_READ_GEOMETRY_DESC_ERROR,
	/** Query read of the device descriptor failed. */
	XUFSPSXC_QRY_READ_DEVICE_DESC_ERROR,
	/** Query write of the configuration descriptor failed. */
	XUFSPSXC_QRY_WRITE_CFG_DESC_ERROR,
	/** SCSI TEST UNIT READY command failed. */
	XUFSPSXC_CMD_TEST_UNIT_RDY_ERROR,
	/** Query read attribute failed (first stage). */
	XUFSPSXC_QRY_READ_ATTR_ERROR1,
	/** Query read attribute failed (second stage). */
	XUFSPSXC_QRY_READ_ATTR_ERROR2,
	XUFSPSXC_QRY_WRITE_ATTR_ERROR, /**< Query write attribute failed. */
	XUFSPSXC_UIC_ERROR /**< UIC command failed. */
};

/**
 * @brief Lists the error codes in bits [11:8] of the driver status.
 */
enum Error_Bits_11_8 {
	XUFSPSXC_UIC_GET_CMD_ERROR = 1U, /**< UIC DME GET command failed. */
	XUFSPSXC_UIC_SET_CMD_ERROR, /**< UIC DME SET command failed. */
	XUFSPSXC_OCS_ERROR, /**< UTP overall command status error. */
	XUFSPSXC_GENERAL_ERROR, /**< General driver error. */
	XUFSPSXC_RESPONSE_ERROR, /**< UPIU response error. */
	XUFSPSXC_RESPONSE_TAG_ERROR, /**< UPIU response task-tag mismatch. */
	XUFSPSXC_STATUS_ERROR, /**< SCSI/UPIU status error. */
	XUFSPSXC_DEVICE_CFG_ERROR, /**< Device configuration error. */
	/** UIC link-startup command failed. */
	XUFSPSXC_UIC_LINK_STARTUP_CMD_ERROR
};

/**
 * @name SCSI Status Codes
 * @brief Defines SCSI command status codes in UPIU responses (bits [7:0]).
 * @{
 */
/** SCSI command completed successfully. */
#define XUFSPSXC_SCSI_GOOD					0x0U
/** SCSI check condition - sense data available. */
#define XUFSPSXC_SCSI_CHK_CONDITION			0x2U
/** SCSI device busy - retry later. */
#define XUFSPSXC_SCSI_BUSY					0x8U
/** SCSI reservation conflict. */
#define XUFSPSXC_SCSI_RESV_CONFLICT			0x18U
/** SCSI task set full - no more commands. */
#define XUFSPSXC_SCSI_TASK_SET_FULL			0x28U
/** @} */

/**
 * @brief Lists the Query UPIU error codes in bits [7:0].
 */
enum Qry_Error_Bits_7_0 {
	XUFSPSXC_QRY_SUCCESS = 0U, /**< Query request succeeded. */
	/** Query parameter is not readable. */
	XUFSPSXC_QRY_PARAM_NOT_READABLE = 0xF6U,
	/** Query parameter is not writable. */
	XUFSPSXC_QRY_PARAM_NOT_WRITABLE,
	/** Query parameter already written. */
	XUFSPSXC_QRY_PARAM_ALRDY_WRITTEN,
	/** Query parameter length is invalid. */
	XUFSPSXC_QRY_PARAM_INVALID_LEN,
	/** Query parameter value is invalid. */
	XUFSPSXC_QRY_PARAM_INVALID_VAL,
	/** Query parameter selector is invalid. */
	XUFSPSXC_QRY_PARAM_INVALID_SELECTOR,
	/** Query parameter index is invalid. */
	XUFSPSXC_QRY_PARAM_INVALID_INDEX,
	XUFSPSXC_QRY_PARAM_INVALID_IDN, /**< Query parameter IDN is invalid. */
	XUFSPSXC_QRY_PARAM_INVALID_OPCODE, /**< Query opcode is invalid. */
	XUFSPSXC_QRY_PARAM_GEN_FAILURE /**< Query parameter general failure. */
};

/**
 * @brief Lists the UTP Overall Command Status codes.
 */
enum Ocs {
	XUFSPSXC_OCS_SUCCESS = 0U, /**< OCS success. */
	/** Invalid command-table attributes. */
	XUFSPSXC_OCS_INVLD_CMD_TBL_ATTR,
	XUFSPSXC_OCS_INVLD_PRDT_ATTR, /**< Invalid PRDT attributes. */
	XUFSPSXC_OCS_MISMATCH_DATA_BUFF_SZ, /**< Data-buffer size mismatch. */
	XUFSPSXC_OCS_MISMATCH_RESP_UPIU_SZ, /**< Response UPIU size mismatch. */
	/** Host-device communication failure. */
	XUFSPSXC_OCS_COMMUNICATION_FAILURE,
	XUFSPSXC_OCS_ABORTED, /**< Command aborted. */
	XUFSPSXC_OCS_HOST_FATAL_ERROR, /**< Host fatal error. */
	XUFSPSXC_OCS_DEV_FATAL_ERROR, /**< Device fatal error. */
	XUFSPSXC_OCS_INVLD_CRYPTO_CFG, /**< Invalid crypto configuration. */
	XUFSPSXC_OCS_GENERAL_CRYPTO_ERROR, /**< General crypto error. */
	XUFSPSXC_OCS_INVLD_OCS_VAL = 0xFU /**< Invalid OCS value. */
};

/**
 * @brief Lists the UIC configuration result codes.
 */
enum Uic_Cfg_Codes {
	XUFSPSXC_INVALID_MIB_ATTRIBUTE = 1U, /**< Invalid MIB attribute. */
	/** Invalid MIB attribute value. */
	XUFSPSXC_INVALID_MIB_ATTRIBUTE_VALUE,
	XUFSPSXC_READ_ONLY_MIB_ATTRIBUTE, /**< Read-only MIB attribute. */
	XUFSPSXC_WRITE_ONLY_MIB_ATTRIBUTE, /**< Write-only MIB attribute. */
	XUFSPSXC_BAD_INDEX, /**< Bad index. */
	XUFSPSXC_LOCKED_MIB_ATTRIBUTE, /**< Locked MIB attribute. */
	XUFSPSXC_BAD_TEST_FEATURE_INDEX, /**< Bad test-feature index. */
	XUFSPSXC_PEER_COMMUNICATION_FAILURE, /**< Peer communication failure. */
	XUFSPSXC_BUSY, /**< Busy. */
	/** DME (device management entity) request failed. */
	XUFSPSXC_DME_FAILURE
};

/**
 * @brief Lists the UIC control result codes.
 */
enum Uic_Ctrl_Codes {
	XUFSPSXC_UIC_CTRL_SUCCESS = 0U, /**< UIC control success. */
	XUFSPSXC_UIC_CTRL_FAILURE /**< UIC control failure. */
};

/**
 * @brief Lists the general driver error codes in bits [7:0].
 */
enum General_Error_Codes {
	XUFSPSXC_SUCCESS = 0U, /**< Success. */
	XUFSPSXC_BOOT_NOT_ENABLED, /**< Boot not enabled. */
	XUFSPSXC_INVALID_BLUN_ENABLED, /**< Invalid boot LUN enable value. */
	XUFSPSXC_DP_ERROR, /**< Device-present error. */
	XUFSPSXC_HCE_ERROR, /**< Host-controller enable error. */
	XUFSPSXC_UTRLRDY_ERROR, /**< UTRL ready error. */
	XUFSPSXC_UTRLDBR_ERROR, /**< UTRL doorbell error. */
	XUFSPSXC_UTRLRSR_ERROR, /**< UTRL run-stop error. */
	XUFSPSXC_UCRDY_ERROR, /**< UIC command ready error. */
	XUFSPSXC_TASKTAG_NOT_MATCHED, /**< Task tag not matched. */
	XUFSPSXC_INVALID_LUN_CFG, /**< Invalid LUN configuration. */
	/** Boot-LUN enable does not match Boot-LUN ID. */
	XUFSPSXC_MISMATCH_BLUNEN_BLUNID,
	/** Boot LUN ID 1 is not configured. */
	XUFSPSXC_BLUNID1_NOT_CONFIGURED,
	/** Boot LUN ID 2 is not configured. */
	XUFSPSXC_BLUNID2_NOT_CONFIGURED,
	XUFSPSXC_BLUNEN_SET_ERROR, /**< Boot-LUN enable set error. */
	XUFSPSXC_UTRLDBR_TIMEOUT, /**< UTRL doorbell timeout. */
	XUFSPSXC_TRANSFER_TIMEOUT, /**< Transfer timeout. */
	XUFSPSXC_SRAM_INIT_TIMEOUT, /**< SRAM init timeout. */
	XUFSPSXC_UIC_IS_ERROR, /**< UIC interrupt-status error. */
	XUFSPSXC_INVALID_BLKCNT, /**< Invalid block count. */
	XUFSPSXC_CCS_TIMEOUT, /**< CCS timeout. */
	XUFSPSXC_CFGRDY_TIMEOUT, /**< Config-ready timeout. */
	XUFSPSXC_TX_FSM_TIMEOUT, /**< TX FSM timeout. */
	XUFSPSXC_RX_FSM_TIMEOUT, /**< RX FSM timeout. */
	XUFSPSXC_INVALID_GEAR_CFG, /**< Invalid gear configuration. */
	XUFSPSXC_POWER_MODE_TIMEOUT, /**< Power-mode timeout. */
	XUFSPSXC_POWER_MODE_FAILURE, /**< Power-mode failure. */
	XUFSPSXC_INVALID_BLUN, /**< Invalid boot LUN. */
	XUFSPSXC_PHY_INIT_FAILURE, /**< PHY initialization failure. */
	XUFSPSXC_LINKUP_ERROR, /**< Linkup error. */
	XUFSPSXC_PWMG1_SET_ERROR, /**< Pwmg1 set error. */
	XUFSPSXC_INVALID_CAPADJ_FACTOR, /**< Invalid capadj factor. */
	XUFSPSXC_INVALID_UD_PARAMS, /**< Invalid ud params. */
	XUFSPSXC_INVALID_LUNID, /**< Invalid lunid. */
	XUFSPSXC_INVALID_MEMTYPE, /**< Invalid memtype. */
	XUFSPSXC_INVALID_TRANS_CODE, /**< Invalid trans code. */
	XUFSPSXC_PHY_NOT_CALIBRATED, /**< PHY not calibrated. */
	XUFSPSXC_INVALID_REFCLKFREQ, /**< Invalid refclkfreq. */
	XUFSPSXC_FAILURE = 0xFFU /**< Failure. */
};

/**
 * @brief Lists the Host Controller Enable values.
 */
enum Hce {
	XUFSPSXC_HCE_DISABLE = 0U, /**< Host controller disable. */
	XUFSPSXC_HCE_ENABLE /**< Host controller enable. */
};

/**
 * @brief Lists the card present status values.
 */
enum Card_Status {
	XUFSPSXC_HCS_CARD_INSERTED = 0U, /**< Hcs card inserted. */
	XUFSPSXC_HCS_CARD_REMOVED /**< Hcs card removed. */
};

/**
 * @brief Lists the UFS logical-unit memory types.
 */
enum memory_types {
	XUFSPSXC_NORM_MEM = 0U, /**< Normal memory. */
	XUFSPSXC_SYS_CODE_MEM, /**< System-code memory. */
	XUFSPSXC_NON_PERSISTANT_MEM, /**< Non-persistent memory. */
	XUFSPSXC_ENHMEM_TYPE1, /**< Enhanced memory type 1. */
	XUFSPSXC_ENHMEM_TYPE2, /**< Enhanced memory type 2. */
	XUFSPSXC_ENHMEM_TYPE3, /**< Enhanced memory type 3. */
	XUFSPSXC_ENHMEM_TYPE4, /**< Enhanced memory type 4. */
};

/**
 * @name UIC Command Opcodes
 * @brief Defines DME (Data Management Entity) opcodes for UIC operations.
 * @{
 */
/** DME GET command to read MIB attributes. */
#define XUFSPSXC_DME_GET_OPCODE				0x1U
/** DME SET command to write MIB attributes. */
#define XUFSPSXC_DME_SET_OPCODE				0x2U
/** Maximum opcode value for UIC configuration commands. */
#define XUFSPSXC_UIC_CFG_CMD_MAX_OPCODE		0xFU
/** DME LINKSTARTUP command to establish link. */
#define XUFSPSXC_DME_LINKSTARTUP_OPCODE		0x16U
/** @} */

/**
 * @brief Lists the UPIU task tag identifiers.
 */
enum Task_Tag {
	XUFSPSXC_NOP_TASK_TAG = 1U, /**< NOP task tag. */
	XUFSPSXC_CMD_WRITE_TASK_TAG, /**< Command WRITE task tag. */
	XUFSPSXC_CMD_READ_TASK_TAG, /**< Command READ task tag. */
	XUFSPSXC_QRY_SET_FLAG_TASK_TAG, /**< Query SET FLAG task tag. */
	XUFSPSXC_QRY_READ_FLAG_TASK_TAG, /**< Query READ FLAG task tag. */
	XUFSPSXC_QRY_READ_DESC_TASK_TAG, /**< Query read descriptor task tag. */
	/** Query write descriptor task tag. */
	XUFSPSXC_QRY_WRITE_DESC_TASK_TAG,
	/** Command TEST UNIT READY task tag. */
	XUFSPSXC_CMD_TEST_UNIT_RDY_TASK_TAG,
	XUFSPSXC_QRY_READ_ATTR_TASK_TAG, /**< Query read attribute task tag. */
	XUFSPSXC_QRY_WRITE_ATTR_TASK_TAG /**< Query write attribute task tag. */
};

/**
 * @name UPIU Transaction Codes
 * @brief Defines the transaction type codes for different UPIU types.
 * @{
 */
/** Transaction code for NOP (No Operation) UPIUs. */
#define XUFSPSXC_NOP_UPIU_TRANS_CODE		0x0U
/** Transaction code for Command UPIUs (SCSI commands). */
#define XUFSPSXC_CMD_UPIU_TRANS_CODE		0x1U
/** Transaction code for Query UPIUs. */
#define XUFSPSXC_QRY_UPIU_TRANS_CODE		0x16U
/** @} */

/**
 * @name UPIU Direction Flags
 * @brief Defines the flags that indicate data transfer direction in UPIUs.
 * @{
 */
/** UPIU flag for write (host to device) operations. */
#define XUFSPSXC_UPIU_FLAGS_WRITE		0x20U
/** UPIU flag for read (device to host) operations. */
#define XUFSPSXC_UPIU_FLAGS_READ		0x40U
/** @} */

/**
 * @name Query Operation Types
 * @brief Defines the operation types for UFS Query UPIUs.
 * @{
 */
/** Query operation: Read descriptor/attribute/flag. */
#define XUFSPSXC_QRY_READ		0x1U
/** Query operation: Write descriptor/attribute/flag. */
#define XUFSPSXC_QRY_WRITE		0x81U
/** @} */

/**
 * @name SCSI Command Codes
 * @brief Defines the SCSI command opcodes used in UFS storage operations.
 * @{
 */
/** SCSI Test Unit Ready command. */
#define XUFSPSXC_SCSI_TEST_UNIT_RDY_CMD		0x0U
/** SCSI Read(10) command for 32-bit LBA. */
#define XUFSPSXC_SCSI_READ10_CMD			0x28U
/** SCSI Write(10) command for 32-bit LBA. */
#define XUFSPSXC_SCSI_WRITE10_CMD			0x2AU
/** SCSI Read(16) command for 64-bit LBA. */
#define XUFSPSXC_SCSI_READ16_CMD			0x88U
/** SCSI Write(16) command for 64-bit LBA. */
#define XUFSPSXC_SCSI_WRITE16_CMD			0x8AU
/** @} */

/**
 * @name Query Command Opcodes
 * @brief Defines the command opcodes for UFS Query operations.
 * @{
 */
/** Query command: Read Descriptor. */
#define XUFSPSXC_QRY_READ_DESC_CMD			0x1U
/** Query command: Write Descriptor. */
#define XUFSPSXC_QRY_WRITE_DESC_CMD			0x2U
/** Query command: Read Attribute. */
#define XUFSPSXC_QRY_READ_ATTR_CMD			0x3U
/** Query command: Write Attribute. */
#define XUFSPSXC_QRY_WRITE_ATTR_CMD			0x4U
/** Query command: Read Flag. */
#define XUFSPSXC_QRY_READ_FLAG_CMD			0x5U
/** Query command: Set Flag. */
#define XUFSPSXC_QRY_SET_FLAG_CMD			0x6U
/** @} */

/**
 * @name UFS Descriptor Identifiers (IDN)
 * @brief Defines the identifier values for different UFS descriptor types.
 * @{
 */
/** Device descriptor identifier. */
#define XUFSPSXC_DEVICE_DESC_IDN			0x0U
/** Configuration descriptor identifier. */
#define XUFSPSXC_CONFIG_DESC_IDN			0x1U
/** Unit descriptor identifier. */
#define XUFSPSXC_UNIT_DESC_IDN				0x2U
/** Geometry descriptor identifier. */
#define XUFSPSXC_GEOMETRY_DESC_IDN			0x7U
/** @} */

/**
 * @name Device Descriptor Constants
 * @brief Defines the lengths and offsets for the device descriptor.
 * @{
 */
/** Length to read for boot enable field. */
#define XUFSPSXC_DEVICE_DESC_BOOTEN_LEN		0x9U
/** Required minimum device descriptor length. */
#define XUFSPSXC_DEVICE_DESC_REQ_LEN		0x1CU
/** Offset of boot enable field in device descriptor. */
#define XUFSPSXC_DEVICE_BOOTEN_OFFSET		0x8U
/** @} */

/**
 * @name Unit Descriptor Constants
 * @brief Defines offsets for unit descriptor fields in the device descriptor.
 * @{
 */
/** Offset of Unit Descriptor 0 base address field. */
#define XUFSPSXC_UD0_BASE_OFFSET			0x1AU
/** Offset of Unit Descriptor length field. */
#define XUFSPSXC_UD_LEN_OFFSET				0x1BU
/** @} */

/**
 * @name Geometry Descriptor Constants and Field Offsets
 * @brief Defines the geometry descriptor length and field byte offsets.
 * @{
 */
/** Total length of geometry descriptor. */
#define XUFSPSXC_GEOMETRY_DESC_LEN			0x57U
/** Offset of total raw device capacity. */
#define XUFSPSXC_DEV_CAPACITY_OFFSET		0x4U
/** qTotalRawDeviceCapacity field width in bytes per UFS spec. */
#define XUFSPSXC_DEV_CAPACITY_BYTES			0x8U
/** Offset of maximum number of LUs field. */
#define XUFSPSXC_MAXNUMLU_OFFSET			0xCU
/** Offset of segment size field. */
#define XUFSPSXC_SEGSZ_OFFSET				0xDU
/** Offset of allocation size field. */
#define XUFSPSXC_ALLOCSZ_OFFSET				0x11U
/** Offset of system code capability adjustment factor. */
#define XUFSPSXC_SYSCODE_CAPADJ_OFFSET		0x24U
/** Offset of non-persistent capability adjustment factor. */
#define XUFSPSXC_NONPERS_CAPADJ_OFFSET		0x2AU
/** Offset of enhanced 1 capability adjustment factor. */
#define XUFSPSXC_ENH1_CAPADJ_OFFSET			0x30U
/** Offset of enhanced type 2 capacity adjustment factor. */
#define XUFSPSXC_ENH2_CAPADJ_OFFSET			0x36U
/** Offset of enhanced type 3 capacity adjustment factor. */
#define XUFSPSXC_ENH3_CAPADJ_OFFSET			0x3CU
/** Offset of enhanced type 4 capacity adjustment factor. */
#define XUFSPSXC_ENH4_CAPADJ_OFFSET			0x42U
/** @} */

/**
 * @name Configuration Descriptor Calculation Macros
 * @brief Defines macros for sizes and offsets in the configuration descriptor.
 * @{
 */
/** Calculate total configuration descriptor length based on unit descriptors */
#define XUFSPSXC_CFG_DESC_LEN(InstancePtr)						(((InstancePtr)->UDLength * 8U) + (InstancePtr)->UD0BaseOffset)
/** Get configuration descriptor header length */
#define XUFSPSXC_CFG_DESC_HEADER_LEN(InstancePtr)				(InstancePtr)->UD0BaseOffset
/** Get unit descriptor length */
#define XUFSPSXC_CFG_DESC_UNITDESC_LEN(InstancePtr)				(InstancePtr)->UDLength
/** Configuration descriptor continuation offset */
#define XUFSPSXC_CFG_DESC_CONT_OFFSET							0x2U
/** Calculate offset for a specific configuration descriptor index */
#define XUFSPSXC_CFG_DESC_OFFSET(InstancePtr, Index)			((Index) * XUFSPSXC_CFG_DESC_LEN(InstancePtr))
/** Calculate offset for LUN enable field in unit descriptor */
#define XUFSPSXC_LU_ENABLE_OFFSET(InstancePtr, LUIndex)		(XUFSPSXC_CFG_DESC_HEADER_LEN(InstancePtr) + ((LUIndex) * XUFSPSXC_CFG_DESC_UNITDESC_LEN(InstancePtr)))
/** Calculate offset for Boot LUN enable field in unit descriptor */
#define XUFSPSXC_BLUNEN_OFFSET(InstancePtr, LUIndex)		(XUFSPSXC_CFG_DESC_HEADER_LEN(InstancePtr) + ((LUIndex) * XUFSPSXC_CFG_DESC_UNITDESC_LEN(InstancePtr)) + 1U)
/** Calculate offset for block size field in unit descriptor */
#define XUFSPSXC_BLKSZ_OFFSET(InstancePtr, LUIndex)			(XUFSPSXC_CFG_DESC_HEADER_LEN(InstancePtr) + ((LUIndex) * XUFSPSXC_CFG_DESC_UNITDESC_LEN(InstancePtr)) + 9U)
/** Calculate offset for memory type field in unit descriptor */
#define XUFSPSXC_MEMTYPE_OFFSET(InstancePtr, LUIndex)		(XUFSPSXC_CFG_DESC_HEADER_LEN(InstancePtr) + ((LUIndex) * XUFSPSXC_CFG_DESC_UNITDESC_LEN(InstancePtr)) + 3U)
/** Calculate offset for number of allocation units field in unit descriptor */
#define XUFSPSXC_NUM_ALLOC_OFFSET(InstancePtr, LUIndex)		(XUFSPSXC_CFG_DESC_HEADER_LEN(InstancePtr) + ((LUIndex) * XUFSPSXC_CFG_DESC_UNITDESC_LEN(InstancePtr)) + 4U)
/** @} */

/**
 * @name SCSI and UFS Constants
 * @brief Defines constants for SCSI and UFS operations.
 * @{
 */
/** Maximum SCSI Command Descriptor Block length. */
#define XUFSPSXC_MAX_CDB_LEN				16U
/** Device initialization flag identifier. */
#define XUFSPSXC_FDEVINIT_FLAG_IDN			0x1U
/** @} */

/**
 * @name LUN and Boot Configuration Constants
 * @brief Defines constants for LUN addressing, boot LUN and logical units.
 * @{
 */
/** Boot LUN A identifier. */
#define XUFSPSXC_BLUN_A			0x1U
/** Boot LUN B identifier. */
#define XUFSPSXC_BLUN_B			0x2U
/** Logical Unit enable value. */
#define XUFSPSXC_LU_ENABLE		0x1U
/** Standard 4KB block size for LU. */
#define XUFSPSXC_LU_BLKSZ_4K	4096U
/** Boot LUN enable attribute identifier. */
#define XUFSPSXC_BLUNEN_ATTRID	0x0U
/** Write operation identifier. */
#define XUFSPSXC_WRITE			0x0U
/** Read operation identifier. */
#define XUFSPSXC_READ			0x1U
/** Invalid LUN identifier. */
#define XUFSPSXC_INVALID_LUN_ID	0xFFU
/** LUN ID value used for Boot LUN identification. */
#define XUFSPSXC_BLUN_ID		0xB0U
/** @} */

/**
 * @name UTP Transfer Request Descriptor Bit Masks
 * @brief Defines bit masks for UTP Transfer Request Descriptor configuration.
 * @{
 */
/** Command type: UFS storage command. */
#define XUFSPSXC_CT_UFS_STORAGE_MASK		0x10000000U
/** Interrupt enable mask for command completion. */
#define XUFSPSXC_INTERRUPT_CMD_MASK			0x1000000U
/** Data direction: Device to Memory (read). */
#define XUFSPSXC_DD_DEV_TO_MEM_MASK			0x4000000U
/** Data direction: Memory to Device (write). */
#define XUFSPSXC_DD_MEM_TO_DEV_MASK			0x2000000U
/** @} */

/**
 * @name Data Transfer Size Limits
 * @brief Defines limits for data transfers and addressing.
 * @{
 */
/** LBA value that exceeds 32-bit addressing, requires READ(16)/WRITE(16). */
#define XUFSPSXC_LBA_OVER_32BIT			0x100000000U
/** Transfer length that exceeds 16-bit limit, requires READ(16)/WRITE(16). */
#define XUFSPSXC_TL_OVER_16BIT			0x10000U
/** 256KB size constant. */
#define XUFSPSXC_256KB					0x40000U
/** @} */

/**
 * @name PWM Speed Gear Modes
 * @brief Defines PWM (Pulse Width Modulation) speed gear configuration values.
 * @{
 */
#define XUFSPSXC_PWM_G1		0x2201U	/**< PWM Gear 1. */
#define XUFSPSXC_PWM_G2		0x2202U	/**< PWM Gear 2. */
#define XUFSPSXC_PWM_G3		0x2203U	/**< PWM Gear 3. */
#define XUFSPSXC_PWM_G4		0x2204U	/**< PWM Gear 4. */
/** @} */

/**
 * @name High Speed (HS) Gear Modes - Rate A
 * @brief Defines High Speed mode configuration values for Rate A series.
 * @{
 */
/** High Speed Gear 1 Rate A. */
#define XUFSPSXC_HS_G1		0x11101U
/** High Speed Gear 2 Rate A. */
#define XUFSPSXC_HS_G2		0x11102U
/** High Speed Gear 3 Rate A. */
#define XUFSPSXC_HS_G3		0x11103U
/** High Speed Gear 4 Rate A. */
#define XUFSPSXC_HS_G4		0x11104U
/** @} */

/**
 * @name High Speed (HS) Gear Modes - Rate B
 * @brief Defines High Speed mode configuration values for Rate B series.
 * @{
 */
/** High Speed Gear 1 Rate B. */
#define XUFSPSXC_HS_G1_B	0x21101U
/** High Speed Gear 2 Rate B. */
#define XUFSPSXC_HS_G2_B	0x21102U
/** High Speed Gear 3 Rate B. */
#define XUFSPSXC_HS_G3_B	0x21103U
/** High Speed Gear 4 Rate B. */
#define XUFSPSXC_HS_G4_B	0x21104U
/** @} */

/**
 * @name PHY Speed Configuration
 * @brief Defines PHY layer speed configuration constants.
 * @{
 */
/** Fast TX/RX speed configuration. */
#define XUFSPSXC_TX_RX_FAST		0x11U
/** Slow TX/RX speed configuration. */
#define XUFSPSXC_TX_RX_SLOW		0x22U
/** High Speed Series A. */
#define XUFSPSXC_HSSERIES_A		1U
/** High Speed Series B. */
#define XUFSPSXC_HSSERIES_B		2U
/** Gear 4 speed configuration. */
#define XUFSPSXC_GEAR4			4U
/** @} */

/**
 * @name Power Mode Status Codes
 * @brief Defines the status codes for power mode change operations.
 * @{
 */
/** Power mode change successful. */
#define XUFSPSXC_PWR_OK			0U
/** Power mode change local error. */
#define XUFSPSXC_PWR_LOCAL		1U
/** Power mode change capability error. */
#define XUFSPSXC_PWR_ERR_CAP	4U
/** Power mode change fatal error. */
#define XUFSPSXC_PWR_FATAL_ERR	5U
/** @} */

/**
 * @name Power Mode Configuration
 * @brief Defines power mode configuration constants.
 * @{
 */
/** Power mode configuration value. */
#define XUFSPSXC_PWR_MODE_VAL	0x100U
/** @} */

/**
 * @name Clock Selection Values
 * @brief Defines reference clock frequency values for UFS PHY configuration.
 * @{
 */
/** 19.2 MHz reference clock. */
#define XUFSPSXC_CLK_SEL_19P2	19200000U
/** 26 MHz reference clock. */
#define XUFSPSXC_CLK_SEL_26		26000000U
/** 38.4 MHz reference clock. */
#define XUFSPSXC_CLK_SEL_38P4	38400000U
/** 52 MHz reference clock. */
#define XUFSPSXC_CLK_SEL_52		52000000U
/** @} */

/** @name Adapt type values for PA_TxHsAdaptType attribute
 * @{
 */
#define XUFSPSXC_PA_REFRESH_ADAPT	0x00U	/**< Refresh Adapt. */
#define XUFSPSXC_PA_INITIAL_ADAPT	0x01U	/**< Initial Adapt. */
#define XUFSPSXC_PA_NO_ADAPT		0x03U	/**< No Adapt. */
/** @} */

/** @name bRefClkFreq attribute values
 * @{
 */
/** bRefClkFreq attribute IDN. */
#define XUFSPSXC_REF_CLK_FREQ_ATTRID			0x0AU
/** 19.2 MHz. */
#define XUFSPSXC_ATTR_VAL_REF_CLK_FREQ_19_2_MHZ	0U
#define XUFSPSXC_ATTR_VAL_REF_CLK_FREQ_26_MHZ	1U	/**< 26 MHz. */
/** 38.4 MHz. */
#define XUFSPSXC_ATTR_VAL_REF_CLK_FREQ_38_4_MHZ	2U
#define XUFSPSXC_ATTR_VAL_REF_CLK_FREQ_52_MHZ	3U	/**< 52 MHz. */
/** Invalid frequency. */
#define XUFSPSXC_ATTR_VAL_REF_CLK_FREQ_INVAL	0xFFU
/** @} */

/*****************************************************************************/
/**
 * @brief Checks whether the given LUN identifier is valid for the device.
 *
 * @param[in]	InstancePtr Pointer to the UFS controller instance.
 * @param[in]	LunId LUN identifier to validate.
 *
 * @return TRUE if the LUN is within the device's LUN count, else FALSE.
 *
 *****************************************************************************/
static INLINE u32 XUfsPsxc_IsValidLun(const XUfsPsxc *InstancePtr, u32 LunId)
{
	return (u32)(LunId < InstancePtr->NumOfLuns);
}

/*****************************************************************************/
/**
 * @brief Checks whether the RX compensation values show an uncalibrated PHY.
 *
 * @param[in]	InstancePtr Pointer to the UFS controller instance.
 *
 * @return 1U if the PHY is uncalibrated, otherwise 0U.
 *
 *****************************************************************************/
static INLINE u32 XUfsPsxc_IsPhyUncalibrated(const XUfsPsxc *InstancePtr)
{
	return (u32)(((InstancePtr->RxCTLECompValL0 == (u32)0x0U) || (InstancePtr->RxCTLECompValL0 == (u32)0xFFU))
		&& ((InstancePtr->RxATTCompValL1 == (u32)0x0U) || (InstancePtr->RxATTCompValL1 == (u32)0xFFU))
		&& ((InstancePtr->RxATTCompValL0 == (u32)0x0U) || (InstancePtr->RxATTCompValL0 == (u32)0xFFU))
		&& ((InstancePtr->RxCTLECompValL1 == (u32)0x0U) || (InstancePtr->RxCTLECompValL1 == (u32)0xFFU)));
}

/*****************************************************************************/
/**
 * @brief Flushes the request UPIU (and PRDT) from cache by transaction type.
 *
 * @param[in]	CmdDescPtr Pointer to the Command Descriptor.
 *
 * @return
 *		- XUFSPSXC_SUCCESS if successful.
 *		- Error code for an invalid transaction type.
 *
 *****************************************************************************/
static INLINE u32 XUfsPsxc_FlushReqUpiuCache(const XUfsPsxc_Xfer_CmdDesc *CmdDescPtr)
{
	volatile u32 Status = (u32)XUFSPSXC_SUCCESS;

	Xil_DCacheFlushRange((INTPTR)&CmdDescPtr->ReqUpiu.UpiuHeader, (INTPTR)sizeof(CmdDescPtr->ReqUpiu.UpiuHeader));
	if (CmdDescPtr->ReqUpiu.UpiuHeader.TransactionType == XUFSPSXC_NOP_UPIU_TRANS_CODE) {
		Xil_DCacheFlushRange((INTPTR)&CmdDescPtr->ReqUpiu.NopOutUpiu, (INTPTR)sizeof(CmdDescPtr->ReqUpiu.NopOutUpiu));
	} else if (CmdDescPtr->ReqUpiu.UpiuHeader.TransactionType == XUFSPSXC_CMD_UPIU_TRANS_CODE) {
		Xil_DCacheFlushRange((INTPTR)&CmdDescPtr->ReqUpiu.CmdUpiu, (INTPTR)sizeof(CmdDescPtr->ReqUpiu.CmdUpiu));
		Xil_DCacheFlushRange((INTPTR)&CmdDescPtr->Prdt, (INTPTR)sizeof(CmdDescPtr->Prdt));
	} else if (CmdDescPtr->ReqUpiu.UpiuHeader.TransactionType == XUFSPSXC_QRY_UPIU_TRANS_CODE) {
		Xil_DCacheFlushRange((INTPTR)&CmdDescPtr->ReqUpiu.QueryReqUpiu, (INTPTR)sizeof(CmdDescPtr->ReqUpiu.QueryReqUpiu));
		Xil_DCacheFlushRange((INTPTR)&CmdDescPtr->ReqUpiu.QueryReqUpiu.Tsf, (INTPTR)sizeof(CmdDescPtr->ReqUpiu.QueryReqUpiu.Tsf));
	} else {
		Status = ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_INVALID_TRANS_CODE;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief Invalidates the response UPIU and transfer descriptor from cache.
 *
 * @param[in]	InstancePtr Pointer to the UFS controller instance.
 * @param[in]	CmdDescPtr Pointer to the Command Descriptor.
 *
 * @return
 *		- XUFSPSXC_SUCCESS if successful.
 *		- Error code for an invalid transaction type.
 *
 *****************************************************************************/
static INLINE u32 XUfsPsxc_InvalidateRespUpiuCache(const XUfsPsxc *InstancePtr, const XUfsPsxc_Xfer_CmdDesc *CmdDescPtr)
{
	volatile u32 Status = (u32)XUFSPSXC_SUCCESS;

	Xil_DCacheInvalidateRange((INTPTR)&CmdDescPtr->RespUpiu.UpiuHeader, (INTPTR)sizeof(CmdDescPtr->RespUpiu.UpiuHeader));
	if (CmdDescPtr->ReqUpiu.UpiuHeader.TransactionType == XUFSPSXC_NOP_UPIU_TRANS_CODE) {
		Xil_DCacheInvalidateRange((INTPTR)&CmdDescPtr->RespUpiu.NopInUpiu, (INTPTR)sizeof(CmdDescPtr->RespUpiu.NopInUpiu));
	} else if (CmdDescPtr->ReqUpiu.UpiuHeader.TransactionType == XUFSPSXC_CMD_UPIU_TRANS_CODE) {
		Xil_DCacheInvalidateRange((INTPTR)&CmdDescPtr->RespUpiu.RespUpiu, (INTPTR)sizeof(CmdDescPtr->RespUpiu.RespUpiu));
	} else if (CmdDescPtr->ReqUpiu.UpiuHeader.TransactionType == XUFSPSXC_QRY_UPIU_TRANS_CODE) {
		Xil_DCacheInvalidateRange((INTPTR)&CmdDescPtr->RespUpiu.QueryRespUpiu, (INTPTR)sizeof(CmdDescPtr->RespUpiu.QueryRespUpiu));
		Xil_DCacheInvalidateRange((INTPTR)&CmdDescPtr->RespUpiu.QueryRespUpiu.Tsf, (INTPTR)sizeof(CmdDescPtr->RespUpiu.QueryRespUpiu.Tsf));
	} else {
		Status = ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_INVALID_TRANS_CODE;
	}

	if (Status == (u32)XUFSPSXC_SUCCESS) {
		Xil_DCacheInvalidateRange((INTPTR)&InstancePtr->req_desc_baseaddr, (INTPTR)sizeof(InstancePtr->req_desc_baseaddr));
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief Fills the UPIU header fields.
 *
 * @param[out]	CmdDescPtr Pointer to the Command Descriptor.
 * @param[in]   	TransType Transaction type code.
 * @param[in]   	Upiu_Dw0 TaskTag[31:24] and Flags[15:8].
 * @param[in]   	QueryTaskMangFn Query or task management function.
 * @param[in]   	DataSegmentLen Data segment length.
 *
 *****************************************************************************/
static INLINE void XUfsPsxc_FillUpiuHeader(XUfsPsxc_Xfer_CmdDesc *CmdDescPtr, u8 TransType, u32 Upiu_Dw0, u8 QueryTaskMangFn, u16 DataSegmentLen)
{
	CmdDescPtr->ReqUpiu.UpiuHeader.TransactionType = TransType;
	CmdDescPtr->ReqUpiu.UpiuHeader.Flags = (u8)(Upiu_Dw0 >> 8U);
	CmdDescPtr->ReqUpiu.UpiuHeader.TaskTag = (u8)(Upiu_Dw0 >> 24U);
	CmdDescPtr->ReqUpiu.UpiuHeader.Query_Task_Mang_Fn = QueryTaskMangFn;
	CmdDescPtr->ReqUpiu.UpiuHeader.DataSegmentLen = Xil_EndianSwap16(DataSegmentLen);
}

/*****************************************************************************/
/**
 * @brief Fills the SCSI command UPIU.
 *
 * @param[in]	    InstancePtr Pointer to the UFS controller instance.
 * @param[out]	CmdDescPtr Pointer to the Command Descriptor.
 * @param[in]	    BlkCnt Number of blocks to transfer.
 * @param[in]	    Cmd SCSI command opcode.
 * @param[in]	    Address Starting Logical Block Address.
 *
 *****************************************************************************/
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

/*****************************************************************************/
/**
 * @brief Fills the Query Request UPIU for descriptor and attribute operations.
 *
 * @param[out]	CmdDescPtr Pointer to the Command Descriptor.
 * @param[in]   	Cmd Query operation command.
 * @param[in]   	Tsf_Dw0 Selector[31:24], Index[23:16], DescId[15:8].
 * @param[in]   	Value Attribute value.
 * @param[in]   	Length Expected data length.
 *
 *****************************************************************************/
static INLINE void XUfsPsxc_FillQryReqUpiu(XUfsPsxc_Xfer_CmdDesc *CmdDescPtr, u8 Cmd, u32 Tsf_Dw0, u32 Value, u16 Length)
{
	CmdDescPtr->ReqUpiu.QueryReqUpiu.Tsf.Opcode = Cmd;
	CmdDescPtr->ReqUpiu.QueryReqUpiu.Tsf.DescId = (u8)(Tsf_Dw0 >> 8U);
	CmdDescPtr->ReqUpiu.QueryReqUpiu.Tsf.Index = (u8)(Tsf_Dw0 >> 16U);
	CmdDescPtr->ReqUpiu.QueryReqUpiu.Tsf.Selector = (u8)(Tsf_Dw0 >> 24U);
	CmdDescPtr->ReqUpiu.QueryReqUpiu.Tsf.Value = Xil_EndianSwap32(Value);
	CmdDescPtr->ReqUpiu.QueryReqUpiu.Tsf.Length = Xil_EndianSwap16(Length);
}

/*****************************************************************************/
/**
 * @brief Fills the UTP Transfer Request Descriptor.
 *
 * @param[out]	    InstancePtr Pointer to the UFS controller instance.
 * @param[in]	CmdDescPtr Pointer to the Command Descriptor.
 * @param[in]	    DataDirection Transfer direction.
 * @param[in]    	RespUpiuLen Expected response UPIU length.
 * @param[in]    	PrdtLen Number of PRDT entries.
 *
 *****************************************************************************/
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

/*****************************************************************************/
/**
 * @brief Fills the UIC Command structure for UniPro/M-PHY operations.
 *
 * @param[out]	UicCmdPtr Pointer to the UIC command structure.
 * @param[in]	    MIBAttr_GenSel MibAttribute[31:16] and
 *		GenSelIndex[15:0].
 * @param[in]	    MIBVal MIB attribute value.
 * @param[in]	    AttrSetType Attribute set type.
 * @param[in]	    Cmd UIC command opcode.
 *
 *****************************************************************************/
static INLINE void XUfsPsxc_FillUICCmd(XUfsPsxc_UicCmd *UicCmdPtr, u32 MIBAttr_GenSel, u32 MIBVal, u32 AttrSetType, u32 Cmd)
{
	UicCmdPtr->Command = (u8)Cmd;
	UicCmdPtr->MibAttribute = (u16)(MIBAttr_GenSel >> 16U);
	UicCmdPtr->GenSelIndex = (u16)MIBAttr_GenSel;
	UicCmdPtr->AttrSetType = (u8)AttrSetType;
	UicCmdPtr->MibValue = MIBVal;
}

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
 * @brief Looks up the device configuration by base address.
 *
 *****************************************************************************/
XUfsPsxc_Config *XUfsPsxc_LookupConfig(UINTPTR BaseAddress);
/* Initialize driver instance from configuration. */
void XUfsPsxc_CfgInitialize(XUfsPsxc *InstancePtr,
							const XUfsPsxc_Config *ConfigPtr);
/* Initialize host controller and UFS device. */
u32 XUfsPsxc_Initialize(XUfsPsxc *InstancePtr);
/* Read data from UFS device in polled mode. */
u32 XUfsPsxc_ReadPolled(XUfsPsxc *InstancePtr, u32 Lun, u64 Address, u32 BlkCnt,
							const u8 *Buff);
/* Write data to UFS device in polled mode. */
u32 XUfsPsxc_WritePolled(XUfsPsxc *InstancePtr, u32 Lun, u64 Address, u32 BlkCnt,
							const u8 *Buff);
/* Check and configure boot LUN requirements. */
u32 XUfsPsxc_CheckBootReq(XUfsPsxc *InstancePtr);
/* Configure UFS PHY speed gear and power mode. */
u32 XUfsPsxc_ConfigureSpeedGear(XUfsPsxc *InstancePtr, u32 SpeedGear);
/* Switch active boot LUN. */
u32 XUfsPsxc_SwitchBootLUN(XUfsPsxc *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* XUFSPSXC_H_ */
/** @} */
