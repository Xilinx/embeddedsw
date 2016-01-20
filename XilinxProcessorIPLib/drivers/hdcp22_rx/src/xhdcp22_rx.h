/******************************************************************************
*
* Copyright (C) 2015 - 2016 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file xhdcp22_rx.h
*
* This is the main header file for the Xilinx HDCP 2.2 Receiver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00  MH   10/30/15 First Release
* 1.01  MH   01/15/15 Added XHdcp22Rx_SetDdcReauthReq to function prototypes.
*                     Replaced function XHdcp22Rx_SetDdcHandles with
*                     XHdcp22Rx_SetCallback.
*</pre>
*
*****************************************************************************/

#ifndef XHDCP22_RX_H		/* prevent circular inclusions */
#define XHDCP22_RX_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xdebug.h"
#include "xtmrctr.h"
#include "xhdcp22_rx_i.h"
#include "xhdcp22_rng.h"
#include "xhdcp22_cipher.h"

/************************** Constant Definitions ****************************/
#define XHDCP22_RX_MAX_MESSAGE_SIZE		534
#define XHDCP22_RX_MAX_LCINIT			1024
#define XHDCP22_RX_DDC_VERSION_REG		0x50
#define XHDCP22_RX_DDC_WRITE_REG		0x60
#define XHDCP22_RX_DDC_RXSTATUS0_REG	0x70
#define XHDCP22_RX_DDC_RXSTATUS1_REG	0x71
#define XHDCP22_RX_DDC_READ_REG			0x80
#define XHDCP22_RX_TMR_CTR_0			0
#define XHDCP22_RX_TMR_CTR_1			1
#define XHDCP22_RX_TEST_DDC_REGMAP_SIZE	5

/************************** Variable Declaration ****************************/

/**************************** Type Definitions ******************************/
typedef void *(*XHdcp22_Rx_StateFunc)(void *InstancePtr);
typedef void (*XHdcp22_Rx_RunHandler)(void *HandlerRef);
typedef void (*XHdcp22_Rx_SetHandler)(void *HandlerRef, u32 Data);
typedef u32  (*XHdcp22_Rx_GetHandler)(void *HandlerRef);

/**
 * These constants are used to define the protocol.
 */
typedef enum
{
	XHDCP22_RX_HDMI,	/**< HDCP22 over HDMI */
	XHDCP22_RX_DP		/**< HDCP22 over DP, Not yet supported */
} XHdcp22_Rx_Protocol;

/**
 * These constants are used to define the mode.
 */
typedef enum
{
	XHDCP22_RX_RECEIVER,	/**< HDCP22 receiver */
	XHDCP22_RX_REPEATER,	/**< HDCP22 repeater upstream interface, Not yet supported */
	XHDCP22_RX_CONVERTER	/**< HDCP22 converter upstream interface, Not yet supported */
} XHdcp22_Rx_Mode;

/**
 * These constants are used to identify callback functions.
 */
typedef enum
{
	XHDCP22_RX_HANDLER_UNDEFINED,
	XHDCP22_RX_HANDLER_DDC_SETREGADDR,
	XHDCP22_RX_HANDLER_DDC_SETREGDATA,
	XHDCP22_RX_HANDLER_DDC_GETREGDATA,
	XHDCP22_RX_HANDLER_DDC_GETWBUFSIZE,
	XHDCP22_RX_HANDLER_DDC_GETRBUFSIZE,
	XHDCP22_RX_HANDLER_DDC_ISWBUFEMPTY,
	XHDCP22_RX_HANDLER_DDC_ISRBUFEMPTY,
	XHDCP22_RX_HANDLER_DDC_CLEARRBUF,
	XHDCP22_RX_HANDLER_DDC_CLEARWBUF,
	XHDCP22_RX_HANDLER_AUTHENTICATED,
	XHDCP22_RX_HANDLER_INVALID
} XHdcp22_Rx_HandlerType;

/**
 * These constants are the authentication and key exchange states.
 */
