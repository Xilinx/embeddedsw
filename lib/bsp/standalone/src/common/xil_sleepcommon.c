/******************************************************************************
* Copyright (c) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
*@file xil_sleepcommon.c
*
* This file contains the sleep API's
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------- -------- -----------------------------------------------
* 6.6 	srm  	 11/02/17 First release
* </pre>
******************************************************************************/


/***************************** Include Files *********************************/
#include "xil_io.h"
#include "sleep.h"

/****************************  Constant Definitions  *************************/


/*****************************************************************************/
/**
*
* This API gives delay in sec
*
* @param            seconds - delay time in seconds
*
* @return           none
*
* @note             none
*
*****************************************************************************/
 void sleep(unsigned int seconds)
 {
#if defined (ARMR5)
	sleep_R5(seconds);
#elif defined (__aarch64__) || defined (ARMA53_32)
	sleep_A53(seconds);
#elif defined (__MICROBLAZE__)
	sleep_MB(seconds);
#else
	sleep_A9(seconds);
#endif

 }

/****************************************************************************/
/**
*
* This API gives delay in usec
*
* @param            useconds - delay time in useconds
*
* @return           none
*
* @note             none
*
*****************************************************************************/
 void usleep(unsigned long useconds)
 {
#if defined (ARMR5)
	usleep_R5(useconds);
#elif defined (__aarch64__) || defined (ARMA53_32)
	usleep_A53(useconds);
#elif defined (__MICROBLAZE__)
	usleep_MB(useconds);
#else
	usleep_A9(useconds);
#endif

 }
