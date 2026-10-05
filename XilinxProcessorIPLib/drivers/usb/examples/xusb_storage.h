/******************************************************************************
* Copyright (C) 2006 Vreelin Engineering, Inc.  All Rights Reserved.
* Copyright (C) 2007 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
 * @file xusb_storage.h
 *
 * This file contains the constants, type definitions, variables and function
 * prototypes used in the mass storage application.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- ------------------------------------------------------------------
 * 1.00a hvm  2/12/07 First release
 * 1.01a hvm  10/2/08 The variables IntLba in Lba and IntBlockCount in
 *			BlockCount are declared as volatile.
 * 2.00a hvm  03/12/09 Modified the RAMDISKSECTORS constant value from 0x4000 to
 *			0x400 as this would reduce the code size and the example
 *			can run in a smaller memory systems.
 * 3.02a hvm  08/16/10 Updated with the little endian support changes.
 * 4.00a hvm  10/25/10 Updated with DmaIntrHandler function prototype. Updated
 *			INQUIRY command with pad values.
 * 4.00a hvm  06/01/11 Modified the USB Mass Storage Command Status Wrapper
 * 			structure. The union for Signature is removed and
 *			only the array definition of Signature is retained.
 *			CR611761 fix.
 * 4.00a hvm  06/24/11 Updated the INQUIRY command fourth parameter value to 0.
 *			CR614794
 * 4.01a hvm  08/11/11 Updated the RamDisk variable to have a 32 bit address
 *			alignment.
 * 4.01a hvm  09/14/11 Fixed the compilation issue at the RamDisk variable
 *			declaration. CR625055.
 * 5.6   pm   07/05/23 Added support for system device-tree flow.
 *
 * </pre>
 *****************************************************************************/

#ifndef  XUSB_STORAGE_H
#define  XUSB_STORAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files **********************************/

#include "xusb_cp9.h"

/************************** Constant Definitions ******************************/

/*
 * Mass storage device Flash size. The following constants are to be modified
 * to get different size of  memory.
 */
#define RAMDISKSECTORS  		0x400	/**< Number of RAM disk sectors. */
#define RAMBLOCKS			128	/**< Number of 32-bit words per RAM disk sector. */

/*
 * Mass storage Class and Sub class codes.
 */
#define MS_SCSI_CMD_SET			0x06	/**< SCSI command set for mass storage. */
#define CLASS_MASS_STORAGE		0x08	/**< USB mass storage class code. */

/*
 * Mass Storage Protocols.
 */
#define MS_BULK_ONLY			0x50	/**< USB mass storage bulk-only transport protocol. */

/*
 * SCSI Commands.
 */
#define SCSI_READ_10			0x28	/**< SCSI READ(10) operation code. */
#define SCSI_READ_12			0xA8	/**< SCSI READ(12) operation code. */
#define SCSI_WRITE_10			0x2A	/**< SCSI WRITE(10) operation code. */
#define SCSI_WRITE_12			0xAA	/**< SCSI WRITE(12) operation code. */
#define SCSI_VERIFY			0x2f	/**< SCSI VERIFY operation code. */
#define SCSI_INQUIRY			0x12	/**< SCSI INQUIRY operation code. */
#define SCSI_READ_FORMAT_CAPACITIES	0x23	/**< SCSI READ FORMAT CAPACITIES operation code. */
#define SCSI_MODE_SENSE			0x1a	/**< SCSI MODE SENSE operation code. */
#define SCSI_READ_CAPACITY		0x25	/**< SCSI READ CAPACITY operation code. */
#define SCSI_TEST_UNIT_READY		0x00	/**< SCSI TEST UNIT READY operation code. */
#define SCSI_REQUEST_SENSE		0x03	/**< SCSI REQUEST SENSE operation code. */
#define SCSI_MEDIA_REMOVAL		0x1e	/**< SCSI PREVENT/ALLOW MEDIUM REMOVAL code. */

#define USBCSW_SIGNATURE 		0x55534253	/**< USB command status wrapper signature. */
#define USBCSW_LENGTH			0x0d	/**< USB command status wrapper size in bytes. */
#define MODESENSE_RETURNALL		0x3f	/**< Mode sense return all pages code. */

/*
 * Valid USB Status block.
 */
