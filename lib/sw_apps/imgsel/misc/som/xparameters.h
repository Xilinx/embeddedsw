#ifndef XPARAMETERS_H   /* prevent circular inclusions */
#define XPARAMETERS_H   /* by using protection macros */

/* Definition for CPU ID */
#define XPAR_CPU_ID 0U

/* Definitions for peripheral PSU_CORTEXA53_0 */
#define XPAR_PSU_CORTEXA53_0_CPU_CLK_FREQ_HZ 1333333008
#define XPAR_PSU_CORTEXA53_0_TIMESTAMP_CLK_FREQ 99999001

/******************************************************************/

/* Canonical definitions for peripheral PSU_CORTEXA53_0 */
#define XPAR_CPU_CORTEXA53_0_CPU_CLK_FREQ_HZ 1333333008
#define XPAR_CPU_CORTEXA53_0_TIMESTAMP_CLK_FREQ 99999001

/******************************************************************/

 /* Definition for PSS REF CLK FREQUENCY */
#define XPAR_PSU_PSS_REF_CLK_FREQ_HZ 33333000U

#include "xparameters_ps.h"

#define XPS_BOARD_KV260_SOM240_1_CONNECTOR_SOM240_SOM240_1_CONNECTOR

/* Number of Fabric Resets */
#define XPAR_NUM_FABRIC_RESETS 1

#define STDIN_BASEADDRESS 0xFF010000
#define STDOUT_BASEADDRESS 0xFF010000

/******************************************************************/

/* Platform specific definitions */
#define PLATFORM_ZYNQMP

/* Peripheral Definitions for peripheral PSU_CRL_APB */
#define XPAR_PSU_CRL_APB_S_AXI_BASEADDR 0xFF5E0000
#define XPAR_PSU_CRL_APB_S_AXI_HIGHADDR 0xFF85FFFF

/* Peripheral Definitions for peripheral PSU_IOUSLCR_0 */
#define XPAR_PSU_IOUSLCR_0_S_AXI_BASEADDR 0xFF180000
#define XPAR_PSU_IOUSLCR_0_S_AXI_HIGHADDR 0xFF23FFFF
/* Peripheral Definitions for peripheral PSU_OCM */
#define XPAR_PSU_OCM_S_AXI_BASEADDR 0xFF960000
#define XPAR_PSU_OCM_S_AXI_HIGHADDR 0xFF96FFFF

/* Peripheral Definitions for peripheral PSU_OCM_RAM_0 */
#define XPAR_PSU_OCM_RAM_0_S_AXI_BASEADDR 0xFFFC0000
#define XPAR_PSU_OCM_RAM_0_S_AXI_HIGHADDR 0xFFFFFFFF

/* Peripheral Definitions for peripheral PSU_OCM_XMPU_CFG */
#define XPAR_PSU_OCM_XMPU_CFG_S_AXI_BASEADDR 0xFFA70000
#define XPAR_PSU_OCM_XMPU_CFG_S_AXI_HIGHADDR 0xFFA7FFFF

/* Peripheral Definitions for peripheral PSU_PMU_GLOBAL_0 */
#define XPAR_PSU_PMU_GLOBAL_0_S_AXI_BASEADDR 0xFFD80000
#define XPAR_PSU_PMU_GLOBAL_0_S_AXI_HIGHADDR 0xFFDBFFFF

/* Peripheral Definitions for peripheral PSU_QSPI_LINEAR_0 */
#define XPAR_PSU_QSPI_LINEAR_0_S_AXI_BASEADDR 0xC0000000
#define XPAR_PSU_QSPI_LINEAR_0_S_AXI_HIGHADDR 0xDFFFFFFF

/* Canonical Definitions for peripheral PSU_CRL_APB */
#define XPAR_PSU_CRL_APB_0_S_AXI_BASEADDR 0xFF5E0000
#define XPAR_PSU_CRL_APB_0_S_AXI_HIGHADDR 0xFF85FFFF

/* Canonical Definitions for peripheral PSU_OCM */
#define XPAR_PSU_OCM_0_S_AXI_BASEADDR 0xFF960000
#define XPAR_PSU_OCM_0_S_AXI_HIGHADDR 0xFF96FFFF

/******************************************************************/

/* Definitions for driver GPIOPS */
#define XPAR_XGPIOPS_NUM_INSTANCES 1

/* Definitions for peripheral PSU_GPIO_0 */
#define XPAR_PSU_GPIO_0_DEVICE_ID 0
#define XPAR_PSU_GPIO_0_BASEADDR 0xFF0A0000
#define XPAR_PSU_GPIO_0_HIGHADDR 0xFF0AFFFF

/******************************************************************/

/* Canonical definitions for peripheral PSU_GPIO_0 */
#define XPAR_XGPIOPS_0_DEVICE_ID XPAR_PSU_GPIO_0_DEVICE_ID
#define XPAR_XGPIOPS_0_BASEADDR 0xFF0A0000
#define XPAR_XGPIOPS_0_HIGHADDR 0xFF0AFFFF

