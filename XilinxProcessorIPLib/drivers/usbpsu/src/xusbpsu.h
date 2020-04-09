/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xusbpsu.h
* @addtogroup usbpsu_v1_7
* @{
* @details
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
*
* </pre>
*
*****************************************************************************/
#ifndef XUSBPSU_H  /* Prevent circular inclusions */
#define XUSBPSU_H  /* by using protection macros  */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/

/* Enable XUSBPSU_HIBERNATION_ENABLE to enable hibernation */
//#define XUSBPSU_HIBERNATION_ENABLE		1

#include "xparameters.h"
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
#if defined (__arm__) || defined (__aarch64__) || (__ICCARM__)
#include "sleep.h"
#endif

#ifdef __MICROBLAZE__
#include "microblaze_sleep.h"
#endif
#include "xil_cache.h"

/************************** Constant Definitions ****************************/

#define NO_OF_TRB_PER_EP		4U

#if defined (PLATFORM_ZYNQMP) || defined (versal)
#define ALIGNMENT_CACHELINE		__attribute__ ((aligned(64)))
#else
#define ALIGNMENT_CACHELINE		__attribute__ ((aligned(32)))
#endif

#define	XUSBPSU_PHY_TIMEOUT		5000U /* in micro seconds */

#define XUSBPSU_EP_DIR_IN		1U
#define XUSBPSU_EP_DIR_OUT		0U

#define XUSBPSU_USB_DIR_OUT		0U	/* to device */
#define XUSBPSU_USB_DIR_IN		0x80U	/* to host */

#define XUSBPSU_ENDPOINT_XFERTYPE_MASK      0x03    /* in bmAttributes */
#define XUSBPSU_ENDPOINT_XFER_CONTROL       0U
#define XUSBPSU_ENDPOINT_XFER_ISOC          1U
#define XUSBPSU_ENDPOINT_XFER_BULK          2U
#define XUSBPSU_ENDPOINT_XFER_INT           3U
#define XUSBPSU_ENDPOINT_MAX_ADJUSTABLE     0x80

#define	XUSBPSU_TEST_J			1U
#define	XUSBPSU_TEST_K			2U
#define	XUSBPSU_TEST_SE0_NAK		3U
#define	XUSBPSU_TEST_PACKET		4U
#define	XUSBPSU_TEST_FORCE_ENABLE	5U

#define XUSBPSU_NUM_TRBS		8U

#define XUSBPSU_EVENT_PENDING		(0x00000001U << 0U)

#define XUSBPSU_EP_ENABLED		(0x00000001U << 0U)
#define XUSBPSU_EP_STALL		(0x00000001U << 1U)
#define XUSBPSU_EP_WEDGE		(0x00000001U << 2U)
#define XUSBPSU_EP_BUSY			((u32)0x00000001U << 4U)
#define XUSBPSU_EP_PENDING_REQUEST	(0x00000001U << 5U)
#define XUSBPSU_EP_MISSED_ISOC		(0x00000001U << 6U)

#define	XUSBPSU_GHWPARAMS0		0U
#define	XUSBPSU_GHWPARAMS1		1U
#define	XUSBPSU_GHWPARAMS2		2U
#define	XUSBPSU_GHWPARAMS3		3U
#define	XUSBPSU_GHWPARAMS4		4U
#define	XUSBPSU_GHWPARAMS5		5U
#define	XUSBPSU_GHWPARAMS6		6U
#define	XUSBPSU_GHWPARAMS7		7U

/* HWPARAMS0 */
#define XUSBPSU_MODE(n)			((n) & 0x7U)
#define XUSBPSU_MDWIDTH(n)		(((n) & 0xFF00) >> 8U)

/* HWPARAMS1 */
#define XUSBPSU_NUM_INT(n)		(((n) & (0x3F << 15U)) >> 15U)

/* HWPARAMS3 */
#define XUSBPSU_NUM_IN_EPS_MASK		((u32)0x0000001FU << (u32)18U)
#define XUSBPSU_NUM_EPS_MASK		((u32)0x0000003FU << (u32)12U)
#define XUSBPSU_NUM_EPS(p)		(((u32)(p) &		\
					(XUSBPSU_NUM_EPS_MASK)) >> (u32)12)