#define CMD_PASSED			0x00	/**< USB CSW status: command passed. */
#define CMD_FAILED			0x01	/**< USB CSW status: command failed. */
#define PHASE_ERROR			0x02	/**< USB CSW status: phase error. */

/*
 * Error codes.
 */
#define NO_ERROR			0	/**< No error occurred. */
#define ERR_NOTERASED			1	/**< Error: block not erased. */
#define ERR_NOFREEBLOCKS		2	/**< Error: no free blocks available. */
#define ERR_USBABORT			3	/**< Error: USB transfer aborted. */
#define ERR_CMDFAILED			4	/**< Error: command failed. */
#define ERR_ECC				5	/**< Error: ECC error. */
#define ERR_BADBZ			6	/**< Error: bad block zone (exact meaning is implementation-specific). */
#define ERR_DSMM			7	/**< Error: device-specific media management error. */

/*
 * Mass Storage file format related constants.
 */
#define FORMATTED_CURRENT		0x02000000	/**< Media is formatted (current format). */
#define UNFORMATTED			0x01000000	/**< Media is unformatted. */
#define BLOCK_SIZE			0x0200	/**< Block size in bytes (512 bytes). */

/*
 * End point types.
 */
#define EP_CONTROL			0	/**< Control Endpoint */
#define EP_ISOCHRONOUS			1	/**< Isochronous Endpoint */
#define EP_BULK				2	/**< Bulk Endpoint */
#define EP_INTERRUPT			3	/**< Interrupt Endpoint */

/************************** Variable Definitions ******************************/

/*
 * Flags used to abort read and write command loops.
 */
u8 Read10Abort = 0;	/**< Flag to abort SCSI READ(10) command loop. */
u8 Write10Abort = 0;	/**< Flag to abort SCSI WRITE(10) command loop. */
u8 ErrCode;	/**< Current error code from last SCSI command. */
extern u16 MaxControlSize;
extern USB_CMD_BUF Ch9_CmdBuf;
extern IntChar UsbMemData;		/* Dual Port memory */

/*
 * A ram array
 */
u32 RamDisk[RAMDISKSECTORS][RAMBLOCKS] __attribute__ ((aligned(4)));	/**< RAM disk storage array. */

/*
 * Logical block address.
 */
union {
	volatile u8 CharLba[4];  /**< LBA as four bytes in native byte order. */
	volatile u32 IntLba;     /**< LBA as a 32-bit integer. */
} Lba;	/**< Logical block address union. */

/*
 * Mass storage memory block count.
 */
union {
	volatile u8 CharBlockCount[4];  /**< Block count as four bytes. */
	volatile u32 IntBlockCount;     /**< Block count as a 32-bit integer. */
} BlockCount;	/**< Block count union. */

/*
 * SCSI Command descriptor blocks.
 */

/** @brief SCSI mode sense command descriptor block structure. */
typedef struct {
	u8 OpCode;	/**< SCSI operation code. */
	u8 lun;	/**< Logical unit number. */
	u8 pagecode;	/**< Page code for mode sense. */
	u8 mb0;	/**< Reserved byte 0. */
	u8 parmlength;	/**< Allocation length for the mode-sense response. */
	u8 mb0_2;	/**< Reserved byte 2. */
	u8 padding[10];	/**< Padding bytes for alignment. */
} SCSI_MODESENSE_CDB, *PSCSI_MODESENSE_CDB;

/** @brief SCSI prevent/allow medium removal command structure. */
typedef struct {
	u8 OpCode;	/**< SCSI operation code. */
	u8 lun;	/**< Logical unit number. */
	u8 mb0[2];	/**< Reserved bytes. */
	u8 prevent;	/**< Prevent removal flag. */
	u8 padding[11];	/**< Padding bytes for alignment. */

} SCSI_MEDIA_REMOVAL_TYPE, *PSCSI_MEDIA_REMOVAL_TYPE;	/**< Pointer to SCSI_MEDIA_REMOVAL_TYPE structure. */

