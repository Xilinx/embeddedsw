/******************************************************************************
* Copyright (C) 2016 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xusbpsu.h
* @addtogroup Overview
* @{
* @details
*
* This section explains the implementation of functions of USBPSU driver.
* This driver supports both USB high-speed and super-speed features for USB
* peripheral mode.
*
* The definitions for endpoints is included by the xusbps_endpoint.c, which
* is implementing the endpoint functions and by xusbps_intr.c.
*
* <b>Initialization & Configuration</b>
*
* The XUsbPsu_Config structure is used by the driver to configure itself.
* Fields inside this structure are properties of XUsbPsu based on its hardware
* build.
*
* To support multiple runtime loading and initialization strategies employed
* by various operating systems, the driver instance can be initialized in the
* following way:
*
*   - XUsbPsu_CfgInitialize(InstancePtr, CfgPtr, EffectiveAddr) - Uses a
*	 configuration structure provided by the caller. If running in a system
*	 with address translation, the parameter EffectiveAddr should be the
* 	 virtual address.
*
* <b>Endpoint Support</b>
*
* This driver supports control, bulk, interrupt and ISO endpoint and its
* applications like mass-storage, HID, audio and composite, etc. Based on
* user application configuration set by the application.
*
* <b>Interrupts</b>
*
* The driver defaults to no interrupts at initialization such that interrupts
* must be enabled if desired. An interrupt is generated for one of the
* following conditions.
*
* - Disconnect Detected Event Enable
* - USB Reset Enable
* - Connection Done Enable
* - Link State Change Event Enable
* - Wakeup Event Enable
*
* The SetupInterruptSystem function setups the interrupt system such that
* interrupts can occur. This function is application specific since the actual
* system may or may not have an interrupt controller.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.0   sg    06/06/16 First release
* 1.1   sg    10/24/16 Update for backward compatibility
*                      Added XUsbPsu_IsSuperSpeed function in xusbpsu.c
* 1.2   mn    01/20/17 removed unnecessary declaration of
*                      XUsbPsu_SetConfiguration in xusbpsu.h
* 1.2   mn    01/30/17 Corrected InstancePtr->UnalignedTx with
*                      Ept->UnalignedTx in xusbpsu_controltransfers.c
* 1.2   mus   02/10/17 Updated data structures to fix compilation errors for IAR
*                      compiler
*       ms    03/17/17 Added readme.txt file in examples folder for doxygen
*                      generation.
*       ms    04/10/17 Modified filename tag to include the file in doxygen
*                      examples.
* 1.4	bk    12/01/18 Modify USBPSU driver code to fit USB common example code
*		       for all USB IPs.
*	myk   12/01/18 Added hibernation support for device mode
*	vak   22/01/18 Added changes for supporting microblaze platform
*	vak   13/03/18 Moved the setup interrupt system calls from driver to
*		       example.
*	vak   24/09/18 Added EnableSuperSpeed in XUsbPsu_Config for speed
*	               negotiation at the time of connection to Host
* 1.5	vak   02/06/19 Added "xusbpsu_endpoint.h" header
* 1.5	vak   03/25/19 Fixed incorrect data_alignment pragma directive for IAR
* 1.6	pm    22/07/19 Removed coverity warnings
* 1.7	pm    14/11/19 Updated number of TRB to improve performance
* 	pm    03/23/20 Restructured the code for more readability and modularity
*	pm    03/14/20 Added clocking support
* 1.8	pm    24/07/20 Fixed MISRA-C and Coverity warnings
* 1.9	pm    03/21/21 Fixed doxygen warnings
* 1.10	pm    08/30/21 Update MACRO to fix plm compilation warnings
*
* </pre>
*
*****************************************************************************/
#ifndef XUSBPSU_H  /* Prevent circular inclusions */
#define XUSBPSU_H  /**< by using protection macros  */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/

/* Enable XUSBPSU_HIBERNATION_ENABLE to enable hibernation */
//#define XUSBPSU_HIBERNATION_ENABLE		1

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xusbpsu_hw.h"
#include "xusbpsu_endpoint.h"
#include "xil_io.h"
#if defined (XCLOCKING)
#include "xil_clocking.h"
#endif

/*
 * The header sleep.h and API usleep() can only be used with an arm design.
 * MB_Sleep() is used for microblaze design.
 */
#if defined (__arm__) || defined (__aarch64__) || defined (__ICCARM__)
#include "sleep.h"
#endif

#ifdef __MICROBLAZE__
#include "microblaze_sleep.h"
#endif
#include "xil_cache.h"

/************************** Constant Definitions ****************************/

/** @cond INTERNAL */
#define NO_OF_TRB_PER_EP		4U	/**< number of TRB*/

#if defined (PLATFORM_ZYNQMP) || defined (versal)
#define ALIGNMENT_CACHELINE		__attribute__ ((aligned(64)))
					/**< ALIGNMENT of CACHELINE 64 */
#else
#define ALIGNMENT_CACHELINE		__attribute__ ((aligned(32)))
					/**< ALIGNMENT of CACHELINE 32 */
#endif

#define	XUSBPSU_PHY_TIMEOUT		5000U	/**< Phy timeout- microseconds */
/** @endcond */

#define XUSBPSU_EP_DIR_IN		1U	/**< Direction IN */
#define XUSBPSU_EP_DIR_OUT		0U	/**< Direction OUT */

