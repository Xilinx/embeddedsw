/******************************************************************************
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * @file pm_api_version.h
 *
 * @addtogroup xpm_versal_apis XilPM APIs
 *****************************************************************************/


#ifndef PM_API_VERSION_H_
#define PM_API_VERSION_H_

/*****************************************************************************/
/**
 * @section EEMI_API_DETAIL XilPM EEMI API Version Detail
 *
 * This section provides details of EEMI API version and it's history for PM APIs of XilPM library.
 *
 * | NAME			| ID	| Platform	| Version| Description								     |
 * |----------------------------|-------|---------------|:------:|---------------------------------------------------------------------------|
 * | PM_GET_API_VERSION		| 0x1   | Both		| 1	 | The API is used to request the version number of the API		     |
 * | PM_SET_CONFIGURATION	| 0x2	| ZynqMP	| 1	 | The API is used to configure the power management framework		     |
 * | ^				| ^	| ^		| ^	 | Note: Deprecated in Versal but supported in ZynqMP			     |
 * | PM_GET_NODE_STATUS		| 0x3	| Both		| 1	 | The API is used to obtain information about current status of a device    |
 * | PM_GET_OP_CHARACTERISTIC	| 0x4	| Both		| 2	 | V1 - The API is used to get operating characteristics of a device	     |
 * | ^				| ^	| ^		| ^	 | V2 - Added support of bitmask functionality, user can check the supported\n									"type" first before performing the actual functionality				       |
 * | PM_REGISTER_NOTIFIER	| 0x5	| Both		| 2	 | V1 - The API is used to register a subsystem to be notified about the\n									device event									       |
 * | ^				| ^	| ^		| ^	 | V2 - Added support of event management functionality			     |
 * | ^				| ^	| ^		| ^	 | Note: V2 is supported in Versal but ZynqMP supports only V1		     |
 * | PM_REQUEST_SUSPEND		| 0x6	| Both		| 1	 | The API is used to send suspend request to another subsystem		     |
 * | PM_SELF_SUSPEND		| 0x7	| Both		| 3	 | V1 - The API is used to suspend a child subsystem			     |
 * | ^				| ^	| ^		| ^	 | V2 - Added support of cpu idle functionality during force powerdown	     |
 * | ^				| ^	| ^		| ^	 | V3 - Added support of CPU off state					     |
 * | ^				| ^	| ^		| ^	 | Note: V3 is supported in Versal and Versal NET but ZynqMP supports only V1|
 * | PM_FORCE_POWERDOWN		| 0x8	| Both		| 2	 | V1 - The API is used to Powerdown other processor or node		     |
 * | ^				| ^	| ^		| ^	 | V2 - Added support of cpu idle functionality	during force powerdown	     |
 * | ^				| ^	| ^		| ^	 | Note: V2 is supported in Versal but ZynqMP supports only V1		     |
 * | PM_ABORT_SUSPEND		| 0x9	| Both		| 1	 | The API is used by a subsystem to abort suspend of a child subsystem	     |
 * | PM_REQUEST_WAKEUP		| 0xA	| Both		| 1	 | The API is used to start-up and wake-up a child subsystem		     |
 * | PM_SET_WAKEUP_SOURCE	| 0xB	| Both		| 1	 | The API is used to set wakeup source					     |
 * | PM_SYSTEM_SHUTDOWN		| 0xC	| Both		| 1	 | The API is used to shutdown or restart the system			     |
 * | PM_REQUEST_NODE		| 0xD	| Both		| 2	 | V1 - The API is used to request the usage of a device		     |
 * | ^				| ^	| ^		| ^	 | V2 - Added support of security policy handling during request device	     |
 * | ^				| ^	| ^		| ^	 | Note: V2 is supported in Versal but ZynqMP supports only V1		     |
 * | PM_RELEASE_NODE		| 0xE	| Both		| 2	 | V1 - The API is used to release the usage of a device		     |
 * | ^				| ^	| ^		| ^	 | V2 - Added support of security policy handling during request device      |
 * | ^				| ^	| ^		| ^	 | Note: V2 is supported in Versal but ZynqMP supports only V1		     |
 * | PM_SET_REQUIREMENT		| 0xF	| Both		| 1	 | The API is used to announce a change in requirement for a specific slave\n									node which is currently in use							       |
 * | PM_SET_MAX_LATENCY		| 0x10	| Both		| 1	 | The API is used to set maximum allowed latency for the device	     |
 * | PM_RESET_ASSERT		| 0x11	| Both		| 1	 | The API is used to reset or de-reset a device			     |
 * | PM_RESET_GET_STATUS	| 0x12	| Both		| 1	 | The API is used to read the device reset state			     |
 * | PM_MMIO_WRITE		| 0x13	| ZynqMP	| 1	 | The API is used to write a value into a register			     |
 * | ^				| ^	| ^		| ^	 | Note: Deprecated in Versal but supported in ZynqMP			     |
 * | PM_MMIO_READ		| 0x14	| ZynqMP	| 1	 | The API is used to read a value from a register			     |
 * | ^				| ^	| ^		| ^	 | Note: Deprecated in Versal but supported in ZynqMP			     |
 * | PM_INIT_FINALIZE		| 0x15	| Both		| 1	 | The API is used to initialize subsystem and release unused devices	     |
 * | PM_GET_CHIPID		| 0x18	| Both		| 1	 | The API is used to request the version and ID code of a chip		     |
 * | PM_PINCTRL_REQUEST		| 0x1C	| Both		| 1	 | The API is used to request the pin					     |
 * | PM_PINCTRL_RELEASE		| 0x1D	| Both		| 1	 | The API is used to release the pin					     |
 * | PM_PINCTRL_GET_FUNCTION	| 0x1E	| Both		| 1	 | The API is used to read the pin function				     |
 * | PM_PINCTRL_SET_FUNCTION	| 0x1F	| Both		| 1	 | The API is used to set the pin function				     |
 * | PM_PINCTRL_CONFIG_PARAM_GET| 0x20	| Both		| 1	 | The API is used to read the pin parameter value			     |
 * | PM_PINCTRL_CONFIG_PARAM_SET| 0x21	| Both		| 2	 | V1 - The API is used to set the pin parameter value			     |
 * | ^				| ^	| ^		| ^	 | V2 - Added support of MIO tri-state controlling functionality	     |
 * | ^				| ^	| ^		| ^	 | Note: V2 is supported in ZynqMP but Versal supports only V1		     |
 * | PM_IOCTL			| 0x22	| Both		| 3	 | V1 - The API is used to perform driver-like IOCTL functions on shared\n									system devices									       |
 * | ^				| ^	| ^		| ^	 | V2 - Added support of bitmask functionality, user can check the supported\n									ID first before performing the actual functionality				       |
 * | ^				| ^	| ^		| ^	 | V3 - Add support of zeroization of AIE data and program memory separately |
 * | ^				| ^	| ^		| ^	 | Note: V3 is supported in Versal but ZynqMP supports only V2	             |
 * | PM_QUERY_DATA		| 0x23	| Both		| 2	 | V1 - The API is used to query information about the platform resources  |
 * | ^				| ^	| ^		| ^	 | V2 - Added support of bitmask functionality, user can check the supported\n									ID first before performing the actual functionality				       |
 * | PM_CLOCK_ENABLE		| 0x24	| Both		| 1	 | The API is used to enable the clock					     |
 * | PM_CLOCK_DISABLE		| 0x25	| Both		| 1	 | The API is used to disable the clock					     |
 * | PM_CLOCK_GETSTATE		| 0x26	| Both		| 1	 | The API is used to read the clock state				     |
 * | PM_CLOCK_SETDIVIDER	| 0x27	| Both		| 1	 | The API is used to set the divider value of the clock		     |
 * | PM_CLOCK_GETDIVIDER	| 0x28	| Both		| 1	 | The API is used to read the clock divider				     |
 * | PM_CLOCK_SETPARENT		| 0x2B	| Both		| 1	 | The API is used to set the parent of the clock			     |
 * | PM_CLOCK_GETPARENT		| 0x2C	| Both		| 1	 | The API is used to read the clock parent				     |
 * | PM_PLL_SET_PARAM		| 0x30	| Both		| 1	 | The API is used to set the parameter of PLL clock			     |
 * | PM_PLL_GET_PARAM		| 0x31	| Both		| 1	 | The API is used to read the parameter of PLL clock			     |
 * | PM_PLL_SET_MODE		| 0x32	| Both		| 1	 | The API is used to set the mode of PLL clock				     |
 * | PM_PLL_GET_MODE		| 0x33	| Both		| 1	 | The API is used to read the mode of PLL clock			     |
 * | PM_REGISTER_ACCESS		| 0x34	| ZynqMP	| 1	 | The API is used for register read/write access data			     |
 * | ^				| ^	| ^		| ^	 | Note: Deprecated in Versal but supported in ZynqMP			     |
 * | PM_EFUSE_ACCESS		| 0x35	| ZynqMP	| 1	 | The API is used to provide access to efuse memory			     |
 * | ^				| ^	| ^		| ^	 | Note: Deprecated in Versal but supported in ZynqMP			     |
 * | PM_FEATURE_CHECK		| 0x3F	| Both		| 2	 | V1 - The API is used to return supported version of the given API	     |
 * | ^				| ^	| ^		| ^	 | V2 - Added support of bitmask payload functionality			     |
 *
 *****************************************************************************/