#define XUSBPSU_NUM_IN_EPS(p)		(((u32)(p) &		\
					(XUSBPSU_NUM_IN_EPS_MASK)) >> (u32)18)

/* HWPARAMS7 */
#define XUSBPSU_RAM1_DEPTH(n)		((n) & 0xFFFFU)

#define XUSBPSU_DEPEVT_XFERCOMPLETE	0x01U
#define XUSBPSU_DEPEVT_XFERINPROGRESS	0x02U
#define XUSBPSU_DEPEVT_XFERNOTREADY	0x03U
#define XUSBPSU_DEPEVT_STREAMEVT	0x06U
#define XUSBPSU_DEPEVT_EPCMDCMPLT	0x07U

/* Within XferNotReady */
#define DEPEVT_STATUS_TRANSFER_ACTIVE	(1U << 3U)

/* Within XferComplete */
#define DEPEVT_STATUS_BUSERR		(1U << 0U)
#define DEPEVT_STATUS_SHORT		(1U << 1U)
#define DEPEVT_STATUS_IOC		(1U << 2U)
#define DEPEVT_STATUS_LST		(1U << 3U)

/* Stream event only */
#define DEPEVT_STREAMEVT_FOUND		1U
#define DEPEVT_STREAMEVT_NOTFOUND	2U

/* Control-only Status */
#define DEPEVT_STATUS_CONTROL_DATA		1U
#define DEPEVT_STATUS_CONTROL_STATUS		2U
#define DEPEVT_STATUS_CONTROL_DATA_INVALTRB	9U
#define DEPEVT_STATUS_CONTROL_STATUS_INVALTRB	0xAU

#define XUSBPSU_ENDPOINTS_NUM			12U

#define XUSBPSU_EVENT_SIZE			4U       /* bytes */
#define XUSBPSU_EVENT_MAX_NUM			64U      /* 2 events/endpoint */
#define XUSBPSU_EVENT_BUFFERS_SIZE		(XUSBPSU_EVENT_SIZE * \
							XUSBPSU_EVENT_MAX_NUM)

#define XUSBPSU_EVENT_TYPE_MASK                 0x000000feU

#define XUSBPSU_EVENT_TYPE_DEV                  0U
#define XUSBPSU_EVENT_TYPE_CARKIT               3U
#define XUSBPSU_EVENT_TYPE_I2C                  4U

#define XUSBPSU_DEVICE_EVENT_DISCONNECT         0U
#define XUSBPSU_DEVICE_EVENT_RESET              1U
#define XUSBPSU_DEVICE_EVENT_CONNECT_DONE       2U
#define XUSBPSU_DEVICE_EVENT_LINK_STATUS_CHANGE 3U
#define XUSBPSU_DEVICE_EVENT_WAKEUP             4U
#define XUSBPSU_DEVICE_EVENT_HIBER_REQ          5U
#define XUSBPSU_DEVICE_EVENT_EOPF               6U
#define XUSBPSU_DEVICE_EVENT_SOF                7U
#define XUSBPSU_DEVICE_EVENT_ERRATIC_ERROR      9U
#define XUSBPSU_DEVICE_EVENT_CMD_CMPL           10U
#define XUSBPSU_DEVICE_EVENT_OVERFLOW           11U

#define XUSBPSU_GEVNTCOUNT_MASK                 0x0000fffcU

/*
 * Control Endpoint state
 */
#define	XUSBPSU_EP0_SETUP_PHASE			1U	/**< Setup Phase */
#define	XUSBPSU_EP0_DATA_PHASE			2U	/**< Data Phase */
#define	XUSBPSU_EP0_STATUS_PHASE		3U	/**< Status Pahse */

/*
 * Link State
 */
#define XUSBPSU_LINK_STATE_MASK			0x0FU

/**
 * @param XusbPsuLinkState This typedef defines link state.
 *
 */