#define XUSBPSU_USB_DIR_OUT		0U	/**< Direction to device */
#define XUSBPSU_USB_DIR_IN		0x80U	/**< Direction to host */

#define XUSBPSU_ENDPOINT_XFERTYPE_MASK      0x03/**< Transfer type mask */
#define XUSBPSU_ENDPOINT_XFER_CONTROL       0U	/**< Control EP */
#define XUSBPSU_ENDPOINT_XFER_ISOC          1U	/**< ISO EP */
#define XUSBPSU_ENDPOINT_XFER_BULK          2U	/**< Bulk EP */
#define XUSBPSU_ENDPOINT_XFER_INT           3U	/**< Interrupt EP */
#define XUSBPSU_ENDPOINT_MAX_ADJUSTABLE     0x80/**< Max EP */

#define	XUSBPSU_TEST_J			1U	/**< Test Mode J */
#define	XUSBPSU_TEST_K			2U	/**< Test Mode K */
#define	XUSBPSU_TEST_SE0_NAK		3U	/**< Test Mode SE0_NAK */
#define	XUSBPSU_TEST_PACKET		4U	/**< Test Mode TEST PACKET */
#define	XUSBPSU_TEST_FORCE_ENABLE	5U	/**< Test Mode FORCE ENABLE */

/** @cond INTERNAL */
#define XUSBPSU_NUM_TRBS		8U	/**< Number of TRB */

#define XUSBPSU_EVENT_PENDING		(0x00000001U << 0U)
						/**< Event pending bit */

#define XUSBPSU_EP_ENABLED		(0x00000001U << 0U)
						/**< EP status Enabled */
#define XUSBPSU_EP_STALL		(0x00000001U << 1U)
						/**< EP status WEDGE */
#define XUSBPSU_EP_WEDGE		(0x00000001U << 2U)
						/**< EP status busy */
#define XUSBPSU_EP_BUSY			((u32)0x00000001U << 4U)
						/**< EP status busy */
#define XUSBPSU_EP_PENDING_REQUEST	(0x00000001U << 5U)
						/**< EP status pending request */
#define XUSBPSU_EP_MISSED_ISOC		(0x00000001U << 6U)
						/**< EP status missed ISOC */

#define	XUSBPSU_GHWPARAMS0		0U	/**< Global Hardware Parameter Register 0 */
#define	XUSBPSU_GHWPARAMS1		1U	/**< Global Hardware Parameter Register 1 */
#define	XUSBPSU_GHWPARAMS2		2U	/**< Global Hardware Parameter Register 2 */
#define	XUSBPSU_GHWPARAMS3		3U	/**< Global Hardware Parameter Register 3 */
#define	XUSBPSU_GHWPARAMS4		4U	/**< Global Hardware Parameter Register 4 */
#define	XUSBPSU_GHWPARAMS5		5U	/**< Global Hardware Parameter Register 5 */
#define	XUSBPSU_GHWPARAMS6		6U	/**< Global Hardware Parameter Register 6 */
#define	XUSBPSU_GHWPARAMS7		7U	/**< Global Hardware Parameter Register 7 */

/* HWPARAMS0 */
#define XUSBPSU_MODE(n)			((n) & 0x7U)
						/**< USB MODE Host, Device
						 * or DRD
						 */
#define XUSBPSU_MDWIDTH(n)		(((n) & 0xFF00) >> 8U)
						/**< DATA bus width */

/* HWPARAMS1 */
#define XUSBPSU_NUM_INT(n)		(((n) & (0x3F << 15U)) >> 15U)
						/**< Number of event buffer in
						 *   devicemode
						 */

/* HWPARAMS3 */
#define XUSBPSU_NUM_IN_EPS_MASK		((u32)0x0000001FU << (u32)18U)
						/**< Number of Device Mode
						 *   Active IN Endpoints mask
						 */
#define XUSBPSU_NUM_EPS_MASK		((u32)0x0000003FU << (u32)12U)
						/**< Number of Device Mode
						 *   Endpoints mask
						 */
#define XUSBPSU_NUM_EPS(p)		(((u32)(p) &		\
					(XUSBPSU_NUM_EPS_MASK)) >> (u32)12)
						/**< Number of Device Mode EP */
#define XUSBPSU_NUM_IN_EPS(p)		(((u32)(p) &		\
					(XUSBPSU_NUM_IN_EPS_MASK)) >> (u32)18)
						/**< Number of Device Mode
						 *   Active IN Endpoints
						 */

/* HWPARAMS7 */
#define XUSBPSU_RAM1_DEPTH(n)		((n) & 0xFFFFU) /**< depth of RAM1 */

#define XUSBPSU_DEPEVT_XFERCOMPLETE	0x01U	/**< Device EP event transfer complete */
#define XUSBPSU_DEPEVT_XFERINPROGRESS	0x02U	/**< Device EP event transfer In-progress */
#define XUSBPSU_DEPEVT_XFERNOTREADY	0x03U	/**< Device EP event transfer not-ready */
#define XUSBPSU_DEPEVT_STREAMEVT	0x06U	/**< Device EP event stream event */
#define XUSBPSU_DEPEVT_EPCMDCMPLT	0x07U	/**< Device EP command complete event */

/* Within XferNotReady */
#define DEPEVT_STATUS_TRANSFER_ACTIVE	(1U << 3U)
						/**< EP event status transfer
						 *   active
						 */