/*****************************************************************************/
/**
 * @section IOCTL_ID_DETAIL XilPM IOCTL IDs Detail
 *
 * This section provides the details of the IOCTL IDs which are supported across the different platforms and their brief descriptions.
 *
 * | Name				| ID	| Platform	| Description				|
 * |------------------------------------|-------|---------------|---------------------------------------|
 * | IOCTL_GET_RPU_OPER_MODE		| 0	| Both		| Get RPU mode				|
 * | IOCTL_SET_RPU_OPER_MODE		| 1	| Both		| Set RPU mode				|
 * | IOCTL_RPU_BOOT_ADDR_CONFIG		| 2	| Both		| RPU boot address config		|
 * | IOCTL_TCM_COMB_CONFIG		| 3	| Both		| TCM config				|
 * | IOCTL_SET_TAPDELAY_BYPASS		| 4	| Both		| TAP delay bypass			|
 * | IOCTL_SD_DLL_RESET			| 6	| Both		| SD DLL reset				|
 * | IOCTL_SET_SD_TAPDELAY		| 7	| Both		| SD TAP delay				|
 * | IOCTL_SET_PLL_FRAC_MODE		| 8	| Both		| Set PLL frac mode			|
 * | IOCTL_GET_PLL_FRAC_MODE		| 9	| Both		| Get PLL frac mode			|
 * | IOCTL_SET_PLL_FRAC_DATA		| 10	| Both		| Set PLL frac data			|
 * | IOCTL_GET_PLL_FRAC_DATA		| 11	| Both		| Get PLL frac data			|
 * | IOCTL_WRITE_GGS			| 12	| Both		| Write GGS				|
 * | IOCTL_READ_GGS			| 13	| Both		| Read GGS				|
 * | IOCTL_WRITE_PGGS			| 14	| Both		| Write PGGS				|
 * | IOCTL_READ_PGGS			| 15	| Both		| Read PGGS				|
 * | IOCTL_ULPI_RESET			| 16	| ZynqMP	| ULPI reset				|
 * | IOCTL_SET_BOOT_HEALTH_STATUS	| 17	| Both		| Set boot status			|
 * | IOCTL_AFI				| 18	| ZynqMP	| AFI					|
 * | IOCTL_OSPI_MUX_SELECT		| 21	| Versal	| OSPI mux select			|
 * | IOCTL_USB_SET_STATE		| 22	| Versal	| USB set state				|
 * | IOCTL_GET_LAST_RESET_REASON	| 23	| Versal	| Get last reset reason			|
 * | IOCTL_AIE_ISR_CLEAR		| 24	| Versal	| AIE ISR clear				|
 * | IOCTL_REGISTER_SGI			| 25	| None		| Register SGI to ATF			|
 * | IOCTL_SET_FEATURE_CONFIG		| 26	| ZynqMP	| Set runtime feature config		|
 * | IOCTL_GET_FEATURE_CONFIG		| 27	| ZynqMP	| Get runtime feature config		|
 * | IOCTL_READ_REG			| 28	| Versal	| Read a 32-bit register		|
 * | IOCTL_MASK_WRITE_REG		| 29	| Versal	| RMW a 32-bit register			|
 * | IOCTL_SET_SD_CONFIG		| 30	| ZynqMP	| Set SD config register value		|
 * | IOCTL_SET_GEM_CONFIG		| 31	| ZynqMP	| Set GEM config register value		|
 * | IOCTL_SET_USB_CONFIG		| 32	| ZynqMP	| Set USB config register value		|
 * | IOCTL_AIE_OPS			| 33	| Versal	| AIE1/AIEML Run Time Operations	|
 * | IOCTL_GET_QOS			| 34	| Versal	| Get Device QoS value			|
 * | IOCTL_GET_APU_OPER_MODE		| 35	| Versal	| Get APU operation mode		|
 * | IOCTL_SET_APU_OPER_MODE		| 36	| Versal	| Set APU operation mode		|
 * | IOCTL_PREPARE_DDR_SHUTDOWN		| 37	| Versal	| Prepare DDR for shut down		|
 * | IOCTL_GET_SSIT_TEMP		| 38	| Versal	| Get secondary SLR min/max temperature |
 *
 *****************************************************************************/