/** @brief SCSI request sense standard response data structure. */
typedef struct {
	u8 error_code;	/**< Error code and valid bit. */
	u8 padding1;	/**< Reserved padding byte. */
	u8 sense_key;	/**< Sense key code. */
	u8 information[4];	/**< Information bytes. */
	u8 additional_sense_length;	/**< Additional sense data length. */
	u8 padding2[4];	/**< Command-specific information bytes. */
	u8 additional_sense_code;	/**< Additional sense code. */
	u8 additional_sense_code_qualifier;	/**< Additional sense code qualifier. */
	u8 padding3[4];	/**< FRU code and sense-key-specific information bytes. */

} REQUEST_SENSE, *PREQUEST_SENSE;	/**< Pointer to REQUEST_SENSE structure. */

/** @brief SCSI inquiry command response data structure. */
typedef struct {
	u8 device_type;	/**< Peripheral device type. */
	u8 RMB;	/**< Removable media bit. */
	u8 mbz;	/**< Reserved byte. */
	u8 mb1;	/**< Response data format. */
	u8 mb1f;	/**< Format-specific flags. */
	char pad[3];	/**< Reserved padding bytes. */
	char vendor[8];	/**< Vendor identification string. */
	char product[16];	/**< Product identification string. */
	char prodrev[4];	/**< Product revision level. */

} INQUIRY, *PINQUIRY;	/**< Pointer to INQUIRY structure. */

/** @brief SCSI read format capacities response structure. */
typedef struct {
	u8 caplstlen[4];	/**< Capacity list data length. */
	u8 maxnumblocks[4];	/**< Maximum number of blocks. */
	u8 cap_code_blocklength[4];	/**< Descriptor code and block length. */

} CAPACITY_LIST, *PCAPACITY_LIST;	/**< Pointer to CAPACITY_LIST structure. */

/** @brief SCSI read capacity command response structure. */
typedef struct {
	u8 lastLBA[4];	/**< Last logical block address. */
	u8 blocklength[4];	/**< Block length in bytes. */
} READ_CAPACITY, *PREAD_CAPACITY;	/**< Pointer to READ_CAPACITY structure. */

/** @brief SCSI mode sense mode parameter list header structure. */
typedef struct {
	u8 mode_data_length;	/**< Mode data length. */
	u8 medium_type_code;	/**< Medium type code. */
	u8 device_spec_params;	/**< Device-specific parameters. */
	u8 block_desc_length;	/**< Block descriptor length. */
} MODESENSE_MPL, *PMODESENSE_MPL;	/**< Pointer to MODESENSE_MPL structure. */

/** @brief SCSI mode sense read-write error recovery page structure. */
typedef struct {
	u8 page_code;	/**< Page code for read/write error recovery. */
	u8 page_length;	/**< Page length in bytes. */
	u8 page_params;	/**< Error recovery parameters bitmask. */
	u8 read_retry_count;	/**< Read retry count. */
	u8 reserved_mb0[4];	/**< Reserved bytes. */
	u8 write_retry_count;	/**< Write retry count. */
	u8 reserved_mb3;	/**< Reserved byte. */
	u8 recovery_limit[2];	/**< Recovery time limit in milliseconds. */

} MODESENSE_RWER, *PMODESENSE_RWER;	/**< Pointer to MODESENSE_RWER structure. */

/** @brief SCSI mode sense flexible disk page structure. */
typedef struct {
	u8 page_code;	/**< Page code for flexible disk. */
	u8 page_length;	/**< Page length in bytes. */
	u8 transfer_rate[2];	/**< Transfer rate in kbits/sec. */
	u8 heads;	/**< Number of heads. */
	u8 sectors_track;	/**< Sectors per track. */
	u8 bytes_sector[2];	/**< Bytes per sector. */
	u8 cylinders[2];	/**< Number of cylinders. */
	u8 mbd8;	/**< Media byte 8 flags. */
	u8 mb0_1[8];	/**< Reserved bytes. */
	u8 motor_on_delay;	/**< Motor-on delay in hundredths of seconds. */
	u8 motor_off_delay;	/**< Motor-off delay in tenths of seconds. */
	u8 mb0_2[7];	/**< Reserved bytes. */
	u8 medium_rotation_rate[2];	/**< Medium rotation rate in RPM. */
	u8 mb0_3[2];	/**< Reserved bytes. */

} MODESENSE_FD, *PMODESENSE_FD;	/**< Pointer to MODESENSE_FD structure. */