typedef enum
{
	XHDCP22_RX_STATE_B0_WAIT_AKEINIT			= 0xB00,	/**< Unauthenticated */
	XHDCP22_RX_STATE_B1_SEND_AKESENDCERT		= 0xB10,	/**< Compute Km: Send AKE_Send_Cert */
	XHDCP22_RX_STATE_B1_WAIT_AKEKM				= 0xB11,	/**< Compute Km: Wait for AKE_No_Stored_km or AKE_Stored_km */
	XHDCP22_RX_STATE_B1_SEND_AKESENDHPRIME		= 0xB12,	/**< Compute Km: Send AKE_Send_H_prime */
	XHDCP22_RX_STATE_B1_SEND_AKESENDPAIRINGINFO	= 0xB13,	/**< Compute Km: Send AKE_Send_Pairing_Info */
	XHDCP22_RX_STATE_B1_WAIT_LCINIT				= 0xB14,	/**< Compute Km: Wait for LCInit */
	XHDCP22_RX_STATE_B2_SEND_LCSENDLPRIME		= 0xB20,	/**< Compute L': Send LC_Send_L_prime */
	XHDCP22_RX_STATE_B2_WAIT_SKESENDEKS			= 0xB21,	/**< Compute L': Wait for SKE_Send_Eks */
	XHDCP22_RX_STATE_B3_COMPUTE_KS				= 0xB30,	/**< Compute Ks */
	XHDCP22_RX_STATE_B4_AUTHENTICATED			= 0xB40 	/**< Authenticated */
} XHdcp22_Rx_State;

/**
 * These constants define the authentication status.
 */
typedef enum
{
	XHDCP22_RX_STATUS_UNAUTHENTICATED,	/**< Unauthenticated */
	XHDCP22_RX_STATUS_COMPUTE_KM,		/**< Compute Km */
	XHDCP22_RX_STATUS_COMPUTE_LPRIME,	/**< Compute L' */
	XHDCP22_RX_STATUS_COMPUTE_KS,		/**< Compute Ks */
	XHDCP22_RX_STATUS_AUTHENTICATED		/**< Authenticated */
} XHdcp22_Rx_Status;

/**
 * These constants define the error conditions encountered during authentication and key exchange.
 */
typedef enum
{
	XHDCP22_RX_ERROR_FLAG_NONE						= 0,	/**< No errors */
	XHDCP22_RX_ERROR_FLAG_MESSAGE_SIZE				= 1,	/**< Message size error */
	XHDCP22_RX_ERROR_FLAG_FORCE_RESET				= 2,	/**< Force reset after error */
	XHDCP22_RX_ERROR_FLAG_PROCESSING_AKEINIT		= 4,	/**< AKE_Init message processing error */
	XHDCP22_RX_ERROR_FLAG_PROCESSING_AKENOSTOREDKM	= 8,	/**< AKE_No_Stored_km message processing error */
	XHDCP22_RX_ERROR_FLAG_PROCESSING_AKESTOREDKM	= 16,	/**< AKE_Stored_km message processing error */
	XHDCP22_RX_ERROR_FLAG_PROCESSING_LCINIT			= 32,	/**< LC_Init message processing error */
	XHDCP22_RX_ERROR_FLAG_PROCESSING_SKESENDEKS		= 64,	/**< SKE_Send_Eks message processing error */
	XHDCP22_RX_ERROR_FLAG_LINK_INTEGRITY			= 128,	/**< Link integrity check error */
	XHDCP22_RX_ERROR_FLAG_DDC_BURST					= 256,	/**< DDC message burst read/write error */
	XHDCP22_RX_ERROR_FLAG_MAX_LCINIT_ATTEMPTS		= 512	/**< Maximum LC_Init attempts error */
} XHdcp22_Rx_ErrorFlag;

/**
 * These constants defines the DDC flags used to determine when messages are available
 * in the write message buffer or when a message has been read out of the read message
 * buffer.
 */
