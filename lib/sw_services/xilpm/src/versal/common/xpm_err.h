/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
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
* 2.00  Ravi   04/22/2020 Use decimal values to align with ZU+
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


#define XPM_SUCCESS                              XST_SUCCESS /**< Returned Success */
#define XPM_FAILURE                              XST_FAILURE /**< Returned Failure */

/****************************** PM Specific Errors ***********************/
/******************************  (2000L) - (2009L) ***********************/
#define XPM_PM_INTERNAL                          (2000L) /**< Internal error occurred */
#define XPM_PM_CONFLICT                          (2001L) /**< Conflicting requirements asserted */
#define XPM_PM_NO_ACCESS                         (2002L) /**< No access to requested node or operation */
#define XPM_PM_INVALID_NODE                      (2003L) /**< API does not apply to node */
#define XPM_PM_DOUBLE_REQ                        (2004L) /**< Duplicate device request */
#define XPM_PM_ABORT_SUSPEND                     (2005L) /**< Abort suspend not allowed */
#define XPM_PM_TIMEOUT                           (2006L) /**< Timeout occurred */
#define XPM_PM_NODE_USED                         (2007L) /**< Node is used and non-shareable */
#define XPM_REG_WRITE_FAILED                     (2008L) /**< Register writting has failed*/

/****************************** Generic API Errors ***********************/
/******************************  (2010L) - (2020L) ***********************/
#define XPM_INVALID_TYPEID                       (2010L) /**< Invalid Reset/ShutdownType */
#define XPM_ERR_WAKEUP                           (2011L) /**< Failed to wakeup core */
#define XPM_ERR_CLEANUP                          (2012L) /**< Failed subsystem cleanup */
#define XPM_NO_FEATURE                           (2013L) /**< Feature check failed because of unsupported feature */
#define XPM_ERR_VERSION                          (2014L) /**< Version not supported */
#define XPM_ERR_IOCTL                            (2015L) /**< IOCTL type not supported */
#define XPM_INVALID_NAME                         (2016L) /**< Generic Error for invalid name, eg. clock name */

/****************************** Reset Based Errors ***********************/
/****************************** (2021L) - (2029L)  ***********************/
#define XPM_ERR_RESET                            (2021L) /**< Generic Reset failure */
#define XPM_ERR_APU_RESET                        (2022L) /**< APU Reset Failure */
#define XPM_ERR_RPU_RESET                        (2023L) /**< RPU Reset Failure */

/***************************** State Errors ******************************/
/***************************** (2030L)- (2035L) **************************/
#define XPM_ERR_SETSTATE                         (2030L) /**< Failure to set state */
#define XPM_ERR_GETSTATE                         (2031L) /**< Failure to get current state */
#define XPM_INVALID_STATE                        (2032L) /**< Entered Invalid state */

/***************************** Subsystem Errors **************************/
/*****************************  (2036L) - (2045L) ************************/
#define XPM_INVALID_SUBSYSID                     (2036L) /**< Invalid subsystem id passed to func */
#define XPM_ERR_SUBSYS_IDLE                      (2037L) /**< Unable to idle subsystem */
#define XPM_ERR_SUBSYS_NOTFOUND                  (2038L) /**< Unable to Find subsystem */
#define XPM_PEND_SUSP_CB_FOUND                   (2039L) /**< Pending suspend cb present in subsystem */

/******************************* Device Errors ***************************/
/****************************** (2046L) - (2055L) ************************/
#define XPM_ERR_DEVICE                           (2046L) /**< Generic Device Error */
#define XPM_INVALID_DEVICEID                     (2047L) /**< Error when invalid Dev Id is passed */
#define XPM_ERR_DEVICE_INIT                      (2048L) /**< Unable to initialize device */
#define XPM_ERR_DEVICE_REQ                       (2049L) /**< Failure to request device */
#define XPM_ERR_DEVICE_RELEASE                   (2050L) /**< Failure to release device */
#define XPM_ERR_DEVICE_BRINGUP                   (2051L) /**< Unable to bring-up device */
#define XPM_ERR_DEVICE_STATUS                    (2052L) /**< Unable to get/set device status */

/*************************** Requirement Errors **************************/
/*************************** (2056L) - (2065L) ***************************/
#define XPM_ERR_REQMNT_REL                       (2056L) /**< Failure to release requirement */
#define XPM_ERR_SET_REQ                          (2057L) /**< Failure to set requirement */

/*************************** Clock Errors ********************************/
/*************************** (2066L) - (2080L) ***************************/
#define XPM_ERR_SET_LATENCY                      (2066L) /**< Failure to set latency for a device*/
#define XPM_INVALID_CLKID                        (2067L) /**< Invalid clock id passed */
#define XPM_INVALID_CLK_SUBNODETYPE              (2068L) /**< Invalid clock sub-node type */
#define XPM_INVALID_PARENT_CLKID                 (2069L) /**< Invalid parent clock id */

/**************************** Power Errors *******************************/
/**************************** (2081L) - (2095L) **************************/
#define XPM_ERR_POWER_STATUS                     (2081L) /**< Failure to get/set power*/
#define XPM_INVALID_PWRDOMAIN                    (2082L) /**< Power Domain does not exist */
#define XPM_ERR_INIT_START                       (2083L) /**< Error while starting power domain initialization */
#define XPM_ERR_INIT_FINISH                      (2084L) /**< Error while finishing power domain initialization */
#define XPM_ERR_SCAN_CLR                         (2085L) /**< Failure to scan clear Power Domain */
#define XPM_ERR_BISR                             (2086L) /**< BISR Failure */
#define XPM_ERR_LBIST                            (2087L) /**< LBIST Failure */
#define XPM_ERR_MBIST_CLR                        (2088L) /**< MBIST Failure */
#define XPM_ERR_HC_PL                            (2089L) /**< Error while housecleaning PL */
#define XPM_ERR_MEM_INIT                         (2090L) /**< Memory Initialization */
#define XPM_ERR_HC_CMPLT                         (2091L) /**< Unable to finish housecleaning */

/*************************** RPU ERRORS **********************************/
/************************** (2096L) - (2099L) ****************************/
#define XPM_INVALID_BOOTADDR                     (2096L) /**< Valid boot address not passed */
#define XPM_INVALID_TCM_CONFIG                   (2097L) /**< Failure to configure TCM */

/************************** DOMAIN ISO ERRORS ****************************/
/************************** (2100L) - (2109L) ****************************/
#define XPM_INVALID_ISO_IDX                      (2100L) /**< Invalid Isolation index passed */

/**************************** SYSMON ERRORS ******************************/
/************************** (2110L) - (2119L) ****************************/
#define XPM_INVALID_DEV_VOLTAGE_GRADE            (2110L) /**< Invalid device voltage grade */
#define XPM_ERR_NEW_DATA_FLAG_TIMEOUT            (2111L) /**< Sysmon new data flag timeout */
#define XPM_ERR_RAIL_VOLTAGE                     (2112L) /**< Power rail not ramped up */

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPM_ERR_H */