/** @brief SCSI mode sense removable block access capacities page structure. */
typedef struct {
	u8 page_code;	/**< Page code for removable block access capabilities. */
	u8 page_length;	/**< Page length in bytes. */
	u8 mb0_1;	/**< Reserved byte. */
	u8 total_luns;	/**< Total number of logical units. */
	u8 mb0_2[8];	/**< Reserved bytes. */

} MODESENSE_RBAC, *PMODESENSE_RBAC;	/**< Pointer to MODESENSE_RBAC structure. */

/** @brief SCSI mode sense timer and protect page structure. */
typedef struct {
	u8 page_code;	/**< Page code for timer and protect. */
	u8 page_length;	/**< Page length in bytes. */
	u8 mb0_1;	/**< Reserved byte. */
	u8 iatm;	/**< Inactivity timer multiplier. */
	u8 mb0_2[3];	/**< Reserved bytes. */
	u8 mb1c;	/**< Media change notification class 1. */

} MODESENSE_TP, *PMODESENSE_TP;	/**< Pointer to MODESENSE_TP structure. */


/** @brief SCSI mode sense short reply structure with header and basic pages. */
typedef struct {
	MODESENSE_MPL mpl;	/**< Mode parameter list header. */
	u8 page1_code;	/**< Page 1 code. */
	u8 page1_length;	/**< Page 1 length. */
	u8 page2_code;	/**< Page 2 code. */
	u8 page2_length;	/**< Page 2 length. */

} MODE_SENSE_REPLY_SHORT, *PMODE_SENSE_REPLY_SHORT;	/**< Pointer to MODE_SENSE_REPLY_SHORT structure. */

/** @brief SCSI mode sense full reply structure with all supported pages. */
typedef struct {
	MODESENSE_MPL mpl;	/**< Mode parameter list header. */
	MODESENSE_RWER rwer;	/**< Read/write error recovery page. */
	MODESENSE_FD fd;	/**< Flexible disk page. */
	MODESENSE_RBAC rbac;	/**< Removable block access capabilities page. */
	MODESENSE_TP tp;	/**< Timer and protect page. */

} MODE_SENSE_REPLY_ALL, *PMODE_SENSE_REPLY_ALL;	/**< Pointer to MODE_SENSE_REPLY_ALL structure. */

/** @brief USB mass storage command block wrapper structure. */
typedef struct {
	/** @brief CBW signature union for byte and word access. */
	union Signature {
		u8 dCBWSignature[4]; /**< Signature bytes array. */
		u32 VALUE;           /**< Signature as a 32-bit word. */

	} Signature; /**< Command block wrapper signature field. */

	u32 dCBWTag; /**< Command block wrapper tag. */

	/** @brief CBW data transfer length union for byte and word access. */
	union Length {
		u8 dCBWDataTransferLength[4]; /**< Transfer length bytes array. */
		u32 VALUE;                    /**< Transfer length as 32-bit word. */

	} Length; /**< Command block wrapper data transfer length. */

	u8 dCBWFlags;	/**< Flags: direction and reserved bits. */
	u8 bCBWLUN;	/**< Logical unit number. */
	u8 bCDBLength;	/**< Length of CBWCDB field in bytes. */

	volatile u8 OpCode;	/**< SCSI operation code. */
	u8 lun;	/**< Logical unit number in CDB. */
	u8 lba[4];	/**< Logical block address. */
	u8 reserved_2;	/**< Reserved byte in CDB. */
	u8 transfer_length[2];	/**< Transfer length in blocks. */
	u8 control;	/**< Control byte. */
	u8 padding[6];	/**< Padding completing the 16-byte CBWCDB field. */

} USBCBW, *PUSBCBW;	/**< Pointer to USBCBW structure. */

/** @brief USB mass storage command status wrapper structure. */
typedef struct {

	u8 dCBWSignature[4]; /**< CSW signature field. */

	u32 dCBWTag; /**< Command block wrapper tag echo. */
	/** @brief CSW data residue union for byte and word access. */
	union Residue {
		u8 dCSWDataResidue[4]; /**< Residue bytes array. */
		u32 value;             /**< Residue as a 32-bit word. */

	} Residue; /**< Data residue for the command. */

	u8 bCSWStatus; /**< Command completion status code. */

} USBCSW, *PUSBCSW;	/**< Pointer to USBCSW structure. */

