/******************************************************************************
*
* Copyright (C) 2014 - 2015 Xilinx, Inc.  All rights reserved.
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
*
* @file xhdcp22_tx.h
*
* This is the main header file for Xilinx HDCP22 TX core. HDCP22 TX core is
* used for authentication using HDCP (High-bandwidth Digital Content Protection)
* according the HDCP 2.2 specifications.
* It consists of:
* - A state machine handling the states as specified in the HDCP revision 2.2
*   specification.
* - Functionality for checking if the HDCP22 RX sink does respond within
*   specified times.
* - Message handling from/to the HDCP22 RX sink.
* - Logging functionality including time stamps.

* <b>Interrupts </b>
*
* HDCP22 TX uses a hardware timer interrupt. The interrupt controller that should
* be used, must be passed with the #XHdcp22Tx_SetInterruptController function.
*
* Application developer needs to register interrupt handler with the processor,
* within their examples. Whenever processor calls registered application's
* interrupt handler associated with interrupt id, application's interrupt
* handler needs to call appropriate peripheral interrupt handler reading
* peripheral's Status register.
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
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  JO     06/24/15 Initial release.
* </pre>
*
******************************************************************************/
#ifndef XHDCP22_TX_H
/**  prevent circular inclusions by using protection macros */
#define XHDCP22_TX_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>
#include "xparameters.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xdebug.h"
#include "xtmrctr.h"
#include "xhdcp22_cipher.h"
#include "xhdcp22_rng.h"

/************************** Constant Definitions *****************************/

/** Start polling if the timer is at (timeout - (timeout/DEFAULT_RX_STATUS_POLLVALUE)):
* - 0 : No polling
* - 1 : Always poll
* - 2 : 50% - start polling if 50% of timeout value has expired
* - 3 : 66%
* - 4 : 75% etc. */
#define XHDCP22_TX_DEFAULT_RX_STATUS_POLLVALUE  4

/* Test flags to trigger errors for unit tests. */

/** Use a certificate test vector. */
#define XHDCP22_TX_TEST_CERT_RX         0x00000001
/** Use a H_Prime test vector.*/
#define XHDCP22_TX_TEST_H1              0x00000002
/** Use a L_Prime test vector.*/
#define XHDCP22_TX_TEST_L1              0x00000004
/** Use a pairing info Ekh(Km) test vector, use i.c.w #XHDCP22_TX_TEST_RCV_TIMEOUT*/
#define XHDCP22_TX_TEST_EKH_KM          0x00000008
/** Invalidate a value */
#define XHDCP22_TX_TEST_INVALID_VALUE   0x00000010
/** Timeout on a received message. */
#define XHDCP22_TX_TEST_RCV_TIMEOUT     0x00000020
/** AKE is forced using a stored Km scenarion. Pairing info is pre-loaded
* with test vectors that forces Stored Km scenario.*/
#define XHDCP22_TX_TEST_STORED_KM       0x00000100
/** Disable timeout checking. this is mainly used to check the HDCP RX core in
* loopback mode in case it cannot meet timing requirements (no offloading to
* hardware) and reponsetimes need to be logged.*/
#define XHDCP22_TX_TEST_NO_TIMEOUT      0x00000200
/** Pairing info is cleared, to force a non-stored Km scenario */
#define XHDCP22_TX_TEST_CLR_PAIRINGINFO 0x00000400
/** DDC base address (0x74 >> 1) */
#define XHDCP22_TX_DDC_BASE_ADDRESS     0x3A

/**
* Needed storage to transmit/receive messages to the HDCP2.2 receiver
* This includes 1 extra byte for the DDC address used.
*/
#define  XHDCP22_TX_MAX_MESSAGE_SIZE    1+534

/**************************** Type Definitions *******************************/

/**
 * These constants are used to identify callback functions.
 */
