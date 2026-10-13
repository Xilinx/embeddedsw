/******************************************************************************
* Copyright (C) 2016 Xilinx, Inc. All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/***************************** Include Files *********************************/

#include <string.h>
#include "dsi_txss_display.h"
#include "xdsitxss.h"
#include "xparameters.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xstatus.h"
#include "xil_io.h"
#include "sleep.h"
#include "xiic.h"

/************************** Constant Definitions *****************************/

/* AXI IIC Base Address for the panel's ATtiny MCU (DSI_TXSS_HIER_AXI_IIC_1
 * in the combined design's block design). */
#define PANEL_IIC_BASEADDR              XPAR_XIIC_1_BASEADDR

/* =========================================================================
 * ATtiny MCU (I2C addr 0x45) Register Map
 * Source: drivers/regulator/rpi-panel-v2-regulator.c (rpi-6.6.y)
 * ========================================================================= */
#define RPI_MCU_I2C_ADDR                0x45   /* ATtiny MCU I2C address    */

#define RPI_MCU_REG_ID                  0x01   /* Device/firmware ID        */
#define RPI_MCU_REG_POWERON             0x02   /* LCD + Touch reset control */
#define RPI_MCU_REG_PWM                 0x03   /* Backlight PWM control     */

/* REG_POWERON (0x02) bit definitions */
#define RPI_MCU_LCD_RESET_BIT           (1 << 0) /* 0=LCD in reset, 1=run  */
#define RPI_MCU_CTP_RESET_BIT           (1 << 1) /* 0=Touch in reset,1=run */

/* REG_PWM (0x03) bit definitions */
#define RPI_MCU_PWM_BL_ENABLE           (1 << 7) /* Backlight ON/OFF       */
#define RPI_MCU_PWM_MAX_BRIGHTNESS      0x1F     /* Max brightness = 31    */

/* =========================================================================
 * MIPI DSI Data Types
 * ========================================================================= */
#define DSI_DCS_SHORT_WRITE             0x05   /* No parameter              */
#define DSI_DCS_SHORT_WRITE_PARAM       0x15   /* 1 parameter               */
#define DSI_DCS_LONG_WRITE              0x39   /* Multiple parameters       */

/* =========================================================================
 * MIPI DCS Standard Commands
 * ========================================================================= */
#define DCS_EXIT_SLEEP_MODE             0x11
#define DCS_SET_DISPLAY_ON              0x29
#define DCS_SET_TEAR_ON                 0x35

/* =========================================================================
 * ILI9881C Private Page Switch Command
 * ========================================================================= */
#define ILI9881C_PAGE_CMD               0xFF
#define ILI9881C_PAGE_MAGIC0            0x98
#define ILI9881C_PAGE_MAGIC1            0x81

#define DSI_TX_BASEADDR                 XPAR_XDSITXSS_0_BASEADDR



/* =========================================================================
 * Panel Resolution & Video Timing (720x1280 @ 60Hz)
 * Pixel clock: 83.33 MHz
 * HTotal = 720 + 239 + 33 + 50 = 1042
 * VTotal = 1280 + 20 + 2  + 30 = 1332
 * ========================================================================= */
#define NO_BYTES_PER_PIXEL              3
#define HORIZONTAL_RESOLUTION           DSI_PANEL_H_RES
#define HORIZONTAL_RESOLUTION_BYTES     (HORIZONTAL_RESOLUTION * \
                                         NO_BYTES_PER_PIXEL)
#define VERTICAL_RESOLUTION             DSI_PANEL_V_RES

#define HACT_VALUE                      HORIZONTAL_RESOLUTION_BYTES
#define HSA_VALUE                       33
#define HFP_VALUE                       239
#define HBP_VALUE                       50
#define VACT_VALUE                      VERTICAL_RESOLUTION
#define VSA_VALUE                       2
#define VFP_VALUE                       20
#define VBP_VALUE                       30
/* BLLP = HTotal×3 − HActive = 1042×3 − 2160 = 966 bytes.
 * In burst mode BLLP fills the blanking period so the ILI9881C's internal
 * pixel counter receives exactly HTotal=1042 pixel-clock periods per line.
 * Non-burst sync-pulse with BLLP=0 leaves timing gaps causing the panel to
 * display content in a centred window with black borders on all four sides. */
#define BLLP_BURST_VALUE                966

/***************** Macros (Inline functions) Definitions *********************/

/* ILI9881C instruction array entry macros */
#define ILI9881C_SWITCH_PAGE(page)      { 1, 0x00, (page) }
#define ILI9881C_COMMAND(reg, val)      { 0, (reg), (val)  }

/**************************** Type Definitions *******************************/

/* Single entry in the ILI9881C init sequence array */
typedef struct {
    u8 is_page_switch;  /* 1 = page switch (long write), 0 = reg write */
    u8 reg;
    u8 val;
} ILI9881C_Instr;

/************************** Variable Definitions *****************************/

static XIic     PanelIic;
static XDsiTxSs DsiTxSsInst;

/*
 * ILI9881C RPi 7-inch (720x1280) Register Init Sequence
 * Ported from rpi_7inch_init[] in panel-ilitek-ili9881c.c (rpi-6.6.y)
 * Order: Page 3 (GIP) -> Page 4 (Power) -> Page 1 (Gamma/Display)
 */