/*
 * Mass Storage Command and Status block instances.
 */
USBCBW CmdBlock;	/**< Current SCSI command block wrapper. */
USBCSW CmdStatusBlock;	/**< Current SCSI command status wrapper. */



USB_STD_DEV_DESC DeviceDescriptor __attribute__ ((aligned(4))) = {
	sizeof(USB_STD_DEV_DESC),	/* Descriptor Size 18 bytes */
	DEVICE_DESCR,	/* This is a device descriptor */
#ifdef __LITTLE_ENDIAN__
	0x0200,		/* USB version */
#else
	0x02,		/* USB version */
#endif
	0,		/* Vendor Specific */
	00,		/* Unused */
	00,		/* Unused */
	0x40,		/* Ep0 Max Pkt Size 64 bytes */
#ifdef __LITTLE_ENDIAN__
	0x03FD,		/* Vendor Id */
	0x0100,		/* Product Id */
	0x0100,		/* BCD device */
#else
	0xFD03,		/* Vendor Id */
	0x0001,		/* Product Id */
	0x01,		/* BCD device */
#endif
	01,		/* String Index of manufacturer */
	02,		/* String Index of product */
	03,		/* String Index of serial number */
	01		/* Number of configurations */
};

USB_STD_QUAL_DESC QualifierDescriptor __attribute__ ((aligned(4))) = {
	sizeof(USB_STD_QUAL_DESC),
	QUALIFIER_DESCR, 00, 02, 0, 00, 00, 0x40, 01, 0
};

FPGA1_CONFIGURATION __attribute__ ((aligned(4))) HsUsbConfig = {

	/*
	 * Configuration descriptor.
	 */
	{
		sizeof(USB_STD_CFG_DESC),	/* Size of config descriptor 9
		bytes */
		CONFIG_DESCR,	/* This is a config descriptor */
		sizeof(HsUsbConfig),	/* Total size of configuration
			LS */
		0x00,	/* Total size of configuration MS */
		0x01,	/* No. Of interfaces 1 */
		CONFIGURATION_ONE,	/* No of configuration values */
		0x00,	/* Configuration string */
		0xc0,	/* Self Powered */
		0x01	/* Uses 2mA from the USB bus */
	}
	,
	/*
	 * FPGA1 Class interface.
	 */
	{
		sizeof(USB_STD_IF_DESC),	/* Interface Descriptor size 9
		bytes */
		INTERFACE_DESCR,	/* This is an interface
			descriptor */
		0x00,	/* Interface number 0 */
		0x00,	/* Alternate set 0 */
		0x02,	/* Number of end points 2 */
		CLASS_MASS_STORAGE,	/* Vendor specific */
		MS_SCSI_CMD_SET,	/* Interface sub class */
		MS_BULK_ONLY,	/* protocol BULK only */
		0x00	/* Interface unused */
	}
	,
	/*
	 * End_point 1 RX descriptor  from device to host.
	 */
	{
		sizeof(USB_STD_EP_DESC),	/* End point descriptor size */
		ENDPOINT_DESCR,	/* This is an end point descriptor */
		0x81,	/* End point one */
		EP_BULK,	/* End point type */
		0x00,	/* Maximum packet  size 512 bytes LS */
		0x02,	/* Maximum packetsize MS */
		0xff	/* Nak rate */
	}
	,
	/*
	 * End_point 2 RX descriptor  from host to device.
	 */
	{
		sizeof(USB_STD_EP_DESC),	/* End point descriptor size */
		ENDPOINT_DESCR,	/* This is an end point descriptor */
		0x02,	/* End point two */
		EP_BULK,	/* End point type */
		0x00,	/* Maximum packet  size 512 bytes LS */
		0x02,	/* Maximum packetsize MS */
		0xff	/* Nak rate */
	}

};