/* Within XferComplete */
#define DEPEVT_STATUS_BUSERR		(1U << 0U) /**< EP Event status bus error */
#define DEPEVT_STATUS_SHORT		(1U << 1U) /**< EP Event status short packet */
#define DEPEVT_STATUS_IOC		(1U << 2U) /**< EP Event status completion */
#define DEPEVT_STATUS_LST		(1U << 3U) /**< EP Event status last packet */

/* Stream event only */
#define DEPEVT_STREAMEVT_FOUND		1U /**< EP Event stream found */
#define DEPEVT_STREAMEVT_NOTFOUND	2U /**< EP Event status Not-found */

/* Control-only Status */
#define DEPEVT_STATUS_CONTROL_DATA		1U /**< Control data  status */
#define DEPEVT_STATUS_CONTROL_STATUS		2U /**< Control status */
#define DEPEVT_STATUS_CONTROL_DATA_INVALTRB	9U /**< Control data invalid TRB */
#define DEPEVT_STATUS_CONTROL_STATUS_INVALTRB	0xAU /**< Control status invalid TRB */

#define XUSBPSU_ENDPOINTS_NUM		12U /**< Total number of endpoint */

#define XUSBPSU_EVENT_SIZE		4U /**< event size in bytes */
#define XUSBPSU_EVENT_MAX_NUM		64U /**< 2 events/endpoint */
/* (event size * maximum number of event) */
#define XUSBPSU_EVENT_BUFFERS_SIZE	256U /**< event size * maximum number of event */

#define XUSBPSU_EVENT_TYPE_MASK		0x000000feU /**< Device Specific Event Mask  */

#define XUSBPSU_EVENT_TYPE_DEV		0U /**< Device Specific Event */
#define XUSBPSU_EVENT_TYPE_CARKIT	3U /**< CARKIT Specific Event- Not support */
#define XUSBPSU_EVENT_TYPE_I2C		4U /**< I2C Specific Event- Not support */

#define XUSBPSU_DEVICE_EVENT_DISCONNECT         0U /**< Disconnect Detected Event Enable */
#define XUSBPSU_DEVICE_EVENT_RESET              1U /**< USB Reset Enable */
#define XUSBPSU_DEVICE_EVENT_CONNECT_DONE       2U /**< Connection Done Enable */
#define XUSBPSU_DEVICE_EVENT_LINK_STATUS_CHANGE 3U /**< USB/Link State Change Event Enable */
#define XUSBPSU_DEVICE_EVENT_WAKEUP             4U /**< Resume/Remote Wakeup Detected Event Enable */
#define XUSBPSU_DEVICE_EVENT_HIBER_REQ          5U /**< Hibernation Request Event */
#define XUSBPSU_DEVICE_EVENT_EOPF               6U /**< U3/L2-L1 Suspend Event Enable */
#define XUSBPSU_DEVICE_EVENT_SOF                7U /**< SOF Event */
#define XUSBPSU_DEVICE_EVENT_ERRATIC_ERROR      9U /**< Erratic Error Event Enable */
#define XUSBPSU_DEVICE_EVENT_CMD_CMPL           10U /**< Generic Command Complete Event */
#define XUSBPSU_DEVICE_EVENT_OVERFLOW           11U /**< Event Buffer Overflow Event */

#define XUSBPSU_GEVNTCOUNT_MASK                 0x0000fffcU /**< Global Event Buffer Count Mask */

/*
 * Control Endpoint state
 */
#define	XUSBPSU_EP0_SETUP_PHASE			1U	/**< Setup Phase */
#define	XUSBPSU_EP0_DATA_PHASE			2U	/**< Data Phase */
#define	XUSBPSU_EP0_STATUS_PHASE		3U	/**< Status Pahse */

/*
 * Link State
 */
#define XUSBPSU_LINK_STATE_MASK			0x0FU /**< Link State Mask */
/** @endcond */

/**
 * @param XusbPsuLinkState This typedef defines link state.
 *
 */
typedef enum {
	XUSBPSU_LINK_STATE_U0 = 0x00U, /**< U0 state/ in HS - ON */
	XUSBPSU_LINK_STATE_U1 =	0x01U, /**< U1 state */
	XUSBPSU_LINK_STATE_U2 =	0x02U, /**< U2 state/ in HS - SLEEP */
	XUSBPSU_LINK_STATE_U3 =	0x03U, /**< U3 state/ in HS - SUSPEND */
	XUSBPSU_LINK_STATE_SS_DIS =	0x04U, /**< SuperSpeed connectivity is disabled */
	XUSBPSU_LINK_STATE_RX_DET =	0x05U, /**< Warm reset, Receiver detection */
	XUSBPSU_LINK_STATE_SS_INACT = 0x06U, /**< Link has failed SuperSpeed operation */
	XUSBPSU_LINK_STATE_POLL	=	0x07U, /**< POLL */
	XUSBPSU_LINK_STATE_RECOV =	0x08U, /**< Retrain SuperSpeed link, Perform Hot reset, Switch to Loop back mode */
	XUSBPSU_LINK_STATE_HRESET =	0x09U, /**< Hot reset using Training sets */
	XUSBPSU_LINK_STATE_CMPLY =	0x0AU, /**< Test the transmitter for compliance to voltage and timing specifications */
	XUSBPSU_LINK_STATE_LPBK	=	0x0BU, /**< For test and fault isolation */
	XUSBPSU_LINK_STATE_RESET = 	0x0EU, /**< RESET */
	XUSBPSU_LINK_STATE_RESUME =	0x0FU, /**< RESUME */
} XusbPsuLinkState;	/**< defines link state */