static const ILI9881C_Instr rpi_7inch_init[] = {

    /* ------------------------------------------------------------------ */
    /* PAGE 3 : GIP Timing & Signal Routing                               */
    /* ------------------------------------------------------------------ */
    ILI9881C_SWITCH_PAGE(3),
    ILI9881C_COMMAND(0x01, 0x00),
    ILI9881C_COMMAND(0x02, 0x00),
    ILI9881C_COMMAND(0x03, 0x73),
    ILI9881C_COMMAND(0x04, 0x00),
    ILI9881C_COMMAND(0x05, 0x00),
    ILI9881C_COMMAND(0x06, 0x0a),
    ILI9881C_COMMAND(0x07, 0x00),
    ILI9881C_COMMAND(0x08, 0x00),
    ILI9881C_COMMAND(0x09, 0x61),
    ILI9881C_COMMAND(0x0a, 0x00),
    ILI9881C_COMMAND(0x0b, 0x00),
    ILI9881C_COMMAND(0x0c, 0x01),
    ILI9881C_COMMAND(0x0d, 0x00),
    ILI9881C_COMMAND(0x0e, 0x00),
    ILI9881C_COMMAND(0x0f, 0x61),
    ILI9881C_COMMAND(0x10, 0x61),
    ILI9881C_COMMAND(0x11, 0x00),
    ILI9881C_COMMAND(0x12, 0x00),
    ILI9881C_COMMAND(0x13, 0x00),
    ILI9881C_COMMAND(0x14, 0x00),
    ILI9881C_COMMAND(0x15, 0x00),
    ILI9881C_COMMAND(0x16, 0x00),
    ILI9881C_COMMAND(0x17, 0x00),
    ILI9881C_COMMAND(0x18, 0x00),
    ILI9881C_COMMAND(0x19, 0x00),
    ILI9881C_COMMAND(0x1a, 0x00),
    ILI9881C_COMMAND(0x1b, 0x00),
    ILI9881C_COMMAND(0x1c, 0x00),
    ILI9881C_COMMAND(0x1d, 0x00),
    ILI9881C_COMMAND(0x1e, 0x40),
    ILI9881C_COMMAND(0x1f, 0x80),
    ILI9881C_COMMAND(0x20, 0x06),
    ILI9881C_COMMAND(0x21, 0x01),
    ILI9881C_COMMAND(0x22, 0x00),
    ILI9881C_COMMAND(0x23, 0x00),
    ILI9881C_COMMAND(0x24, 0x00),
    ILI9881C_COMMAND(0x25, 0x00),
    ILI9881C_COMMAND(0x26, 0x00),
    ILI9881C_COMMAND(0x27, 0x00),
    ILI9881C_COMMAND(0x28, 0x33),
    ILI9881C_COMMAND(0x29, 0x03),
    ILI9881C_COMMAND(0x2a, 0x00),
    ILI9881C_COMMAND(0x2b, 0x00),
    ILI9881C_COMMAND(0x2c, 0x00),
    ILI9881C_COMMAND(0x2d, 0x00),
    ILI9881C_COMMAND(0x2e, 0x00),
    ILI9881C_COMMAND(0x2f, 0x00),
    ILI9881C_COMMAND(0x30, 0x00),
    ILI9881C_COMMAND(0x31, 0x00),
    ILI9881C_COMMAND(0x32, 0x00),
    ILI9881C_COMMAND(0x33, 0x00),
    ILI9881C_COMMAND(0x34, 0x04),
    ILI9881C_COMMAND(0x35, 0x00),
    ILI9881C_COMMAND(0x36, 0x00),
    ILI9881C_COMMAND(0x37, 0x00),
    ILI9881C_COMMAND(0x38, 0x3c),
    ILI9881C_COMMAND(0x39, 0x00),
    ILI9881C_COMMAND(0x3a, 0x00),
    ILI9881C_COMMAND(0x3b, 0x00),
    ILI9881C_COMMAND(0x3c, 0x00),
    ILI9881C_COMMAND(0x3d, 0x00),
    ILI9881C_COMMAND(0x3e, 0x00),
    ILI9881C_COMMAND(0x3f, 0x00),
    ILI9881C_COMMAND(0x40, 0x00),
    ILI9881C_COMMAND(0x41, 0x00),
    ILI9881C_COMMAND(0x42, 0x00),
    ILI9881C_COMMAND(0x43, 0x00),
    ILI9881C_COMMAND(0x44, 0x00),

    /* GIP_2 : Gate Output Sequencing Patterns */
    ILI9881C_COMMAND(0x50, 0x10),
    ILI9881C_COMMAND(0x51, 0x32),
    ILI9881C_COMMAND(0x52, 0x54),
    ILI9881C_COMMAND(0x53, 0x76),
    ILI9881C_COMMAND(0x54, 0x98),
    ILI9881C_COMMAND(0x55, 0xba),
    ILI9881C_COMMAND(0x56, 0x10),
    ILI9881C_COMMAND(0x57, 0x32),
    ILI9881C_COMMAND(0x58, 0x54),
    ILI9881C_COMMAND(0x59, 0x76),
    ILI9881C_COMMAND(0x5a, 0x98),
    ILI9881C_COMMAND(0x5b, 0xba),
    ILI9881C_COMMAND(0x5c, 0xdc),
    ILI9881C_COMMAND(0x5d, 0xfe),

    /* GIP_3 : Gate Signal Routing to Panel Lines */
    ILI9881C_COMMAND(0x5e, 0x00),
    ILI9881C_COMMAND(0x5f, 0x0e),
    ILI9881C_COMMAND(0x60, 0x0f),
    ILI9881C_COMMAND(0x61, 0x0c),
    ILI9881C_COMMAND(0x62, 0x0d),
    ILI9881C_COMMAND(0x63, 0x06),
    ILI9881C_COMMAND(0x64, 0x07),
    ILI9881C_COMMAND(0x65, 0x02),
    ILI9881C_COMMAND(0x66, 0x02),
    ILI9881C_COMMAND(0x67, 0x02),
    ILI9881C_COMMAND(0x68, 0x02),
    ILI9881C_COMMAND(0x69, 0x01),
    ILI9881C_COMMAND(0x6a, 0x00),
    ILI9881C_COMMAND(0x6b, 0x02),
    ILI9881C_COMMAND(0x6c, 0x15),
    ILI9881C_COMMAND(0x6d, 0x14),
    ILI9881C_COMMAND(0x6e, 0x02),
    ILI9881C_COMMAND(0x6f, 0x02),
    ILI9881C_COMMAND(0x70, 0x02),
    ILI9881C_COMMAND(0x71, 0x02),
    ILI9881C_COMMAND(0x72, 0x02),
    ILI9881C_COMMAND(0x73, 0x02),
    ILI9881C_COMMAND(0x74, 0x02),
    ILI9881C_COMMAND(0x75, 0x0e),
    ILI9881C_COMMAND(0x76, 0x0f),
    ILI9881C_COMMAND(0x77, 0x0c),
    ILI9881C_COMMAND(0x78, 0x0d),
    ILI9881C_COMMAND(0x79, 0x06),
    ILI9881C_COMMAND(0x7a, 0x07),
    ILI9881C_COMMAND(0x7b, 0x02),
    ILI9881C_COMMAND(0x7c, 0x02),
    ILI9881C_COMMAND(0x7d, 0x02),
    ILI9881C_COMMAND(0x7e, 0x02),
    ILI9881C_COMMAND(0x7f, 0x01),
    ILI9881C_COMMAND(0x80, 0x00),
    ILI9881C_COMMAND(0x81, 0x02),
    ILI9881C_COMMAND(0x82, 0x14),
    ILI9881C_COMMAND(0x83, 0x15),
    ILI9881C_COMMAND(0x84, 0x02),
    ILI9881C_COMMAND(0x85, 0x02),
    ILI9881C_COMMAND(0x86, 0x02),
    ILI9881C_COMMAND(0x87, 0x02),
    ILI9881C_COMMAND(0x88, 0x02),
    ILI9881C_COMMAND(0x89, 0x02),
    ILI9881C_COMMAND(0x8A, 0x02),

    /* ------------------------------------------------------------------ */
    /* PAGE 4 : Power & Voltage Settings                                  */
    /* ------------------------------------------------------------------ */
    ILI9881C_SWITCH_PAGE(4),
    ILI9881C_COMMAND(0x6C, 0x15),   /* VGH = 15V                         */
    ILI9881C_COMMAND(0x6E, 0x2A),   /* VGL setting                       */
    ILI9881C_COMMAND(0x6F, 0x33),   /* VGL regulator                     */
    ILI9881C_COMMAND(0x3B, 0x98),
    ILI9881C_COMMAND(0x3a, 0x94),
    ILI9881C_COMMAND(0x8D, 0x14),
    ILI9881C_COMMAND(0x87, 0xBA),
    ILI9881C_COMMAND(0x26, 0x76),
    ILI9881C_COMMAND(0xB2, 0xD1),
    ILI9881C_COMMAND(0xB5, 0x06),
    ILI9881C_COMMAND(0x38, 0x01),
    ILI9881C_COMMAND(0x39, 0x00),

    /* ------------------------------------------------------------------ */
    /* PAGE 1 : Display Control, VCOM & Gamma Curves                      */
    /* ------------------------------------------------------------------ */
    ILI9881C_SWITCH_PAGE(1),
    ILI9881C_COMMAND(0x22, 0x0A),   /* Display function control          */
    ILI9881C_COMMAND(0x31, 0x00),   /* Display inversion off             */
    ILI9881C_COMMAND(0x53, 0x7d),   /* VCOM1 — per rpi_7inch_init rpi-6.6.y  */
    ILI9881C_COMMAND(0x55, 0x8f),   /* VCOM2 — per rpi_7inch_init rpi-6.6.y  */
    ILI9881C_COMMAND(0x40, 0x33),
    ILI9881C_COMMAND(0x50, 0x96),   /* VREG1OUT positive voltage         */
    ILI9881C_COMMAND(0x51, 0x96),   /* VREG2OUT negative voltage         */
    ILI9881C_COMMAND(0x60, 0x23),   /* SDTI source timing                */

    /* Positive Gamma Curve (A0-B3) */
    ILI9881C_COMMAND(0xA0, 0x08),
    ILI9881C_COMMAND(0xA1, 0x1d),
    ILI9881C_COMMAND(0xA2, 0x2a),
    ILI9881C_COMMAND(0xA3, 0x10),
    ILI9881C_COMMAND(0xA4, 0x15),
    ILI9881C_COMMAND(0xA5, 0x28),
    ILI9881C_COMMAND(0xA6, 0x1c),
    ILI9881C_COMMAND(0xA7, 0x1d),
    ILI9881C_COMMAND(0xA8, 0x7e),
    ILI9881C_COMMAND(0xA9, 0x1d),
    ILI9881C_COMMAND(0xAA, 0x29),
    ILI9881C_COMMAND(0xAB, 0x6b),
    ILI9881C_COMMAND(0xAC, 0x1a),
    ILI9881C_COMMAND(0xAD, 0x18),
    ILI9881C_COMMAND(0xAE, 0x4b),
    ILI9881C_COMMAND(0xAF, 0x20),
    ILI9881C_COMMAND(0xB0, 0x27),
    ILI9881C_COMMAND(0xB1, 0x50),
    ILI9881C_COMMAND(0xB2, 0x64),
    ILI9881C_COMMAND(0xB3, 0x39),

    /* Negative Gamma Curve (C0-D3) */
    ILI9881C_COMMAND(0xC0, 0x08),
    ILI9881C_COMMAND(0xC1, 0x1d),
    ILI9881C_COMMAND(0xC2, 0x2a),
    ILI9881C_COMMAND(0xC3, 0x10),
    ILI9881C_COMMAND(0xC4, 0x15),
    ILI9881C_COMMAND(0xC5, 0x28),
    ILI9881C_COMMAND(0xC6, 0x1c),
    ILI9881C_COMMAND(0xC7, 0x1d),
    ILI9881C_COMMAND(0xC8, 0x7e),
    ILI9881C_COMMAND(0xC9, 0x1d),
    ILI9881C_COMMAND(0xCA, 0x29),
    ILI9881C_COMMAND(0xCB, 0x6b),
    ILI9881C_COMMAND(0xCC, 0x1a),
    ILI9881C_COMMAND(0xCD, 0x18),
    ILI9881C_COMMAND(0xCE, 0x4b),
    ILI9881C_COMMAND(0xCF, 0x20),
    ILI9881C_COMMAND(0xD0, 0x27),
    ILI9881C_COMMAND(0xD1, 0x50),
    ILI9881C_COMMAND(0xD2, 0x64),
    ILI9881C_COMMAND(0xD3, 0x39),
};

