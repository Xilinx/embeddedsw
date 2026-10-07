/******************************************************************************
* Copyright (c) 2014 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file kill.c
 * @brief kill stub implementation for standalone BSP.
 */

#include <signal.h>
#include <unistd.h>
#include "xil_types.h"

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((weak)) s32 _kill(pid_t pid, s32 sig);

#ifdef __cplusplus
}
#endif

/*
 * kill -- go out via exit...
 */

/**
 * @brief Terminate the single process via _exit() when pid is 1.
 * @param[in] pid Process ID; values other than 1 are ignored.
 * @param[in] sig Exit status passed to _exit().
 * @return 0 when pid is not 1; does not return when pid is 1.
 */
__attribute__((weak)) int kill(pid_t pid, int sig)
{
  if(pid == 1) {
    _exit(sig);
  }
  return 0;
}

/**
 * @brief Terminate the single process via _exit() when pid is 1.
 * @param[in] pid Process ID; values other than 1 are ignored.
 * @param[in] sig Exit status passed to _exit().
 * @return 0 when pid is not 1; does not return when pid is 1.
 */
__attribute__((weak)) s32 _kill(pid_t pid, s32 sig)
{
  if(pid == 1) {
    _exit(sig);
  }
  return 0;
}