typedef enum
{
	XHDCP22_RX_DDC_FLAG_NONE				= 0,	/**< Clear DDC flag */
	XHDCP22_RX_DDC_FLAG_WRITE_MESSAGE_READY	= 1,	/**< Complete message available in write buffer */
	XHDCP22_RX_DDC_FLAG_READ_MESSAGE_READY	= 2		/**< Complete message read out of read buffer */
} XHdcp22_Rx_DdcFlag;

/**
* These constants are the general logging events.
*/
typedef enum {
	XHDCP22_RX_LOG_EVT_NONE,			/**< Log Event None */
	XHDCP22_RX_LOG_EVT_INFO,			/**< Log General Info Event */
	XHDCP22_RX_LOG_EVT_INFO_STATE,		/**< Log State Info Event */
	XHDCP22_RX_LOG_EVT_INFO_MESSAGE,	/**< Log Messsage Info Event */
	XHDCP22_RX_LOG_EVT_DEBUG,			/**< Log Debug Event */
	XHDCP22_RX_LOG_EVT_ERROR,			/**< Log Error Event */
	XHDCP22_RX_LOG_EVT_USER,			/**< User logging */
	XHDCP22_RX_LOG_EVT_INVALID			/**< Last value the list, only used for checking */
} XHdcp22_Rx_LogEvt;

/**
 * These constants are the detailed logging events.
 */
typedef enum
{
	XHDCP22_RX_LOG_INFO_RESET,						/**< Reset event */
	XHDCP22_RX_LOG_INFO_ENABLE,						/**< Enable event */
	XHDCP22_RX_LOG_INFO_DISABLE,					/**< Disable event */
	XHDCP22_RX_LOG_INFO_REQAUTH_REQ,				/**< Reauthentication request */
	XHDCP22_RX_LOG_INFO_ENCRYPTION_ENABLE,			/**< Encryption enabled */
	XHDCP22_RX_LOG_INFO_WRITE_MESSAGE_AVAILABLE,	/**< Write message available */
	XHDCP22_RX_LOG_INFO_READ_MESSAGE_COMPLETE,		/**< Read message complete */
	XHDCP22_RX_LOG_DEBUG_COMPUTE_RSA,				/**< RSA decryption of Km computation start */
	XHDCP22_RX_LOG_DEBUG_COMPUTE_RSA_DONE,			/**< RSA decryption of Km computation done */
	XHDCP22_RX_LOG_DEBUG_COMPUTE_KM,				/**< Authentication Km computation start */
	XHDCP22_RX_LOG_DEBUG_COMPUTE_KM_DONE,			/**< Authentication Km computation done */
	XHDCP22_RX_LOG_DEBUG_COMPUTE_HPRIME,			/**< Authentication HPrime computation start */
	XHDCP22_RX_LOG_DEBUG_COMPUTE_HPRIME_DONE,		/**< Authentication HPrime computation done */
	XHDCP22_RX_LOG_DEBUG_COMPUTE_EKH,				/**< Pairing EKh computation start */
	XHDCP22_RX_LOG_DEBUG_COMPUTE_EKH_DONE,			/**< Pairing Ekh computation done */
	XHDCP22_RX_LOG_DEBUG_COMPUTE_LPRIME,			/**< Locality check LPrime computation start */
	XHDCP22_RX_LOG_DEBUG_COMPUTE_LPRIME_DONE,		/**< Locality check LPrime computation done */
	XHDCP22_RX_LOG_DEBUG_COMPUTE_KS,				/**< Session key exchange Ks computation start */
	XHDCP22_RX_LOG_DEBUG_COMPUTE_KS_DONE			/**< Session key exchange Ks computation done */
} XHdcp22_Rx_LogData;

/**
 * These constants are used for setting up the desired unit test for standalong testing.
 */