#define RPI_7INCH_INIT_LEN  (sizeof(rpi_7inch_init) / sizeof(rpi_7inch_init[0]))

/************************** Function Prototypes ******************************/

static u32  RpiMcu_I2cWrite(u8 reg, u8 val);
static u32  RpiMcu_I2cRead(u8 reg, u8 *val_out);
static u32  RpiMcu_Init(void);
static u32  RpiMcu_PanelPowerOn(void);
static u32  RpiMcu_TouchEnable(void);
static u32  RpiMcu_SetBacklight(u8 brightness);
static u32  RpiMcu_EnableBacklight(void);

static void ILI9881C_SwitchPage(XDsiTxSs *InstPtr, u8 page);
static void ILI9881C_WriteReg(XDsiTxSs *InstPtr, u8 reg, u8 val);
static void ILI9881C_RunInitSequence(XDsiTxSs *InstPtr);
static u32  ILI9881C_PanelInit(XDsiTxSs *InstPtr);

static int  InitPanelIic(void);

/************************** Function Definitions *****************************/

/* ==========================================================================
 *                     ATtiny MCU I2C Functions
 * ==========================================================================
 * The RPi Touch Display 2 uses an ATtiny MCU at I2C address 0x45.
 * It controls panel power, LCD reset, touch reset, and backlight.
 * All communication uses simple register write/read (2-byte protocol):
 *   Write : [reg_addr, value]
 *   Read  : [reg_addr] repeated-start [read_byte]
 * ========================================================================== */