typedef enum
{
	XHDCP22_TX_HANDLER_UNDEFINED,
	XHDCP22_TX_HANDLER_DDC_WRITE,
	XHDCP22_TX_HANDLER_DDC_READ,
	XHDCP22_TX_HANDLER_INVALID
} XHdcp22_Tx_HandlerType;

/**
* These constants specify the different states in the internal state machine.
*/
typedef enum
{
	XHDCP22_TX_STATE_H0,      /**< No Rx Attached. */
	XHDCP22_TX_STATE_H1,      /**< Transmit Low value content. */
	XHDCP22_TX_STATE_A0,      /**< Known HDCP2 Capable Rx. */
	XHDCP22_TX_STATE_A1,      /**< Exchange Km. */
	XHDCP22_TX_STATE_A1_1,    /**< substate of A1: wait for AKE_SEND_CERT. */
	XHDCP22_TX_STATE_A1_NSK0, /**< No stored Km substate of A1: wait for AKE_SEND_H_PRIME. */
	XHDCP22_TX_STATE_A1_NSK1, /**< No stored Km substate of A1, wait for AKE_SEND_PAIRING_INFO. */
	XHDCP22_TX_STATE_A1_SK0,  /**< Stored substate of A1, wait for AKE_SEND_PAIRING_INFO. */
	XHDCP22_TX_STATE_A2,      /**< Locality Check. */
	XHDCP22_TX_STATE_A2_1,    /**< Locality Check, receiving and verify L_Prime. */
	XHDCP22_TX_STATE_A3,      /**< Exchange Ks. */
	XHDCP22_TX_STATE_A4,      /**< Test for repeater. */
	XHDCP22_TX_STATE_A5,      /**< Authenticated. */
	XHDCP22_TX_STATE_A6,      /**< Wait for receiver ID list. */
	XHDCP22_TX_STATE_A7,      /**< Verify Receiver ID List. */
	XHDCP22_TX_STATE_A8,      /**< Send Receiver ID List acknowledgment. */
	XHDCP22_TX_STATE_A9,      /**< Content Stream Management. */
	XHDCP22_TX_NUM_STATES     /**< Number of states in the state machine. */
}XHdcp22_Tx_StateType;

/**
* These constants specify return values on polling with #XHdcp22Tx_Poll.
* Depending on this return value, encryption of the HDMI signal should be
* set to encrypted or un-encrypted. See also #XHdcp22Tx_EnableEncryption and
* #XHdcp22Tx_DisableEncryption
*/
typedef enum
{
	XHDCP22_TX_INCOMPATIBLE_RX,         /**< A HDCP2 compatible receiver is not found. */
	XHDCP22_TX_AUTHENTICATION_BUSY,     /**< Authentication is busy. */
	XHDCP22_TX_AUTHENTICATED,           /**< Authentication is completed successfully. */
	XHDCP22_TX_UNAUTHENTICATED,         /**< Authentication failed. */
	XHDCP22_TX_REAUTHENTICATE_REQUESTED /**< ReAuthentication requested.*/
} XHdcp22_Tx_AuthenticationType;

/**
* These constants are events as stored in the logging list.
*/
typedef enum {
	XHDCP22_TX_LOG_EVT_NONE,            /**< Log Event None. */
	XHDCP22_TX_LOG_EVT_STATE,           /**< State of the state machine. */
	XHDCP22_TX_LOG_EVT_POLL_RESULT,     /**< Authentication result of polling. */
	XHDCP22_TX_LOG_EVT_ENABLED,         /**< HDCP2.2 core is enabled or disabled. */
	XHDCP22_TX_LOG_EVT_RESET,           /**< HDCP2.2 core is reset. */
	XHDCP22_TX_LOG_EVT_ENCR_ENABLED,    /**< HDCP2.2 stream is encrypted or not. */
	XHDCP22_TX_LOG_EVT_TEST_ERROR,      /**< An error was detected in one of the test modes. */
	XHDCP22_TX_LOG_EVT_DBG,             /**< Log event for debugging. */
	XHDCP22_TX_LOG_EVT_LCCHK_COUNT,     /**< Number of times Locality check has been done. */
	XHDCP22_TX_LOG_EVT_USER,            /**< User logging. */
	XHDCP22_TX_LOG_INVALID              /**< Last value the list, only used for checking. */
} XHdcp22_Tx_LogEvt;