typedef enum {
	XHDCP22_RX_TEST_FLAG_NONE,			/**< No directed test */
	XHDCP22_RX_TEST_FLAG_NOSTOREDKM,	/**< Directed test flag for no stored Km */
	XHDCP22_RX_TEST_FLAG_STOREDKM,		/**< Directed test flag for no storeed Km then stored Km */
	XHDCP22_RX_TEST_FLAG_INVALID		/**< Last value the list, only used for checking */
} XHdcp22_Rx_TestFlags;

/**
 * These constants are used to set the cores test mode.
 */
typedef enum {
	XHDCP22_RX_TESTMODE_DISABLED,	/**< Test mode disabled */
	XHDCP22_RX_TESTMODE_NO_TX,		/**< Test mode to emulate transmitter internally used for unit testing */
	XHDCP22_RX_TESTMODE_SW_TX,		/**< Test mode to emulate transmitter externally used for loopback testing */
	XHDCP22_RX_TESTMODE_INVALID		/**< Last value the list, only used for checking */
} XHdcp22_Rx_TestMode;

/**
 * This typedef is the test structure used for standalone driver testing.
 */
typedef struct
{
	XHdcp22_Rx_TestFlags TestFlag;
	XHdcp22_Rx_TestMode TestMode;
	XHdcp22_Rx_TestState State;
	XHdcp22_Rx_TestState *NextStateVector;
	int TestReturnCode;
	u32 NextStateOffset;
	u32 NextStateSize;
	u32 NextStateStatus;
	XHdcp22_Rx_TestDdcReg DdcRegisterMap[XHDCP22_RX_TEST_DDC_REGMAP_SIZE];
	u32 DdcRegisterMapAddress;
	u32 DdcRegisterAddress;
	u8  WriteMessageBuffer[XHDCP22_RX_MAX_MESSAGE_SIZE];
	u32 WriteMessageSize;
	u32 WriteMessageOffset;
	u8  ReadMessageBuffer[XHDCP22_RX_MAX_MESSAGE_SIZE];
	u32 ReadMessageSize;
	u32 ReadMessageOffset;
	u32 WaitCounter;
	u8  Rrx[XHDCP22_RX_RRX_SIZE];
	u8  RxCaps[XHDCP22_RX_RXCAPS_SIZE];
	u8  Verbose;
} XHdcp22_Rx_Test;

/**
 * This typedef is used to store handles to function pointers
 */
typedef struct
{
	XHdcp22_Rx_SetHandler DdcSetAddressCallback;				/* Function pointer for setting DDC register address */
	void                  *DdcSetAddressCallbackRef;			/* To be passed to callback function */
	u8                    IsDdcSetAddressCallbackSet;			/* This flag is set true when the callback has been registered */

	XHdcp22_Rx_SetHandler DdcSetDataCallback;					/* Function pointer for setting DDC register data */
	void                  *DdcSetDataCallbackRef;				/* To be passed to callback function */
	u8                    IsDdcSetDataCallbackSet;				/* this flag is set true when the callback has been registered */

	XHdcp22_Rx_GetHandler DdcGetDataCallback;					/* Function pointer for getting DDC register data */
	void                  *DdcGetDataCallbackRef;				/* To be passed to callback function */
	u8                    IsDdcGetDataCallbackSet;				/* This flag is set true when the callback has been registered */

	XHdcp22_Rx_GetHandler DdcGetWriteBufferSizeCallback;		/* Function pointer for getting DDC write buffer message size */
	void                  *DdcGetWriteBufferSizeCallbackRef;	/* To be passed to callback function */
	u8                    IsDdcGetWriteBufferSizeCallbackSet;	/* This flag is set true when the callback has been registered */

	XHdcp22_Rx_GetHandler DdcGetReadBufferSizeCallback;			/* Function pointer for getting DDC read buffer message size */
	void                  *DdcGetReadBufferSizeCallbackRef;		/* To be passed to callback function */
	u8                    IsDdcGetReadBufferSizeCallbackRefSet;	/* This flag is set true when the callback has been registered */

	XHdcp22_Rx_GetHandler DdcIsWriteBufferEmptyCallback;		/* Function pointer for checking DDC write buffer empty */
	void                  *DdcIsWriteBufferEmptyCallbackRef;	/* To be passed to callback function */
	u8                    IsDdcIsWriteBufferEmptyCallbackSet;	/* This flag is set true when the callback has been registered */

	XHdcp22_Rx_GetHandler DdcIsReadBufferEmptyCallback;			/* Function pointer for checking DDC read buffer empty */
	void                  *DdcIsReadBufferEmptyCallbackRef;		/* To be passed to callback function */
	u8                    IsDdcIsReadBufferEmptyCallbackSet;	/* This flag is set true when the callback has been registered */

	XHdcp22_Rx_RunHandler DdcClearReadBufferCallback;			/* Function pointer for clearing DDC read buffer */
	void                  *DdcClearReadBufferCallbackRef;		/* To be passed to callback function */
	u8                    IsDdcClearReadBufferCallbackSet;		/* This flag is set true when the callback has been registered */

	XHdcp22_Rx_RunHandler DdcClearWriteBufferCallback;			/* Function pointer for clearing DDC write buffer */
	void                  *DdcClearWriteBufferCallbackRef;		/* To be passed to callback function */
	u8                    IsDdcClearWriteBufferCallbackSet;		/* This flag is set true when the callback has been registered */

	u8                    IsDdcAllCallbacksSet;					/* This flag is set true when all the DDC callbacks have been registered */

	XHdcp22_Rx_RunHandler AuthenticatedCallback;				/* Function pointer for clearing DDC write buffer */
	void                  *AuthenticatedCallbackRef;			/* To be passed to callback function */
	u8                    IsAuthenticatedCallbackSet;			/* This flag is set true when the callback has been registered */
} XHdcp22_Rx_Handles;