static u32 RpiMcu_I2cWrite(u8 reg, u8 val)
{
    u8  buf[2];
    int bytes_sent;

    buf[0] = reg;
    buf[1] = val;

    bytes_sent = XIic_Send(PANEL_IIC_BASEADDR,
                           RPI_MCU_I2C_ADDR,
                           buf,
                           2,
                           XIIC_STOP);

    if (bytes_sent != 2) {
        xil_printf("[PANEL I2C] ERROR: Write reg=0x%02X val=0x%02X failed "
                   "(sent=%d)\r\n", reg, val, bytes_sent);
        return XST_FAILURE;
    }

    usleep(200); /* Small inter-command gap for ATtiny to process */
    return XST_SUCCESS;
}

static u32 RpiMcu_I2cRead(u8 reg, u8 *val_out)
{
    u8  buf[1];
    int bytes_sent;
    int bytes_recv;

    /* Write register address with repeated start */
    buf[0]     = reg;
    bytes_sent = XIic_Send(PANEL_IIC_BASEADDR,
                           RPI_MCU_I2C_ADDR,
                           buf,
                           1,
                           XIIC_REPEATED_START);

    if (bytes_sent != 1) {
        xil_printf("[PANEL I2C] ERROR: Read addr phase failed (sent=%d)\r\n",
                   bytes_sent);
        return XST_FAILURE;
    }

    usleep(300); /* ATtiny address-to-data turnaround delay */

    /* Read one byte */
    bytes_recv = XIic_Recv(PANEL_IIC_BASEADDR,
                           RPI_MCU_I2C_ADDR,
                           val_out,
                           1,
                           XIIC_STOP);

    if (bytes_recv != 1) {
        xil_printf("[PANEL I2C] ERROR: Read data phase failed (recv=%d)\r\n",
                   bytes_recv);
        return XST_FAILURE;
    }

    return XST_SUCCESS;
}

