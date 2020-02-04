/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xpm_err.h
*
* This is the header file which contains status codes for the PLM, PLMI
* and loader.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  Amit   05/08/2019 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPM_ERR_H
#define XPM_ERR_H

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
 * XXXX - PLM/LOADER/XPLMI error codes as defined in xplmi_status.h
 * YYYY - Libraries / Drivers error code as defined in respective modules
 */


#define XPM_SUCCESS                              XST_SUCCESS
#define XPM_FAILURE                              XST_FAILURE

/****************************** PM Specific Errors ***********************/
/******************************  0x2000 - 0x2007  ************************/
#define XPM_PM_INTERNAL                          0x2000L /* Internal error
							 occurred */
#define XPM_PM_CONFLICT                          0x2001L /* Conflicting require-
							 ments asserted */
#define XPM_PM_NO_ACCESS                         0x2002L /* No access to reque-
							 sted node or operation */
#define XPM_PM_INVALID_NODE                      0x2003L /* API does not apply¬
							 to node */
#define XPM_PM_DOUBLE_REQ                        0x2004L /* Duplicate device
							 request */
#define XPM_PM_ABORT_SUSPEND                     0x2005L /* Abort suspend not
							 allowed */
#define XPM_PM_TIMEOUT                           0x2006L /* Timeout occurred */

#define XPM_PM_NODE_USED                         0x2007L /* Node is used and
							 non-shareable */

/****************************** Generic API Errors ***********************/
/******************************  0x2010 - 0x2020  ************************/

#define XPM_INVALID_TYPEID                       0x2010 /* Invalid Reset/Shutd-
							   ownType */
#define XPM_ERR_WAKEUP                           0x2011 /* Failed to wakeup core
							 */
#define XPM_ERR_CLEANUP                          0x2012 /* Failed subsys cleanup
							 */
#define XPM_NO_FEATURE                           0x2013 /* Feature check failed
							 because of unsupported1
							 feature */
#define XPM_ERR_VERSION                          0x2014 /* Version not supported
							 */
#define XPM_ERR_IOCTL                            0x2015 /* IOCTL type not suppor
							  ted */
#define XPM_INVALID_NAME                         0x2016 /* Generic Error for in-
							   valid name, eg. clock
							   name */

/****************************** Reset Based Errors ***********************/
/******************************  0x2021 - 0x202F  ************************/

#define XPM_ERR_RESET                            0x2021 /* Generic Reset failure
							 */
#define XPM_ERR_APU_RESET                        0x2022 /* APU Reset Failure */
#define XPM_ERR_RPU_RESET                        0x2023 /* RPU Reset Failure */

/***************************** State Errors ****************************/
/*****************************  0x2030 - 0x2035 ************************/

#define XPM_ERR_SETSTATE                         0x2030 /* Failure to set state
							 */
#define XPM_ERR_GETSTATE                         0x2031 /* Failure to get curr-
							   ent state */
#define XPM_INVALID_STATE                        0x2032 /* Entered Invalid state
							 */

/***************************** Subsystem Errors *************************/
/*****************************  0x2036 - 0x2045 *************************/

#define XPM_INVALID_SUBSYSID                     0x2036 /* Invalid subsystem id
							   passed to func */
#define XPM_ERR_SUBSYS_IDLE                      0x2037 /* Unable to idle subs-
							   ystem */
#define XPM_ERR_SUBSYS_NOTFOUND			 0x2038 /* Unable to Find subs-
							   ystem */

/******************************* Device Errors **************************/
/****************************** 0x2046 - 0x2055 *************************/

#define XPM_ERR_DEVICE                           0x2046 /* Generic Device Error
							 */
#define XPM_INVALID_DEVICEID                     0x2047 /* Error when invalid Dev
							 Id is passed */
#define XPM_ERR_DEVICE_INIT                      0x2048 /* Unable to initialize
							 device */
#define XPM_ERR_DEVICE_REQ                       0x2049 /* Failure to request
							 device */
#define XPM_ERR_DEVICE_RELEASE                   0x2050 /* Failue to release
							  device */
#define XPM_ERR_DEVICE_BRINGUP                   0x2051 /* Unable to bringup de-
							   vice*/
#define XPM_ERR_DEVICE_STATUS                    0x2052 /* Unable to get/set dev-
							   ice status */

/*************************** Requirement Errors ************************/
/***************************** 0x2056 - 0x2065 *************************/

#define XPM_ERR_REQMNT_REL                       0x2056 /* Failure to release
							 requirement */
#define XPM_ERR_SET_REQ                          0x2057 /* Failure to set requi-
							   rement */

/*************************** Clock Errors ******************************/
/***************************** 0x2066 - 0x2080 *************************/

#define XPM_ERR_SET_LATENCY                      0x2066 /* Failure to set laten-
							   cy for a device*/
#define XPM_INVALID_CLKID                        0x2067 /* Invalid clock id pas-
							   sed */
#define XPM_INVALID_CLK_SUBNODETYPE              0x2068 /* Invalid clock
							   subnode type */
/**************************** Power Errors ****************************/
/**************************** 0x2081 - 0x2095 *************************/

#define XPM_ERR_POWER_STATUS                     0x2081 /* Failure to get/set
							   power*/
#define XPM_INVALID_PWRDOMAIN                    0x2082 /* Power Domain does not
							   exist */

#define XPM_ERR_INIT_START                       0x2083 /* Error while starting
							   power domain initiali
							   zation */
#define XPM_ERR_INIT_FINISH                      0x2084 /*Error while finishing
							  power domain initiali-
							  zation */
#define XPM_ERR_SCAN_CLR                         0x2085 /* Failure to scan clear
							   Power Domain */
#define XPM_ERR_BISR                             0x2086 /* BISR Failure */
#define XPM_ERR_LBIST                            0x2087 /* LBIST Failure */
#define XPM_ERR_MBIST_CLR                        0x2088 /* MBIST Failure */
#define XPM_ERR_HC_PL                            0x2089 /* Error while housecle-
							   aning PL */
#define XPM_ERR_MEM_INIT                         0x208A /* Memory Initialization
							 */
#define XPM_ERR_HC_CMPLT                         0x208B /* Unable to finish
							   housecleaning */

/*************************** RPU ERRORS ******************************/
/************************** 0X2096 - 0X20A5 **************************/
#define XPM_INVALID_BOOTADDR                     0x2096 /* Valid boot address
							   not passed */
#define XPM_INVALID_TCM_CONFIG                   0x2097 /* Failure to configure
							   TCM */

/************************** DOMAIN ISO ERRORS ************************/
/************************** 0X20A6 - 0X20B0 **************************/

#define XPM_INVALID_ISO_IDX                      0x20A6 /* Invalid Isolation in-
							   dex passed */

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPM_ERR_H */
