# Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT


set(INTC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../intc/src/")
set(SCUGIC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../scugic/src/")
find_package(INTC QUIET)
set(defs "")
list(APPEND defs "#ifndef XCOMMON_DRV_CONFIG_H_\n")
list(APPEND defs "#define XCOMMON_DRV_CONFIG_H_\n\n")
if ((EXISTS "${INTC_DIR}/INTCConfig.cmake") AND
    DRIVER_INSTANCES)
    list(APPEND defs "#define XPAR_AXI_INTC\n")
endif()

find_package(SCUGIC QUIET)
if ((EXISTS "${SCUGIC_DIR}/SCUGICConfig.cmake") AND
     DRIVER_INSTANCES)
    list(APPEND defs "#define XPAR_SCUGIC\n")
endif()
list(APPEND defs "\n#endif")
file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/xcommon_drv_config.h ${defs})
collect (PROJECT_LIB_SOURCES xinterrupt_wrap.c)
collect (PROJECT_LIB_HEADERS xinterrupt_wrap.h)
collect (PROJECT_LIB_HEADERS xcommon_drv_config.h)