static u32 RpiMcu_Init(void)
{
    u8  id  = 0;
    u32 ret;

    /* Read REG_ID to verify MCU is present and communicating */
    ret = RpiMcu_I2cRead(RPI_MCU_REG_ID, &id);
    if (ret != XST_SUCCESS) {
        xil_printf("[PANEL I2C] ERROR: Cannot read REG_ID. "
                   "Check I2C wiring and address.\r\n");
        return XST_FAILURE;
    }

    /* Assert both LCD and touch resets (clear POWERON register) */
    ret = RpiMcu_I2cWrite(RPI_MCU_REG_POWERON, 0x00);
    if (ret != XST_SUCCESS)
        return XST_FAILURE;

    usleep(10000); /* 10ms — hold reset */

    /* Disable backlight during init */
    ret = RpiMcu_I2cWrite(RPI_MCU_REG_PWM, 0x00);
    if (ret != XST_SUCCESS)
        return XST_FAILURE;

    return XST_SUCCESS;
}

static u32 RpiMcu_PanelPowerOn(void)
{
    u32 ret;

    /* Deassert LCD reset only — touch stays in reset */
    ret = RpiMcu_I2cWrite(RPI_MCU_REG_POWERON, RPI_MCU_LCD_RESET_BIT);
    if (ret != XST_SUCCESS) {
        xil_printf("[PANEL I2C] ERROR: PanelPowerOn failed\r\n");
        return XST_FAILURE;
    }

    usleep(20000); /* 20ms — panel stabilization after reset */

    return XST_SUCCESS;
}

static u32 RpiMcu_TouchEnable(void)
{
    u32 ret;

    /* Deassert both LCD and touch resets */
    ret = RpiMcu_I2cWrite(RPI_MCU_REG_POWERON,
                          RPI_MCU_LCD_RESET_BIT | RPI_MCU_CTP_RESET_BIT);
    if (ret != XST_SUCCESS) {
        xil_printf("[PANEL I2C] ERROR: TouchEnable failed\r\n");
        return XST_FAILURE;
    }


    usleep(10000); /* 10ms — touch controller boot time */

    return XST_SUCCESS;
}

static u32 RpiMcu_SetBacklight(u8 brightness)
{
    u8  pwm_val;
    u32 ret;

    /* Clamp to valid range */
    if (brightness > RPI_MCU_PWM_MAX_BRIGHTNESS)
        brightness = RPI_MCU_PWM_MAX_BRIGHTNESS;

    /* Set enable bit + brightness value */
    pwm_val = RPI_MCU_PWM_BL_ENABLE | (brightness & RPI_MCU_PWM_MAX_BRIGHTNESS);

    ret = RpiMcu_I2cWrite(RPI_MCU_REG_PWM, pwm_val);
    if (ret != XST_SUCCESS) {
        xil_printf("[PANEL I2C] ERROR: SetBacklight failed\r\n");
        return XST_FAILURE;
    }

    return XST_SUCCESS;
}

static u32 RpiMcu_EnableBacklight(void)
{
   return RpiMcu_SetBacklight(RPI_MCU_PWM_MAX_BRIGHTNESS);
}

/* ==========================================================================
 *                  ILI9881C DSI Functions
 * ========================================================================== */

/*****************************************************************************/
/**
* Switch the ILI9881C active register page.
* Sends DCS Long Write (0x39): {0xFF, 0x98, 0x81, page_num}
*
* Register map (DSI TX core base):
*   0x0034 = XDSI_DATA_OFFSET    — long packet payload FIFO
*   0x0030 = XDSI_COMMAND_OFFSET — command queue (header triggers TX)
*   0x002C = XDSI_STATUS_OFFSET  — status: bit-7 = RDY_FOR_LONG
*
* Header is written BEFORE the payload: the ILI9881C never sends a BTA
* acknowledge, so writing payload-then-header (the naive order) leaves
* WAIT_FOR_DATA stuck at 1 forever after the first long write. Writing the
* header first, then the payload, lets the core clear WAIT_FOR_DATA itself
* as soon as the payload arrives.
******************************************************************************/
static void ILI9881C_SwitchPage(XDsiTxSs *InstPtr, u8 page)
{
    UINTPTR base    = InstPtr->DsiPtr->Config.BaseAddr;
    u32     timeout;
    u32     stat;

    /* Long-packet payload: {0xFF, 0x98, 0x81, page} packed little-endian */
    u32 payload = (u32)ILI9881C_PAGE_CMD            |
                  ((u32)ILI9881C_PAGE_MAGIC0 <<  8) |
                  ((u32)ILI9881C_PAGE_MAGIC1 << 16) |
                  ((u32)page                << 24);

    /* Long-packet header: DT=0x39 (DCS Long Write), WC_LSB=4, WC_MSB=0 */
    u32 header = (u32)DSI_DCS_LONG_WRITE        |
                 (4U << XDSI_SPKTR_BYTE1_SHIFT) |
                 (0U << XDSI_SPKTR_BYTE2_SHIFT);



    /* Pre-check: wait until no command is in progress before starting */
    timeout = 50000;
    do {
        stat = XDsi_ReadReg(base, XDSI_STATUS_OFFSET);
    } while ((stat & XDSI_INPOGRESS_MASK) && --timeout);

    /* Step 1 — Write long-packet HEADER to COMMAND queue (0x0030) FIRST */
    XDsi_WriteReg(base, XDSI_COMMAND_OFFSET, header);

    /* Step 2 — Write payload to DATA FIFO (0x0034) */
    XDsi_WriteReg(base, XDSI_DATA_OFFSET, payload);

    /* Step 3 — Wait for LP transmission to complete */
    timeout = 50000;
    do {
        stat = XDsi_ReadReg(base, XDSI_STATUS_OFFSET);
    } while ( ((stat & XDSI_WAIT_FOR_DATA_MASK) ||
               (stat & XDSI_INPOGRESS_MASK)      )
              && --timeout );


}