/**
 * @param XusbPsuLinkStateChange This typedef defines link state change.
 *
 */
typedef enum {
	XUSBPSU_LINK_STATE_CHANGE_U0 =		0x00U, /**< U0 /in HS - ON */
	XUSBPSU_LINK_STATE_CHANGE_SS_DIS =	0x04U, /**< SuperSpeed connectivity is disabled */
	XUSBPSU_LINK_STATE_CHANGE_RX_DET =	0x05U, /**< Warm reset, Receiver detection */
	XUSBPSU_LINK_STATE_CHANGE_SS_INACT =	0x06U, /**< Link has failed SuperSpeed operation */
	XUSBPSU_LINK_STATE_CHANGE_RECOV =	0x08U, /**< Retrain SuperSpeed link, Perform Hot reset, Switch to Loop back mode */
	XUSBPSU_LINK_STATE_CHANGE_CMPLY =	0x0AU, /**< Test the transmitter for compliance to voltage and timing specifications */
} XusbPsuLinkStateChange; /**< link state change */

/*
 * Device States
 */
#define		XUSBPSU_STATE_ATTACHED		0U /**< Device State Attach */
#define		XUSBPSU_STATE_POWERED		1U /**< Device State Power */
#define		XUSBPSU_STATE_DEFAULT		2U /**< Device State Default */
#define		XUSBPSU_STATE_ADDRESS		3U /**< Device State Address */
#define		XUSBPSU_STATE_CONFIGURED	4U /**< Device State Configure */
#define		XUSBPSU_STATE_SUSPENDED		5U /**< Device State Suspend */

/*
 * Device Speeds
 */
#define		XUSBPSU_SPEED_UNKNOWN		0U /**< Device Speed Unknown */
#define		XUSBPSU_SPEED_LOW		1U /**< Device Speed Low */
#define		XUSBPSU_SPEED_FULL		2U /**< Device Speed Full */
#define		XUSBPSU_SPEED_HIGH		3U /**< Device Speed High */
#define		XUSBPSU_SPEED_SUPER		4U /**< Device Speed Speed */

/** @cond INTERNAL */
/*
 * return Physical EP number as dwc3 mapping
 */
#define XUSBPSU_PhysicalEp(epnum, direction)	(((epnum) << 1U ) | (direction))
						/**< Return Physical EP number
						 *   as dwc3 mapping
						 */
/** @endcond */

/**************************** Type Definitions ******************************/

/**
 * struct XUsbPsu_EvtBuffer - Software Event buffer representation
 * @param BuffAddr: Event Buffer address
 * @param Offset: Event Buffer offset
 * @param Count: Event Buffer count
 * @param Flags: Event Buffer Flag - PENDING /NOT PENDING
 */
struct XUsbPsu_EvtBuffer {
	void	*BuffAddr;	/**< Event Buffer address */
	u32	Offset;		/**< Event Buffer offset */
	u32	Count;		/**< Event Buffer count */
	u32	Flags;		/**< Event Buffer Flag - PENDING /NOT PENDING */
}; /**< Software Event buffer representation */

/**
 * struct XUsbPsu_Trb - Transfer Request Block - Hardware format
 * @param BufferPtrLow: Data buffer pointer to low 32 bits
 * @param BufferPtrHigh: Data buffer pointer to high 32-bits
 * @param Size: Buffer Size
 * @param Ctrl: Transfer Request Block Control parameter
 */
#if defined (__ICCARM__)
#pragma pack(push, 1)
#endif
struct XUsbPsu_Trb {
	u32	BufferPtrLow;	/**< Data buffer pointer to low 32 bits */
	u32	BufferPtrHigh;	/**< Data buffer pointer to high 32-bits */
	u32	Size;		/**< Buffer Size */
	u32	Ctrl;		/**< Transfer Request Block Control parameter */
#if defined (__ICCARM__)
}; /**< Transfer Request Block - Hardware format */
#pragma pack(pop)
#else
} __attribute__((packed)); /**< Transfer Request Block - Hardware format */
#endif

/**
 * struct XUsbPsu_EpParams - Endpoint Parameters
 * @param Param2: Parameter 2
 * @param Param1: Parameter 1
 * @param Param0: Parameter 0
 */
struct XUsbPsu_EpParams {
	u32	Param2;		/**< Parameter 2 */
	u32	Param1;		/**< Parameter 1 */
	u32	Param0;		/**< Parameter 0 */
}; /**< Endpoint Parameters */

/**
 * struct SetupPacket - USB Standard Control Request
 * @param bRequestType: Characteristics of request
 * @param bRequest: Type of request
 * @param wValue: Word-sized field that varies according to request
 * @param wIndex: Used to pass an index or offset
 * @param wLength: Number of bytes to transfer if there is a Data stage
 */
#if defined (__ICCARM__)
#pragma pack(push, 1)
#endif
typedef struct {
        u8  bRequestType;	/**< Characteristics of request */
        u8  bRequest;		/**< Type of request */
        u16 wValue;		/**< Word-sized that varies according to request */
        u16 wIndex;		/**< Used to pass an index or offset */
        u16 wLength;		/**< Number of bytes to transfer */
#if defined (__ICCARM__)
} SetupPacket;			/**< USB Standard Control Request */
#pragma pack(pop)
#else
} __attribute__ ((packed)) SetupPacket; /**< USB Standard Control Request */
#endif

