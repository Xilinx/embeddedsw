/******************************************************************************
* Copyright (C) 2006 Vreelin Engineering, Inc.  All Rights Reserved.
* Copyright (C) 2007 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
 * @file xusb_storage.h
 *
 * This file contains the constants, type definitions, variables and function
 * prototypes used in the mass storage application.
 *
 * @note	None.
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
#define RAMDISKSECTORS  		0x400
#define RAMBLOCKS			128

/*
 * Mass storage Class and Sub class codes.
 */
#define MS_SCSI_CMD_SET			0x06
#define CLASS_MASS_STORAGE		0x08

/*
 * Mass Storage Protocols.
 */
#define MS_BULK_ONLY			0x50

/*
 * SCSI Commands.
 */
#define SCSI_READ_10			0x28
#define SCSI_READ_12			0xA8
#define SCSI_WRITE_10			0x2A
#define SCSI_WRITE_12			0xAA
#define SCSI_VERIFY			0x2f
#define SCSI_INQUIRY			0x12
#define SCSI_READ_FORMAT_CAPACITIES	0x23
#define SCSI_MODE_SENSE			0x1a
#define SCSI_READ_CAPACITY		0x25
#define SCSI_TEST_UNIT_READY		0x00
#define SCSI_REQUEST_SENSE		0x03
#define SCSI_MEDIA_REMOVAL		0x1e

#define USBCSW_SIGNATURE 		0x55534253
#define USBCSW_LENGTH			0x0d
#define MODESENSE_RETURNALL		0x3f

/*
 * Valid USB Status block.
 */
#define CMD_PASSED			0x00
#define CMD_FAILED			0x01
#define PHASE_ERROR			0x02

/*
 * Error codes.
 */
#define NO_ERROR			0
#define ERR_NOTERASED			1
#define ERR_NOFREEBLOCKS		2
#define ERR_USBABORT			3
#define ERR_CMDFAILED			4
#define ERR_ECC				5
#define ERR_BADBZ			6
#define ERR_DSMM			7

/*
 * Mass Storage file format related constants.
 */
#define FORMATTED_CURRENT		0x02000000
#define UNFORMATTED			0x01000000
#define BLOCK_SIZE			0x0200

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
u8 Read10Abort = 0;
u8 Write10Abort = 0;
u8 ErrCode;
extern u16 MaxControlSize;
extern USB_CMD_BUF Ch9_CmdBuf;
extern IntChar UsbMemData;		/* Dual Port memory */

/*
 * A ram array
 */
u32 RamDisk[RAMDISKSECTORS][RAMBLOCKS] __attribute__ ((aligned(4)));

/*
 * Logical block address.
 */
union {
	volatile u8 CharLba[4];
	volatile u32 IntLba;
} Lba;

/*
 * Mass storage memory block count.
 */
union {
	volatile u8 CharBlockCount[4];
	volatile u32 IntBlockCount;
} BlockCount;

/*
 * SCSI Command descriptor blocks.
 */

/*
 * Modesense.
 */
typedef struct {
	u8 OpCode;
	u8 lun;
	u8 pagecode;
	u8 mb0;
	u8 parmlength;
	u8 mb0_2;
	u8 padding[10];
} SCSI_MODESENSE_CDB, *PSCSI_MODESENSE_CDB;

/*
 * Media Removal.
 */
typedef struct {
	u8 OpCode;
	u8 lun;
	u8 mb0[2];
	u8 prevent;
	u8 padding[11];

} SCSI_MEDIA_REMOVAL_TYPE, *PSCSI_MEDIA_REMOVAL_TYPE;

/*
 * Responses to SCSI commands.
 */

/*
 * Request Sense standard data.
 */
typedef struct {
	u8 error_code;
	u8 padding1;
	u8 sense_key;
	u8 information[4];
	u8 additional_sense_length;
	u8 padding2[4];
	u8 additional_sense_code;
	u8 additional_sense_code_qualifier;
	u8 padding3[4];

} REQUEST_SENSE, *PREQUEST_SENSE;

/*
 * Inquiry Data.
 */
typedef struct {
	u8 device_type;
	u8 RMB;
	u8 mbz;
	u8 mb1;
	u8 mb1f;
	char pad[3];
	char vendor[8];
	char product[16];
	char prodrev[4];

} INQUIRY, *PINQUIRY;

/*
 * Read Format Capacities.
 */
typedef struct {
	u8 caplstlen[4];
	u8 maxnumblocks[4];
	u8 cap_code_blocklength[4];

} CAPACITY_LIST, *PCAPACITY_LIST;

typedef struct {
	u8 lastLBA[4];
	u8 blocklength[4];
} READ_CAPACITY, *PREAD_CAPACITY;

/*
 * MODESENSE Mode Parameter List.
 */
typedef struct {
	u8 mode_data_length;
	u8 medium_type_code;
	u8 device_spec_params;
	u8 block_desc_length;
} MODESENSE_MPL, *PMODESENSE_MPL;

/*
 * MODESENSE Read-Write Error Recovery Page.
 */
typedef struct {
	u8 page_code;
	u8 page_length;
	u8 page_params;
	u8 read_retry_count;
	u8 reserved_mb0[4];
	u8 write_retry_count;
	u8 reserved_mb3;
	u8 recovery_limit[2];

} MODESENSE_RWER, *PMODESENSE_RWER;

/*
 * Modesense Flexible Disk Page.
 */