/*****************************************************************************/
/**
* Write a single register on the active ILI9881C page.
* Sends DCS Short Write 1-param (0x15): {reg, value}
******************************************************************************/
static void ILI9881C_WriteReg(XDsiTxSs *InstPtr, u8 reg, u8 val)
{
    UINTPTR base    = InstPtr->DsiPtr->Config.BaseAddr;
    u32     timeout = 50000; /* ~100 ms */
    u32     stat;

    /* Poll STATUS 0x002C before writing short packet:
     *   bit 6  (RDY_FOR_SHORT) = 1  → IP ready to accept short-packet header
     *   bit 11 (INPOGRESS)     = 0  → no command currently executing */
    do {
        stat = XDsi_ReadReg(base, XDSI_STATUS_OFFSET);
    } while ( !( (stat & XDSI_RDY_FOR_SHORT_MASK) &&
                !(stat & XDSI_INPOGRESS_MASK)      )
              && --timeout );

    InstPtr->SpktData.VcId     = 0;
    InstPtr->SpktData.DataType = DSI_DCS_SHORT_WRITE_PARAM; /* 0x15 */
    InstPtr->SpktData.Data0    = reg;
    InstPtr->SpktData.Data1    = val;
    XDsiTxSs_SendShortPacket(InstPtr);
}

static void ILI9881C_RunInitSequence(XDsiTxSs *InstPtr)
{
    u32 i;
    u32 len        = RPI_7INCH_INIT_LEN;
    u32 short_sent = 0;
    u32 short_total;

    short_total = 0;
    for (i = 0; i < len; i++)
        if (!rpi_7inch_init[i].is_page_switch) short_total++;



    for (i = 0; i < len; i++) {
        if (rpi_7inch_init[i].is_page_switch) {
            ILI9881C_SwitchPage(InstPtr, rpi_7inch_init[i].val);
        } else {
            ILI9881C_WriteReg(InstPtr,
                              rpi_7inch_init[i].reg,
                              rpi_7inch_init[i].val);
            short_sent++;
        }
    }

}

/*****************************************************************************/
/**
* Complete ILI9881C + ATtiny MCU panel initialization sequence.
*
*   [I2C]  MCU Init      : Verify MCU, assert both resets, BL off
*   [I2C]  Panel Power ON: Deassert LCD reset (BIT0=1)
*   [DSI]  CMD MODE      : ILI9881C register init (Pages 3->4->1)
*   [DSI]  CMD MODE      : Page 0, Tear ON, Sleep Out (120ms), Display ON
*   [I2C]  Touch Enable  : Deassert touch reset (BIT1=1)
*   [I2C]  Backlight ON  : full brightness
******************************************************************************/
static u32 ILI9881C_PanelInit(XDsiTxSs *InstPtr)
{
    u32 Status;

    xil_printf("\r\n===== ILI9881C Panel Initialization Start =====\r\n");

    Status = RpiMcu_Init();
    if (Status != XST_SUCCESS) {
        xil_printf("ERROR: ATtiny MCU init failed.\r\n");
        return XST_FAILURE;
    }

    Status = RpiMcu_PanelPowerOn();
    if (Status != XST_SUCCESS) {
        xil_printf("ERROR: Panel power-on failed.\r\n");
        return XST_FAILURE;
    }

    ILI9881C_RunInitSequence(InstPtr);
    ILI9881C_SwitchPage(InstPtr, 0);

    /* Enable Tearing Effect (VSYNC signal to host) */
    InstPtr->SpktData.VcId     = 0;
    InstPtr->SpktData.DataType = DSI_DCS_SHORT_WRITE_PARAM; /* 0x15 */
    InstPtr->SpktData.Data0    = DCS_SET_TEAR_ON;           /* 0x35 */
    InstPtr->SpktData.Data1    = 0x00;
    XDsiTxSs_SendShortPacket(InstPtr);
    xil_printf("[ILI9881C] Sent Tear-On (0x35)\r\n");
    usleep(10000);

    /* Exit Sleep Mode — mandatory 120ms wait after */
    InstPtr->SpktData.VcId     = 0;
    InstPtr->SpktData.DataType = DSI_DCS_SHORT_WRITE;       /* 0x05 */
    InstPtr->SpktData.Data0    = DCS_EXIT_SLEEP_MODE;       /* 0x11 */
    InstPtr->SpktData.Data1    = 0x00;
    XDsiTxSs_SendShortPacket(InstPtr);
    xil_printf("[ILI9881C] Sent Sleep Out (0x11) — waiting 120ms...\r\n");
    usleep(120000); /* 120ms mandatory per MIPI DCS specification */

    /* Display ON */
    InstPtr->SpktData.VcId     = 0;
    InstPtr->SpktData.DataType = DSI_DCS_SHORT_WRITE;       /* 0x05 */
    InstPtr->SpktData.Data0    = DCS_SET_DISPLAY_ON;        /* 0x29 */
    InstPtr->SpktData.Data1    = 0x00;
    XDsiTxSs_SendShortPacket(InstPtr);
    xil_printf("[ILI9881C] Sent Display On (0x29)\r\n");
    usleep(20000); /* 20ms stabilization */

    Status = RpiMcu_TouchEnable();
    if (Status != XST_SUCCESS) {
        xil_printf("WARNING: Touch enable failed (non-fatal).\r\n");
    }

    Status = RpiMcu_EnableBacklight();
    if (Status != XST_SUCCESS) {
        xil_printf("ERROR: Backlight enable failed.\r\n");
        return XST_FAILURE;
    }

    xil_printf("===== ILI9881C Panel Initialization Complete =====\r\n\r\n");
    return XST_SUCCESS;
}

