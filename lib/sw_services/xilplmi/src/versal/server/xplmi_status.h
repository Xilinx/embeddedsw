/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file versal/xplmi_status.h
*
* This is the header file which contains status codes for the PLM, PLMI
* and loader in versal platform
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   07/13/2018 Initial release
* 1.01  bsv  06/11/2019 Added TCM power up code to Xilloader to fix issue in
*                       R5-1 split mode functionality
*       bsv  06/17/2019 Added support for CFI and CFU error handling
*       bsv  06/26/2019 Added secondary boot support
*       vnsl 07/30/2019 Added code to load secure headers
*       ma   08/01/2019 Added LPD init code
*       ma   08/24/2019 Added SSIT commands
*       kc   08/28/2019 Added descriptions to PLM error codes
*       scs  08/29/2019 Added API to validate extended ID Code
* 1.02  bsv  10/31/2019 Added USB secondary boot mode support
*       kc   12/17/2019 Add deferred error mechanism for mask poll
*       bsv  02/12/2020 Added support for SD/eMMC raw boot mode
*       bsv  02/13/2020 XilPlmi generic commands should not be supported
*                       via IPI
*       ma   02/18/2020 Added event logging code
*       bsv  02/23/2020 Added multi partition support for SD/eMMC FS boot modes
*       kc   02/27/2020 Added SEM support for partial reconfiguration
*       bsv  02/28/2020 Added support for delay handoff
*       har  02/18/2020 Added major error code for Security
*       bsv  04/04/2020 Code clean up
*       kc   04/23/2020 Added interrupt support for SEU events
* 1.03  bsv  06/27/2020 Add dual stacked mode support
*       bsv  07/01/2020 Unmount file system after loading PDIs
*       skd  07/14/2020 Added a macro for DMA transfer error
*       kal  07/21/2020 Added a macro for Data copy failure for Security
*       bsv  07/29/2020 Added error codes related to delay load
*       kpt  07/30/2020 Added error code for Meta header length overflow
*       kc   08/04/2020 Added error code NPLL lock status for master SLR
*       har  08/11/2020 Added error code for authenticated JTAG
*       td   08/19/2020 Fixed MISRA C violations Rule 10.3
*       rama 08/21/2020 Added error code for STL
*       bm   09/21/2020 Added error codes for DFx Compatibility Check
*       bm   09/24/2020 Added error code for FuncID mismatch
*       bsv  10/13/2020 Code clean up
*       kpt  10/19/2020 Added error code for glitch detection
*       td   10/19/2020 MISRA C Fixes
* 1.04  td   11/23/2020 Added error code for XilPdi_ReadBootHdr failure
*       bsv  12/02/2020 Replaced SEM_CFR error code with generic SEM error code
*       bm   12/15/2020 Added Update Multiboot related error codes
*       bm   12/16/2020 Added error code when Secure code is not enabled
*       kpt  01/21/2021 Added error code for Auth Jtag revoke id failure
*       bsv  01/29/2021 Added APIs for checking and clearing NPI errors
*       har  02/01/2021 Added error code for mismatch of encryption key source
*       bsv  02/09/2021 Added error code for invalid PdiSrc in subsystem Pdi load
*       ma   02/12/2021 Added error code for IPI CRC mismatch and read error
*       kpt  02/16/2021 Added error codes for invalid key source when encryption
*                       only is enabled and when secure validations are failed
*       bsv  02/28/2021 Added code to avoid unaligned NPI writes
* 1.05  har  03/02/2021 Added error code for failure to update AAD
*       ma   03/04/2021 Added error code for PLMI IPI access failure
*       bm   03/04/2021 Added error code for invalid elf load address
*       skd  03/12/2021 Added error codes for PSM keep alive failure
*       bm   03/16/2021 Added error codes for Image Upgrade logic
*       har  03/17/2021 Updated description for XLOADER_ERR_SECURE_NOT_ALLOWED
*       bm   04/03/2021 Added error codes for task creation in scheduler
*       bm   04/10/2021 Added error codes for scheduler updates
*       rp   04/22/2021 Added error codes for request/release boot device
*       bm   05/10/2021 Added error codes for unsupported pdi version
*       har  05/19/2021 Added error code for encrypted partition in case of
*                       non secure state of boot
*       td   05/20/2021 Added error code for NPI locking error
* 1.06  kpt  06/23/2021 Added error code for DNA comparison failure
*       ma   06/28/2021 Added error codes related to proc command
*       ma   07/12/2021 Added error code for scheduler task missed error
*       bm   07/12/2021 Added error code for setting IRO frequency
*       bsv  07/16/2021 Fix doxygen warnings
*       bsv  07/16/2021 Added Macronix flash support
*       bm   08/09/2021 Cleared PMC CDO buffer by default after processing
*       bsv  08/17/2021 Code clean up
*       ma   08/30/2021 Added error code for SSIT errors
*       kpt  09/09/2021 Added error code XLOADER_ERR_SECURE_CLEAR_FAIL
* 1.07  ma   11/25/2021 Added error code XPLMI_ERR_PROC_INVALID_ADDRESS_RANGE
*       bsv  03/17/2022 Add support for A72 elfs to run from TCM
* 1.08  ma   05/10/2022 Added error codes related SSIT PLM to PLM communication
*       ma   06/21/2022 Added error codes related to Get Handoff Parameters
*       bm   07/06/2022 Refactor versal and versal_net code
*       ma   07/08/2022 Add support for Tamper Trigger over IPI
*       dc   07/12/2022 Moved buffer clear status to here from xilloader
*       ma   07/25/2022 Enhancements to secure lockdown code
*       ma   08/10/2022 Added error code XPLMI_SSIT_INTR_NOT_ENABLED
*       bm   08/24/2022 Support Begin, Break and End commands across chunk
*                       boundaries
* 1.09  ng   11/11/2022 Fixed doxygen file name error
* 1.08  skg  12/07/2022 Added error code for additional PPKs
*       bm   01/03/2023 Notify Other SLRs about Secure Lockdown
*       sk   01/11/2023 Updated error code for image store
*       bm   02/04/2023 Added support to return warnings
*       sk   02/23/2023 Added error code for SSIT slave EoPDI SYNC status
*       bm   03/09/2023 Add NULL check for module before using it
*       bm   03/11/2023 Added error code for XPlmi_PreInit failure
*       dd   03/28/2023 Updated doxygen comments
* 1.09  bm   06/13/2023 Add API to just log PLM error
*       bm   06/23/2023 Added error codes for ipi access filtering
*       bm   07/06/2023 Added XPLMI_ERR_MAX_RECURSIVE_CDO_PROCESS error code
*       am   07/07/2023 Added error code for Read IHT optional data
*       sk   07/20/2023 Corrected XPLMI_WARNING_MAJOR_MASK define
*       sk   07/31/2023 Moved Image Store error codes to plat header
*       yog  08/18/2023 Added error XLOADER_ERR_PLM_MH_SEC_MISMATCH error code
*       yog  08/25/2023 Removed XLOADER_ERR_PLM_MH_SEC_MISMATCH error code
*       mss  09/04/2023 Added error code for Null Check of EmInit params
* 2.0   ng   11/11/2023 Added error code for User modules
* 2.00  ng   01/26/2024 Updated minor error codes
*       am   03/02/2024 Added XLOADER_ERR_ZEROIZE_DIGEST_TABLE error code
*       mss  03/13/2024 MISRA-C violatiom Rule 17.8 fixed
* 2.1   mb   06/21/2024 Added error code for AES initialization fail
*       pre  07/11/2024 Implemented secure PLM to PLM communication
*       pre  08/16/2024 Added XPLMI_SSIT_NO_PENDING_EVENTS,XPLMI_SSIT_SECURE_COMM_KEYWRITE_FAILURE
*                       error codes
*       rama 09/05/2024 Added XPLM_ERR_STL_DIAG_TASK_CREATE, XPLM_ERR_STL_DIAG_TASK_REMOVE
*                       error codes for STL diagnostic task scheduling
*       pre  10/07/2024 Removed XPLMI_SSIT_NO_PENDING_EVENTS error code
*       ma   09/23/2024 Added error codes related to PSM to PLM IPI events
*       tri  03/13/2025 Added XLOADER_ERR_DATA_MEASUREMENT, XLOADER_PRTN_HASH_NOT_PRESENT error code
*       pre  03/19/2025 Added XPLMI_CMD_IN_PROGRESS error state
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPLMI_STATUS_H
#define XPLMI_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xstatus.h"