/**
* These constants are used to set the core into testing mode with #XHdcp22Tx_TestSetMode.
*/
typedef enum {
	XHDCP22_TX_TESTMODE_DISABLED,       /**< Testmode is disabled. */
	XHDCP22_TX_TESTMODE_SW_RX,          /**< Actual HDCP2.2 RX component is connected. */
	XHDCP22_TX_TESTMODE_NO_RX,          /**< HDCP2.2 RX software component is not available and will be emulated. */
	XHDCP22_TX_TESTMODE_UNIT,           /**< HDCP2.2 RX is emulated, #XHdcp22Tx_LogDisplay shows source code.*/
	XHDCP22_TX_TESTMODE_USE_TESTKEYS,   /**< Use test keys as defined in Errata to HDCP on HDMI Specification
	                                         Revision 2.2, February 09, 2015. */
	XHDCP22_TX_TESTMODE_INVALID         /**< Last value the list, only used for checking. */
} XHdcp22_Tx_TestMode;

/**
 * These constants are used to define the used protocol.
 */
typedef enum {
	XHDCP22_TX_HDMI,                    /**< HDMI protocol. */
	XHDCP22_TX_DP,                      /**< Display Port protocol. */
} XHdcp22_Tx_Protocol;

/**
 * These constants are used to define the used mode.
 */
typedef enum {
	XHDCP22_TX_TRANSMITTER,             /**< Module acts as a HDCP 2.2 transmitter. */
	XHDCP22_TX_REPEATER,                /**< Module acts as a HDCP 2.2 repeater.  */
	XHDCP22_TX_CONVERTER                /**< Module acts as a HDCP 2.2 converter.  */
} XHdcp22_Tx_Mode;


/**
* Callback type for status.
*
* @param  CallbackRef is a callback reference passed in by the upper
*         layer when setting the callback functions, and passed back to
*         the upper layer when the callback is invoked.
*
* @return None.
*
* @note   None.
*
*/
typedef void (*XHdcp22_Tx_Callback)(void *CallbackRef);

/**
* This typedef contains configuration information for the device.
*/
typedef struct
{
	/** DeviceId is the unique ID of the device. */
	u16 DeviceId;
	/** Base Address is the physical base address of the device's registers. */
	u32 BaseAddress;
	/** HDMI or DP (Always HDCP22_TX_HDMI: Currently DP is not supported). */
	XHdcp22_Tx_Protocol Protocol;
	/** Future expansion. */
	XHdcp22_Tx_Mode Mode;
	/** DeviceId of the used cipher. */
	u16 CipherId;
	/** Device Id of the random generator. */
	u16 RngId;
	/** DeviceId of the internal used timer. */
	u16 TimerDeviceId;
} XHdcp22_Tx_Config;

/**
 * The current state and data for internal used timer.
 */
typedef struct
{
	/** Expiration flag set when the hardware timer has interrupted. */
	u8 TimerExpired;
	/** Keep track of why the timer was started (message or status checking). */
	u8 ReasonId;
	/** Keep track of the start value of the timer. */
	u32 InitialTicks;
	/** The hardware timer instance.*/
	XTmrCtr TmrCtr;
} XHdcp22_Tx_Timer;

