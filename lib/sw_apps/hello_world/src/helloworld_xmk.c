/******************************************************************************
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/* helloworld_xmk.c: launch a thread that prints out Hello World */

#include "xmk.h"
#include "sys/init.h"
#include "platform.h"

#include <stdio.h>

void *hello_world(void *arg)
{
    print("Hello World\r\n");
}

int main()
{
    init_platform();

    /* Initialize xilkernel */
    xilkernel_init();

    /* add a thread to be launched once xilkernel starts */
    xmk_add_static_thread(hello_world, 0);

    /* start xilkernel - does not return control */
    xilkernel_start();

    /* Never reached */
    cleanup_platform();

    return 0;
}