/************************** Constant Definitions *****************************/
/**
 * @name PLM error codes description
 *
 * 0xXXXXYYYY - Error code format
 * XXXX - Major error code - PLM/LOADER/XPLMI error codes
 *		as defined in xplmi_status.h
 * YYYY - Minor error code - Libraries / Drivers error code
 *		as defined in respective modules
 */
#define XPLMI_STATUS_MODULE_MASK			(0x4000FFFFU)
#define XPLMI_ERR_CDO_CMD_MASK				(0x1FFFU)
#define XPLMI_STATUS_SHIFT				(16U)
#define XPLMI_ERR_CODE_MASK				(0x7FFFFFFFU)

#define XPLMI_WARNING_STATUS_MASK			(0x40000000)

/*
 * Use these masks to change any major error or minor error into a warning.
 * It's expected to OR the error code with any of these masks as required.
 * This will set the 30th bit of Status which is ultimately used to classify
 * between error and warning. Note that this bit will be cleared before
 * printing or writing to the FW_ERR register.
 */
#define XPLMI_WARNING_MAJOR_MASK	0x4000 /**< To set warning in major error code */
#define XPLMI_WARNING_MINOR_MASK	0x40000000 /**< To set warning in minor error code */

/*
 * In case of failure of any security operation, the buffer must be
 * cleared.In case of success/failure in clearing the buffer,
 * the following error codes shall be updated in the status
 */
#define XLOADER_SEC_CHUNK_CLEAR_ERR		((u32)0x20U << 8U)
#define XLOADER_SEC_BUF_CLEAR_ERR		((u32)0x80U << 8U)
				/**< Error in clearing buffer */
#define XLOADER_SEC_BUF_CLEAR_SUCCESS	((u32)0x40U << 8U)
				/**< Buffer is successfully cleared */
/**
 * Status for PLM functions
 */