/******************************************************************/

/* Definitions for driver IICPS */
#define XPAR_XIICPS_NUM_INSTANCES 1

/* Definitions for peripheral PSU_I2C_1 */
#define XPAR_PSU_I2C_1_DEVICE_ID 0
#define XPAR_PSU_I2C_1_BASEADDR 0xFF030000
#define XPAR_PSU_I2C_1_HIGHADDR 0xFF03FFFF
#define XPAR_PSU_I2C_1_I2C_CLK_FREQ_HZ 99999001

/******************************************************************/

/* Canonical definitions for peripheral PSU_I2C_1 */
#define XPAR_XIICPS_0_DEVICE_ID XPAR_PSU_I2C_1_DEVICE_ID
#define XPAR_XIICPS_0_BASEADDR 0xFF030000
#define XPAR_XIICPS_0_HIGHADDR 0xFF03FFFF
#define XPAR_XIICPS_0_I2C_CLK_FREQ_HZ 99999001

/******************************************************************/

/* Definition for input Clock */
#define XPAR_PSU_I2C_1_REF_CLK I2C1_REF
/* Definitions for driver QSPIPSU */
#define XPAR_XQSPIPSU_NUM_INSTANCES 1

/* Definitions for peripheral PSU_QSPI_0 */
#define XPAR_PSU_QSPI_0_DEVICE_ID 0
#define XPAR_PSU_QSPI_0_BASEADDR 0xFF0F0000
#define XPAR_PSU_QSPI_0_HIGHADDR 0xFF0FFFFF
#define XPAR_PSU_QSPI_0_QSPI_CLK_FREQ_HZ 124998749
#define XPAR_PSU_QSPI_0_QSPI_MODE 0
#define XPAR_PSU_QSPI_0_QSPI_BUS_WIDTH 2

/******************************************************************/
#define XPAR_PSU_QSPI_0_IS_CACHE_COHERENT 0
#define XPAR_PSU_QSPI_0_REF_CLK QSPI_REF
/* Canonical definitions for peripheral PSU_QSPI_0 */
#define XPAR_XQSPIPSU_0_DEVICE_ID XPAR_PSU_QSPI_0_DEVICE_ID
#define XPAR_XQSPIPSU_0_BASEADDR 0xFF0F0000
#define XPAR_XQSPIPSU_0_HIGHADDR 0xFF0FFFFF
#define XPAR_XQSPIPSU_0_QSPI_CLK_FREQ_HZ 124998749
#define XPAR_XQSPIPSU_0_QSPI_MODE 0
#define XPAR_XQSPIPSU_0_QSPI_BUS_WIDTH 2
#define XPAR_XQSPIPSU_0_IS_CACHE_COHERENT 0

/******************************************************************/

/* Definitions for driver RESETPS and CLOCKPS */
#define XPAR_XCRPSU_NUM_INSTANCES 1U

/* Definitions for peripheral PSU_CR_0 */
#define XPAR_PSU_CR_DEVICE_ID 0

/******************************************************************/

/* Definitions for peripheral PSU_CRF_APB */
#define XPAR_PSU_CRF_APB_S_AXI_BASEADDR 0xFD1A0000
#define XPAR_PSU_CRF_APB_S_AXI_HIGHADDR 0xFD2DFFFF

/******************************************************************/

/* Canonical definitions for peripheral PSU_CR_0 */
#define XPAR_XCRPSU_0_DEVICE_ID 0

/******************************************************************/

/* Definitions for driver UARTPS */
#define XPAR_XUARTPS_NUM_INSTANCES 1

/* Definitions for peripheral PSU_UART_1 */
#define XPAR_PSU_UART_1_DEVICE_ID 0
#define XPAR_PSU_UART_1_BASEADDR 0xFF010000
#define XPAR_PSU_UART_1_HIGHADDR 0xFF01FFFF
#define XPAR_PSU_UART_1_UART_CLK_FREQ_HZ 99999001
#define XPAR_PSU_UART_1_HAS_MODEM 0

/******************************************************************/

/* Canonical definitions for peripheral PSU_UART_1 */
#define XPAR_XUARTPS_0_DEVICE_ID XPAR_PSU_UART_1_DEVICE_ID
#define XPAR_XUARTPS_0_BASEADDR 0xFF010000
#define XPAR_XUARTPS_0_HIGHADDR 0xFF01FFFF
#define XPAR_XUARTPS_0_UART_CLK_FREQ_HZ 99999001
#define XPAR_XUARTPS_0_HAS_MODEM 0

/******************************************************************/

/* Definition for input Clock */
#define XPAR_PSU_UART_1_REF_CLK UART1_REF

/******************************************************************/

#endif  /* end of protection macro */