FPGA1_CONFIGURATION __attribute__ ((aligned(4))) FsUsbConfig = {

	/*
	 * Configuration descriptor.
	 */
	{
		sizeof(USB_STD_CFG_DESC),	/* Size of config descriptor 9
		bytes */
		CONFIG_DESCR,	/* This is a config descriptor */
		sizeof(FsUsbConfig),	/* Total size of configuration
			LS */
		0x00,	/* Total size of configuration MS */
		0x01,	/* No. Of interfaces 1 */
		CONFIGURATION_ONE,	/* No of configuration values */
		0x00,	/* Configuration string */
		0xc0,	/* Self Powered */
		0x01	/* Uses 2mA from the USB bus */
	}
	,
	/*
	 * FPGA1 Class interface.
	 */
	{
		sizeof(USB_STD_IF_DESC),	/* Interface Descriptor size 9
		bytes */
		INTERFACE_DESCR,	/* This is an interface
			descriptor */
		0x00,	/* Interface number 0 */
		0x00,	/* Alternate set 0 */
		0x02,	/* Number of end points 2 */
		CLASS_MASS_STORAGE,	/* Vendor specific */
		MS_SCSI_CMD_SET,	/* Interface sub class */
		MS_BULK_ONLY,	/* protocol BULK only */
		0x00	/* Interface unused */
	}
	,
	/*
	 * End_point  1 RX descriptor  from device to host.
	 */
	{
		sizeof(USB_STD_EP_DESC),	/* End point descriptor size */
		ENDPOINT_DESCR,	/* This is an end point descriptor */
		0x81,	/* End point one */
		EP_BULK,	/* End point type */
		0x40,	/* Maximum packet  size 64 bytes LS */
		0x00,	/* Maximum packetsize MS */
		0x00	/* Nak rate */
	}
	,
	/*
	 * End_point 2 RX descriptor  from host to device.
	 */
	{
		sizeof(USB_STD_EP_DESC),	/* End point descriptor size */
		ENDPOINT_DESCR,	/* This is an end point descriptor */
		0x02,	/* End point two */
		EP_BULK,	/* End point type */
		0x40,	/* Maximum packet  size 64 bytes LS */
		0x00,	/* Maximum packetsize MS */
		0x00	/* Nak rate */
	}

};

USB_STD_STRING_DESC LangId __attribute__ ((aligned(4))) = {
	/*
	 * Language ID codes.
	 */
	4, STRING_DESCR, {
		0x0904
	}
};

USB_STD_STRING_MAN_DESC Manufacturer __attribute__ ((aligned(4))) = {
	/*
	 * Manufacturer String.
	 */
	sizeof(USB_STD_STRING_MAN_DESC), STRING_DESCR, {
		'X', 0, 'I', 0, 'L', 0, 'I', 0, 'N', 0, 'X', 0, ' ', 0
	}
};

USB_STD_STRING_PS_DESC ProductString __attribute__ ((aligned(4))) = {
	/*
	 * Product ID String.
	 */
	sizeof(USB_STD_STRING_PS_DESC), STRING_DESCR, {
		'F', 0, 'P', 0, 'G', 0, 'A', 0, '1', 0
	}
};

USB_STD_STRING_SN_DESC SerialNumber __attribute__ ((aligned(4))) = {
	/*
	 * Product ID String.
	 */
	sizeof(USB_STD_STRING_SN_DESC), STRING_DESCR, {
		'0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '1', 0,
		'7', 0, '1', 0, '5', 0, '0', 0, '4', 0, '2', 0,
		'6', 0, '2', 0, '0', 0, '0', 0, '5', 0, '7', 0, '4', 0
	}
};

/*
 * Mass Storage device enumeration related structure initializations.
 */
/** @brief Request sense response data buffer. */
REQUEST_SENSE Prss = { 0x70, 0, 0x06, {'0', '0', '0', '0'}, 0x0a, {'0', '0', '0', '0'},
		       0x28, 0x0, {'0', '0', '0', '0'}
		     };

/** @brief Inquiry response data buffer. */
INQUIRY Piq = { 0, 0x80, 0, 0, 0x1f, {0, 0, 0}, {'X', 'i', 'l', 'i', 'n', 'x' },
	{'S', 'M', 'S', 'C', ' ', 'F', 'l', 'a', 's', 'h', ' ', 'D', 'e', 'm', 'o'},
	{'0', '0', '0', '1'}
};