/**
 * struct XUsbPsu_Ep - Endpoint representation
 * @param Handler: User handler
 * @param EpTrb: TRB used by endpoint
 * @param EpStatus: Flags to represent Endpoint status
 * @param EpSavedState: Endpoint status saved at the time of hibernation
 * @param RequestedBytes: RequestedBytes for transfer
 * @param BytesTxed: Actual Bytes transferred
 * @param Interval: Data transfer service interval
 * @param TrbEnqueue: number of TRB enqueue
 * @param TrbDequeue: number of TRB dequeue
 * @param MaxSize: Size of endpoint
 * @param CurUf: current microframe
 * @param BufferPtr: Buffer location
 * @param ResourceIndex: Resource Index assigned to Endpoint by core
 * @param PhyEpNum: Physical Endpoint Number in core
 * @param UsbEpNum: USB Endpoint Number
 * @param Type: Type of Endpoint - Control/BULK/INTERRUPT/ISOC
 * @param Direction: Direction - EP_DIR_OUT/EP_DIR_IN
 * @param UnalignedTx: Unaligned Tx flag - 0/1
 */
struct XUsbPsu_Ep {
	void (*Handler)(void *, u32, u32);
						/**< User handler called
						 *   when data is sent for IN Ep
						 *   and received for OUT Ep
						 */
#if defined (__ICCARM__)
    #pragma data_alignment = 64
	struct XUsbPsu_Trb EpTrb[NO_OF_TRB_PER_EP + 1U]; /**< One extra Trb is
							  * for Link Trb
							  */
#else
	struct XUsbPsu_Trb EpTrb[NO_OF_TRB_PER_EP + 1U] ALIGNMENT_CACHELINE;
						/**< TRB used by endpoint
						 *   One extra Link TRB
						 */
#endif
	u32	EpStatus;	/**< Flags to represent Endpoint status */
	u32	EpSavedState;	/**< Endpoint status saved at the time of
				 *   hibernation
				 */
	u32	RequestedBytes;	/**< RequestedBytes for transfer */
	u32	BytesTxed;	/**< Actual Bytes transferred */
	u32	Interval;	/**< Data transfer service interval */
	u32	TrbEnqueue;	/**< number of TRB enqueue */
	u32	TrbDequeue;	/**< number of TRB dequeue */
	u16	MaxSize;	/**< Size of endpoint */
	u16	CurUf;		/**< current microframe */
	u8	*BufferPtr;	/**< Buffer location */
	u8	ResourceIndex;	/**< Resource Index assigned to
				 *   Endpoint by core
				 */
	u8	PhyEpNum;	/**< Physical Endpoint Number in core */
	u8	UsbEpNum;	/**< USB Endpoint Number */
	u8	Type;		/**< Type of Endpoint -
				 *   Control/BULK/INTERRUPT/ISOC
				 */
	u8	Direction;	/**< Direction - EP_DIR_OUT/EP_DIR_IN */
	u8	UnalignedTx;	/**< Unaligned Tx flag - 0/1 */
}; /**< Endpoint representation */

/**
 * struct XUsbPsu_Config - Configuration information for the USB
 * @param DeviceId: Unique ID of controller
 * @param BaseAddress: Core register base address
 * @param IsCacheCoherent: Describes whether Cache Coherent or not
 * @param EnableSuperSpeed: Set to enable super speed support
 * @param RefClk: Input clocks
 */
typedef struct {
        u16 DeviceId;		/**< Unique ID of controller */
	UINTPTR BaseAddress;	/**< Core register base address */
	u8 IsCacheCoherent;	/**< Describes whether Cache Coherent or not */
	u8 EnableSuperSpeed;	/**< Set to enable super speed support */
#if defined (XCLOCKING)
	u32 RefClk;		/**< Input clocks */
#endif
} XUsbPsu_Config; /**< Configuration information for the USB */

typedef XUsbPsu_Config Usb_Config; /**< typedef configuration structure */

/**
 * struct Usb_DevData - Application device data
 * @param Speed: device speed - full/low/high/super
 * @param State: device state
 * @param PrivateData: USB device application data
 */
struct Usb_DevData {
	u8 Speed; /**< USB device data speed */
	u8 State; /**< USB device data state */

	void *PrivateData; /**< USB device application data */
}; /**< Application Data */

/**
 * USB Device Controller representation
 */
/**
 * struct XUsbPsu - USB Device Controller representation
 * @param SetupData: Setup data packet
 * @param Ep0_Trb: TRB for control transfers
 * @param ConfigPtr: Configuration info pointer
 * @param eps: Endpoints array
 * @param Evt: Usb event buffer
 * @param EpParams: Endpoint Parameters
 * @param BaseAddress: Core register base address
 * @param DevDescSize: Device descriptor size
 * @param ConfigDescSize: Config descriptor size
 * @param AppData: Application data
 * @param *Chapter9: USB Chapter9 function handler
 * @param *ResetIntrHandler: Reset function handler
 * @param *DisconnectIntrHandler: Disconnect function handler
 * @param *DevDesc: Device descriptor pointer
 * @param *ConfigDesc: Config descriptor pointer
 * @param EventBuffer: Event buffer array
 * @param NumOutEps: Number of out endpoints
 * @param NumInEps: Number of in endpoint
 * @param ControlDir: Control endpoint direction
 * @param IsInTestMode: USB test mode flag
 * @param TestMode:  Test Mode
 * @param Ep0State: Control EP state
 * @param LinkState: USB link state
 * @param UnalignedTx: Unaligned transfer flag
 * @param IsConfigDone: Flag - Check config is done or not
 * @param IsThreeStage: USB three stage communication
 * @param IsHibernated: Flag - Hibernated state
 * @param HasHibernation: Has hibernation support
 * @param *data_ptr: pointer for storing applications data
 */
