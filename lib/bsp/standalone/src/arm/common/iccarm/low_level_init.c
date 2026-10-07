/******************************************************************************
* Copyright (c) 2009 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file low_level_init.c
 * @brief Low-level BSP initialization for IAR toolchain.
 */


/*
 * This module contains the function `__low_level_init', a function
 * that is called before the `main' function of the program.  Normally
 * low-level initializations - such as setting the preferred interrupt
 * level or setting the watchdog - can be performed here.
 *
 * Note that this function is called before the data segments are
 * initialized, this means that this function cannot rely on the
 * values of global or static variables.
 *
 * When this function returns zero, the startup code will inhibit the
 * initialization of the data segments. The result is faster startup,
 * the drawback is that neither global nor static data will be
 * initialized.
 */

#include "xil_types.h"
#ifdef __cplusplus
extern "C" {
#endif


s32 __low_level_init(void);

/**
 * @brief Low-level hardware initialisation called before main(); returns 1.
 * @return 1 to enable data-segment initialisation, 0 to skip it.
 */
s32 __low_level_init(void)
{
  /*==================================*/
  /*  Initialize hardware.            */
  /*==================================*/

  /*==================================*/
  /* Choose if segment initialization */
  /* should be done or not.           */
  /* Return: 0 to omit seg_init       */
  /*         1 to run seg_init        */
  /*==================================*/
  return 1;
}


#ifdef __cplusplus
}
#endif