/** @brief Capacity list response data buffer. */
CAPACITY_LIST Pcl = { {'8', '0', '0', '0'}, {
		(RAMDISKSECTORS & 0xFF),
		((RAMDISKSECTORS & 0xFF00) >> 8),
		((RAMDISKSECTORS & 0xFF0000) >> 16),
		((RAMDISKSECTORS & 0xFF000000) >> 24)
	},
	{
		((FORMATTED_CURRENT | BLOCK_SIZE) & 0xFF),
		(((FORMATTED_CURRENT | BLOCK_SIZE) & 0xFF00) >> 8),
		(((FORMATTED_CURRENT | BLOCK_SIZE) & 0xFF0000) >> 16),
		(((FORMATTED_CURRENT | BLOCK_SIZE) & 0xFF0000) >> 24)
	}
};

READ_CAPACITY Prc;	/**< READ_CAPACITY response buffer. */

PSCSI_MEDIA_REMOVAL_TYPE Pmr;	/**< Pointer to SCSI_MEDIA_REMOVAL_TYPE buffer. */

/** @brief Short mode sense reply buffer. */
MODE_SENSE_REPLY_SHORT Pmsd_s = { {0x12, 0, 0x00, 0x00},
	0x00, 0x00, 0x00, 0x1c
};
/** @brief Full mode sense reply buffer. */
MODE_SENSE_REPLY_ALL Pmsd_l = { {0x43, 0, 0x00, 0x00},
	{
		0x1, 0x0a, 0, 0x03, {0, 0, 0, 0},
		0x80, 0x03, { 0, 0}
	},
	{
		0x05, 0x1e, {0x13, 0x88}, 0x00, 0x010,
		{0x3f, 00}, {0x00, 0x03}, 0xd8,
		{0, 0, 0, 0, 0, 0, 0, 0}, 0x05, 0x1e,
		{0, 0, 0, 0, 0, 0, 0}, {0x1, 0x68},
		{0x0, 0x0}
	},
	{0x1b, 0x0a, 0, 1, {0, 0, 0, 0, 0, 0, 0, 0}},
	{0x1c, 0x06, 0x0, 0x5, {0, 0, 0}, 0x1c}
};


PSCSI_MODESENSE_CDB Pms_cdb;	/**< Pointer to mode sense CDB. */

/************************** Function Prototypes *******************************/

/**
 * @brief Initializes the USB interface configuration and descriptors.
 *
 * @param[in] InstancePtr is a pointer to the XUsb instance.
 */
void InitUsbInterface(XUsb *InstancePtr);
/**
 * @brief USB interface interrupt handler for device-level events.
 *
 * @param[in] CallBackRef is the callback reference passed from the interrupt
 *            handler, which in our case is a pointer to the driver instance.
 * @param[in] IntrStatus is a bit mask indicating pending interrupts.
 */
void UsbIfIntrHandler(void *CallBackRef, u32 IntrStatus);
/**
 * @brief Generic endpoint interrupt handler.
 *
 * @param[in] CallBackRef is the callback reference (USB instance pointer).
 * @param[in] EpNum is the endpoint number that triggered the interrupt.
 * @param[in] IntrStatus is the endpoint interrupt status bitmask.
 */
void EpIntrHandler(void *CallBackRef, u8 EpNum, u32 IntrStatus);
/**
 * @brief USB endpoint 0 interrupt handler for control transfers.
 *
 * @param[in] CallBackRef is the callback reference passed from the interrupt
 *            handler, which in our case is a pointer to the driver instance.
 * @param[in] EpNum is the end point number.
 * @param[in] IntrStatus is a bit mask indicating pending interrupts.
 */
void Ep0IntrHandler(void *CallBackRef, u8 EpNum, u32 IntrStatus);
/**
 * @brief USB endpoint 1 interrupt handler for bulk-in transfers.
 *
 * @param[in] CallBackRef is the callback reference passed from the interrupt
 *            handler, which in our case is a pointer to the driver instance.
 * @param[in] EpNum is the end point number.
 * @param[in] IntrStatus is a bit mask indicating pending interrupts.
 */
void Ep1IntrHandler(void *CallBackRef, u8 EpNum, u32 IntrStatus);
/**
 * @brief USB endpoint 2 interrupt handler for bulk-out transfers.
 *
 * @param[in] CallBackRef is the callback reference passed from the interrupt
 *            handler, which in our case is a pointer to the driver instance.
 * @param[in] EpNum is the end point number.
 * @param[in] IntrStatus is a bit mask indicating pending interrupts.
 */