/**
* This typedef contains information about the HDCP22 transmitter.
*/
typedef struct
{
	XHdcp22_Tx_Protocol Protocol;       /**< Copy of configuration setting Protocol.*/

	/* state handling. */
	XHdcp22_Tx_StateType CurrentState;  /**< Current state of the internal state machine. */
	XHdcp22_Tx_StateType PrvState;      /**< Previous state of the internal state machine. */
	u8 Rtx[8];                          /**< Internal used Rtx. */
	u8 Rrx[8];                          /**< Internal used Rrx. */
	u8 Rn[8];                           /**< Internal used Rn. */
	void *StateContext;                 /**< Context used internally by the state machine. */
	u16  LocalityCheckCounter;          /**< Locality may attempt 1024 times. */
	u8 MsgAvailable;                    /**< Message is available for reading. */

	/** The result after a call to #XHdcp22Tx_Poll. */
	XHdcp22_Tx_AuthenticationType AuthenticationStatus;

	/** Is re-authentication requested by HDCP 2.2 RX. */
	u8 ReAuthenticationRequested;

	/** HDCP RX status read on timer interrupt. */
	u16 RxStatus;

	/** Is HDCP TX enabled (state machine is active). */
	u8 IsEnabled;

	/** Is HDMI data encryption enabled. */
	u8 IsEncryptionEnabled;

	/** Is the receiver a HDCP 2.2 type. */
	u8 IsReceiverHDCP2Capable;

	/** The currently used polling value see also #XHDCP22_TX_DEFAULT_RX_STATUS_POLLVALUE. */
	u32 PollingValue;
}XHdcp22_Tx_Info;

/**
 * This typedef is used to store logging events.
 */
typedef struct {
	u8  LogEvent;       /**< Event that has been triggered. */
	u16  Data;          /**< Optional data. */
	u32 TimeStamp;      /**< Time stamp on when event occurred. Only used for time critical events. */
} XHdcp22_Tx_LogItem;

/**
* This typedef contains the HDCP22 log list.
*/
typedef struct {
	XHdcp22_Tx_LogItem LogItems[256]; /**< Data. */
	u16 Tail;                         /**< Tail pointer. */
	u16 Head;                         /**< Head pointer. */
	u8 Verbose;                       /**< Logging is extended with debug events. */
} XHdcp22_Tx_Log;

/**
* This typedef contains the HDCP22 test parameters and settings.
*/
typedef struct {
  XHdcp22_Tx_TestMode TestMode; /**< Current used test mode. */
  u32 TestFlags;                /**< Current used test flags. */
  u8 CurrentDdcAddress;         /**< Current DDC address by the testing framework. */
} XHdcp22_Tx_Test;

/**
* Callback type used for calling DDC read and write functions.
*
* @param  DeviceAddress is the (i2c) device address of the HDCP port.
* @param  ByteCount is the amount of data bytes in the buffer to read or write.
* @param  BufferPtr is a pointer to a buffer that is used for reading or writing.
* @param  Stop is a flag to control if a stop token is set or not.
* @param  RefPtr is a callback reference passed in by the upper layer when setting
*         the DDC reading and writing functions, and passed back to the upper layer when
*         the callback is invoked.
*
* @return
*         - XST_SUCCESS The read action was successful.
*         - XST_FAILURE The read action failed.
*
* @note   None.
*
*/
typedef int (*XHdcp22_Tx_DdcHandler)(u8 DeviceAddress, u16 ByteCount, u8* BufferPtr,
             u8 Stop, void *RefPtr);

/**
* The XHdcpTx driver instance data. An instance must be allocated for each
* HDCP TX core in use.
*/
typedef struct
{
	/** Config */
	XHdcp22_Tx_Config Config;
	/** Is the component ready for usage. */
	u32 IsReady;

	/** Is the attached receiver HDCP2 capable. */
	u8 IsReceiverHDCP2Capable;

	/** Function pointer for reading DDC (Rx HDCP DeviceAddress: 0x74)
	    using the XHdcp22_Tx_Ddc stucture as parameter. */
	XHdcp22_Tx_DdcHandler  DdcRead;
	/** Set if DdcRead handler is defined. */
	u8 IsDdcReadSet;

	/** Function pointer for writing DDC (Rx HDCP DeviceAddress: 0x74)
	    using the XHdcp22_Tx_Ddc stucture as parameter. */
	XHdcp22_Tx_DdcHandler  DdcWrite;
	/** Set if DdcWrite handler is defined. */
	u8 IsDdcWriteSet;

	/** Reference pointer set with #XHdcp22Tx_SetCallback function. */
	void *DdcHandlerRef;

	/** Internal used timer. */
	XHdcp22_Tx_Timer Timer;

	/** Internal used hardware random number generator. */
	XHdcp22_Rng Rng;

	/** Internal used cipher. */
	XHdcp22_Cipher Cipher;

	/** Info. */
	XHdcp22_Tx_Info Info;

	/** Logging. */
	XHdcp22_Tx_Log Log;

	/** Testing. */
	XHdcp22_Tx_Test Test;

	/** Message buffer for messages that are sent/received. */
	u8 MessageBuffer[XHDCP22_TX_MAX_MESSAGE_SIZE];
}XHdcp22_Tx;