/**
 * This typedef is used to store temporary HDCP-RX parameters for computations
 */
typedef struct
{
	u8 Rtx[XHDCP22_RX_RTX_SIZE];
	u8 TxCaps[XHDCP22_RX_TXCAPS_SIZE];
	u8 Rrx[XHDCP22_RX_RRX_SIZE];
	u8 RxCaps[XHDCP22_RX_RXCAPS_SIZE];
	u8 Km[XHDCP22_RX_KM_SIZE];
	u8 HPrime[XHDCP22_RX_HPRIME_SIZE];
	u8 LPrime[XHDCP22_RX_LPRIME_SIZE];
	u8 EKh[XHDCP22_RX_EKH_SIZE];
	u8 Kd[XHDCP22_RX_KD_SIZE];
	u8 Rn[XHDCP22_RX_RN_SIZE];
	u8 Riv[XHDCP22_RX_RIV_SIZE];
	u8 Ks[XHDCP22_RX_KS_SIZE];
} XHdcp22_Rx_Parameters;

/**
 * This typedef is used to store logging events.
 */
typedef struct {
	u16 LogEvent;	/**< Event that has been triggered */
	u16 Data;		/**< Optional data */
	u32 TimeStamp;	/**< Timestamp on when event occured. Only used for time critical events */
} XHdcp22_Rx_LogItem;

/**
* This typedef contains the HDCP22 log list.
*/
typedef struct {
	XHdcp22_Rx_LogItem LogItems[256];	/**< Data */
	u8 Tail;							/**< Tail pointer */
	u8 Head;							/**< Head pointer */
	u8 Verbose;							/**< Logging is extended with debug events. */
} XHdcp22_Rx_Log;

/**
 * This typedef provides information about status of HDCP-RX authentication.
 */
typedef struct
{
	u8  IsEnabled;							/**< Flag indicates that device is enabled */
	u8  IsNoStoredKm;						/**< Flag indicates that AKE_No_Stored_Km message was received */
	u8  LCInitAttempts;						/**< Number of LC_Init attempts */
	u32 ErrorFlag;							/**< Flag to capture error events that require service, set using XHdcp22_Rx_ErrorFlag */
	u32 ErrorFlagSticky;					/**< Flag to capture all error conditions persistently */
	u32 DdcFlag;							/**< Flag to capture DDC events, set using XHdcp22_Rx_DdcFlag */
	XHdcp22_Rx_State CurrentState;			/**< State machine current state */
	XHdcp22_Rx_State NextState;				/**< State machine next state */
	XHdcp22_Rx_Status AuthenticationStatus;	/**< Authentication status */
} XHdcp22_Rx_Info;