void Ep2IntrHandler(void *CallBackRef, u8 EpNum, u32 IntrStatus);
/**
 * @brief Processes received SCSI commands from the host.
 *
 * @param[in] InstancePtr is a pointer to the XUsb instance.
 */
void ProcessRxCmd(XUsb *InstancePtr);
/**
 * @brief Handles SCSI READ(10) command.
 *
 * @param[in] InstancePtr is a pointer to the XUsb instance.
 * @param[in] pCmdBlock is a pointer to the Mass Storage Command Block wrapper.
 * @param[in] pStatusBlock is a pointer to the Mass Storage Status wrapper.
 */
void Read10(XUsb *InstancePtr, PUSBCBW pCmdBlock, PUSBCSW pStatusBlock);
/**
 * @brief Handles SCSI WRITE(10) command.
 *
 * @param[in] InstancePtr is a pointer to the XUsb instance.
 * @param[in] pCmdBlock is a pointer to the Mass Storage Command Block wrapper.
 * @param[out] pStatusBlock is a pointer to the Mass Storage Status wrapper.
 */
void Write10(XUsb *InstancePtr, PUSBCBW pCmdBlock, PUSBCSW pStatusBlock);
/**
 * @brief Handles SCSI REQUEST SENSE command.
 *
 * @param[in] InstancePtr is a pointer to the XUsb instance.
 * @param[in] pCmdBlock is a pointer to the command block wrapper.
 * @param[in] pStatusBlock is a pointer to the command status wrapper.
 */
void RequestSense(XUsb *InstancePtr, PUSBCBW pCmdBlock, PUSBCSW pStatusBlock);
/**
 * @brief Handles SCSI PREVENT/ALLOW MEDIUM REMOVAL command.
 *
 * @param[in] pCmdBlock is a pointer to the command block wrapper.
 * @param[in] pStatusBlock is a pointer to the command status wrapper.
 */
void Media_Removal(PUSBCBW pCmdBlock, PUSBCSW pStatusBlock);
/**
 * @brief Handles SCSI MODE SENSE command.
 *
 * @param[in] InstancePtr is a pointer to the XUsb instance.
 * @param[in] pCmdBlock is a pointer to the command block wrapper.
 * @param[in] pStatusBlock is a pointer to the command status wrapper.
 */
void ModeSense(XUsb *InstancePtr, PUSBCBW pCmdBlock, PUSBCSW pStatusBlock);
/**
 * @brief Handles SCSI READ CAPACITY command.
 *
 * @param[in] InstancePtr is a pointer to the XUsb instance.
 * @param[in] pCmdBlock is a pointer to the command block wrapper.
 * @param[in] pStatusBlock is a pointer to the command status wrapper.
 */
void ReadCapacity(XUsb *InstancePtr, PUSBCBW pCmdBlock, PUSBCSW pStatusBlock);
/**
 * @brief Handles SCSI READ FORMAT CAPACITIES command.
 *
 * @param[in] InstancePtr is a pointer to the XUsb instance.
 * @param[in] pCmdBlock is a pointer to the command block wrapper.
 * @param[in] pStatusBlock is a pointer to the command status wrapper.
 */
void RFC(XUsb *InstancePtr, PUSBCBW pCmdBlock, PUSBCSW pStatusBlock);
/**
 * @brief Handles SCSI INQUIRY command.
 *
 * @param[in] InstancePtr is a pointer to the XUsb instance.
 * @param[in] pCmdBlock is a pointer to the command block wrapper.
 * @param[in] pStatusBlock is a pointer to the command status wrapper.
 */
void Inquiry(XUsb *InstancePtr, PUSBCBW pCmdBlock, PUSBCSW pStatusBlock);
/**
 * @brief Handles USB mass storage class reset request.
 *
 * @param[in] InstancePtr is a pointer to the XUsb instance.
 */
void MassStorageReset(XUsb *InstancePtr);
/**
 * @brief Handles USB GET MAX LUN class request.
 *
 * @param[in] InstancePtr is a pointer to the XUsb instance.
 */
void GetMaxLUN(XUsb *InstancePtr);

#ifndef SDT
static int SetupInterruptSystem(XUsb *InstancePtr);
#endif

#ifdef __cplusplus
}
#endif

#endif /* XUSB_STORAGE_H */