struct XUsbPsu {
#if defined (__ICCARM__)
    #pragma data_alignment = 64
	SetupPacket SetupData;	/**< Setup data packet */
    #pragma data_alignment = 64
	struct XUsbPsu_Trb Ep0_Trb;	/**< TRB for control transfers */
#else
	SetupPacket SetupData ALIGNMENT_CACHELINE;
				/**< Setup Packet buffer */
	struct XUsbPsu_Trb Ep0_Trb ALIGNMENT_CACHELINE;
#endif
				/**< TRB for control transfers */
	XUsbPsu_Config *ConfigPtr;	/**< Configuration info pointer */
	struct XUsbPsu_Ep eps[XUSBPSU_ENDPOINTS_NUM]; /**< Endpoints */
	struct XUsbPsu_EvtBuffer Evt;	/**< Usb event buffer */
	struct XUsbPsu_EpParams EpParams; /**< Endpoint Parameters */
	u32 BaseAddress;	/**< Core register base address */
	u32 DevDescSize;	/**< Device descriptor size */
	u32 ConfigDescSize;	/**< Config descriptor size */
	struct Usb_DevData *AppData; /**< Allication Data */
	void (*Chapter9)(struct Usb_DevData *, SetupPacket *);
				/**< USB Chapter9 function handler */
	void (*ResetIntrHandler)(struct Usb_DevData *);
				/**< Reset function handler */
	void (*DisconnectIntrHandler)(struct Usb_DevData *);
				/**< Disconnect function handler */
	void *DevDesc;		/**< Device descriptor pointer */
	void *ConfigDesc;	/**< Config descriptor pointer */
#if defined(__ICCARM__)
    #pragma data_alignment = XUSBPSU_EVENT_BUFFERS_SIZE
	u8 EventBuffer[XUSBPSU_EVENT_BUFFERS_SIZE]; /**< Event buffer array */
#else
	u8 EventBuffer[XUSBPSU_EVENT_BUFFERS_SIZE]
			__attribute__((aligned(XUSBPSU_EVENT_BUFFERS_SIZE)));
				/**< Event buffer array */
#endif
	u8 NumOutEps;		/**< Number of out endpoints */
	u8 NumInEps;		/**< Number of in endpoint */
	u8 ControlDir;		/**< Control endpoint direction */
	u8 IsInTestMode;	/**< USB test mode flag */
	u8 TestMode;		/**< Test Mode */
	u8 Ep0State;		/**< Control EP state */
	u8 LinkState;		/**< Usb link state */
	u8 UnalignedTx;		/**< Unaligned transfer flag */
	u8 IsConfigDone;	/**< Flag - Check config is done or not */
	u8 IsThreeStage;	/**< USB three stage communication */
	u8 IsHibernated;	/**< Hibernated state */
	u8 HasHibernation;	/**< Has hibernation support */
	void *data_ptr;		/**< pointer for storing applications data */
}; /**< USB Device Controller representation */

/**
 * struct XUsbPsu_Event_Type - Device Endpoint Events type
 * @param : Device specific event
 * @param : Event types
 * @param : Reserved, not used
 */
#if defined (__ICCARM__)
#pragma pack(push, 1)
#endif
struct XUsbPsu_Event_Type {
	u32	Is_DevEvt:1;	/**< Device specific event */
	u32	Type:7;		/**< Event types */
	u32	Reserved8_31:24; /**< Reserved, not used */
#if defined (__ICCARM__)
}; /**< Device Endpoint Events type */
#pragma pack(pop)
#else
} __attribute__((packed)); /**< Device Endpoint Events type */
#endif
/**
 * struct XUsbPsu_event_depvt - Device Endpoint Events
 * @param Is_EpEvt: indicates this is an endpoint event
 * @param endpoint_number: number of the endpoint
 * @param endpoint_event: The event we have:
 *	0x00	- Reserved
 *	0x01	- XferComplete
 *	0x02	- XferInProgress
 *	0x03	- XferNotReady
 *	0x04	- RxTxFifoEvt (IN->Underrun, OUT->Overrun)
 *	0x05	- Reserved
 *	0x06	- StreamEvt
 *	0x07	- EPCmdCmplt
 * @param Reserved11_10: Reserved, don't use.
 * @param Status: Indicates the status of the event. Refer to databook for
 *	more information.
 * @param Parameters: Parameters of the current event. Refer to databook for
 *	more information.
 */