typedef struct {
	u8 page_code;
	u8 page_length;
	u8 transfer_rate[2];
	u8 heads;
	u8 sectors_track;
	u8 bytes_sector[2];
	u8 cylinders[2];
	u8 mbd8;
	u8 mb0_1[8];
	u8 motor_on_delay;
	u8 motor_off_delay;
	u8 mb0_2[7];
	u8 medium_rotation_rate[2];
	u8 mb0_3[2];

} MODESENSE_FD, *PMODESENSE_FD;

/*
 * Modesense Removable Block Access Capacities Page.
 */
typedef struct {
	u8 page_code;
	u8 page_length;
	u8 mb0_1;
	u8 total_luns;
	u8 mb0_2[8];

} MODESENSE_RBAC, *PMODESENSE_RBAC;

/*
 * Modesense Timer and Protect Page.
 */
typedef struct {
	u8 page_code;
	u8 page_length;
	u8 mb0_1;
	u8 iatm;
	u8 mb0_2[3];
	u8 mb1c;

} MODESENSE_TP, *PMODESENSE_TP;


typedef struct {
	MODESENSE_MPL mpl;
	u8 page1_code;
	u8 page1_length;
	u8 page2_code;
	u8 page2_length;

} MODE_SENSE_REPLY_SHORT, *PMODE_SENSE_REPLY_SHORT;

typedef struct {
	MODESENSE_MPL mpl;
	MODESENSE_RWER rwer;
	MODESENSE_FD fd;
	MODESENSE_RBAC rbac;
	MODESENSE_TP tp;

} MODE_SENSE_REPLY_ALL, *PMODE_SENSE_REPLY_ALL;

/*
 * USB Mass Storage Command Block Wrapper.
 */
typedef struct {
	union Signature {
		u8 dCBWSignature[4];
		u32 VALUE;

	} Signature;

	u32 dCBWTag;

	union Length {
		u8 dCBWDataTransferLength[4];
		u32 VALUE;

	} Length;

	u8 dCBWFlags;
	u8 bCBWLUN;
	u8 bCDBLength;

	volatile u8 OpCode;
	u8 lun;
	u8 lba[4];
	u8 reserved_2;
	u8 transfer_length[2];
	u8 control;
	u8 padding[6];

} USBCBW, *PUSBCBW;

/*
 * USB Mass Storage Command Status Wrapper.
 */
typedef struct {

	u8 dCBWSignature[4];

	u32 dCBWTag;
	union Residue {
		u8 dCSWDataResidue[4];
		u32 value;

	} Residue;

	u8 bCSWStatus;

} USBCSW, *PUSBCSW;

/*
 * Mass Storage Command and Status block instances.
 */
USBCBW CmdBlock;
USBCSW CmdStatusBlock;



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
		CONFIG_DESCR,	/* This is a conifig descriptor */
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
REQUEST_SENSE Prss = { 0x70, 0, 0x06, {'0', '0', '0', '0'}, 0x0a, {'0', '0', '0', '0'},
		       0x28, 0x0, {'0', '0', '0', '0'}
		     };

INQUIRY Piq = { 0, 0x80, 0, 0, 0x1f, {0, 0, 0}, {'X', 'i', 'l', 'i', 'n', 'x' },
	{'S', 'M', 'S', 'C', ' ', 'F', 'l', 'a', 's', 'h', ' ', 'D', 'e', 'm', 'o'},
	{'0', '0', '0', '1'}
};

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

READ_CAPACITY Prc;

PSCSI_MEDIA_REMOVAL_TYPE Pmr;

MODE_SENSE_REPLY_SHORT Pmsd_s = { {0x12, 0, 0x00, 0x00},
	0x00, 0x00, 0x00, 0x1c
};
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


PSCSI_MODESENSE_CDB Pms_cdb;

/************************** Function Prototypes *******************************/

void InitUsbInterface(XUsb *InstancePtr);
void UsbIfIntrHandler(void *CallBackRef, u32 IntrStatus);
void EpIntrHandler(void *CallBackRef, u8 EpNum, u32 IntrStatus);
void Ep0IntrHandler(void *CallBackRef, u8 EpNum, u32 IntrStatus);
void Ep1IntrHandler(void *CallBackRef, u8 EpNum, u32 IntrStatus);
void Ep2IntrHandler(void *CallBackRef, u8 EpNum, u32 IntrStatus);
void ProcessRxCmd(XUsb *InstancePtr);
void Read10(XUsb *InstancePtr, PUSBCBW pCmdBlock, PUSBCSW pStatusBlock);
void Write10(XUsb *InstancePtr, PUSBCBW pCmdBlock, PUSBCSW pStatusBlock);
void RequestSense(XUsb *InstancePtr, PUSBCBW pCmdBlock, PUSBCSW pStatusBlock);
void Media_Removal(PUSBCBW pCmdBlock, PUSBCSW pStatusBlock);
void ModeSense(XUsb *InstancePtr, PUSBCBW pCmdBlock, PUSBCSW pStatusBlock);
void ReadCapacity(XUsb *InstancePtr, PUSBCBW pCmdBlock, PUSBCSW pStatusBlock);
void RFC(XUsb *InstancePtr, PUSBCBW pCmdBlock, PUSBCSW pStatusBlock);
void Inquiry(XUsb *InstancePtr, PUSBCBW pCmdBlock, PUSBCSW pStatusBlock);
void MassStorageReset(XUsb *InstancePtr);
void GetMaxLUN(XUsb *InstancePtr);

#ifndef SDT
static int SetupInterruptSystem(XUsb *InstancePtr);
#endif

#ifdef __cplusplus
}
#endif

#endif /* XUSB_STORAGE_H */