/*****************************************************************************/
/**
* Initialize the AXI IIC instance used for the panel's ATtiny MCU.
* Named distinctly from mipi_cfg.c's InitIIC()/IicSensor, which are the
* camera sensor's separate I2C bus (XIIC_2).
******************************************************************************/
static int InitPanelIic(void)
{
    int          Status;
    XIic_Config *ConfigPtr;

    memset(&PanelIic, 0, sizeof(XIic));

    ConfigPtr = XIic_LookupConfig(PANEL_IIC_BASEADDR);
    if (ConfigPtr == NULL) {
        return XST_FAILURE;
    }

    Status = XIic_CfgInitialize(&PanelIic, ConfigPtr, ConfigPtr->BaseAddress);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    Status = XIic_SelfTest(&PanelIic);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }
    return Status;
}

/*****************************************************************************/
/**
* One-time DSI TX Subsystem + ATtiny MCU + ILI9881C panel bring-up.
* See dsi_txss_display.h for the split rationale vs DsiPath_StartVideo().
******************************************************************************/
u32 DsiPath_Init(void)
{
    u32              Status = XST_FAILURE;
    XDsiTxSs_Config *ConfigPtr;
    XDsi_VideoTiming Timing;

    Status = InitPanelIic();
    if (Status != XST_SUCCESS) {
        xil_printf("\r\nPanel IIC Init Failed\r\n");
        return XST_FAILURE;
    }
    xil_printf("Panel IIC init done.\r\n");

    /* Lookup and initialize DSI TX Subsystem */
    ConfigPtr = XDsiTxSs_LookupConfig(DSI_TX_BASEADDR);
    if (!ConfigPtr) {
        xil_printf("ERROR: XDsiTxSs_LookupConfig failed.\r\n");
        return XST_FAILURE;
    }

    Status = XDsiTxSs_CfgInitialize(&DsiTxSsInst, ConfigPtr,
                                     ConfigPtr->BaseAddr);
    if (Status != XST_SUCCESS) {
        xil_printf("MIPI DSI TX SS config initialization failed.\r\n");
        return XST_FAILURE;
    }

    XDsiTxSs_Reset(&DsiTxSsInst);

    do {
        Status = XDsiTxSs_IsControllerReady(&DsiTxSsInst);
    } while (!Status);
    xil_printf("DSI Controller ready.\r\n");

    /* Switch to COMMAND MODE for panel init */
    XDsiTxSs_SetDSIMode(&DsiTxSsInst, XDSI_COMMAND_MODE);
    usleep(5000);

    /* Full panel initialization (ATtiny MCU I2C + ILI9881C DSI commands) */
    Status = ILI9881C_PanelInit(&DsiTxSsInst);
    if (Status != XST_SUCCESS) {
        xil_printf("ERROR: ILI9881C Panel init failed.\r\n");
        return XST_FAILURE;
    }

     usleep(10000);
    /* Disable DSI before changing timing */
    Status= XDsiTxSs_Activate(&DsiTxSsInst, XDSITXSS_DSI, XDSITXSS_DISABLE);
    usleep(10000);

    /* Switch to VIDEO MODE */
    XDsiTxSs_SetDSIMode(&DsiTxSsInst, XDSI_VIDEO_MODE);


    /* Protocol Config Register */
    Xil_Out32(DSI_TX_BASEADDR + 0x0004, 0x3F01);

    /* Configure video timing — panel native 720x1280 @ 60Hz */
    Timing.HActive     = HACT_VALUE;
    Timing.HBackPorch  = HBP_VALUE;
    Timing.HFrontPorch = HFP_VALUE;
    Timing.HSyncWidth  = HSA_VALUE;
    Timing.VActive     = VACT_VALUE;
    Timing.VBackPorch  = VBP_VALUE;
    Timing.VFrontPorch = VFP_VALUE;
    Timing.VSyncWidth  = VSA_VALUE;
    Timing.BLLPBurst   = BLLP_BURST_VALUE;

    XDsiTxSs_SetCustomVideoInterfaceTiming(&DsiTxSsInst,
                                           XDSI_VM_BURST_MODE,
                                           &Timing);

    return XST_SUCCESS;
}