/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/* Initialization function in xhdcp22_tx_sinit.c */
XHdcp22_Tx_Config  *XHdcp22Tx_LookupConfig (u16 DeviceId);

/* Initialization and control functions in xhdcp_tx.c */
int XHdcp22Tx_CfgInitialize(XHdcp22_Tx *InstancePtr, XHdcp22_Tx_Config *CfgPtr,
                            u32 EffectiveAddr);
int XHdcp22Tx_Reset(XHdcp22_Tx *InstancePtr);
int XHdcp22Tx_ClearPairingInfo(XHdcp22_Tx *InstancePtr);
int XHdcp22Tx_Authenticate (XHdcp22_Tx *InstancePtr);
XHdcp22_Tx_AuthenticationType XHdcp22Tx_Poll(XHdcp22_Tx *InstancePtr);
int XHdcp22Tx_Enable (XHdcp22_Tx *InstancePtr);
int XHdcp22Tx_Disable (XHdcp22_Tx *InstancePtr);
int XHdcp22Tx_EnableEncryption (XHdcp22_Tx *InstancePtr);
int XHdcp22Tx_DisableEncryption (XHdcp22_Tx *InstancePtr);
u8 XHdcp22Tx_IsEnabled (XHdcp22_Tx *InstancePtr);
u8 XHdcp22Tx_IsEncryptionEnabled (XHdcp22_Tx *InstancePtr);
u8 XHdcp22Tx_IsInProgress (XHdcp22_Tx *InstancePtr);
u8 XHdcp22Tx_IsAuthenticated (XHdcp22_Tx *InstancePtr);

/* Set DDC handler function pointers. */
int XHdcp22Tx_SetCallback(XHdcp22_Tx *InstancePtr,
                          XHdcp22_Tx_HandlerType HandlerType,
                          void *CallbackFunc, void *CallbackRef);

/* Return the internal timer instance. */
XTmrCtr* XHdcp22Tx_GetTimer(XHdcp22_Tx *InstancePtr);

/* Logging and testing */
void XHdcp22Tx_LogReset(XHdcp22_Tx *InstancePtr, u8 Verbose);
void XHdcp22Tx_LogWr(XHdcp22_Tx *InstancePtr, XHdcp22_Tx_LogEvt Evt, u16 Data);
XHdcp22_Tx_LogItem* XHdcp22Tx_LogRd(XHdcp22_Tx *InstancePtr);
void XHdcp22Tx_LogDisplay(XHdcp22_Tx *InstancePtr);
void XHdcp22Tx_TestSetMode(XHdcp22_Tx *InstancePtr, XHdcp22_Tx_TestMode Mode,
                           u32 TestFlags);
u8 XHdcp22Tx_TestCheckResults(XHdcp22_Tx *InstancePtr,
                              XHdcp22_Tx_LogItem *Expected, u32 nExpected);
void XHdcp22Tx_SetMessagePollingValue(XHdcp22_Tx *InstancePtr, u32 PollingValue);

/* Functions for loading authentication constants */
void XHdcp22Tx_LoadLc128(XHdcp22_Tx *InstancePtr, const u8 *Lc128Ptr);

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