#if defined (__ICCARM__)
#pragma pack(push, 1)
#endif
struct XUsbPsu_Event_Epevt {
	u32	Is_EpEvt:1; /**< indicates this is an endpoint event */
	u32	Epnumber:5; /**< number of the endpoint */
	u32	Endpoint_Event:4; /**< endpoint event */
	u32	Reserved11_10:2; /**< Reserved, not used */
	u32	Status:4; /**< Indicates the status of the event */
	u32	Parameters:16; /**< Parameters of the current event */
#if defined (__ICCARM__)
}; /**< Device Endpoint Events */
#pragma pack(pop)
#else
} __attribute__((packed)); /**< Device Endpoint Events */
#endif
/**
 * struct XUsbPsu_event_devt - Device Events
 * @param Is_DevEvt: indicates this is a non-endpoint event
 * @param Device_Event: indicates it's a device event. Should read as 0x00
 * @param Type: indicates the type of device event.
 *	0	- DisconnEvt
 *	1	- USBRst
 *	2	- ConnectDone
 *	3	- ULStChng
 *	4	- WkUpEvt
 *	5	- Reserved
 *	6	- EOPF
 *	7	- SOF
 *	8	- Reserved
 *	9	- ErrticErr
 *	10	- CmdCmplt
 *	11	- EvntOverflow
 *	12	- VndrDevTstRcved
 * @param Reserved15_12: Reserved, not used
 * @param Event_Info: Information about this event
 * @param Reserved31_25: Reserved, not used
 */
#if defined (__ICCARM__)
#pragma pack(push, 1)
#endif
struct XUsbPsu_Event_Devt {
	u32	Is_DevEvt:1; /**< non-endpoint event */
	u32	Device_Event:7; /**< device event */
	u32	Type:4; /**< type of device event */
	u32	Reserved15_12:4; /**< Reserved, not used */
	u32	Event_Info:9; /**< Information about this event */
	u32	Reserved31_25:7; /**< Reserved, not used */
#if defined (__ICCARM__)
}; /**< Device Events */
#pragma pack(pop)
#else
} __attribute__((packed)); /**< Device Events */
#endif
/**
 * struct XUsbPsu_event_gevt - Other Core Events
 * @param one_bit: indicates this is a non-endpoint event (not used)
 * @param device_event: indicates it's (0x03) Carkit or (0x04) I2C event.
 * @param phy_port_number: self-explanatory
 * @param reserved31_12: Reserved, not used.
 */
#if defined (__ICCARM__)
#pragma pack(push, 1)
#endif
struct XUsbPsu_Event_Gevt {
	u32	Is_GlobalEvt:1; /**< non-endpoint event (not used)*/
	u32	Device_Event:7; /**< it's (0x03) Carkit or (0x04) I2C event */
	u32	Phy_Port_Number:4; /**< Phy_Port_Number:4 */
	u32	Reserved31_12:20; /**< reserved31_12 */
#if defined (__ICCARM__)
}; /**< Core events */
#pragma pack(pop)
#else
} __attribute__((packed)); /**< Core events */
#endif
/**
 * union XUsbPsu_event - representation of Event Buffer contents
 * @param Raw: raw 32-bit event
 * @param Type: type of the event
 * @param Epevt: Device Endpoint Event
 * @param Devt: Device Event
 * @param Gevt: Global Event
 */
union XUsbPsu_Event {
	u32				Raw;	/**< raw 32-bit event */
	struct XUsbPsu_Event_Type	Type;	/**< type of the event */
	struct XUsbPsu_Event_Epevt	Epevt;	/**< Device Endpoint Event */
	struct XUsbPsu_Event_Devt	Devt;	/**< Device Event */
	struct XUsbPsu_Event_Gevt	Gevt;	/**< Global Event */
}; /**< Representation of Event Buffers */

/************************** Variable Definitions *****************************/
extern XUsbPsu_Config XUsbPsu_ConfigTable[]; /**< Configuration table */

/***************** Macros (Inline Functions) Definitions *********************/
/** @cond INTERNAL */
#if defined (__ICCARM__)
#define IS_ALIGNED(x, a)	(((x) & ((u32)(a) - 1U)) == 0U) /**< parameter
								 * aligned
								 */
#else
#define IS_ALIGNED(x, a)	(((x) & ((typeof(x))(a) - 1U)) == 0U)
						/**< parameter aligned */
#endif

#if defined (__ICCARM__)
#define roundup(x, y) (((((x) + (u32)(y - 1U)) / (u32)y) * (u32)y))
						/**< roundup value based on
						 * input parameter
						 */

#else
#define roundup(x, y) (                                 \
        (((x) + (u32)((typeof(y))(y) - 1U)) / \
			(u32)((typeof(y))(y))) * \
				(u32)((typeof(y))(y))               \
)	/**< roundup value based on input parameter */
#endif
#define DECLARE_DEV_DESC(Instance, desc)			\
	(Instance).DevDesc = &(desc); 					\
	(Instance).DevDescSize = sizeof((desc)) /**< Device descriptor
						 * declaration
						 */

#define DECLARE_CONFIG_DESC(Instance, desc) 		\
	(Instance).ConfigDesc = &(desc); 				\
	(Instance).ConfigDescSize = sizeof((desc)) /**< Config descriptor
						    * declaration
						    */
/** @endcond */

/*****************************************************************************/
/**
* @brief
*
* This function returns the data pointer of driver instance.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be
*		worked on.
*
* @return	data pointer of driver instance.
*
* @note		None.
*
******************************************************************************/
static inline void *XUsbPsu_get_drvdata(struct XUsbPsu *InstancePtr) {
	return InstancePtr->data_ptr;
}