/*****************************************************************************/
/**
* Activate the DSI TX Subsystem to begin streaming whatever video is
* currently presented on its AXI4-Stream input (the VPSS scaler's output,
* once the switch is steered to the DSI path). Call this LAST, only once
* the upstream sensor/CSI/scaler chain has already been rebuilt and started
* by the caller (see DsiPath_Reinit()) — matches the ZCU102 reference's
* EnableDSI(), which is likewise called only after CamReset()/EnableCSI()/
* SetupCameraSensor()/StartSensor() have all completed.
******************************************************************************/
u32 DsiPath_StartVideo(void)
{
    u32 Status;

    Status = XDsiTxSs_Activate(&DsiTxSsInst, XDSITXSS_DSI, XDSITXSS_ENABLE);
    xil_printf("DSI core enabled. \r\n");
    /* XDsiTxSs_Activate returns XST_INVALID_PARAM (15) when the MIPI TX PHY
     * has no register interface (IsMipiTxPhyRegIntfcPresent=0). The DSI core
     * is enabled regardless — treat this as success. */

    return XST_SUCCESS;
}

/*****************************************************************************/
/**
* Deactivate the DSI TX Subsystem. Call this first, whenever DSI is about
* to stop being the active output (or right before DsiPath_Reinit(), to
* prepare for re-entering DSI) — matches the ZCU102 reference's
* DisableDSI(), which is always called before its InitDSI() reset step.
******************************************************************************/
void DsiPath_StopVideo(void)
{
    XDsiTxSs_Activate(&DsiTxSsInst, XDSITXSS_DSI, XDSITXSS_DISABLE);
    xil_printf("DSI core disabled.\r\n");
}

/*****************************************************************************/
/**
* Hardware-reset the DSI TX Subsystem and re-apply its VIDEO mode timing
* configuration, without starting it. Matches the ZCU102 reference's
* InitDSI(): a bare XDsiTxSs_Activate(DISABLE)/(ENABLE) toggle is not
* sufficient to recover the core from having starved on a cut-off AXI4-
* Stream input (proven empirically — that approach was tried and caused a
* FIFO under-run on the very first switch to DSI). XDsiTxSs_Reset() clears
* the byte-stream FIFO and protocol engine state at the register level, so
* the core comes back in the same known-good state it was in right after
* boot, instead of whatever state a starve event left it in.
*
* Call this every time DSI is about to become (or stop being) the active
* output, before DsiPath_StartVideo() — never call DsiPath_StartVideo()
* without a preceding DsiPath_Reinit() once the core has ever been
* disabled/starved.
******************************************************************************/
void DsiPath_Reinit(void)
{
    u32              Status;
    XDsi_VideoTiming Timing;

    /* Redundant if DsiPath_StopVideo() already ran — harmless, matches the
     * belt-and-suspenders disable-before-reset pattern in the reference. */
    XDsiTxSs_Activate(&DsiTxSsInst, XDSITXSS_DSI, XDSITXSS_DISABLE);
    XDsiTxSs_Reset(&DsiTxSsInst);
    usleep(100000);

    do {
        Status = XDsiTxSs_IsControllerReady(&DsiTxSsInst);
    } while (!Status);

    /* XDsiTxSs_Reset() leaves the core's Command-Mode bit cleared (i.e.
     * already "video mode" per XDsi_IsModeEnabled() — the same state
     * DsiPath_Init() explicitly transitions into via this same call after
     * its own reset). Calling it again here is expected to return
     * XST_INVALID_PARAM when the core is already in that state — a no-op,
     * not an error; tolerated deliberately, same as the PHY-register-
     * interface quirk already tolerated in DsiPath_StartVideo(). Kept for
     * correctness in case that assumption ever changes, not for its
     * return value. */
    XDsiTxSs_SetDSIMode(&DsiTxSsInst, XDSI_VIDEO_MODE);

    /* Protocol Config Register + video timing — identical to the sequence
     * in DsiPath_Init(), replayed here since XDsiTxSs_Reset() clears it. */
    Xil_Out32(DSI_TX_BASEADDR + 0x0004, 0x3F01);

    Timing.HActive     = HACT_VALUE;
    Timing.HBackPorch  = HBP_VALUE;
    Timing.HFrontPorch = HFP_VALUE;
    Timing.HSyncWidth  = HSA_VALUE;
    Timing.VActive     = VACT_VALUE;
    Timing.VBackPorch  = VBP_VALUE;
    Timing.VFrontPorch = VFP_VALUE;
    Timing.VSyncWidth  = VSA_VALUE;
    Timing.BLLPBurst   = BLLP_BURST_VALUE;

    XDsiTxSs_SetCustomVideoInterfaceTiming(&DsiTxSsInst,
                                           XDSI_VM_BURST_MODE,
                                           &Timing);

    xil_printf("DSI reinitialized .\r\n");
    usleep(1000000);
}