typedef enum {
	XUSBPSU_LINK_STATE_U0 = 0x00U, /**< in HS - ON */
	XUSBPSU_LINK_STATE_U1 =	0x01U,
	XUSBPSU_LINK_STATE_U2 =	0x02U, /**< in HS - SLEEP */
	XUSBPSU_LINK_STATE_U3 =	0x03U, /**< in HS - SUSPEND */
	XUSBPSU_LINK_STATE_SS_DIS =	0x04U,
	XUSBPSU_LINK_STATE_RX_DET =	0x05U,
	XUSBPSU_LINK_STATE_SS_INACT = 0x06U,
	XUSBPSU_LINK_STATE_POLL	=	0x07U,
	XUSBPSU_LINK_STATE_RECOV =	0x08U,
	XUSBPSU_LINK_STATE_HRESET =	0x09U,
	XUSBPSU_LINK_STATE_CMPLY =	0x0AU,
	XUSBPSU_LINK_STATE_LPBK	=	0x0BU,
	XUSBPSU_LINK_STATE_RESET = 	0x0EU,
	XUSBPSU_LINK_STATE_RESUME =	0x0FU,
}XusbPsuLinkState;

/**
 * @param XusbPsuLinkStateChange This typedef defines link state change.
 *
 */
typedef enum {
	XUSBPSU_LINK_STATE_CHANGE_U0 = 0x00U, /**< in HS - ON */
	XUSBPSU_LINK_STATE_CHANGE_SS_DIS =	0x04U,
	XUSBPSU_LINK_STATE_CHANGE_RX_DET =	0x05U,
	XUSBPSU_LINK_STATE_CHANGE_SS_INACT = 0x06U,
	XUSBPSU_LINK_STATE_CHANGE_RECOV =	0x08U,
	XUSBPSU_LINK_STATE_CHANGE_CMPLY =	0x0AU,
}XusbPsuLinkStateChange;

/*
 * Device States
 */
#define		XUSBPSU_STATE_ATTACHED			0U
#define		XUSBPSU_STATE_POWERED			1U
#define		XUSBPSU_STATE_DEFAULT			2U
#define		XUSBPSU_STATE_ADDRESS			3U
#define		XUSBPSU_STATE_CONFIGURED		4U
#define		XUSBPSU_STATE_SUSPENDED			5U

/*
 * Device Speeds
 */
#define		XUSBPSU_SPEED_UNKNOWN			0U
#define		XUSBPSU_SPEED_LOW			1U
#define		XUSBPSU_SPEED_FULL			2U
#define		XUSBPSU_SPEED_HIGH			3U
#define		XUSBPSU_SPEED_SUPER			4U

/*
 * return Physical EP number as dwc3 mapping
 */
#define XUSBPSU_PhysicalEp(epnum, direction)	(((epnum) << 1U ) | (direction))

/**************************** Type Definitions ******************************/

/**
 * Software Event buffer representation
 */
struct XUsbPsu_EvtBuffer {
	void	*BuffAddr;
	u32		Offset;
	u32		Count;
	u32		Flags;
};

/**
 * Transfer Request Block - Hardware format
 */
#if defined (__ICCARM__)
#pragma pack(push, 1)
#endif
struct XUsbPsu_Trb {
	u32		BufferPtrLow;
	u32		BufferPtrHigh;
	u32		Size;
	u32		Ctrl;
#if defined (__ICCARM__)
};
#pragma pack(pop)
#else
} __attribute__((packed));
#endif

/*
 * Endpoint Parameters
 */
struct XUsbPsu_EpParams {
	u32	Param2;		/**< Parameter 2 */
	u32	Param1;		/**< Parameter 1 */
	u32	Param0;		/**< Parameter 0 */
};

/**
 * USB Standard Control Request
 */
#if defined (__ICCARM__)
#pragma pack(push, 1)
#endif
typedef struct {
        u8  bRequestType;
        u8  bRequest;
        u16 wValue;
        u16 wIndex;
        u16 wLength;
#if defined (__ICCARM__)
}SetupPacket;
#pragma pack(pop)
#else
} __attribute__ ((packed)) SetupPacket;
#endif

/**
 * Endpoint representation
 */
