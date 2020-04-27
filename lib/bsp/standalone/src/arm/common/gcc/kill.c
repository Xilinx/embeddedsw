/******************************************************************************
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include <signal.h>
#include <unistd.h>
#include "xil_types.h"

#ifdef __cplusplus
extern "C" {
	__attribute__((weak)) int _kill(pid_t pid, int sig);
}
#endif

/*
 * kill -- go out via exit...
 */

__attribute__((weak)) int kill(pid_t pid, int sig)
{
  if(pid == 1) {
    _exit(sig);
  }
  return 0;
}

__attribute__((weak)) int _kill(pid_t pid, int sig)
{
  if(pid == 1) {
    _exit(sig);
  }
  return 0;
}