/*****************************************************************************/
/**
 * @section QUERY_ID_DETAIL XilPM QUERY IDs Detail
 *
 * This section provides the details of the QUERY IDs which are supported across the different platforms and their brief descriptions.
 *
 * | Name					| ID	| Platform	| Description				|
 * |--------------------------------------------|-------|---------------|---------------------------------------|
 * | XPM_QID_INVALID				| 0	| Both		| Invalid Query ID			|
 * | XPM_QID_CLOCK_GET_NAME			| 1	| Both		| Get clock name			|
 * | XPM_QID_CLOCK_GET_TOPOLOGY			| 2	| Both		| Get clock topology			|
 * | XPM_QID_CLOCK_GET_FIXEDFACTOR_PARAMS	| 3	| Both		| Get clock fixedfactor parameter	|
 * | XPM_QID_CLOCK_GET_MUXSOURCES		| 4	| Both		| Get clock mux sources			|
 * | XPM_QID_CLOCK_GET_ATTRIBUTES		| 5	| Both		| Get clock attributes			|
 * | XPM_QID_PINCTRL_GET_NUM_PINS		| 6	| Both		| Get total pins			|
 * | XPM_QID_PINCTRL_GET_NUM_FUNCTIONS		| 7	| Both		| Get total pin functions		|
 * | XPM_QID_PINCTRL_GET_NUM_FUNCTION_GROUPS	| 8	| Both		| Get total pin function groups		|
 * | XPM_QID_PINCTRL_GET_FUNCTION_NAME		| 9	| Both		| Get pin function name			|
 * | XPM_QID_PINCTRL_GET_FUNCTION_GROUPS	| 10	| Both		| Get pin function groups		|
 * | XPM_QID_PINCTRL_GET_PIN_GROUPS		| 11	| Both		| Get pin groups			|
 * | XPM_QID_CLOCK_GET_NUM_CLOCKS		| 12	| Both		| Get number of clocks			|
 * | XPM_QID_CLOCK_GET_MAX_DIVISOR		| 13	| Both		| Get max clock divisor			|
 * | XPM_QID_PLD_GET_PARENT			| 14	| Versal	| Get PLD parent			|
 * | XPM_QID_PINCTRL_GET_ATTRIBUTES		| 15	| Versal	| Get pin attributes			|
 *
 *****************************************************************************/