struct XUsbPsu_Ep {
	void (*Handler)(void *, u32, u32);
						/** < User handler called
						 *   when data is sent for IN Ep
						 *   and received for OUT Ep
						 */
#if defined (__ICCARM__)
    #pragma data_alignment = 64
	struct XUsbPsu_Trb EpTrb[NO_OF_TRB_PER_EP + 1U]; /**< One extra Trb is for Link Trb */
#else
	struct XUsbPsu_Trb EpTrb[NO_OF_TRB_PER_EP + 1U] ALIGNMENT_CACHELINE;/**< TRB used by endpoint */
#endif
	u32	EpStatus;	/**< Flags to represent Endpoint status */
	u32	EpSavedState;	/**< Endpoint status saved at the time of hibernation */
	u32	RequestedBytes;	/**< RequestedBytes for transfer */
	u32	BytesTxed;	/**< Actual Bytes transferred */
	u32	Interval;	/**< Data transfer service interval */
	u32	TrbEnqueue;
	u32	TrbDequeue;
	u16	MaxSize;	/**< Size of endpoint */
	u16	CurUf;		/**< current microframe */
	u8	*BufferPtr;	/**< Buffer location */
	u8	ResourceIndex;	/**< Resource Index assigned to
				 *  Endpoint by core
				 */
	u8	PhyEpNum;	/**< Physical Endpoint Number in core */
	u8	UsbEpNum;	/**< USB Endpoint Number */
	u8	Type;		/**< Type of Endpoint -
				 *	 Control/BULK/INTERRUPT/ISOC
				 */
	u8	Direction;	/**< Direction - EP_DIR_OUT/EP_DIR_IN */
	u8	UnalignedTx;
};

/**
 * This typedef contains configuration information for the USB
 * device.
 */
typedef struct {
        u16 DeviceId;		/**< Unique ID of controller */
        u32 BaseAddress;	/**< Core register base address */
	u8 IsCacheCoherent;	/**< Describes whether Cache Coherent or not */
	u8 EnableSuperSpeed;	/**< Set to enable super speed support */
#if defined (XCLOCKING)
	u32 RefClk;		/**< Input clocks */
#endif
} XUsbPsu_Config;

typedef XUsbPsu_Config Usb_Config;

struct Usb_DevData {
	u8 Speed;
	u8 State;

	void *PrivateData;
};

/**
 * USB Device Controller representation
 */
struct XUsbPsu {
#if defined (__ICCARM__)
    #pragma data_alignment = 64
	SetupPacket SetupData;
    #pragma data_alignment = 64
	struct XUsbPsu_Trb Ep0_Trb;
#else
	SetupPacket SetupData ALIGNMENT_CACHELINE;
					/**< Setup Packet buffer */
	struct XUsbPsu_Trb Ep0_Trb ALIGNMENT_CACHELINE;
#endif
					/**< TRB for control transfers */
	XUsbPsu_Config *ConfigPtr;	/**< Configuration info pointer */
	struct XUsbPsu_Ep eps[XUSBPSU_ENDPOINTS_NUM]; /**< Endpoints */
	struct XUsbPsu_EvtBuffer Evt;
	struct XUsbPsu_EpParams EpParams;
	u32 BaseAddress;	/**< Core register base address */
	u32 DevDescSize;
	u32 ConfigDescSize;
	struct Usb_DevData *AppData;
	void (*Chapter9)(struct Usb_DevData *, SetupPacket *);
	void (*ResetIntrHandler)(struct Usb_DevData *);
	void (*DisconnectIntrHandler)(struct Usb_DevData *);
	void *DevDesc;
	void *ConfigDesc;
#if defined(__ICCARM__)
    #pragma data_alignment = XUSBPSU_EVENT_BUFFERS_SIZE
	u8 EventBuffer[XUSBPSU_EVENT_BUFFERS_SIZE];
#else
	u8 EventBuffer[XUSBPSU_EVENT_BUFFERS_SIZE]
			__attribute__((aligned(XUSBPSU_EVENT_BUFFERS_SIZE)));
#endif
	u8 NumOutEps;
	u8 NumInEps;
	u8 ControlDir;
	u8 IsInTestMode;
	u8 TestMode;
	u8 Ep0State;
	u8 LinkState;
	u8 UnalignedTx;
	u8 IsConfigDone;
	u8 IsThreeStage;
	u8 IsHibernated;                /**< Hibernated state */
	u8 HasHibernation;              /**< Has hibernation support */
	void *data_ptr;		/* pointer for storing applications data */
};

