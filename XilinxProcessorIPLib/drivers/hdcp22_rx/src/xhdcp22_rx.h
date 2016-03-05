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
* @addtogroup hdcp22_rx_v1_0
* @{
* @details
*
* This is the main header file for the Xilinx HDCP 2.2 Receiver device
* driver. The HDCP 2.2 Receiver driver implements the authentication
* state machine.
* It consists of:
* - A state machine handling the states as specified in the HDCP revision 2.2
*   specification.
* - Message handling from/to the HDCP 2.2 transmitter.
* - Logging functionality including time stamps.
*
* <b>Software Initialization and Configuration</b>
*
* The application needs to do the following steps to run the Receiver.
* - Call XHdcp22Rx_LookupConfig using the device ID to find the
*   core configuration instance.
* - Call XHdcp22Rx_CfgInitialize to intitialize the device instance.
* - Call XHdcp22Rx_SetCallback to set the pointers to the callback
*   functions defined by the enumerated type XHdcp22_Rx_HandlerType.
* - Call XHdcp22Rx_CalcMontNPrime to calculate NPrimeP.
* - Call XHdcp22Rx_CalcMontNPrime to calculate NPrimeQ.
* - Call XHdcp22Rx_LoadPublicCert to load the DCP public certificate.
* - Call XHdcp22Rx_LoadPrivateKey to load the RSA private key.
* - Call XHdcp22Rx_LoadLc128 to load the DCP global constant.
* - Call XHdcp22Rx_LoadMontNPrimeP to load NPrimeP.
* - Call XHdcp22Rx_LoadMontNPrimeQ to load NPrimeQ.
* - Call XHdcp22Rx_LogReset to reset the log buffer.
* - The following functions should be called in the interfacing
*   protocol driver (i.e. HDMI) to set event flags:
*   - XHdcp22Rx_SetLinkError
*   - XHdcp22Rx_SetDdcError
*   - XHdcp22Rx_SetWriteMessageAvailable
*   - XHdcp22Rx_SetReadMessageComplete
* - Call XHdcp22Rx_Enable to enable the state machine.
* - Call XHdcp22Rx_Poll to run the Receiver state machine. The
*   call to this function is non-blocking and should be called
*   repeatedly in a spin loop as long as the receiver is active.
*
* <b>Interrupts</b>
*
* None.
*
* <b> Threads </b>
*
* This driver is not thread safe. Any needs for threads or thread mutual
* exclusion must be satisfied by the layer above this driver.
*
* <b> Asserts </b>
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining at
* compile time, the NDEBUG identifier. By default, asserts are turned on and it
* is recommended that users leave asserts on during development.
*
* <b> Building the driver </b>
*
* The HDCP TX driver is composed of several source files. This allows the user
* to build and link only those parts of the driver that are necessary.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00  MH   10/30/15 First Release
* 1.01  MH   01/15/16 Added XHdcp22Rx_SetDdcReauthReq to function prototypes.
*                     Replaced function XHdcp22Rx_SetDdcHandles with
*                     XHdcp22Rx_SetCallback.
* 1.02  MH   03/02/16 Updated to change NPrimeP and NPrimeQ from pointer
*                     to array. Added function XHDCP22Rx_GetVersion.
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
#include "xhdcp22_rng.h"
#include "xhdcp22_mmult.h"
#include "xhdcp22_cipher.h"

/************************** Constant Definitions ****************************/
/** Maximum LC_Init attempts */
#define XHDCP22_RX_MAX_LCINIT			1024
/** Address of DDC version regiser */
#define XHDCP22_RX_DDC_VERSION_REG		0x50
/** Address of DDC write message regiser */
#define XHDCP22_RX_DDC_WRITE_REG		0x60
/** Address of first DDC RxStatus register */
#define XHDCP22_RX_DDC_RXSTATUS0_REG	0x70
/** Address of second DDC RxStatus register */
#define XHDCP22_RX_DDC_RXSTATUS1_REG	0x71
/** Address of DDC read message regiser */
#define XHDCP22_RX_DDC_READ_REG			0x80
/** First timer counter */
#define XHDCP22_RX_TMR_CTR_0			0
/** Second timer counter */
#define XHDCP22_RX_TMR_CTR_1			1

/************************** Variable Declaration ****************************/

/**************************** Type Definitions ******************************/
/** Type for pointer to state function */
typedef void *(*XHdcp22_Rx_StateFunc)(void *InstancePtr);
/** Type for pointer to single input function */
typedef void (*XHdcp22_Rx_RunHandler)(void *HandlerRef);
/** Type for pointer to two input function */
typedef void (*XHdcp22_Rx_SetHandler)(void *HandlerRef, u32 Data);
/** Type for pointer to single input function with a return value */
typedef u32  (*XHdcp22_Rx_GetHandler)(void *HandlerRef);

/**
 * These constants are used to define the protocol.
 */
typedef enum
{
	/** HDCP22 over HDMI */
	XHDCP22_RX_HDMI,
	/** HDCP22 over DP, Not yet supported */
	XHDCP22_RX_DP
} XHdcp22_Rx_Protocol;

/**
 * These constants are used to define the mode.
 */
typedef enum
{
	/** HDCP22 receiver */
	XHDCP22_RX_RECEIVER,
	/** HDCP22 repeater upstream interface, Not yet supported */
	XHDCP22_RX_REPEATER,
	/** HDCP22 converter upstream interface, Not yet supported */
	XHDCP22_RX_CONVERTER
} XHdcp22_Rx_Mode;

/**
 * These constants are used to identify callback functions.
 */
typedef enum
{
	/** Undefined */
	XHDCP22_RX_HANDLER_UNDEFINED,
	/** Identifier for callback function used to set the DDC register address */
	XHDCP22_RX_HANDLER_DDC_SETREGADDR,
	/** Identifier for callback function used to set the DDC register data */
	XHDCP22_RX_HANDLER_DDC_SETREGDATA,
	/** Identifier for callback function used to get the DDC register data */
	XHDCP22_RX_HANDLER_DDC_GETREGDATA,
	/** Identifier for callback function used to get the DDC the write buffer size */
	XHDCP22_RX_HANDLER_DDC_GETWBUFSIZE,
	/** Identifier for callback function used to get the DDC the read buffer size */
	XHDCP22_RX_HANDLER_DDC_GETRBUFSIZE,
	/** Identifier for callback function used to check if the DDC write buffer is empty */
	XHDCP22_RX_HANDLER_DDC_ISWBUFEMPTY,
	/** Identifier for callback function used to check if the DDC read buffer is empty */
	XHDCP22_RX_HANDLER_DDC_ISRBUFEMPTY,
	/** Identifier for callback function used to clear the DDC read buffer */
	XHDCP22_RX_HANDLER_DDC_CLEARRBUF,
	/** Identifier for callback function used to clear the DDC write buffer */
	XHDCP22_RX_HANDLER_DDC_CLEARWBUF,
	/** Identifier for callback function used to execute user defined routine at authentication */
	XHDCP22_RX_HANDLER_AUTHENTICATED,
	/** Invalid */
	XHDCP22_RX_HANDLER_INVALID
} XHdcp22_Rx_HandlerType;

/**
 * These constants are the authentication and key exchange states.
 */
typedef enum
{
	/** Unauthenticated */
	XHDCP22_RX_STATE_B0_WAIT_AKEINIT			= 0xB00,
	/** Compute Km: Send AKE_Send_Cert */
	XHDCP22_RX_STATE_B1_SEND_AKESENDCERT		= 0xB10,
	/** Compute Km: Wait for AKE_No_Stored_km or AKE_Stored_km */
	XHDCP22_RX_STATE_B1_WAIT_AKEKM				= 0xB11,
	/** Compute Km: Send AKE_Send_H_prime */
	XHDCP22_RX_STATE_B1_SEND_AKESENDHPRIME		= 0xB12,
	/** Compute Km: Send AKE_Send_Pairing_Info */
	XHDCP22_RX_STATE_B1_SEND_AKESENDPAIRINGINFO	= 0xB13,
	/** Compute Km: Wait for LCInit */
	XHDCP22_RX_STATE_B1_WAIT_LCINIT				= 0xB14,
	/** Compute L': Send LC_Send_L_prime */
	XHDCP22_RX_STATE_B2_SEND_LCSENDLPRIME		= 0xB20,
	/** Compute L': Wait for SKE_Send_Eks */
	XHDCP22_RX_STATE_B2_WAIT_SKESENDEKS			= 0xB21,
	/** Compute Ks */
	XHDCP22_RX_STATE_B3_COMPUTE_KS				= 0xB30,
	/** Authenticated */
	XHDCP22_RX_STATE_B4_AUTHENTICATED			= 0xB40
} XHdcp22_Rx_State;

/**
 * These constants define the authentication status.
 */
typedef enum
{
	/** Unauthenticated */
	XHDCP22_RX_STATUS_UNAUTHENTICATED,
	/** Compute Km */
	XHDCP22_RX_STATUS_COMPUTE_KM,
	/** Compute L' */
	XHDCP22_RX_STATUS_COMPUTE_LPRIME,
	/** Compute Ks */
	XHDCP22_RX_STATUS_COMPUTE_KS,
	/** Authenticated */
	XHDCP22_RX_STATUS_AUTHENTICATED
} XHdcp22_Rx_Status;

/**
 * These constants define the error conditions encountered during authentication and key exchange.
 */
typedef enum
{
	/** No errors */
	XHDCP22_RX_ERROR_FLAG_NONE						= 0,
	/** Message size error */
	XHDCP22_RX_ERROR_FLAG_MESSAGE_SIZE				= 1,
	/** Force reset after error */
	XHDCP22_RX_ERROR_FLAG_FORCE_RESET				= 2,
	/** AKE_Init message processing error */
	XHDCP22_RX_ERROR_FLAG_PROCESSING_AKEINIT		= 4,
	/** AKE_No_Stored_km message processing error */
	XHDCP22_RX_ERROR_FLAG_PROCESSING_AKENOSTOREDKM	= 8,
	/** AKE_Stored_km message processing error */
	XHDCP22_RX_ERROR_FLAG_PROCESSING_AKESTOREDKM	= 16,
	/** LC_Init message processing error */
	XHDCP22_RX_ERROR_FLAG_PROCESSING_LCINIT			= 32,
	/** SKE_Send_Eks message processing error */
	XHDCP22_RX_ERROR_FLAG_PROCESSING_SKESENDEKS		= 64,
	/** Link integrity check error */
	XHDCP22_RX_ERROR_FLAG_LINK_INTEGRITY			= 128,
	/** DDC message burst read/write error */
	XHDCP22_RX_ERROR_FLAG_DDC_BURST					= 256,
	/** Maximum LC_Init attempts error */
	XHDCP22_RX_ERROR_FLAG_MAX_LCINIT_ATTEMPTS		= 512
} XHdcp22_Rx_ErrorFlag;

/**
 * These constants defines the DDC flags used to determine when messages are available
 * in the write message buffer or when a message has been read out of the read message
 * buffer.
 */
typedef enum
{
	/** Clear DDC flag */
	XHDCP22_RX_DDC_FLAG_NONE				= 0,
	/** Complete message available in write buffer */
	XHDCP22_RX_DDC_FLAG_WRITE_MESSAGE_READY	= 1,
	/** Complete message read out of read buffer */
	XHDCP22_RX_DDC_FLAG_READ_MESSAGE_READY	= 2
} XHdcp22_Rx_DdcFlag;

/**
* These constants are the general logging events.
*/
typedef enum {
	/** Log Event None */
	XHDCP22_RX_LOG_EVT_NONE,
	/** Log General Info Event */
	XHDCP22_RX_LOG_EVT_INFO,
	/** Log State Info Event */
	XHDCP22_RX_LOG_EVT_INFO_STATE,
	/** Log Messsage Info Event */
	XHDCP22_RX_LOG_EVT_INFO_MESSAGE,
	/** Log Debug Event */
	XHDCP22_RX_LOG_EVT_DEBUG,
	/** Log Error Event */
	XHDCP22_RX_LOG_EVT_ERROR,
	/** User logging */
	XHDCP22_RX_LOG_EVT_USER,
	/** Last value the list, only used for checking */
	XHDCP22_RX_LOG_EVT_INVALID
} XHdcp22_Rx_LogEvt;

/**
 * These constants are the detailed logging events.
 */
typedef enum
{
	/** Reset event */
	XHDCP22_RX_LOG_INFO_RESET,
	/** Enable event */
	XHDCP22_RX_LOG_INFO_ENABLE,
	/** Disable event */
	XHDCP22_RX_LOG_INFO_DISABLE,
	/** Reauthentication request */
	XHDCP22_RX_LOG_INFO_REQAUTH_REQ,
	/** Encryption enabled */
	XHDCP22_RX_LOG_INFO_ENCRYPTION_ENABLE,
	/** Write message available */
	XHDCP22_RX_LOG_INFO_WRITE_MESSAGE_AVAILABLE,
	/** Read message complete */
	XHDCP22_RX_LOG_INFO_READ_MESSAGE_COMPLETE,
	/** RSA decryption of Km computation start */
	XHDCP22_RX_LOG_DEBUG_COMPUTE_RSA,
	/** RSA decryption of Km computation done */
	XHDCP22_RX_LOG_DEBUG_COMPUTE_RSA_DONE,
	/** Authentication Km computation start */
	XHDCP22_RX_LOG_DEBUG_COMPUTE_KM,
	/** Authentication Km computation done */
	XHDCP22_RX_LOG_DEBUG_COMPUTE_KM_DONE,
	/** Authentication HPrime computation start */
	XHDCP22_RX_LOG_DEBUG_COMPUTE_HPRIME,
	/** Authentication HPrime computation done */
	XHDCP22_RX_LOG_DEBUG_COMPUTE_HPRIME_DONE,
	/** Pairing EKh computation start */
	XHDCP22_RX_LOG_DEBUG_COMPUTE_EKH,
	/** Pairing Ekh computation done */
	XHDCP22_RX_LOG_DEBUG_COMPUTE_EKH_DONE,
	/** Locality check LPrime computation start */
	XHDCP22_RX_LOG_DEBUG_COMPUTE_LPRIME,
	/** Locality check LPrime computation done */
	XHDCP22_RX_LOG_DEBUG_COMPUTE_LPRIME_DONE,
	/** Session key exchange Ks computation start */
	XHDCP22_RX_LOG_DEBUG_COMPUTE_KS,
	/** Session key exchange Ks computation done */
	XHDCP22_RX_LOG_DEBUG_COMPUTE_KS_DONE
} XHdcp22_Rx_LogData;

/**
 * This typedef is the test DDC register definition.
 */
typedef struct
{
	u8 Address;
	u8 Name[20];
	int Access;
	u8 Value;
} XHdcp22_Rx_TestDdcReg;

/**
 * This typedef is the test structure used for standalone driver testing.
 */
typedef struct
{
	int                    TestFlag;
	int                    TestMode;
	int                    State;
	int                    *NextStateVector;
	int                    TestReturnCode;
	u32                    NextStateOffset;
	u32                    NextStateSize;
	u32                    NextStateStatus;
	XHdcp22_Rx_TestDdcReg  DdcRegisterMap[5];
	u32                    DdcRegisterMapAddress;
	u32                    DdcRegisterAddress;
	u8                     WriteMessageBuffer[534];
	u32                    WriteMessageSize;
	u32                    WriteMessageOffset;
	u8                     ReadMessageBuffer[534];
	u32                    ReadMessageSize;
	u32                    ReadMessageOffset;
	u32                    WaitCounter;
	u8                     Rrx[8];
	u8                     RxCaps[3];
	u8                     Verbose;
} XHdcp22_Rx_Test;

/**
 * This typedef is used to store handles to function pointers
 */
typedef struct
{
	/** Function pointer used to set the DDC register address */
	XHdcp22_Rx_SetHandler DdcSetAddressCallback;
	/** To be passed to callback function */
	void                  *DdcSetAddressCallbackRef;
	/** This flag is set true when the callback has been registered */
	u8                    IsDdcSetAddressCallbackSet;

	/** Function pointer used to set the DDC register data */
	XHdcp22_Rx_SetHandler DdcSetDataCallback;
	/** To be passed to callback function */
	void                  *DdcSetDataCallbackRef;
	/** This flag is set true when the callback has been registered */
	u8                    IsDdcSetDataCallbackSet;

	/** Function pointer used to get the DDC register data */
	XHdcp22_Rx_GetHandler DdcGetDataCallback;
	/** To be passed to callback function */
	void                  *DdcGetDataCallbackRef;
	/** This flag is set true when the callback has been registered */
	u8                    IsDdcGetDataCallbackSet;

	/** Function pointer used to get the DDC write buffer size */
	XHdcp22_Rx_GetHandler DdcGetWriteBufferSizeCallback;
	/** To be passed to callback function */
	void                  *DdcGetWriteBufferSizeCallbackRef;
	/** This flag is set true when the callback has been registered */
	u8                    IsDdcGetWriteBufferSizeCallbackSet;

	/** Function pointer used to get the DDC read buffer size */
	XHdcp22_Rx_GetHandler DdcGetReadBufferSizeCallback;
	/** To be passed to callback function */
	void                  *DdcGetReadBufferSizeCallbackRef;
	/** This flag is set true when the callback has been registered */
	u8                    IsDdcGetReadBufferSizeCallbackRefSet;

	/** Function pointer used to check if the DDC write buffer is empty */
	XHdcp22_Rx_GetHandler DdcIsWriteBufferEmptyCallback;
	/** To be passed to callback function */
	void                  *DdcIsWriteBufferEmptyCallbackRef;
	/** This flag is set true when the callback has been registered */
	u8                    IsDdcIsWriteBufferEmptyCallbackSet;

	/** Function pointer used to check if the DDC read buffer is empty */
	XHdcp22_Rx_GetHandler DdcIsReadBufferEmptyCallback;
	/** To be passed to callback function */
	void                  *DdcIsReadBufferEmptyCallbackRef;
	/** This flag is set true when the callback has been registered */
	u8                    IsDdcIsReadBufferEmptyCallbackSet;

	/** Function pointer used to clear the DDC read buffer */
	XHdcp22_Rx_RunHandler DdcClearReadBufferCallback;
	/** To be passed to callback function */
	void                  *DdcClearReadBufferCallbackRef;
	/** This flag is set true when the callback has been registered */
	u8                    IsDdcClearReadBufferCallbackSet;

	/** Function pointer used to clear the DDC write buffer */
	XHdcp22_Rx_RunHandler DdcClearWriteBufferCallback;
	/** To be passed to callback function */
	void                  *DdcClearWriteBufferCallbackRef;
	/** This flag is set true when the callback has been registered */
	u8                    IsDdcClearWriteBufferCallbackSet;

	/** This flag is set true when all the DDC callbacks have been registered */
	u8                    IsDdcAllCallbacksSet;

	/** Function pointer for clearing DDC write buffer */
	XHdcp22_Rx_RunHandler AuthenticatedCallback;
	/** To be passed to callback function */
	void                  *AuthenticatedCallbackRef;
	/** This flag is set true when the callback has been registered */
	u8                    IsAuthenticatedCallbackSet;
} XHdcp22_Rx_Handles;

/**
 * This typedef is used to store temporary parameters for computations
 */
typedef struct
{
	u8 Rtx[8];
	u8 TxCaps[3];
	u8 Rrx[8];
	u8 RxCaps[3];
	u8 Km[16];
	u8 HPrime[32];
	u8 LPrime[32];
	u8 EKh[16];
	u8 Kd[32];
	u8 Rn[8];
	u8 Riv[8];
	u8 Ks[16];
} XHdcp22_Rx_Parameters;

/**
 * This typedef is used to store logging events.
 */
typedef struct {
	/** Event that has been triggered */
	u16 LogEvent;
	/** Optional data */
	u16 Data;
	/** Timestamp on when event occured. Only used for time critical events */
	u32 TimeStamp;
} XHdcp22_Rx_LogItem;

/**
* This typedef contains the HDCP22 log list.
*/
typedef struct {
	/** Data */
	XHdcp22_Rx_LogItem LogItems[256];
	/** Tail pointer */
	u8 Tail;
	/** Head pointer */
	u8 Head;
	/** Logging is extended with debug events. */
	u8 Verbose;
} XHdcp22_Rx_Log;

/**
 * This typedef provides information about status of HDCP-RX authentication.
 */
typedef struct
{
	/** Flag indicates that device is enabled */
	u8  IsEnabled;
	/** Flag indicates that AKE_No_Stored_Km message was received */
	u8  IsNoStoredKm;
	/** Number of LC_Init attempts */
	u8  LCInitAttempts;
	/** Flag to capture error events that require service, set using XHdcp22_Rx_ErrorFlag */
	u32 ErrorFlag;
	/** Flag to capture all error conditions persistently */
	u32 ErrorFlagSticky;
	/** Flag to capture DDC events, set using XHdcp22_Rx_DdcFlag */
	u32 DdcFlag;
	/** State machine current state */
	XHdcp22_Rx_State CurrentState;
	/** State machine next state */
	XHdcp22_Rx_State NextState;
	/** Authentication status */
	XHdcp22_Rx_Status AuthenticationStatus;
} XHdcp22_Rx_Info;

/**
 * This typedef contains configuration information for the device.
 */
typedef struct
{
	/** Unique ID of device instance */
	u16 DeviceId;
	/** Base address of subsystem */
	u32 BaseAddress;
	/** HDCP22 over specified protocol (i.e. hdmi) */
	XHdcp22_Rx_Protocol Protocol;
	/** HDCP22 mode (i.e. receiver, repeater, or converter) */
	XHdcp22_Rx_Mode Mode;
	/** Timer device instance ID */
	u32 TimerDeviceId;
	/** Cipher device instance ID */
	u32 CipherDeviceId;
	/** Mongomery multiplier device instance ID */
	u32 MontMultDeviceId;
	/** Random number generator device instance ID */
	u32 RngDeviceId;
} XHdcp22_Rx_Config;

/**
 * The XHdcp driver instance data. The user is required to
 * allocate a variable of this type for every HDCP-RX device in the
 * system. A pointer to a variable of this type is then passed to the driver
 * API functions.
 */
typedef struct
{
	/** HDCP-RX config structure */
	XHdcp22_Rx_Config Config;
	/** Indicates device is initialized and ready */
	u32 IsReady;
	/** RxCaps set during initialization */
	u8 RxCaps[3];
	/** DCP public certificate pointer */
	const u8 *PublicCertPtr;
	/** RSA private key pointer */
	const u8 *PrivateKeyPtr;
	/** Montgomery NPrimeP array */
	u8 NPrimeP[64];
	/** Montgomery NPrimeQ array */
	u8 NPrimeQ[64];
	/** HDCP-RX authentication and key exchange info */
	XHdcp22_Rx_Info Info;
	/** HDCP-RX authentication and key exchange parameters */
	XHdcp22_Rx_Parameters Params;
	/** State function pointer */
	XHdcp22_Rx_StateFunc StateFunc;
	/** Message handles */
	XHdcp22_Rx_Handles Handles;
	/** Log instance */
	XHdcp22_Rx_Log Log;
	/** Montgomery multiplier instance */
	XHdcp22_mmult MmultInst;
	/** Timer instance */
	XTmrCtr TimerInst;
	/** Random number generator instance */
	XHdcp22_Rng RngInst;
	/** Cipher instance */
	XHdcp22_Cipher CipherInst;
	/** Message structure */
	u8 MessageBuffer[534];
	/** Message size */
	int MessageSize;
#ifdef _XHDCP22_RX_TEST_
	/** Test instance */
	XHdcp22_Rx_Test Test;
#endif
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
u32  XHdcp22Rx_GetVersion(XHdcp22_Rx *InstancePtr);

/* Functions for checking status */
u8   XHdcp22Rx_IsEnabled(XHdcp22_Rx *InstancePtr);
u8   XHdcp22Rx_IsEncryptionEnabled(XHdcp22_Rx *InstancePtr);
u8   XHdcp22Rx_IsInProgress(XHdcp22_Rx *InstancePtr);
u8   XHdcp22Rx_IsAuthenticated(XHdcp22_Rx *InstancePtr);
u8   XHdcp22Rx_IsError(XHdcp22_Rx *InstancePtr);

/* Functions used in callback routines */
void XHdcp22Rx_SetLinkError(XHdcp22_Rx *InstancePtr);
void XHdcp22Rx_SetDdcError(XHdcp22_Rx *InstancePtr);
void XHdcp22Rx_SetWriteMessageAvailable(XHdcp22_Rx *InstancePtr);
void XHdcp22Rx_SetReadMessageComplete(XHdcp22_Rx *InstancePtr);
void XHdcp22Rx_SetDdcReauthReq(XHdcp22_Rx *InstancePtr);

/* Functions for loading authentication constants */
void XHdcp22Rx_LoadLc128(XHdcp22_Rx *InstancePtr, const u8 *Lc128Ptr);
void XHdcp22Rx_LoadPublicCert(XHdcp22_Rx *InstancePtr, const u8 *PublicCertPtr);
int  XHdcp22Rx_LoadPrivateKey(XHdcp22_Rx *InstancePtr, const u8 *PrivateKeyPtr);

/* Functions for logging */
void XHdcp22Rx_LogReset(XHdcp22_Rx *InstancePtr, u8 Verbose);
u32  XHdcp22Rx_LogGetTimeUSecs(XHdcp22_Rx *InstancePtr);
void XHdcp22Rx_LogWr(XHdcp22_Rx *InstancePtr, u16 Evt, u16 Data);
XHdcp22_Rx_LogItem* XHdcp22Rx_LogRd(XHdcp22_Rx *InstancePtr);
void XHdcp22Rx_LogDisplay(XHdcp22_Rx *InstancePtr);


#ifdef __cplusplus
}
#endif

#endif /* XHDCP22_RX_H_ */

/** @} */