typedef enum {
	/* Status codes used in PLMI */
	/* XPLMI error codes common for all platforms are from 0x100 to 0x19F */
	XPLMI_ERR_DMA_LOOKUP = 0x100,	/**< 0x100 - Error when DMA driver
					  lookup fails. */
	XPLMI_ERR_DMA_CFG,		/**< 0x101 - Error when DMA driver
					  config fails. */
	XPLMI_ERR_DMA_SELFTEST,		/**< 0x102 - Error when DMA Self test
					  fails. It occurs when DMA is in reset
					  and PLM tries to initialize it */

	XPLMI_ERR_IOMOD_INIT,		/**< 0x103 - Error when IOModule
					  driver look up fails */
	XPLMI_ERR_IOMOD_START,		/**< 0x104 - Error when IOModule
					  driver startup fails */
	XPLMI_ERR_IOMOD_CONNECT,	/**< 0x105 - Error when IOModule
					  driver connection fails */

	XPLMI_ERR_MODULE_MAX,		/**< 0x106 - Error when PLMI module
					  is not registered. Can occur when
					  invalid CDO CMD is processed
					  by Xilplmi */
	XPLMI_ERR_CMD_APIID,		/**< 0x107 - Error when valid module
					  and unregistered CMD ID is
					  processed by xilplmi */
	XPLMI_ERR_CMD_HANDLER_NULL,	/**< 0x108 - Error when no command
					 handler is registered by module
					  for CDO CMD */
	XPLMI_ERR_RESERVED_109,		/**< 0x109 - XPLMI_ERR_RESERVED_109 */
	XPLMI_ERR_RESERVED_10A,		/**< 0x10A - XPLMI_ERR_RESERVED_10A*/

	XPLMI_ERR_CDO_HDR_ID,		/**< 0x10B - Error when valid CDO header
					  ID is not present in CDO header. Can
					  happen when different partition
					  type is processed as CDO */
	XPLMI_ERR_CDO_CHECKSUM,		/**< 0x10C - Error when CDO header
					  checksum is wrong. Can happen when
					  CDO header is corrupted */

	XPLMI_ERR_UART_DEV_PM_REQ,	/**< 0x10D - Error when XilPM request
					  device for UART fails. PM error code
					  is present in PLM minor code */
	XPLMI_ERR_UART_LOOKUP,		/**< 0x10E - Error when UART driver
					  lookup fails. */
	XPLMI_ERR_UART_CFG,		/**< 0x10F - Error when UART driver
					  config fails. */

	XPLMI_ERR_SSIT_MASTER_SYNC,	/**< 0x110 - Error when SSIT slave
					  sync fails with master */
	XPLMI_ERR_RESERVED_111, /**< 0x111 - XPLMI_ERR_RESERVED_111 */
	XPLMI_ERR_RESERVED_112, /**< 0x112 - XPLMI_ERR_RESERVED_112 */
	XPLMI_ERR_RESERVED_113, /**< 0x113 - XPLMI_ERR_RESERVED_113 */
	XPLMI_ERR_RESERVED_114, /**< 0x114 - XPLMI_ERR_RESERVED_114 */
	XPLMI_ERR_IPI_CMD, /**< 0x115 - Error when command execution through
						IPI is not supported because it's a CDO or
						due to access permission restriction. See
						minor error code for more details */
	XPLMI_ERR_REGISTER_IOMOD_HANDLER, /**< 0x116 - Error when registering
						IoModule Handler */
	XPLMI_ERR_RESERVED_117, /**< 0x117 - XPLMI_ERR_RESERVED_117 */
	XPLMI_ERR_RESERVED_118, /**< 0x118 - XPLMI_ERR_RESERVED_118 */
	XPLMI_ERR_RESERVED_119, /**< 0x119 - XPLMI_ERR_RESERVED_119 */
	XPLMI_ERR_INVALID_INTR_ID_DISABLE, /**< 0x11A Invalid Interrupt ID used
					     to disable interrupt */
	XPLMI_ERR_INVALID_INTR_ID_CLEAR,   /**< 0x11B Invalid Interrupt ID used
					     to clear interrupt */
	XPLMI_ERR_INVALID_INTR_ID_REGISTER, /**< 0x11C Invalid Interrupt ID used
					     to register interrupt handler */
	XPLMI_ERR_DMA_XFER_WAIT,	    /**< 0x11D Dma transfer wait failed */
	XPLMI_ERR_NON_BLOCK_DMA_WAIT_SRC,   /**< 0x11E Non Block Dma transfer wait
						failed in Src channel*/
	XPLMI_ERR_NON_BLOCK_DMA_WAIT_DEST,  /**< 0x11F Non Block Dma transfer wait
						failed in Dest channel WaitForDone */
	XPLMI_ERR_NON_BLOCK_SRC_DMA_WAIT,   /**< 0x120 Non Block Src Dma transfer
						wait failed */
	XPLMI_ERR_NON_BLOCK_DEST_DMA_WAIT,  /**< 0x121 Non Block Dest Dma transfer
						wait failed */
	XPLMI_ERR_DMA_XFER_WAIT_SRC,	    /**< 0x122 Dma Xfer failed in Src Channel
						wait for done */
	XPLMI_ERR_DMA_XFER_WAIT_DEST,	    /**< 0x123 Dma Xfer failed in Dest Channel
						wait for done */
	XPLMI_ERR_UART_MEMSET,		    /**< 0x124 Memset of UartPsv Instance failed */
	XPLMI_ERR_MEMCPY_COPY_CMD,		/**< 0x125 Error during memcpy of
						CdoCopyCmd */
	XPLMI_ERR_MEMCPY_CMD_EXEC,		/**< 0x126 Error during memcpy of
						CdoCmdExecute */
	XPLMI_ERR_MEMCPY_IMAGE_INFO,	/**< 0x127 Error during memcpy of
						XLoader_ImageInfo */
	XPLMI_ERR_UART_PSV_SET_BAUD_RATE,	/**< 0x128 Error during setting
						XUartPsv_SetBaudRate to XPLMI_UART_BAUD_RATE */
	XPLMI_ERR_IO_MOD_INTR_NUM_REGISTER, /**< 0x129 Invalid IoModule Interrupt
						Number used to register interrupt handler */
	XPLMI_ERR_IO_MOD_INTR_NUM_CLEAR,	/**< 0x12A Invalid IoModule interrupt
						Number used to clear interrupt */
	XPLMI_ERR_IO_MOD_INTR_NUM_DISABLE,	/**< 0x12B Invalid IoModule interrupt
						Number used to disable interrupt */
	XPLMI_NPI_ERR,	/**< 0x12C NPI errors */
	XPLMI_IPI_CRC_MISMATCH_ERR,	/**< 0x12D - IPI CRC mismatch error */
	XPLMI_IPI_READ_ERR, /**< 0x12E - Error in processing IPI request
						It could be due to invalid message length error when
						CRC is enabled or invalid buffer address error from
						driver */
	XPLMI_ERR_UNALIGNED_DMA_XFER,	/**< 0x12F - Error during DMA involving
			of unaligned SrcAddr, DestAddr or number of words */
	XPLMI_ERR_RESERVED1, /**< 0x130 - Reserved Error 1 */
	XPLMI_ERR_TASK_EXISTS,	/**< 0x131 - Error when the task that is being
						added to scheduler already exists */
	XPLMI_ERR_INVALID_TASK_TYPE, /**< 0x132 - Error when invalid task type is
						used to add tasks in scheduler */
	XPLMI_ERR_INVALID_TASK_PERIOD, /**< 0x133 - Error when invalid task period is
						used to add tasks in scheduler */
	XPLMI_ERR_NPI_LOCK, /**< 0x134 - Error locking NPI address space */
	XPLMI_ERR_RESERVED_135, /**< 0x135 - XPLMI_ERR_RESERVED_135. */
	XPLMI_ERR_RESERVED_136, /**< 0x136 - XPLMI_ERR_RESERVED_136. */
	XPLMI_ERR_RESERVED_137, /**< 0x137 - XPLMI_ERR_RESERVED_137. */
	XPLMI_ERR_RESERVED_138, /**< 0x138 - XPLMI_ERR_RESERVED_138. */
	XPLMI_ERR_SCHED_TASK_MISSED, /**< 0x139 Scheduler task missed executing
						at the scheduled interval */
	XPLMI_ERR_SET_PMC_IRO_FREQ, /**< 0x13A - Error when setting PMC IRO frequency
						is failed */
	XPLMI_ERR_RESERVED_13B, /**< 0x13B - XPLMI_ERR_RESERVED_13B. */
	XPLMI_ERR_PROC_INVALID_ADDRESS_RANGE, /**< 0x13C - Error when the given address
	                    range for storing Proc commands is invalid */
	XPLMI_ERR_CDO_CMD_BREAK_CHUNKS_NOT_SUPPORTED, /**< 0x13D - Error when end
						and break command are in separate chunks */
	XPLMI_ERR_RESERVED_13E, /**< 0x13E - XPLMI_ERR_RESERVED_13E. */
	XPLMI_ERR_RESERVED_13F, /**< 0x13F - XPLMI_ERR_RESERVED_13F */
	XPLMI_INVALID_BREAK_LENGTH, /**< 0x140 - Error when the break length required to jump
				      is less than the processed CDO length */
	XPLMI_ERR_MODULE_NOT_REGISTERED, /**< 0x141 - Error when the module of the CDO/IPI command
					   used is not registered */
	XPLMI_ERR_PRE_INIT,	/**< 0x142 - Error PLMI pre initialization failed */
	XPLMI_ERR_MAX_RECURSIVE_CDO_PROCESS, /**< 0x143 - Error when max recursive CDO processing
						has been occurred */
	XPLMI_ERR_EMINIT_INVALID_PARAM, /**< 0x144 Error if Params are Invalid */
	XPLMI_CMD_IN_PROGRESS, /**< 0x145 - Indicates command is in progress */

	/** Platform specific Status codes used in PLMI from 0x1A0 to 0x1FF */
	XPLMI_SSIT_EVENT_VECTOR_TABLE_IS_FULL = 0x1A0, /**< 0x1A0 - Error when the SSIT event
	                    vector table is full and PLM is trying to register a new
	                    event */
	XPLMI_SSIT_WRONG_EVENT_ORIGIN_MASK, /**< 0x1A1 - Error when the event origin is
	                    not valid */
	XPLMI_INVALID_SLR_TYPE, /**< 0x1A2 - Error when any SSIT event related APIs are
	                    called from SLR where it is not supported */
	XPLMI_INVALID_SLR_INDEX, /**< 0x1A3 - Error when the given SlrIndex is invalid */
	XPLMI_EVENT_NOT_SUPPORTED_BETWEEN_SLAVE_SLRS, /**< 0x1A4 - Error when the
	                    request to trigger an event between slave slrs which is not
	                    supported */
	XPLMI_SSIT_INVALID_EVENT, /**< 0x1A5 - Error when request is received to trigger
	                    an event which is not registered with PLM */
	XPLMI_SSIT_EVENT_IS_PENDING, /**< 0x1A6 - Error when an event is pending and the
	                    request comes to trigger the same again */
	XPLMI_SSIT_EVENT_NOT_ACKNOWLEDGED, /**< 0x1A7 - Error when the triggered event is
	                    not acknowledged within given TimeOut value */
	XPLMI_SSIT_BUF_SIZE_EXCEEDS, /**< 0x1A8 - Error when SSIT request or response
	                    buffer size exceeds */
	XPLMI_EVENT_NOT_SUPPORTED_FROM_SLR, /**< 0x1A9 - Error when the given
	                    event is not supported to be triggered from the running SLR */
	XPLMI_SSIT_EVENT_IS_NOT_PENDING, /**< 0x1AA - Error when an event is not pending and
		                the request comes to write to the response buffer */
	XPLMI_SSIT_INTR_NOT_ENABLED, /**< 0x1AB - SSIT interrupts are not enabled.
	                    Hence, cannot trigger the event */
	XPLMI_ERR_SSIT_EOPDI_SYNC, /**< 0x1AC - SSIT error in slave PDI load status SYNC
				     to master */
	XPLMI_ERR_SSIT_MSG_EVENT_NO_ACCESS, /**< 0x1AD - Error when the IPI message event
					received by Slave SLR is not permitted due to
					access permission violation */
	XPLMI_SSIT_SECURE_COMM_INVALID_SRCADDR, /**< 0X1AE - Error if configure secure
	                communication has invalid source address in payload */
    XPLMI_SSIT_SECURE_COMM_FAIL_AT_MASTER_ENCRYPTION, /**< 0x1AF - Error if secure plm
	                to plm communication fails at encryption on master side*/
	XPLMI_SSIT_SECURE_COMM_FAIL_AT_MASTER_DECRYPTION, /**< 0x1B0 - Error if secure plm
	                to plm communication fails at decryption on master side*/
	XPLMI_SSIT_SECURE_COMM_FAIL_AT_SLAVE_ENCRYPTION, /**< 0x1B1 - Error if secure plm
	                to plm communication fails at encryption on slave side*/
	XPLMI_SSIT_SECURE_COMM_FAIL_AT_SLAVE_DECRYPTION, /**< 0x1B2 - Error if secure plm
	                to plm communication fails at decryption on slave side*/
	XPLMI_IPI_MAX_BUF_SIZE_EXCEEDS, /**< 0x1B3 - Error when IPI request size exceeds */
	XPLMI_SSIT_SECURE_COMM_KEYWRITE_FAILURE, /**< 0x1B4 - Error if key write fails during
	                 secure plm to plm communication establishment */
	XPlMI_INVALID_BUFFER_INDEX_FOR_PSM_IPI_TASK, /**< 0x1B6 PSM IPI handler task received
						invalid buffer index*/
	XPLMI_IPI_PSM_TO_PLM_EVENT_INFO_INVALID, /**< 0x1B7 PSM IPI event structure not initialized */

	/** Status codes used in PLM */
	/* PLM error codes common for all platforms are from 0x200 to 0x29F */
	XPLM_ERR_TASK_CREATE = 0x200,	/**< 0x200 - Error when task create
					  fails. This can happen when max
					  tasks are created */
	XPLM_ERR_PM_MOD,		/**< 0x201 - Error initializing
					  the PM Module */
	XPLM_ERR_LPD_MOD,		/**< 0x202 - Error initializing
					  the LPD modules */
	XPLM_ERR_EXCEPTION,		/**< 0x203 - Exception has occurred
					  during PLM initialization. EAR and
					  ESR are printed on UART console
					  if enabled */
	XPLM_ERR_NPLL_LOCK,		/**< 0x204 - Unable to lock NOC PLL
					  for master SLR devices */
	XPLM_ERR_STL_MOD,		/**< 0x205 - Error initializing
					  the STL Module */
	XPLM_ERR_KEEP_ALIVE_TASK_CREATE,	/**< 0x206 - Error while
						  creating PSM keep alive
						  task */
	XPLM_ERR_KEEP_ALIVE_TASK_REMOVE,	/**< 0x207 - Error while
						  removing PSM keep alive
						  task */
	XPLM_ERR_PSM_NOT_ALIVE,			/**< 0x208 - PSM is not alive */
	XPLM_ERR_IPI_SEND,			/**< 0x209 - Error while sending
						     IPI */
	XPLM_ERR_PMC_RAM_MEMSET,	/**< 0x20A - Error while clearing the
						PMC CDO region in PMC RAM */
	XPLM_ERR_STL_DIAG_TASK_CREATE, /**< 0x20B - Error while creating
								STL diagnostic task */
	XPLM_ERR_STL_DIAG_TASK_REMOVE, /**< 0x20C - Error while removing
								STL diagnostic task */

	/** Status codes used in XLOADER */
	/* PLM error codes specific to platform are from 0x2A0 to 0x2FF */

	/* Xilloader error codes common for all platforms are from 0x300 to 0x39F */
	XLOADER_UNSUPPORTED_BOOT_MODE = 0x300, /**< 0x300 - Error for
					 unsupported bootmode. It occurs if
					 invalid boot mode is selected or
					 selected boot mode peripheral is
					 not selected in CIPS */
	XLOADER_ERR_IMGHDR_TBL = 0x302,	/**< 0x302 - Multiple conditions can
					  give this error.
					  - If PLM is unable to read
					  image header table */
	XLOADER_ERR_IMGHDR,		/**< 0x303 - Error if image header
					  checksum fails */
	XLOADER_ERR_PRTNHDR,		/**< 0x304 - Error if partition header
					  checksum fails */
	XLOADER_ERR_WAKEUP_A72_0,	/**< 0x305 - Error waking up the A72-0
					  during handoff. Check the PLM minor
					  code for PM error code. */
	XLOADER_ERR_WAKEUP_A72_1,	/**< 0x306 - Error waking up the A72-1
					  during handoff. Check the PLM minor
					  code for PM error code. */
	XLOADER_ERR_WAKEUP_R5_0,	/**< 0x307 - Error waking up the R5-0
					  during handoff. Check the PLM minor
					  code for PM error code. */
	XLOADER_ERR_WAKEUP_R5_1,	/**< 0x308 - Error waking up the R5-1
					  during handoff. Check the PLM minor
					  code for PM error code. */
	XLOADER_ERR_WAKEUP_R5_L,	/**< 0x309 - Error waking up the R5-L
					  during handoff. Check the PLM minor
					  code for PM error code. */
	XLOADER_ERR_WAKEUP_PSM,		/**< 0x30A - Error waking up the PSM
					  during handoff. Check the PLM minor
					  code for PM error code. */
	XLOADER_ERR_PL_NOT_PWRUP,	/**< 0x30B - Error powering up the PL */
	XLOADER_ERR_UNSUPPORTED_OSPI,	/**< 0x30C - Error for unsupported OSPI
					  flash */
	XLOADER_ERR_UNSUPPORTED_OSPI_SIZE, /**< 0x30D - Error for unsupported
					     OSPI flash size */
	XLOADER_ERR_OSPI_INIT,		/**< 0x30E - Error when OSPI driver
					  lookup fails. This happens when OSPI
					  is not selected in CIPS */
	XLOADER_ERR_OSPI_CFG,		/**< 0x30F - Error when OSPI driver
					  CFG fails */
	XLOADER_ERR_OSPI_SEL_FLASH_CS0,	/**< 0x310 - Error when OSPI driver is
					  unable to select flash CS0. Check minor
					  code for OSPI driver error code */
	XLOADER_ERR_OSPI_READID,	/**< 0x311 - Error when OSPI ReadID
					  fails */
	XLOADER_ERR_OSPI_READ,		/**< 0x312 - Error when OSPI driver read
					  fails. Check minor code for OSPI
					  driver error code */
	XLOADER_ERR_OSPI_4BMODE,	/**< 0x313 - Error when OSPI is unable
					  to enter/exit 4B mode */

	XLOADER_ERR_QSPI_READ_ID,	/**< 0x314 - Error when QSPI read
					  fails */
	XLOADER_ERR_UNSUPPORTED_QSPI_FLASH_ID,	/**< 0x315 - Error when QSPI flash ID is
							not supported */
	XLOADER_ERR_QSPI_INIT,		/**< 0x316 - Error when QSPI driver look
					  up or cfg fails */
	XLOADER_ERR_QSPI_MANUAL_START,	/**< 0x317 - Error when QSPI driver
					  manual start fails */
	XLOADER_ERR_QSPI_PRESCALER_CLK, /**< 0x318 - Error when QSPI driver
					  Prescalar setting fails */
	XLOADER_ERR_QSPI_CONNECTION,	/**< 0x319 - Error when invalid QSPI
					  connection listed other than Single,
					  Dual, stacked*/
	XLOADER_ERR_QSPI_READ,		/**< 0x31A - Error when QSPI driver
					  Read fails */
	XLOADER_ERR_QSPI_LENGTH,	/**< 0x31B - Error when QSPI read length
					  is greater than flash size */

	XLOADER_ERR_SD_INIT,		/**< 0x31C - Error when SD mount fails*/
	XLOADER_ERR_SD_F_OPEN,		/**< 0x31D - Error when SD file open
					  fails. This can happen when file is
					  not present or read from SD fails.
					  File system error code is present in
					  PLM minor code */
	XLOADER_ERR_SD_F_LSEEK,		/**< 0x31E - Error when f_seek fails
					  while reading from SD card */
	XLOADER_ERR_SD_F_READ,		/**< 0x31F - Error while reading from
					  SD card */

	XLOADER_ERR_IMG_ID_NOT_FOUND,	/**< 0x320 - Error when Image ID is
					  not found in subsystem while reloading
					  image */
	XLOADER_ERR_TCM_ADDR_OUTOF_RANGE, /**< 0x321 - Error while loading to
					    TCM and if address is out of range*/
	XLOADER_ERR_CFRAME_LOOKUP,	/**< 0x322 - Error when CFRAME driver
					  look up fails */
	XLOADER_ERR_CFRAME_CFG,		/**< 0x323 - Error when CFRAME driver
					  CFG fails */
	XLOADER_ERR_UNSUPPORTED_SEC_BOOT_MODE, /**< 0x324 - Error for
					 unsupported secondary boot mode */
	XLOADER_ERR_SECURE_METAHDR,	/**< 0x325 - Error when meta header
					  secure validations fail. */
	XLOADER_ERR_GEN_IDCODE,		/**< 0X326 - Error if Vivado configured part (IDCODE)
						mismatches with the actual part */
	XLOADER_ERR_USB_LOOKUP,		/**< 0x327 - Error when USB lookup fails*/
	XLOADER_ERR_USB_CFG,		/**< 0x328 - Error when USB cfg initialize fails */
	XLOADER_ERR_USB_START,		/**< 0x329 - Error when USB fails to start */
	XLOADER_ERR_DFU_DWNLD,	/**< 0x32A - Error when pdi fails to download */
	XLOADER_ERR_DEFERRED_CDO_PROCESS, /**< 0x32B - Error if the expected value is not
						same as actual value for the address polled
						using mask_poll CDO cmd, but the CDO command
						is configured to defer it till the whole CDO
						processing is complete. */
	XLOADER_ERR_SD_LOOKUP,		/**< 0x32C - Error when SD look up fails */
	XLOADER_ERR_SD_CFG,		/**< 0x32D - Error when SD config fails */
	XLOADER_ERR_SD_CARD_INIT,	/**< 0x32E - Error when SD card init fails */
	XLOADER_ERR_MMC_PART_CONFIG, /**< 0x32F - Error when MMC part
						configuration fails */
	XLOADER_ERR_SEM_STOP_SCAN,	/**< 0x330 - Error while stopping the
					  SEM Scan */
	XLOADER_ERR_SEM_INIT,	/**< 0x331 - Error while starting the
					  SEM Scan */
	XLOADER_ERR_DELAY_ATTRB,	/**< 0x332 - Error when both delay handoff
					  and delay load are set for the same image */
	XLOADER_ERR_NUM_HANDOFF_CPUS,	/**< 0x333 - Error when number of CPUs
						exceed max count */
	XLOADER_ERR_OSPI_CONN_MODE, /**< 0x334 - Error when OSPI mode is not
						supported */
	XLOADER_ERR_OSPI_SEL_FLASH_CS1,	/**< 0x335 - Error when OSPI driver is
					  unable to select flash CS1. Check minor
					  code for OSPI driver error code */
	XLOADER_ERR_OSPI_SDR_NON_PHY, /**< 0x336 - Error when OSPI driver is unable
						to set the controller to SDR NON PHY mode */
	XLOADER_ERR_OSPI_COPY_OVERFLOW, /**< 0x337 - Error when source address in
						OSPI copy exceeds flash size */
	XLOADER_ERR_SD_F_CLOSE,		/**< 0x338 - Error on closure of file in SD
							filesystem modes */
	XLOADER_ERR_SD_UMOUNT,		/**< 0X339 - Error on unmounting filesystem */
	XLOADER_ERR_DMA_XFER,		/**< 0x33A DMA Transfer failed */
	XLOADER_ERR_DMA_XFER_SD_RAW,		/**< 0x33B DMA Transfer failed in SD Raw*/
	XLOADER_ERR_CONFIG_SUBSYSTEM,		/**< 0X33C - Error while configuring subsystem */
	XLOADER_ERR_COPY_TO_MEM,		/**< 0x33D - Error on copying image to DDR with
						the copy to memory attribute enabled */
	XLOADER_ERR_DELAY_LOAD,		/**< 0x33E - When the image has delay load
						attribute set and the boot source is SMAP, SBI,
						PCIE or JTAG, the image is copied to PMC RAM to
						free it from the SBI buffers. Errors occurred
						during such copies to PMC RAM is denoted using
						this error code. */
	XLOADER_ERR_ADD_TASK_SCHEDULER,	/**<0x33F - Error while adding task to
						the scheduler */
	XLOADER_ERR_SD_MAX_BOOT_FILES_LIMIT,	/**< 0x340 - Error code returned when search
							for bootable file crosses max limit */
	XLOADER_ERR_UNSUPPORTED_QSPI_FLASH_SIZE, /**< 0x341 - Error when QSPI flash Size is
							not supported */
	XLOADER_ERR_PM_DEV_PSM_PROC,		/**< 0x342 - Failed in XPM Request Device for
							PM_DEV_PSM_PROC */
	XLOADER_ERR_PM_DEV_IOCTL_RPU0_SPLIT,	/**<0x343 - Failed in XPM Device Ioctl for
							RPU0_0 in SPLIT mode */
	XLOADER_ERR_PM_DEV_IOCTL_RPU1_SPLIT,	/**< 0x344 - Failed in XPM Device Ioctl for
							RPU0_1 in SPLIT mode */
	XLOADER_ERR_PM_DEV_IOCTL_RPU0_LOCKSTEP, /**< 0x345 - Failed to XPM Device Ioctl for
							RPU0_0 in LOCKSTEP mode */
	XLOADER_ERR_PM_DEV_IOCTL_RPU1_LOCKSTEP, /**< 0x346 - Failed to XPM Device Ioctl for
							RPU0_1 in LOCKSTEP mode */
	XLOADER_ERR_PM_DEV_TCM_0_A,		/**< 0x347 - Failed to XPM Request Device for
							PM_DEV_TCM_0_A */
	XLOADER_ERR_PM_DEV_TCM_0_B,		/**< 0x348 - Failed to XPM Request Device for
							PM_DEV_TCM_0_B */
	XLOADER_ERR_PM_DEV_TCM_1_A,		/**< 0x349 - Failed in XPM Request Device for
							PM_DEV_TCM_1_A */
	XLOADER_ERR_PM_DEV_TCM_1_B,		/**< 0x34A - Failed in XPM Request Device for
							PM_DEV_TCM_1_B */
	XLOADER_ERR_PM_DEV_DDR_0,		/**< 0x34B - Failed to XPM Request Device for
							PM_DEV_DDR_0 */
	XLOADER_ERR_PM_DEV_QSPI,		/**< 0x34C - Failed to XPM Request Device for
							PM_DEV_QSPI */
	XLOADER_ERR_PM_DEV_SDIO_0,		/**< 0x34D - Failed to XPM Request Device for
							PM_DEV_SDIO_0 */
	XLOADER_ERR_PM_DEV_SDIO_1,		/**< 0x34E - Failed to XPM Request Device for
							PM_DEV_SDIO_1 */
	XLOADER_ERR_PM_DEV_USB_0,		/**< 0x34F - Failed to XPM Request Device for
							PM_DEV_USB_0 */
	XLOADER_ERR_PM_DEV_OSPI,		/**< 0x350 - Failed to XPM Request Device for
							PM_DEV_OSPI */
	XLOADER_ERR_DEV_NOT_DEFINED,		/**< 0x351 - Device ID of the image to be
							loaded is not defined */
	XLOADER_ERR_PARENT_QUERY_VERIFY,	/**< 0x352 - Failed to Query Parent ID of
							an image while verifying its Image UIDs */
	XLOADER_ERR_INCOMPATIBLE_CHILD_IMAGE,	/**< 0x353 - Error while checking
							compatibility of a image with it's parent */
	XLOADER_ERR_NO_VALID_PARENT_IMG_ENTRY,	/**< 0x354 - Error if No Valid Parent
							Image Entry is found in the ImageInfo Table */
	XLOADER_ERR_INVALIDATE_CHILD_IMG,	/**< 0x355 - Error while Invalidating the
							Child Image Entry */
	XLOADER_ERR_INVALID_PARENT_IMG_ID,	/**< 0x356 - Error when Invalid ParentImgID
							is obtained when queried for parent ImgID */
	XLOADER_ERR_IMAGE_INFO_TBL_OVERFLOW,	/**< 0x357 - Error when ImageInfo Table
							overflows */
	XLOADER_ERR_FUNCTION_ID_MISMATCH,	/**< 0x358 - Error when Function ID given
							while loading Image from DDR is not matching with the ID
							stored in Image Header */
	XLOADER_ERR_MEMSET,	/**< 0x359 - Error during memset */
	XLOADER_DDR_COPY_UNSUPPORTED_PARAMS,	/**< 0x35A - Error when source address,
							destination address or length params passed to
							XLoader_DdrCopy are not word aligned */
	XLOADER_ERR_INIT_CDO,	/**< 0x35B - XPlmi_InitCdo failed */
	XLOADER_ERR_INVALID_ELF_LOAD_ADDR,	/**< 0x35C - Error when the load address of the
							elf is not valid */
	XLOADER_ERR_UNSUPPORTED_MULTIBOOT_FLASH_TYPE, /**<0X35D - Error due to unsupported Flash Type
							used with Update Multiboot command */
	XLOADER_ERR_UNSUPPORTED_MULTIBOOT_PDISRC, /**< 0x35E - Error due to unsupported PdiSrc
							used with Update Multiboot command */
	XLOADER_ERR_UNSUPPORTED_FILE_NUM,	/**< 0x35F - Error due to unsupported Filenum
							used to update multiboot register */
	XLOADER_ERR_UNSUPPORTED_MULTIBOOT_OFFSET, /**< 0x360 - Error when given multiboot offset
							is not valid (not a multiple of 32K) */
	XLOADER_ERR_SECURE_NOT_ENABLED,		/**< 0x361 - Error as secure critical code
							is excluded and Secure boot is attempted */
	XLOADER_ERR_UNSUPPORTED_SUBSYSTEM_PDISRC,	/**< 0x362 - Error when
						unsupported PdiSrc is used for
						subsystem load */
	XLOADER_ERR_RESERVED0,		/**< 0x363 - XLoader Reserved Error 0 */
	XLOADER_ERR_RESERVED1,		/**< 0x364 - XLoader Reserved Error 1 */
	XLOADER_ERR_RESERVED2,		/**< 0x365 - XLoader Reserved Error 2 */
	XLOADER_ERR_RESERVED3,		/**< 0x366 - XLoader Reserved Error 3 */
	XLOADER_ERR_RELEASE_PM_DEV_DDR_0,	/**< 0x367 - Failed to XPM Release Device for
							PM_DEV_DDR_0 */
	XLOADER_ERR_REQUEST_BOOT_DEVICE,	/**< 0x368 - Failed to Request Boot Device */
	XLOADER_ERR_RELEASE_BOOT_DEVICE,	/**< 0x369 - Failed to Release Boot Device */
	XLOADER_ERR_OSPI_DUAL_BYTE_OP_DISABLE,	/**< 0x36A - Failed to disable DUAL BYTE OP */
	XLOADER_ERR_INVALID_TCM_ADDR,	/**< 0x36B - Invalid TCM address for A72 elfs */
	XLOADER_ERR_RESERVED_36C, /**< 0x36C - XLOADER_ERR_RESERVED_36C */
	XLOADER_ERR_RESERVED_36D, /**< 0x36D - XLOADER_ERR_RESERVED_36D */
	XLOADER_ERR_RESERVED_36E, /**< 0x36E - XLOADER_ERR_RESERVED_36E */
	XLOADER_ERR_RESERVED_36F, /**< 0x36F - XLOADER_ERR_RESERVED_36F */
	XLOADER_ERR_RESERVED_370, /**< 0x370 - XLOADER_ERR_RESERVED_370 */
	XLOADER_SLD_DETECTED_SKIP_PRTN_PROCESS, /**< 0x371 - Skip processing rest of the partitions
						as secure lockdown has been triggered */
	XLOADER_ERR_RESERVED4, /**< 0x372 - XLoader Reserved Error 4 */
	XLOADER_ERR_ECDSA_NOT_ENABLED, /**< 0x373 - ECDSA code is excluded */
	XLOADER_ERR_RSA_NOT_ENABLED, /**< 0x374 - RSA code is excluded */
	XLOADER_ERR_INVALID_PRTNCOPY_DEST_ADDR,	/**< 0x375 - Invalid Destination address in
												PrtnCopy API for Raw Partition Loading*/

	/* Xilloader error codes specific to platform are from 0x3A0 to 0x3FF */

	/**< Security Major error codes */
	/* Security error codes common for all platforms are from 0x600 to 0x69F */
	XLOADER_ERR_INIT_GET_DMA = 0x600,
		/**< 0x600 Failed to get DMA instance at time of initialization */
	XLOADER_ERR_INIT_INVALID_CHECKSUM_TYPE,
		/**< 0x601 only SHA3 checksum is supported */
	XLOADER_ERR_INIT_CHECKSUM_COPY_FAIL,
		/**< 0x602 Failed when copying Checksum from flash device */
	XLOADER_ERR_INIT_AC_COPY_FAIL,
		/**< 0x603 Failed when copying AC from flash device */
	XLOADER_ERR_INIT_CHECKSUM_INVLD_WITH_AUTHDEC,
		/**< 0x604 Failed as checksum was enabled with authentication
				 and encryption enabled */

	XLOADER_ERR_DMA_TRANSFER,
		/**< 0x605 DMA Transfer failed while copying */

	XLOADER_ERR_IHT_AUTH_DISABLED,
		/**< 0x606 Authentication is not enabled for Image Header table */
	XLOADER_ERR_IHT_GET_DMA,
		/**< 0x607 Failed to get DMA instance for IHT authentication */
	XLOADER_ERR_IHT_COPY_FAIL,
		/**< 0x608 Failed when copying IHT AC from flash device */
	XLOADER_ERR_IHT_HASH_CALC_FAIL,
		/**< 0x609 Failed to calculate hash for IHT authentication */
	XLOADER_ERR_IHT_AUTH_FAIL,
		/**< 0x60A Failed to authenticate IHT */

	XLOADER_ERR_HDR_COPY_FAIL,
		/**< 0x60B Failed when copying IH/PH from flash device */
	XLOADER_ERR_HDR_AES_OP_FAIL,
		/**< 0x60C Failed due to AES init or Decrypt init or key selection
		failure */
	XLOADER_ERR_HDR_DEC_FAIL,
		/**< 0x60D Failed to decrypt IH/PH */
	XLOADER_ERR_HDR_AUTH_FAIL,
		/**< 0x60E Failed to authenticate IH/PH */
	XLOADER_ERR_HDR_NOT_SECURE,
		/**< 0x60F Neither authentication nor encryption enabled for IH/PH */
	XLOADER_ERR_HDR_GET_DMA,
		/**< 0x610 Failed to get DMA instance for IH/PH
		authentication/decryption */
	XLOADER_ERR_HDR_HASH_CALC_FAIL,
		/**< 0x611 Failed to calculate hash for IH/PH authentication */
	XLOADER_ERR_HDR_NOT_ENCRYPTED,
		/**< 0x612 IH/PH is not encrypted */
	XLOADER_ERR_HDR_AUTH_DISABLED,
		/**< 0x613 Authentication disabled for IH/PH */

	XLOADER_ERR_SEC_IH_READ_FAIL,
		/**< 0x614 Failed to read IH */
	XLOADER_ERR_SEC_PH_READ_FAIL,
		/**< 0x615 Failed to read PH */

	XLOADER_ERR_PRTN_HASH_CALC_FAIL,
		/**< 0x616 Hash calculation failed for partition authentication */
	XLOADER_ERR_PRTN_AUTH_FAIL,
		/**< 0x617 Partition authentication failed */
	XLOADER_ERR_PRTN_HASH_COMPARE_FAIL,
		/**< 0x618 Partition hash comparison failed */
	XLOADER_ERR_PRTN_DECRYPT_FAIL,
		/**< 0x619 Partition decryption failed */

	XLOADER_ERR_HWROT_EFUSE_AUTH_COMPULSORY,
		/**< 0x61A PPK Programmed but eFuse authentication is disabled */
	XLOADER_ERR_HWROT_BH_AUTH_NOT_ALLOWED,
		/**< 0x61B PPK Programmed and BH authentication is enabled */
	XLOADER_ERR_AUTH_EN_PPK_HASH_ZERO,
		/**< 0x61C PPK not programmed and authentication is enabled */
	XLOADER_ERR_ENCONLY_ENC_COMPULSORY,
		/**< 0x61D Encryption is disabled */
	XLOADER_ERR_KAT_FAILED,
		/**< 0x61E KAT failed */
	XLOADER_ERR_DATA_COPY_FAIL,
		/**< 0x61F Data copy to internal memory failed */
	XLOADER_ERR_METAHDR_LEN_OVERFLOW,
		/**< 0x620 Failed when total size is greater than Metahdr length */

	XLOADER_ERR_AUTH_JTAG_EFUSE_AUTH_COMPULSORY,
		/**< 0x621 Jtag Authentication failed when PPK not programmed */
	XLOADER_ERR_AUTH_JTAG_DISABLED,
		/**< 0x622 Jtag Authentication disable efuse bit is set */
	XLOADER_ERR_AUTH_JTAG_PPK_VERIFY_FAIL,
		/**< 0x623 Jtag Authentication failed when verification of PPK
		hash verification failed */
	XLOADER_ERR_AUTH_JTAG_SIGN_VERIFY_FAIL,
		/**< 0x624 Jtag Authentication failed when verification of
		signature failed*/
	XLOADER_ERR_AUTH_JTAG_EXCEED_ATTEMPTS,
		/**< 0x625 Jtag Authentication failed more than once */
	XLOADER_ERR_AUTH_JTAG_GET_DMA,
		/**< 0x626 Failed to get DMA instance for JTAG authentication */
	XLOADER_ERR_AUTH_JTAG_HASH_CALCULATION_FAIL,
		/**< 0x627 Hash calculation failed before signature verification */
	XLOADER_ERR_AUTH_JTAG_DMA_XFR,
		/**< 0x628 Failed to get Auth Jtag data with DMA Xfr */
	XLOADER_ERR_MEMSET_SECURE_PTR,
		/**< 0x629 Error during memset for SecurePtr */
	XLOADER_ERR_GLITCH_DETECTED,
		/**< 0x62A Error glitch detected */
	XLOADER_ERR_AUTH_JTAG_SPK_REVOKED,
		/**< 0X62B Jtag Authentication failed when revoke id is
		programmed */
	XLOADER_ERR_METAHDR_KEYSRC_MISMATCH,
		/**< 0x62C Metaheader Key Source does not match PLM Key Source */
	XLOADER_ERR_PRTN_ENC_ONLY_KEYSRC,
		/**< 0x62D Invalid key source when encryption only is enabled */
	XLOADER_ERR_SECURE_NOT_ALLOWED,
		/**< 0x62E Error when state of boot is non secure */
	XLOADER_ERR_HDR_AAD_UPDATE_FAIL,
		/**< 0x62F Updating IHT as AAD failed during secure header
		  * decryption */
	XLOADER_ERR_UNSUPPORTED_PDI_VER,
		/**< 0x630 PDI version used in secure operations is unsupported */
	XLOADER_ERR_PRTN_DECRYPT_NOT_ALLOWED,
		/**< 0x631 Partition is not allowed to be encrypted if State
		 of boot is non secure */
	XLOADER_ERR_AUTH_JTAG_INVALID_DNA,
		/**< 0x632 User provided Device DNA is not valid **/
	XLOADER_ERR_SEC_IH_VERIFY_FAIL,
		/**< 0x633 Failed to verify checksum of image headers */
	XLOADER_ERR_SEC_PH_VERIFY_FAIL,
		/**< 0x634 Failed to verify checksum of partition headers */
	XLOADER_ERR_SECURE_CLEAR_FAIL,
		/**< 0x635 Failed to place either AES,RSA,SHA3 engine in reset */
	XLOADER_ERR_READ_IHT_OPTIONAL_DATA,
		/**< 0x636 Error while reading IHT optional data */
	XLOADER_ERR_STORE_DIGEST_TABLE,
		/**< 0x637 Error while storing digest table */
	XLOADER_ERR_ZEROIZE_DIGEST_TABLE,
		/**< 0x638 Error while zeroizing digest table */
	XLOADER_ERR_SEC_AES_INIT_FAIL,
		/**< 0x639 Error when AES initialization is failed */
	XLOADER_ERR_DATA_MEASUREMENT,   /**< Error in data measurement */
	XLOADER_PRTN_HASH_NOT_PRESENT,
		/**< Partition hash not present, data measurement not possible */
	XLOADER_MEASURE_ROM_FAILURE,	/**< Error while ROM digest transfer to TPM */
	XLOADER_MEASURE_PLM_FAILURE,	/**< Error while PLM digest transfer to TPM */

	/* Security error codes specific to platform are from 0x6A0 to 0x6FF */
	XLOADER_EFUSE_5_PPK_FEATURE_NOT_SUPPORTED = 0x6A0,
	    /**< 0x6A0 Additional PPks are not enabled*/

	XPLMI_ERR_CDO_CMD = 0x2000,
		/**< 0x2XXX, CDO command handler has failed.
		 * [12:8] contains Module ID, [7:0] contains API ID.
		 * Refer Minor code for Handler error code */

	XPLMI_ERR_USER_MODULE_CDO_CMD = 0x3000,
		/**< 0x3XXX, CDO command handler has failed for user modules.
		 * [12:8] contains Module ID, [7:0] contains API ID.
		 * Refer Minor code for Handler error code */
} XPlmiStatus_t;

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
static inline int XPlmi_UpdateStatus(XPlmiStatus_t PlmiStatus, int ModuleStatus)
{
	u32 UStatus = (u32)PlmiStatus;
	u32 UModuleStatus = (u32)ModuleStatus;

	UStatus = UStatus << XPLMI_STATUS_SHIFT;
	UStatus = UStatus | (UModuleStatus & XPLMI_STATUS_MODULE_MASK);
	UStatus = UStatus & XPLMI_ERR_CODE_MASK;

	return (int)UStatus;
}

/************************** Function Prototypes ******************************/
void XPlmi_ErrMgr(int ErrStatusVal);
void XPlmi_LogPlmErr(int ErrStatusVal);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_STATUS_H */