/**
 * This typedef contains configuration information for the device.
 */
typedef struct
{
	u16 DeviceId;					/**< Unique ID of device */
	u32 BaseAddress;				/**< Base address of subsystem */
	XHdcp22_Rx_Protocol Protocol;	/**< HDCP22 over specified protocol */
	XHdcp22_Rx_Mode Mode;			/**< HDCP22 mode, receiver, repeater, or converter */
	u32 CipherDeviceId;				/**< Cipher device ID */
	u32 MontMultDeviceId;			/**< Mongomery multiplier device ID */
	u32 RngDeviceId;				/**< Random number generator device ID */
	u32 TimerDeviceId;				/**< Timer device ID */
} XHdcp22_Rx_Config;

/**
 * The XHdcp driver instance data. The user is required to
 * allocate a variable of this type for every HDCP-RX device in the
 * system. A pointer to a variable of this type is then passed to the driver
 * API functions.
 */
typedef struct
{
	XHdcp22_Rx_Config Config;			/**< HDCP-RX config structure */
	u32 IsReady;						/**< Indicates device is initialized and ready */
	u8 RxCaps[XHDCP22_RX_RXCAPS_SIZE];	/**< RxCaps set during initialization */
	const u8 *PublicCertPtr;			/**< DCP public certificate pointer */
	const u8 *PrivateKeyPtr;			/**< RSA private key pointer */
	const u8 *NPrimePPtr;				/**< Montgomery NPrimeP pointer */
	const u8 *NPrimeQPtr;				/**< Montgomery NPrimeQ pointer */
	XHdcp22_Rx_Info Info;				/**< HDCP-RX authentication and key exchange info */
	XHdcp22_Rx_Parameters Params;		/**< HDCP-RX authentication and key exchange parameters */
	XHdcp22_Rx_StateFunc StateFunc;		/**< State function pointer */
	XHdcp22_Rx_Handles Handles;			/**< Message handles */
	XHdcp22_Rx_Test Test;				/**< Test instance */
	XHdcp22_Rx_Log Log;					/**< Log instance */
	XHdcp22_mmult MmultInst;			/**< Montgomery multiplier instance */
	XTmrCtr TimerInst;					/**< Timer instance */
	XHdcp22_Rng RngInst;				/**< Random number generator instance */
	XHdcp22_Cipher CipherInst;			/**< Cipher instance */
	u8 MessageBuffer[XHDCP22_RX_MAX_MESSAGE_SIZE]; /**< Message structure */
	int MessageSize;					/**< Message size */
} XHdcp22_Rx;

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/
/* Functions for initializing and running driver */
XHdcp22_Rx_Config *XHdcp22Rx_LookupConfig(u16 DeviceId);
int  XHdcp22Rx_CfgInitialize(XHdcp22_Rx *InstancePtr, XHdcp22_Rx_Config *ConfigPtr, u32 EffectiveAddr);
int  XHdcp22Rx_Enable(XHdcp22_Rx *InstancePtr);
int  XHdcp22Rx_Disable(XHdcp22_Rx *InstancePtr);
int  XHdcp22Rx_Reset(XHdcp22_Rx *InstancePtr);
void XHdcp22Rx_Poll(XHdcp22_Rx *InstancePtr);
int  XHdcp22Rx_SetCallback(XHdcp22_Rx *InstancePtr, XHdcp22_Rx_HandlerType HandlerType, void *CallbackFunc, void *CallbackRef);