/*****************************************************************************/
/**
 * @section GET_OP_CHAR_DETAIL XilPM GET_OP_CHAR IDs Detail
 *
 * This section provides the details of the GET_OP_CHAR IDs which are supported across the different platforms and their brief descriptions.
 *
 * | Name				| ID	| Platform	| Description					|
 * |------------------------------------|-------|---------------|-----------------------------------------------|
 * | PM_OPCHAR_TYPE_POWER		| 1	| ZynqMP	| Operating characteristic ID power		|
 * | PM_OPCHAR_TYPE_TEMP		| 2	| Versal	| Operating characteristic ID temperature	|
 * | PM_OPCHAR_TYPE_LATENCY		| 3	| Both		| Operating characteristic ID latency		|
 *
 *****************************************************************************/


/**
 * PM API IDs
 */
typedef enum {
	PM_API_MIN,					/**< 0x0 */
	PM_GET_API_VERSION,				/**< 0x1 */
	PM_SET_CONFIGURATION,				/**< 0x2 */
	PM_GET_NODE_STATUS,				/**< 0x3 */
	PM_GET_OP_CHARACTERISTIC,			/**< 0x4 */
	PM_REGISTER_NOTIFIER,				/**< 0x5 */
	PM_REQUEST_SUSPEND,				/**< 0x6 */
	PM_SELF_SUSPEND,				/**< 0x7 */
	PM_FORCE_POWERDOWN,				/**< 0x8 */
	PM_ABORT_SUSPEND,				/**< 0x9 */
	PM_REQUEST_WAKEUP,				/**< 0xA */
	PM_SET_WAKEUP_SOURCE,				/**< 0xB */
	PM_SYSTEM_SHUTDOWN,				/**< 0xC */
	PM_REQUEST_NODE,				/**< 0xD */
	PM_RELEASE_NODE,				/**< 0xE */
	PM_SET_REQUIREMENT,				/**< 0xF */
	PM_SET_MAX_LATENCY,				/**< 0x10 */
	PM_RESET_ASSERT,				/**< 0x11 */
	PM_RESET_GET_STATUS,				/**< 0x12 */
	PM_MMIO_WRITE,					/**< 0x13 */
	PM_MMIO_READ,					/**< 0x14 */
	PM_INIT_FINALIZE,				/**< 0x15 */
	PM_FPGA_LOAD,					/**< 0x16 */
	PM_FPGA_GET_STATUS,				/**< 0x17 */
	PM_GET_CHIPID,					/**< 0x18 */
	PM_SECURE_RSA_AES,				/**< 0x19 */
	PM_SECURE_SHA,					/**< 0x1A */
	PM_SECURE_RSA,					/**< 0x1B */
	PM_PINCTRL_REQUEST,				/**< 0x1C */
	PM_PINCTRL_RELEASE,				/**< 0x1D */
	PM_PINCTRL_GET_FUNCTION,			/**< 0x1E */
	PM_PINCTRL_SET_FUNCTION,			/**< 0x1F */
	PM_PINCTRL_CONFIG_PARAM_GET,			/**< 0x20 */
	PM_PINCTRL_CONFIG_PARAM_SET,			/**< 0x21 */
	PM_IOCTL,					/**< 0x22 */
	PM_QUERY_DATA,					/**< 0x23 */
	PM_CLOCK_ENABLE,				/**< 0x24 */
	PM_CLOCK_DISABLE,				/**< 0x25 */
	PM_CLOCK_GETSTATE,				/**< 0x26 */
	PM_CLOCK_SETDIVIDER,				/**< 0x27 */
	PM_CLOCK_GETDIVIDER,				/**< 0x28 */
	PM_CLOCK_SETRATE,				/**< 0x29 */
	/* PM_CLOCK_GETRATE API is deprecated */
	PM_RESERVE_ID,					/**< 0x2A */
	PM_CLOCK_SETPARENT,				/**< 0x2B */
	PM_CLOCK_GETPARENT,				/**< 0x2C */
	PM_SECURE_IMAGE,				/**< 0x2D */
	PM_FPGA_READ,					/**< 0x2E */
	PM_SECURE_AES,					/**< 0x2F */
	PM_PLL_SET_PARAMETER,				/**< 0x30 */
	PM_PLL_GET_PARAMETER,				/**< 0x31 */
	PM_PLL_SET_MODE,				/**< 0x32 */
	PM_PLL_GET_MODE,				/**< 0x33 */
	PM_REGISTER_ACCESS,				/**< 0x34 */
	PM_EFUSE_ACCESS,				/**< 0x35 */
	PM_ADD_SUBSYSTEM,				/**< 0x36 */
	PM_DESTROY_SUBSYSTEM,				/**< 0x37 */
	PM_DESCRIBE_NODES,				/**< 0x38 */
	PM_ADD_NODE,					/**< 0x39 */
	PM_ADD_NODE_PARENT,				/**< 0x3A */
	PM_ADD_NODE_NAME,				/**< 0x3B */
	PM_ADD_REQUIREMENT,				/**< 0x3C */
	PM_SET_CURRENT_SUBSYSTEM,			/**< 0x3D */
	PM_INIT_NODE,					/**< 0x3E */
	PM_FEATURE_CHECK,				/**< 0x3F */
	PM_ISO_CONTROL,					/**< 0x40 */
	PM_ACTIVATE_SUBSYSTEM,				/**< 0x41 */
	PM_SET_NODE_ACCESS,				/**< 0x42 */
	PM_BISR,					/**< 0x43 */
	PM_APPLY_TRIM,					/**< 0x44 */
	PM_NOC_CLOCK_ENABLE,				/**< 0x45 */
	PM_IF_NOC_CLOCK_ENABLE,				/**< 0x46 */
	PM_FORCE_HOUSECLEAN,				/**< 0x47 */
	PM_FPGA_GET_VERSION,				/**< 0x48 */
	PM_FPGA_GET_FEATURE_LIST,			/**< 0x49 */
	PM_API_MAX					/**< 0x4A */
} XPm_ApiId;

#endif  /* PM_API_VERSION_H_ */