#if defined (__ICCARM__)
#pragma pack(push, 1)
#endif
struct XUsbPsu_Event_Type {
	u32	Is_DevEvt:1;
	u32	Type:7;
	u32	Reserved8_31:24;
#if defined (__ICCARM__)
};
#pragma pack(pop)
#else
} __attribute__((packed));
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
	u32	Is_EpEvt:1;
	u32	Epnumber:5;
	u32	Endpoint_Event:4;
	u32	Reserved11_10:2;
	u32	Status:4;
	u32	Parameters:16;
#if defined (__ICCARM__)
};
#pragma pack(pop)
#else
} __attribute__((packed));
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
	u32	Is_DevEvt:1;
	u32	Device_Event:7;
	u32	Type:4;
	u32	Reserved15_12:4;
	u32	Event_Info:9;
	u32	Reserved31_25:7;
#if defined (__ICCARM__)
};
#pragma pack(pop)
#else
} __attribute__((packed));
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
	u32	Is_GlobalEvt:1;
	u32	Device_Event:7;
	u32	Phy_Port_Number:4;
	u32	Reserved31_12:20;
#if defined (__ICCARM__)
};
#pragma pack(pop)
#else
} __attribute__((packed));
#endif
/**
 * union XUsbPsu_event - representation of Event Buffer contents
 * @param raw: raw 32-bit event
 * @param type: the type of the event
 * @param depevt: Device Endpoint Event
 * @param devt: Device Event
 * @param gevt: Global Event
 */
union XUsbPsu_Event {
	u32				Raw;
	struct XUsbPsu_Event_Type	Type;
	struct XUsbPsu_Event_Epevt	Epevt;
	struct XUsbPsu_Event_Devt	Devt;
	struct XUsbPsu_Event_Gevt	Gevt;
};

/***************** Macros (Inline Functions) Definitions *********************/
#if defined (__ICCARM__)
#define IS_ALIGNED(x, a)	(((x) & ((u32)(a) - 1U)) == 0U)
#else
#define IS_ALIGNED(x, a)	(((x) & ((typeof(x))(a) - 1U)) == 0U)
#endif

#if defined (__ICCARM__)
#define roundup(x, y) (((((x) + (u32)(y - 1U)) / (u32)y) * (u32)y))

#else
#define roundup(x, y) (                                 \
        (((x) + (u32)((typeof(y))(y) - 1U)) / \
			(u32)((typeof(y))(y))) * \
				(u32)((typeof(y))(y))               \
)
#endif
#define DECLARE_DEV_DESC(Instance, desc)			\
	(Instance).DevDesc = &(desc); 					\
	(Instance).DevDescSize = sizeof((desc))

#define DECLARE_CONFIG_DESC(Instance, desc) 		\
	(Instance).ConfigDesc = &(desc); 				\
	(Instance).ConfigDescSize = sizeof((desc))

static inline void *XUsbPsu_get_drvdata(struct XUsbPsu *InstancePtr) {
	return InstancePtr->data_ptr;
}

static inline void XUsbPsu_set_drvdata(struct XUsbPsu *InstancePtr,
		void *data) {
	InstancePtr->data_ptr = data;
}

static inline void XUsbPsu_set_ch9handler(
		struct XUsbPsu *InstancePtr,
		void (*func)(struct Usb_DevData *, SetupPacket *)) {
	InstancePtr->Chapter9 = func;
}

static inline void XUsbPsu_set_rsthandler(
		struct XUsbPsu *InstancePtr,
		void (*func)(struct Usb_DevData *)) {
	InstancePtr->ResetIntrHandler = func;
}

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
s32 XUsbPsu_SetU1SleepTimeout(struct XUsbPsu *InstancePtr, u8 Sleep);
s32 XUsbPsu_SetU2SleepTimeout(struct XUsbPsu *InstancePtr, u8 Sleep);
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
void XUsbPsu_WakeUpIntrHandler(void *XUsbPsuInstancePtr);

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