/*****************************************************************************/
/**
* @brief
*
* This function set driver data from driver like speed/state..
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be
*		worked on.
* @param	data is a void pointer
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static inline void XUsbPsu_set_drvdata(struct XUsbPsu *InstancePtr,
		void *data) {
	InstancePtr->data_ptr = data;
}

/*****************************************************************************/
/**
* @brief
*
* This function is used as chapter 9 interrupt handler.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be
*		worked on.
* @param	func USB Chapter9 function handler
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static inline void XUsbPsu_set_ch9handler(
		struct XUsbPsu *InstancePtr,
		void (*func)(struct Usb_DevData *, SetupPacket *)) {
	InstancePtr->Chapter9 = func;
}

/*****************************************************************************/
/**
* @brief
*
* This function handles USB resets.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be
*		worked on.
* @param	func USB Reset function handler
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static inline void XUsbPsu_set_rsthandler(
		struct XUsbPsu *InstancePtr,
		void (*func)(struct Usb_DevData *)) {
	InstancePtr->ResetIntrHandler = func;
}

/*****************************************************************************/
/**
* @brief
*
* This function is used as disconnect interrupt handler.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be
*		worked on.
* @param	func USB Disconnect function handler
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static inline void XUsbPsu_set_disconnect(
		struct XUsbPsu *InstancePtr,
		void (*func)(struct Usb_DevData *)) {
	InstancePtr->DisconnectIntrHandler = func;
}

/************************** Function Prototypes ******************************/

/*
 * Functions in xusbpsu.c
 */
s32 XUsbPsu_CfgInitialize(struct XUsbPsu *InstancePtr,
			XUsbPsu_Config *ConfigPtr, u32 BaseAddress);
s32 XUsbPsu_Start(struct XUsbPsu *InstancePtr);
s32 XUsbPsu_Stop(struct XUsbPsu *InstancePtr);
s32 XUsbPsu_SetU1SleepTimeout(struct XUsbPsu *InstancePtr, u8 Timeout);
s32 XUsbPsu_SetU2SleepTimeout(struct XUsbPsu *InstancePtr, u8 Timeout);
s32 XUsbPsu_AcceptU1U2Sleep(struct XUsbPsu *InstancePtr);
s32 XUsbPsu_U1SleepEnable(struct XUsbPsu *InstancePtr);
s32 XUsbPsu_U2SleepEnable(struct XUsbPsu *InstancePtr);
s32 XUsbPsu_U1SleepDisable(struct XUsbPsu *InstancePtr);
s32 XUsbPsu_U2SleepDisable(struct XUsbPsu *InstancePtr);
s32 XUsbPsu_IsSuperSpeed(struct XUsbPsu *InstancePtr);

/*
 * Functions in xusbpsu_command.c
 */
s32 XUsbPsu_EpEnable(struct XUsbPsu *InstancePtr, u8 UsbEpNum, u8 Dir,
				u16 Maxsize, u8 Type, u8 Restore);
s32 XUsbPsu_EpDisable(struct XUsbPsu *InstancePtr, u8 UsbEpNum, u8 Dir);

/*
 * Functions in xusbpsu_endpoint.c
 */
void XUsbPsu_ClearStalls(struct XUsbPsu *InstancePtr);
s32 XUsbPsu_EpBufferSend(struct XUsbPsu *InstancePtr, u8 UsbEp,
				u8 *BufferPtr, u32 BufferLen);
s32 XUsbPsu_EpBufferRecv(struct XUsbPsu *InstancePtr, u8 UsbEp,
				u8 *BufferPtr, u32 Length);
void XUsbPsu_EpSetStall(struct XUsbPsu *InstancePtr, u8 Epnum, u8 Dir);
void XUsbPsu_EpClearStall(struct XUsbPsu *InstancePtr, u8 Epnum, u8 Dir);
void XUsbPsu_SetEpHandler(struct XUsbPsu *InstancePtr, u8 Epnum,
			u8 Dir, void (*Handler)(void *, u32, u32));
s32 XUsbPsu_IsEpStalled(struct XUsbPsu *InstancePtr, u8 Epnum, u8 Dir);
void XUsbPsu_StopTransfer(struct XUsbPsu *InstancePtr, u8 UsbEpNum,
				u8 Dir, u8 Force);

/*
 * Functions in xusbpsu_intr.c
 */
void XUsbPsu_IntrHandler(void *XUsbPsuInstancePtr);
void XUsbPsu_EnableIntr(struct XUsbPsu *InstancePtr, u32 Mask);
void XUsbPsu_DisableIntr(struct XUsbPsu *InstancePtr, u32 Mask);

/*
 * Hibernation Functions
 */
#ifdef XUSBPSU_HIBERNATION_ENABLE
void XUsbPsu_WakeUpIntrHandler(void *XUsbPsuInstancePtr);
#endif

/*
 * Functions in xusbpsu_device.c
 */
s32 XUsbPsu_SetDeviceAddress(struct XUsbPsu *InstancePtr, u16 Addr);
void XUsbPsu_Idle(struct XUsbPsu *InstancePtr);
void XUsbPsu_SetSpeed(struct XUsbPsu *InstancePtr, u32 Speed);
void XUsbPsu_Sleep(u32 USeconds);

/*
 * Functions in xusbpsu_controltransfer.c
 */
void XUsbPsu_Ep0StallRestart(struct XUsbPsu *InstancePtr);

/*
 * Functions in xusbpsu_sinit.c
 */
XUsbPsu_Config *XUsbPsu_LookupConfig(u16 DeviceId);

#ifdef __cplusplus
}
#endif

#endif  /* End of protection macro. */
/** @} */