/* Functions for checking status */
u8   XHdcp22Rx_IsEnabled(XHdcp22_Rx *InstancePtr);
u8   XHdcp22Rx_IsEncryptionEnabled(XHdcp22_Rx *InstancePtr);
u8   XHdcp22Rx_IsInProgress(XHdcp22_Rx *InstancePtr);
u8   XHdcp22Rx_IsAuthenticated(XHdcp22_Rx *InstancePtr);
u8   XHdcp22Rx_IsError(XHdcp22_Rx *InstancePtr);

/* Functions used in callback routines */
void XHdcp22Rx_SetEncryptionEnabled(XHdcp22_Rx *InstancePtr);
void XHdcp22Rx_SetLinkError(XHdcp22_Rx *InstancePtr);
void XHdcp22Rx_SetDdcError(XHdcp22_Rx *InstancePtr);
void XHdcp22Rx_SetDdcReauthReq(XHdcp22_Rx *InstancePtr);
void XHdcp22Rx_SetWriteMessageAvailable(XHdcp22_Rx *InstancePtr);
void XHdcp22Rx_SetReadMessageComplete(XHdcp22_Rx *InstancePtr);

/* Functions for loading authentication constants */
int  XHdcp22Rx_CalcMontNPrime(u8 *NPrime, const u8 *N, int NDigits);
void XHdcp22Rx_LoadLc128(XHdcp22_Rx *InstancePtr, const u8 *Lc128Ptr);
void XHdcp22Rx_LoadPublicCert(XHdcp22_Rx *InstancePtr, const u8 *PublicCertPtr);
void XHdcp22Rx_LoadPrivateKey(XHdcp22_Rx *InstancePtr, const u8 *PrivateKeyPtr);
void XHdcp22Rx_LoadMontNPrimeP(XHdcp22_Rx *InstancePtr, const u8 *NPrimePPtr);
void XHdcp22Rx_LoadMontNPrimeQ(XHdcp22_Rx *InstancePtr, const u8 *NPrimeQPtr);
void XHdcp22Rx_LoadRxCaps(XHdcp22_Rx *InstancePtr, const u8 *RxCapsPtr);

/* Functions for logging */
void XHdcp22Rx_PrintDump(u8 Enable, char *String, const u8 *Data, int Length);
void XHdcp22Rx_LogReset(XHdcp22_Rx *InstancePtr, u8 Verbose);
u32  XHdcp22Rx_LogGetTimeUSecs(XHdcp22_Rx *InstancePtr);
void XHdcp22Rx_LogWr(XHdcp22_Rx *InstancePtr, u16 Evt, u16 Data);
XHdcp22_Rx_LogItem* XHdcp22Rx_LogRd(XHdcp22_Rx *InstancePtr);
void XHdcp22Rx_LogDisplay(XHdcp22_Rx *InstancePtr);

/* Functions for testing */
int  XHdcp22Rx_TestSetMode(XHdcp22_Rx *InstancePtr, XHdcp22_Rx_TestMode TestMode, XHdcp22_Rx_TestFlags TestVectorFlag);
int  XHdcp22Rx_TestRun(XHdcp22_Rx *InstancePtr);
u8   XHdcp22Rx_TestIsFinished(XHdcp22_Rx *InstancePtr);
u8   XHdcp22Rx_TestIsPassed(XHdcp22_Rx *InstancePtr);
int  XHdcp22Rx_TestLoadKeys(XHdcp22_Rx *InstancePtr);
void XHdcp22Rx_TestSetVerbose(XHdcp22_Rx *InstancePtr, u8 Verbose);

/* Function for test with DDC stub interface */
int  XHdcp22Rx_TestDdcWriteReg(XHdcp22_Rx *InstancePtr, u8 DeviceAddress, int Size, u8 *Data, u8 Stop);
int  XHdcp22Rx_TestDdcReadReg(XHdcp22_Rx *InstancePtr, u8 DeviceAddress, int Size, u8 *Data, u8 Stop);


#ifdef __cplusplus
}
#endif

#endif /* XHDCP22_RX_H_ */
