/*
 * Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 */

#include "pm_common.h"
#include "pm_slave.h"
#include "pm_periph.h"
#include "pm_usb.h"
#include "pm_requirement.h"

#define PINMUX_FN(name, fn, sel, do)	\
static const PmPinMuxFn pinMux##name = \
	{ \
		.select = sel, \
		.fid = fn, \
		PIN_##do \
	}

#define PIN_NULL	\
		.slaves = NULL, \
		.slavesCnt = 0U

#define PIN_BIND(c)	\
		.slaves = c, \
		.slavesCnt = ARRAY_SIZE(c)

#define PINMUX(id)	\
	static const PmPinMuxFn* const pmPinMux##id[]

#define PINMUX_REF(pinmux)	\
	{	\
		.pinMux = pinmux,	\
		.pinMuxSize = ARRAY_SIZE(pinmux),	\
	}

#define FN(name)	&pinMux##name

#define DEFINE_PIN(id)	\
	{	\
		.pinMux = pmPinMux##id,	\
		.owner = 0U,	\
	}

#define PM_PIN_PARAM_RO		(1 << 0U)
#define PM_PIN_PARAM_2_BITS	(1 << 1U)

#define PM_PIN_PARAM_PER_REG	26U

#define IOU_SLCR_BANK0_CTRL0	(IOU_SLCR_BASE + 0x138U)
#define IOU_SLCR_BANK0_CTRL1	(IOU_SLCR_BASE + 0x154U)
#define PM_IOU_SLCR_BANK_OFFSET	(IOU_SLCR_BANK0_CTRL1 - IOU_SLCR_BANK0_CTRL0)

#define PM_PIN_PARAM_GET_ADDR(pinId, regOffset)	\
	(IOU_SLCR_BANK0_CTRL0 + \
	PM_IOU_SLCR_BANK_OFFSET * (pinId / PM_PIN_PARAM_PER_REG) + regOffset)

#define IOU_SLCR_BANK1_CTRL5	(IOU_SLCR_BASE + 164U)

#define FIX_BANK1_CTRL5(shift)	\
	shift = ((shift < 12U) ? (shift + 14U) : (shift - 12U))

#define SWAP_BITS_BANK1_CTRL5(val)	\
	val = ((val & 0x3FFFU) << 12U) | ((val >> 14U) & 0xFFFU)

/**
 * PmPinMuxFn - PIN mux function model
 * @slaves	Pointer to the array of associated slaves
 * @slavesCnt	Number of elements in 'slaves' array
 * @fid		Function ID
 * @select	Select value
 */
typedef struct PmPinMuxFn {
	const PmSlave** const slaves;
	const u8 slavesCnt;
	const u8 fid;
	const u8 select;
} PmPinMuxFn;

/**
 * PmMioPin - MIO PIN model
 * @pinMux	Pointer to the array of mux function pointers
 * @owner	IPI mask of the master which requested PIN control
 */
typedef struct PmMioPin {
	const PmPinMuxFn* const* const pinMux;
	u32 owner;
} PmMioPin;

/**
 * PmPinParam - PIN parameter structure
 * @offset	Register offset
 * @flags	Parameter flags
 */
typedef struct PmPinParam {
	const u8 offset;
	const u8 flags;
} PmPinParam;

static const PmSlave* pmCan0Slaves[] = { &pmSlaveCan0_g };
static const PmSlave* pmCan1Slaves[] = { &pmSlaveCan1_g };
static const PmSlave* pmGpioSlaves[] = { &pmSlaveGpio_g };
static const PmSlave* pmI2C0Slaves[] = { &pmSlaveI2C0_g };
static const PmSlave* pmI2C1Slaves[] = { &pmSlaveI2C1_g };
static const PmSlave* pmQSpiSlaves[] = { &pmSlaveQSpi_g };
static const PmSlave* pmSpi0Slaves[] = { &pmSlaveSpi0_g };
static const PmSlave* pmSpi1Slaves[] = { &pmSlaveSpi1_g };
static const PmSlave* pmSD0Slaves[] = { &pmSlaveSD0_g };
static const PmSlave* pmSD1Slaves[] = { &pmSlaveSD1_g };
static const PmSlave* pmNandSlaves[] = { &pmSlaveNand_g };
static const PmSlave* pmTtc0Slaves[] = { &pmSlaveTtc0_g };
static const PmSlave* pmTtc1Slaves[] = { &pmSlaveTtc1_g };
static const PmSlave* pmTtc2Slaves[] = { &pmSlaveTtc2_g };
static const PmSlave* pmTtc3Slaves[] = { &pmSlaveTtc3_g };
static const PmSlave* pmUart0Slaves[] = { &pmSlaveUart0_g };
static const PmSlave* pmUart1Slaves[] = { &pmSlaveUart1_g };
static const PmSlave* pmUsb0Slaves[] = { &pmSlaveUsb0_g.slv };
static const PmSlave* pmUsb1Slaves[] = { &pmSlaveUsb1_g.slv };
static const PmSlave* pmSwdt1Slaves[] = { &pmSlaveFpdWdt_g };
static const PmSlave* pmPcieSlaves[] = { &pmSlavePcie_g };
static const PmSlave* pmDPSlaves[] = { &pmSlaveDP_g };
static const PmSlave* pmEth0Slaves[] = { &pmSlaveEth0_g };
static const PmSlave* pmEth1Slaves[] = { &pmSlaveEth1_g };
static const PmSlave* pmEth2Slaves[] = { &pmSlaveEth2_g };
static const PmSlave* pmEth3Slaves[] = { &pmSlaveEth3_g };
static const PmSlave* pmGemTsuSlaves[] = { &pmSlaveEth0_g,
					   &pmSlaveEth1_g,
					   &pmSlaveEth2_g,
					   &pmSlaveEth3_g };

PINMUX_FN(Can0,		PINCTRL_FUNC_CAN0,	0x20U,	BIND(pmCan0Slaves));
PINMUX_FN(Can1,		PINCTRL_FUNC_CAN1,	0x20U,	BIND(pmCan1Slaves));
PINMUX_FN(Eth0,		PINCTRL_FUNC_ETHERNET0,	0x02U,	BIND(pmEth0Slaves));
PINMUX_FN(Eth1,		PINCTRL_FUNC_ETHERNET1,	0x02U,	BIND(pmEth1Slaves));
PINMUX_FN(Eth2,		PINCTRL_FUNC_ETHERNET2,	0x02U,	BIND(pmEth2Slaves));
PINMUX_FN(Eth3,		PINCTRL_FUNC_ETHERNET3,	0x02U,	BIND(pmEth3Slaves));
PINMUX_FN(GemTsu,	PINCTRL_FUNC_GEMTSU0,	0x02U,	BIND(pmGemTsuSlaves));
PINMUX_FN(Gpio,		PINCTRL_FUNC_GPIO0,	0x00U,	BIND(pmGpioSlaves));
PINMUX_FN(I2C0,		PINCTRL_FUNC_I2C0,	0x40U,	BIND(pmI2C0Slaves));
PINMUX_FN(I2C1,		PINCTRL_FUNC_I2C1,	0x40U,	BIND(pmI2C1Slaves));
PINMUX_FN(Mdio0,	PINCTRL_FUNC_MDIO0,	0x60U,	BIND(pmEth0Slaves));
PINMUX_FN(Mdio1,	PINCTRL_FUNC_MDIO1,	0x80U,	BIND(pmEth1Slaves));
PINMUX_FN(Mdio2,	PINCTRL_FUNC_MDIO2,	0xA0U,	BIND(pmEth2Slaves));
PINMUX_FN(Mdio3,	PINCTRL_FUNC_MDIO3,	0xC0U,	BIND(pmEth3Slaves));
PINMUX_FN(QSpi,		PINCTRL_FUNC_QSPI0,	0x02U,	BIND(pmQSpiSlaves));
PINMUX_FN(QSpiFbClk,	PINCTRL_FUNC_QSPI_FBCLK,0x02U,	BIND(pmQSpiSlaves));
PINMUX_FN(QSpiSS,	PINCTRL_FUNC_QSPI_SS,	0x02U,	BIND(pmQSpiSlaves));
PINMUX_FN(Spi0,		PINCTRL_FUNC_SPI0,	0x80U,	BIND(pmSpi0Slaves));
PINMUX_FN(Spi1,		PINCTRL_FUNC_SPI1,	0x80U,	BIND(pmSpi1Slaves));
PINMUX_FN(Spi0SS,	PINCTRL_FUNC_SPI0_SS,	0x80U,	BIND(pmSpi0Slaves));
PINMUX_FN(Spi1SS,	PINCTRL_FUNC_SPI1_SS,	0x80U,	BIND(pmSpi1Slaves));
PINMUX_FN(Sdio0,	PINCTRL_FUNC_SDIO0,	0x08U,	BIND(pmSD0Slaves));
PINMUX_FN(Sdio0Pc,	PINCTRL_FUNC_SDIO0_PC,	0x08U,	BIND(pmSD0Slaves));
PINMUX_FN(Sdio0Cd,	PINCTRL_FUNC_SDIO0_CD,	0x08U,	BIND(pmSD0Slaves));
PINMUX_FN(Sdio0Wp,	PINCTRL_FUNC_SDIO0_WP,	0x08U,	BIND(pmSD0Slaves));
PINMUX_FN(Sdio1,	PINCTRL_FUNC_SDIO1,	0x10U,	BIND(pmSD1Slaves));
PINMUX_FN(Sdio1Pc,	PINCTRL_FUNC_SDIO1_PC,	0x10U,	BIND(pmSD1Slaves));
PINMUX_FN(Sdio1Cd,	PINCTRL_FUNC_SDIO1_CD,	0x10U,	BIND(pmSD1Slaves));
PINMUX_FN(Sdio1Wp,	PINCTRL_FUNC_SDIO1_WP,	0x10U,	BIND(pmSD1Slaves));
PINMUX_FN(Nand,		PINCTRL_FUNC_NAND0,	0x04U,	BIND(pmNandSlaves));
PINMUX_FN(NandCe,	PINCTRL_FUNC_NAND0_CE,	0x04U,	BIND(pmNandSlaves));
PINMUX_FN(NandRb,	PINCTRL_FUNC_NAND0_RB,	0x04U,	BIND(pmNandSlaves));
PINMUX_FN(NandDqs,	PINCTRL_FUNC_NAND0_DQS,	0x04U,	BIND(pmNandSlaves));
PINMUX_FN(Ttc0Clk,	PINCTRL_FUNC_TTC0_CLK,	0xA0U,	BIND(pmTtc0Slaves));
PINMUX_FN(Ttc0Wav,	PINCTRL_FUNC_TTC0_WAV,	0xA0U,	BIND(pmTtc0Slaves));
PINMUX_FN(Ttc1Clk,	PINCTRL_FUNC_TTC1_CLK,	0xA0U,	BIND(pmTtc1Slaves));
PINMUX_FN(Ttc1Wav,	PINCTRL_FUNC_TTC1_WAV,	0xA0U,	BIND(pmTtc1Slaves));
PINMUX_FN(Ttc2Clk,	PINCTRL_FUNC_TTC2_CLK,	0xA0U,	BIND(pmTtc2Slaves));
PINMUX_FN(Ttc2Wav,	PINCTRL_FUNC_TTC2_WAV,	0xA0U,	BIND(pmTtc2Slaves));
PINMUX_FN(Ttc3Clk,	PINCTRL_FUNC_TTC3_CLK,	0xA0U,	BIND(pmTtc3Slaves));
PINMUX_FN(Ttc3Wav,	PINCTRL_FUNC_TTC3_WAV,	0xA0U,	BIND(pmTtc3Slaves));
PINMUX_FN(Uart0,	PINCTRL_FUNC_UART0,	0xC0U,	BIND(pmUart0Slaves));
PINMUX_FN(Uart1,	PINCTRL_FUNC_UART1,	0xC0U,	BIND(pmUart1Slaves));
PINMUX_FN(Usb0,		PINCTRL_FUNC_USB0,	0x04U,	BIND(pmUsb0Slaves));
PINMUX_FN(Usb1,		PINCTRL_FUNC_USB1,	0x04U,	BIND(pmUsb1Slaves));
PINMUX_FN(Swdt0Clk,	PINCTRL_FUNC_SWDT0_CLK,	0x60U,	NULL);
PINMUX_FN(Swdt0Rst,	PINCTRL_FUNC_SWDT0_RST,	0x60U,	NULL);
PINMUX_FN(Swdt1Clk,	PINCTRL_FUNC_SWDT1_CLK,	0x60U,	BIND(pmSwdt1Slaves));
PINMUX_FN(Swdt1Rst,	PINCTRL_FUNC_SWDT1_RST,	0x60U,	BIND(pmSwdt1Slaves));
PINMUX_FN(Pmu,		PINCTRL_FUNC_PMU0,	0x08U,	NULL);
PINMUX_FN(Pcie,		PINCTRL_FUNC_PCIE0,	0x04U,	BIND(pmPcieSlaves));
PINMUX_FN(Csu,		PINCTRL_FUNC_CSU0,	0x18U,	NULL);
PINMUX_FN(Dp,		PINCTRL_FUNC_DPAUX0,	0x18U,	BIND(pmDPSlaves));
PINMUX_FN(PJtag,	PINCTRL_FUNC_PJTAG0,	0x60U,	NULL);
PINMUX_FN(Trace,	PINCTRL_FUNC_TRACE0,	0xE0U,	NULL);
PINMUX_FN(TraceClk,	PINCTRL_FUNC_TRACE0_CLK,0xE0U,	NULL);
PINMUX_FN(TestScan,	PINCTRL_FUNC_TESTSCAN0,	0x10U,	NULL);

/*
 * Mux select data is defined as follows:
 *
 *	L0_SEL[0],	L0_SEL[1],
 *	L1_SEL[0],	L1_SEL[1],
 *	L2_SEL[0],	L2_SEL[1],	L2_SEL[2],	L2_SEL[3],
 *	L3_SEL[0],	L3_SEL[1],	L3_SEL[2],	L3_SEL[3],
 *	L3_SEL[4],	L3_SEL[5],	L3_SEL[6],	L3_SEL[7],	NULL
 *
 * Note: L0_SEL[0], L1_SEL[0], and L2_SEL[0] are always reserved. If an element
 * in a pattern (matrix) above is missing an empty place is preserved to be able
 * to easily compare definitions with spec.
 */
PINMUX(0) = {		FN(QSpi),

					FN(TestScan),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(PJtag),
	FN(Spi0),	FN(Ttc3Clk),	FN(Uart1),	FN(TraceClk),	NULL
};
PINMUX(1) = {		FN(QSpi),

					FN(TestScan),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(PJtag),
	FN(Spi0SS),	FN(Ttc3Wav),	FN(Uart1),	FN(TraceClk),	NULL
};
PINMUX(2) = {		FN(QSpi),

					FN(TestScan),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(PJtag),
	FN(Spi0SS),	FN(Ttc2Clk),	FN(Uart0),	FN(Trace),	NULL
};
PINMUX(3) = {		FN(QSpi),

					FN(TestScan),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(PJtag),
	FN(Spi0SS),	FN(Ttc2Wav),	FN(Uart0),	FN(Trace),	NULL
};
PINMUX(4) = {		FN(QSpi),

					FN(TestScan),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(Swdt1Clk),
	FN(Spi0),	FN(Ttc1Clk),	FN(Uart1),	FN(Trace),	NULL
};
PINMUX(5) = {		FN(QSpiSS),

					FN(TestScan),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(Swdt1Rst),
	FN(Spi0),	FN(Ttc1Wav),	FN(Uart1),	FN(Trace),	NULL
};
PINMUX(6) = {		FN(QSpiFbClk),

					FN(TestScan),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(Swdt0Clk),
	FN(Spi1),	FN(Ttc0Clk),	FN(Uart0),	FN(Trace),	NULL
};
PINMUX(7) = {		FN(QSpiSS),

					FN(TestScan),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(Swdt0Rst),
	FN(Spi1SS),	FN(Ttc0Wav),	FN(Uart0),	FN(Trace),	NULL
};
PINMUX(8) = {		FN(QSpi),

					FN(TestScan),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(Swdt1Clk),
	FN(Spi1SS),	FN(Ttc3Clk),	FN(Uart1),	FN(Trace),	NULL
};
PINMUX(9) = {		FN(QSpi),
			FN(NandCe),
					FN(TestScan),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(Swdt1Rst),
	FN(Spi1SS),	FN(Ttc3Wav),	FN(Uart1),	FN(Trace),	NULL
};
PINMUX(10) = {		FN(QSpi),
			FN(NandRb),
					FN(TestScan),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(Swdt0Clk),
	FN(Spi1),	FN(Ttc2Clk),	FN(Uart0),	FN(Trace),	NULL
};
PINMUX(11) = {		FN(QSpi),
			FN(NandRb),
					FN(TestScan),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(Swdt0Rst),
	FN(Spi1),	FN(Ttc2Wav),	FN(Uart0),	FN(Trace),	NULL
};
PINMUX(12) = {		FN(QSpi),
			FN(NandDqs),
					FN(TestScan),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(PJtag),
	FN(Spi0),	FN(Ttc1Clk),	FN(Uart1),	FN(Trace),	NULL
};
PINMUX(13) = {
			FN(Nand),
			FN(Sdio0),	FN(TestScan),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(PJtag),
	FN(Spi0SS),	FN(Ttc1Wav),	FN(Uart1),	FN(Trace),	NULL
};
PINMUX(14) = {
			FN(Nand),
			FN(Sdio0),	FN(TestScan),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(PJtag),
	FN(Spi0SS),	FN(Ttc0Clk),	FN(Uart0),	FN(Trace),	NULL
};
PINMUX(15) = {
			FN(Nand),
			FN(Sdio0),	FN(TestScan),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(PJtag),
	FN(Spi0SS),	FN(Ttc0Wav),	FN(Uart0),	FN(Trace),	NULL
};
PINMUX(16) = {
			FN(Nand),
			FN(Sdio0),	FN(TestScan),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(Swdt1Clk),
	FN(Spi0),	FN(Ttc3Clk),	FN(Uart1),	FN(Trace),	NULL
};
PINMUX(17) = {
			FN(Nand),
			FN(Sdio0),	FN(TestScan),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(Swdt1Rst),
	FN(Spi0),	FN(Ttc3Wav),	FN(Uart1),	FN(Trace),	NULL
};
PINMUX(18) = {
			FN(Nand),
			FN(Sdio0),	FN(TestScan),	FN(Csu),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(Swdt0Clk),
	FN(Spi1),	FN(Ttc2Clk),	FN(Uart0),			NULL
};
PINMUX(19) = {
			FN(Nand),
			FN(Sdio0),	FN(TestScan),	FN(Csu),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(Swdt0Rst),
	FN(Spi1SS),	FN(Ttc2Wav),	FN(Uart0),			NULL
};
PINMUX(20) = {
			FN(Nand),
			FN(Sdio0),	FN(TestScan),	FN(Csu),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(Swdt1Clk),
	FN(Spi1SS),	FN(Ttc1Clk),	FN(Uart1),			NULL
};
PINMUX(21) = {
			FN(Nand),
			FN(Sdio0),	FN(TestScan),	FN(Csu),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(Swdt1Rst),
	FN(Spi1SS),	FN(Ttc1Wav),	FN(Uart1),			NULL
};
PINMUX(22) = {
			FN(Nand),
			FN(Sdio0),	FN(TestScan),	FN(Csu),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(Swdt0Clk),
	FN(Spi1),	FN(Ttc0Clk),	FN(Uart0),			NULL
};
PINMUX(23) = {
			FN(Nand),
			FN(Sdio0Pc),	FN(TestScan),	FN(Csu),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(Swdt0Rst),
	FN(Spi1),	FN(Ttc0Wav),	FN(Uart0),			NULL
};
PINMUX(24) = {
			FN(Nand),
			FN(Sdio0Cd),	FN(TestScan),	FN(Csu),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(Swdt1Clk),
			FN(Ttc3Clk),	FN(Uart1),			NULL
};
PINMUX(25) = {
			FN(Nand),
			FN(Sdio0Wp),	FN(TestScan),	FN(Csu),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(Swdt1Rst),
			FN(Ttc3Wav),	FN(Uart1),			NULL
};
PINMUX(26) = {		FN(Eth0),
			FN(NandCe),
			FN(Pmu),	FN(TestScan),	FN(Csu),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(PJtag),
	FN(Spi0),	FN(Ttc2Clk),	FN(Uart0),	FN(Trace),	NULL
};
PINMUX(27) = {		FN(Eth0),
			FN(NandRb),
			FN(Pmu),	FN(TestScan),	FN(Dp),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(PJtag),
	FN(Spi0SS),	FN(Ttc2Wav),	FN(Uart0),	FN(Trace),	NULL
};
PINMUX(28) = {		FN(Eth0),
			FN(NandRb),
			FN(Pmu),	FN(TestScan),	FN(Dp),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(PJtag),
	FN(Spi0SS),	FN(Ttc1Clk),	FN(Uart1),	FN(Trace),	NULL
};
PINMUX(29) = {		FN(Eth0),
			FN(Pcie),
			FN(Pmu),	FN(TestScan),	FN(Dp),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(PJtag),
	FN(Spi0SS),	FN(Ttc1Wav),	FN(Uart1),	FN(Trace),	NULL
};
PINMUX(30) = {		FN(Eth0),
			FN(Pcie),
			FN(Pmu),	FN(TestScan),	FN(Dp),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(Swdt0Clk),
	FN(Spi0),	FN(Ttc0Clk),	FN(Uart0),	FN(Trace),	NULL
};
PINMUX(31) = {		FN(Eth0),
			FN(Pcie),
			FN(Pmu),	FN(TestScan),	FN(Csu),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(Swdt0Rst),
	FN(Spi0),	FN(Ttc0Wav),	FN(Uart0),	FN(Trace),	NULL
};
PINMUX(32) = {		FN(Eth0),
			FN(NandDqs),
			FN(Pmu),	FN(TestScan),	FN(Csu),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(Swdt1Clk),
	FN(Spi1),	FN(Ttc3Clk),	FN(Uart1),	FN(Trace),	NULL
};
PINMUX(33) = {		FN(Eth0),
			FN(Pcie),
			FN(Pmu),	FN(TestScan),	FN(Csu),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(Swdt1Rst),
	FN(Spi1SS),	FN(Ttc3Wav),	FN(Uart1),	FN(Trace),	NULL
};
PINMUX(34) = {		FN(Eth0),
			FN(Pcie),
			FN(Pmu),	FN(TestScan),	FN(Dp),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(Swdt0Clk),
	FN(Spi1SS),	FN(Ttc2Clk),	FN(Uart0),	FN(Trace),	NULL
};
PINMUX(35) = {		FN(Eth0),
			FN(Pcie),
			FN(Pmu),	FN(TestScan),	FN(Dp),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(Swdt0Rst),
	FN(Spi1SS),	FN(Ttc2Wav),	FN(Uart0),	FN(Trace),	NULL
};
PINMUX(36) = {		FN(Eth0),
			FN(Pcie),
			FN(Pmu),	FN(TestScan),	FN(Dp),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(Swdt1Clk),
	FN(Spi1),	FN(Ttc1Clk),	FN(Uart1),	FN(Trace),	NULL
};
PINMUX(37) = {		FN(Eth0),
			FN(Pcie),
			FN(Pmu),	FN(TestScan),	FN(Dp),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(Swdt1Rst),
	FN(Spi1),	FN(Ttc1Wav),	FN(Uart1),	FN(Trace),	NULL
};
PINMUX(38) = {		FN(Eth1),

			FN(Sdio0),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(PJtag),
	FN(Spi0),	FN(Ttc0Clk),	FN(Uart0),	FN(TraceClk),	NULL
};
PINMUX(39) = {		FN(Eth1),

			FN(Sdio0Cd),	FN(Sdio1),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(PJtag),
	FN(Spi0SS),	FN(Ttc0Wav),	FN(Uart0),	FN(TraceClk),	NULL
};
PINMUX(40) = {		FN(Eth1),

			FN(Sdio0),	FN(Sdio1),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(PJtag),
	FN(Spi0SS),	FN(Ttc3Clk),	FN(Uart1),	FN(Trace),	NULL
};
PINMUX(41) = {		FN(Eth1),

			FN(Sdio0),	FN(Sdio1),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(PJtag),
	FN(Spi0SS),	FN(Ttc3Wav),	FN(Uart1),	FN(Trace),	NULL
};
PINMUX(42) = {		FN(Eth1),

			FN(Sdio0),	FN(Sdio1),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(Swdt0Clk),
	FN(Spi0),	FN(Ttc2Clk),	FN(Uart0),	FN(Trace),	NULL
};
PINMUX(43) = {		FN(Eth1),

			FN(Sdio0),	FN(Sdio1Pc),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(Swdt0Rst),
	FN(Spi0),	FN(Ttc2Wav),	FN(Uart0),	FN(Trace),	NULL
};
PINMUX(44) = {		FN(Eth1),

			FN(Sdio0),	FN(Sdio1Wp),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(Swdt1Clk),
	FN(Spi1),	FN(Ttc1Clk),	FN(Uart1),			NULL
};
PINMUX(45) = {		FN(Eth1),

			FN(Sdio0),	FN(Sdio1Cd),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(Swdt1Rst),
	FN(Spi1SS),	FN(Ttc1Wav),	FN(Uart1),			NULL
};
PINMUX(46) = {		FN(Eth1),

			FN(Sdio0),	FN(Sdio1),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(Swdt0Clk),
	FN(Spi1SS),	FN(Ttc0Clk),	FN(Uart0),			NULL
};
PINMUX(47) = {		FN(Eth1),

			FN(Sdio0),	FN(Sdio1),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(Swdt0Rst),
	FN(Spi1SS),	FN(Ttc0Wav),	FN(Uart0),			NULL
};
PINMUX(48) = {		FN(Eth1),

			FN(Sdio0),	FN(Sdio1),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(Swdt1Clk),
	FN(Spi1),	FN(Ttc3Clk),	FN(Uart1),			NULL
};
PINMUX(49) = {		FN(Eth1),

			FN(Sdio0Pc),	FN(Sdio1),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(Swdt1Rst),
	FN(Spi1),	FN(Ttc3Wav),	FN(Uart1),			NULL
};
PINMUX(50) = {		FN(GemTsu),

			FN(Sdio0Wp),	FN(Sdio1),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(Swdt0Clk),
	FN(Mdio1),	FN(Ttc2Clk),	FN(Uart0),			NULL
};
PINMUX(51) = {		FN(GemTsu),

			FN(Sdio0Wp),	FN(Sdio1),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(Swdt0Rst),
	FN(Mdio1),	FN(Ttc2Wav),	FN(Uart0),			NULL
};
PINMUX(52) = {		FN(Eth2),
			FN(Usb0),

	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(PJtag),
	FN(Spi0),	FN(Ttc1Clk),	FN(Uart1),	FN(TraceClk),	NULL
};
PINMUX(53) = {		FN(Eth2),
			FN(Usb0),

	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(PJtag),
	FN(Spi0SS),	FN(Ttc1Wav),	FN(Uart1),	FN(TraceClk),	NULL
};
PINMUX(54) = {		FN(Eth2),
			FN(Usb0),

	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(PJtag),
	FN(Spi0SS),	FN(Ttc0Clk),	FN(Uart0),	FN(Trace),	NULL
};
PINMUX(55) = {		FN(Eth2),
			FN(Usb0),

	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(PJtag),
	FN(Spi0SS),	FN(Ttc0Wav),	FN(Uart0),	FN(Trace),	NULL
};
PINMUX(56) = {		FN(Eth2),
			FN(Usb0),

	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(Swdt1Clk),
	FN(Spi0),	FN(Ttc3Clk),	FN(Uart1),	FN(Trace),	NULL
};
PINMUX(57) = {		FN(Eth2),
			FN(Usb0),

	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(Swdt1Rst),
	FN(Spi0),	FN(Ttc3Wav),	FN(Uart1),	FN(Trace),	NULL
};
PINMUX(58) = {		FN(Eth2),
			FN(Usb0),

	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(PJtag),
	FN(Spi1),	FN(Ttc2Clk),	FN(Uart0),	FN(Trace),	NULL
};
PINMUX(59) = {		FN(Eth2),
			FN(Usb0),

	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(PJtag),
	FN(Spi1SS),	FN(Ttc2Wav),	FN(Uart0),	FN(Trace),	NULL
};
PINMUX(60) = {		FN(Eth2),
			FN(Usb0),

	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(PJtag),
	FN(Spi1SS),	FN(Ttc1Clk),	FN(Uart1),	FN(Trace),	NULL
};
PINMUX(61) = {		FN(Eth2),
			FN(Usb0),

	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(PJtag),
	FN(Spi1SS),	FN(Ttc1Wav),	FN(Uart1),	FN(Trace),	NULL
};
PINMUX(62) = {		FN(Eth2),
			FN(Usb0),

	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(Swdt0Clk),
	FN(Spi1),	FN(Ttc0Clk),	FN(Uart0),	FN(Trace),	NULL
};
PINMUX(63) = {		FN(Eth2),
			FN(Usb0),

	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(Swdt0Rst),
	FN(Spi1),	FN(Ttc0Wav),	FN(Uart0),	FN(Trace),	NULL
};
PINMUX(64) = {		FN(Eth3),
			FN(Usb1),
			FN(Sdio0),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(Swdt1Clk),
	FN(Spi0),	FN(Ttc3Clk),	FN(Uart1),	FN(Trace),	NULL
};
PINMUX(65) = {		FN(Eth3),
			FN(Usb1),
			FN(Sdio0Cd),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(Swdt1Rst),
	FN(Spi0SS),	FN(Ttc3Wav),	FN(Uart1),	FN(Trace),	NULL
};
PINMUX(66) = {		FN(Eth3),
			FN(Usb1),
			FN(Sdio0),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(Swdt0Clk),
	FN(Spi0SS),	FN(Ttc2Clk),	FN(Uart0),	FN(Trace),	NULL
};
PINMUX(67) = {		FN(Eth3),
			FN(Usb1),
			FN(Sdio0),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(Swdt0Rst),
	FN(Spi0SS),	FN(Ttc2Wav),	FN(Uart0),	FN(Trace),	NULL
};
PINMUX(68) = {		FN(Eth3),
			FN(Usb1),
			FN(Sdio0),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(Swdt1Clk),
	FN(Spi0),	FN(Ttc1Clk),	FN(Uart1),	FN(Trace),	NULL
};
PINMUX(69) = {		FN(Eth3),
			FN(Usb1),
			FN(Sdio0),	FN(Sdio1Wp),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(Swdt1Rst),
	FN(Spi0),	FN(Ttc1Wav),	FN(Uart1),	FN(Trace),	NULL
};
PINMUX(70) = {		FN(Eth3),
			FN(Usb1),
			FN(Sdio0),	FN(Sdio1Pc),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(Swdt0Clk),
	FN(Spi1),	FN(Ttc0Clk),	FN(Uart0),			NULL
};
PINMUX(71) = {		FN(Eth3),
			FN(Usb1),
			FN(Sdio0),	FN(Sdio1),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(Swdt0Rst),
	FN(Spi1SS),	FN(Ttc0Wav),	FN(Uart0),			NULL
};
PINMUX(72) = {		FN(Eth3),
			FN(Usb1),
			FN(Sdio0),	FN(Sdio1),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(Swdt1Clk),
	FN(Spi1SS),			FN(Uart1),			NULL
};
PINMUX(73) = {		FN(Eth3),
			FN(Usb1),
			FN(Sdio0),	FN(Sdio1),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(Swdt1Rst),
	FN(Spi1SS),			FN(Uart1),			NULL
};
PINMUX(74) = {		FN(Eth3),
			FN(Usb1),
			FN(Sdio0),	FN(Sdio1),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(Swdt0Clk),
	FN(Spi1),			FN(Uart0),			NULL
};
PINMUX(75) = {		FN(Eth3),
			FN(Usb1),
			FN(Sdio0Pc),	FN(Sdio1),
	FN(Gpio),	FN(Can0),	FN(I2C0),	FN(Swdt0Rst),
	FN(Spi1),			FN(Uart0),			NULL
};
PINMUX(76) = {

			FN(Sdio0Wp),	FN(Sdio1),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(Mdio0),
	FN(Mdio1),	FN(Mdio2),	FN(Mdio3),			NULL
};
PINMUX(77) = {

					FN(Sdio1Cd),
	FN(Gpio),	FN(Can1),	FN(I2C1),	FN(Mdio0),
	FN(Mdio1),	FN(Mdio2),	FN(Mdio3),			NULL
};

static PmMioPin pmPinMuxCtrl[] = {
	DEFINE_PIN(0), DEFINE_PIN(1), DEFINE_PIN(2), DEFINE_PIN(3),
	DEFINE_PIN(4), DEFINE_PIN(5), DEFINE_PIN(6), DEFINE_PIN(7),
	DEFINE_PIN(8), DEFINE_PIN(9),
	DEFINE_PIN(10), DEFINE_PIN(11), DEFINE_PIN(12), DEFINE_PIN(13),
	DEFINE_PIN(14), DEFINE_PIN(15), DEFINE_PIN(16), DEFINE_PIN(17),
	DEFINE_PIN(18), DEFINE_PIN(19),
	DEFINE_PIN(20), DEFINE_PIN(21), DEFINE_PIN(22), DEFINE_PIN(23),
	DEFINE_PIN(24), DEFINE_PIN(25), DEFINE_PIN(26), DEFINE_PIN(27),
	DEFINE_PIN(28), DEFINE_PIN(29),
	DEFINE_PIN(30), DEFINE_PIN(31), DEFINE_PIN(32), DEFINE_PIN(33),
	DEFINE_PIN(34), DEFINE_PIN(35), DEFINE_PIN(36), DEFINE_PIN(37),
	DEFINE_PIN(38), DEFINE_PIN(39),
	DEFINE_PIN(40), DEFINE_PIN(41), DEFINE_PIN(42), DEFINE_PIN(43),
	DEFINE_PIN(44), DEFINE_PIN(45), DEFINE_PIN(46), DEFINE_PIN(47),
	DEFINE_PIN(48), DEFINE_PIN(49),
	DEFINE_PIN(50), DEFINE_PIN(51), DEFINE_PIN(52), DEFINE_PIN(53),
	DEFINE_PIN(54), DEFINE_PIN(55), DEFINE_PIN(56), DEFINE_PIN(57),
	DEFINE_PIN(58), DEFINE_PIN(59),
	DEFINE_PIN(60), DEFINE_PIN(61), DEFINE_PIN(62), DEFINE_PIN(63),
	DEFINE_PIN(64), DEFINE_PIN(65), DEFINE_PIN(66), DEFINE_PIN(67),
	DEFINE_PIN(68), DEFINE_PIN(69),
	DEFINE_PIN(70), DEFINE_PIN(71), DEFINE_PIN(72), DEFINE_PIN(73),
	DEFINE_PIN(74), DEFINE_PIN(75), DEFINE_PIN(76), DEFINE_PIN(77),
};

static PmPinParam pmPinParams[] = {
	[PINCTRL_CONFIG_SLEW_RATE] = {
		.offset = 0x14U,
		.flags = 0U,
	},
	[PINCTRL_CONFIG_BIAS_STATUS] = {
		.offset = 0x10U,
		.flags = 0U,
	},
	[PINCTRL_CONFIG_PULL_CTRL] = {
		.offset = 0xCU,
		.flags = 0U,
	},
	[PINCTRL_CONFIG_SCHMITT_CMOS] = {
		.offset = 0x8U,
		.flags = 0U,
	},
	[PINCTRL_CONFIG_DRIVE_STRENGTH] = {
		.offset = 0x0U,
		.flags = PM_PIN_PARAM_2_BITS,
	},
	[PINCTRL_CONFIG_VOLTAGE_STATUS] = {
		.offset = 0x18U,
		.flags = PM_PIN_PARAM_RO,
	},
};

/**
 * PmPinCtrlRequestInt() - Request PIN control
 * @ipiMask	IPI mask of the master
 * @pinId	ID of the pin in question
 *
 * @return	XST_SUCCESS if succesfully requested
 *		XST_INVALID_PARAM if pinId argument is not valid
 *		XST_PM_NO_ACCESS if PIN control is already requested by another
 *		master
 */
int PmPinCtrlRequestInt(const u32 ipiMask, const u32 pinId)
{
	int status = XST_SUCCESS;

	if (pinId >= ARRAY_SIZE(pmPinMuxCtrl)) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	if (0U != pmPinMuxCtrl[pinId].owner) {
		if (ipiMask == pmPinMuxCtrl[pinId].owner) {
			goto done;
		}
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	pmPinMuxCtrl[pinId].owner = ipiMask;

done:
	return status;
}

/**
 * PmPinCtrlReleaseInt() - Release PIN control
 * @ipiMask	IPI mask of the master
 * @pinId	ID of the pin in question
 *
 * @return	XST_SUCCESS if succesfully released
 *		XST_INVALID_PARAM if pinId argument is not valid
 *		XST_FAILURE if PIN control has not been previously requested
 */
int PmPinCtrlReleaseInt(const u32 ipiMask, const u32 pinId)
{
	int status = XST_SUCCESS;

	if (pinId >= ARRAY_SIZE(pmPinMuxCtrl)) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	if (ipiMask != pmPinMuxCtrl[pinId].owner) {
		status = XST_FAILURE;
		goto done;
	}

	pmPinMuxCtrl[pinId].owner = 0U;

done:
	return status;
}

/**
 * PmPinCtrlGetFunctionInt() - Get currently configured PIN function
 * @pinId	ID of the PIN
 * @fnId	Location to store function ID
 *
 * @return	XST_SUCCESS if function is get
 *		XST_INVALID_PARAM if provided argument is invalid
 *		XST_PM_INTERNAL if function cannot be mapped
 */
int PmPinCtrlGetFunctionInt(const u32 pinId, u32* const fnId)
{
	int status = XST_PM_INTERNAL;
	u32 reg, i;

	if (pinId >= ARRAY_SIZE(pmPinMuxCtrl)) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	reg = XPfw_Read32(IOU_SLCR_BASE + 4U * pinId);
	for (i = 0U; NULL != pmPinMuxCtrl[pinId].pinMux[i]; i++) {
		if (pmPinMuxCtrl[pinId].pinMux[i]->select == reg) {
			*fnId = pmPinMuxCtrl[pinId].pinMux[i]->fid;
			status = XST_SUCCESS;
			break;
		}
	};

done:
	return status;
}

/**
 * PmPinCtrlCheckPerms() - Check if master has permission to control the PIN
 * @ipiMask	IPI mask of the target master
 * @pinId	ID of the target PIN
 *
 * @return	XST_SUCCESS is master is allowed to control the PIN
 *		XST_INVALID_PARAM if pinId is invalid
 *		XST_PM_NO_ACCESS if master is no allowed to control the PIN
 */
int PmPinCtrlCheckPerms(const u32 ipiMask, const u32 pinId)
{
	int status = XST_SUCCESS;

	if (pinId >= ARRAY_SIZE(pmPinMuxCtrl)) {
		status = XST_INVALID_PARAM;
		goto done;
	}
	/* If the pin has not been previously requested return error */
	if (ipiMask != pmPinMuxCtrl[pinId].owner) {
		status = XST_PM_NO_ACCESS;
		goto done;
	}

done:
	return status;
}

/**
 * PmPinCtrlSetFunctionInt() - Set PIN function
 * @master	Master that attempts to set the PIN function
 * @pinId	ID of the PIN
 * @fnId	Function ID
 *
 * @return	XST_SUCCESS if function is get
 *		XST_INVALID_PARAM if provided argument is invalid
 *		XST_PM_INTERNAL if function cannot be mapped base on current
 *		configuration
 */
int PmPinCtrlSetFunctionInt(const PmMaster* const master, const u32 pinId,
			    const u32 fnId)
{
	int status = XST_INVALID_PARAM;
	u32 val, i, s;

	for (i = 0U; NULL != pmPinMuxCtrl[pinId].pinMux[i]; i++) {
		const PmPinMuxFn* const fn = pmPinMuxCtrl[pinId].pinMux[i];

		if (fn->fid != fnId) {
			continue;
		}

		/* Found function, now check if the master can set it */
		status = XST_SUCCESS;
		val = fn->select;
		for (s = 0U; s < fn->slavesCnt; s++) {
			PmRequirement* req = PmRequirementGet(master,
							      fn->slaves[s]);
			/*
			 * If there is not struct master is not allowed to use
			 * the slave that is associated with the target pin fn.
			 */
			if (NULL == req) {
				status = XST_PM_NO_ACCESS;
				break;
			}
		}
		break;
	};
	if (XST_SUCCESS == status) {
		XPfw_Write32(IOU_SLCR_BASE + 4U * pinId, val);
	}

done:
	return status;
}

/**
 * PmPinCtrlGetParam() - Get PIN configuration parameter value
 * @pinId	ID of the PIN
 * @paramId	ID of the PIN parameter
 * @value	Location to store the parameter value
 *
 * @return	XST_SUCCESS or
 *		XST_INVALID_PARAM if provided argument is invalid
 *
 * @note	See note in PmPinCtrlSetParam().
 */
int PmPinCtrlGetParam(const u32 pinId, const u32 paramId, u32* const value)
{
	int status = XST_SUCCESS;
	u32 addr, val, shift;

	if (paramId >= ARRAY_SIZE(pmPinParams)) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	shift = pinId % PM_PIN_PARAM_PER_REG;
	addr = PM_PIN_PARAM_GET_ADDR(pinId, pmPinParams[paramId].offset);
	val = XPfw_Read32(addr);

	/* Workaround the hardware bug in bank1_ctrl5 */
	if (IOU_SLCR_BANK1_CTRL5 == addr) {
		SWAP_BITS_BANK1_CTRL5(val);
	}

	*value = (val >> shift) & 0x1U;

	if (0U != (PM_PIN_PARAM_2_BITS & pmPinParams[paramId].flags)) {
		addr += 4U;
		val = XPfw_Read32(addr);
		val = (val >> shift) & 0x1U;
		*value = (*value << 1U) | val;
	}

done:
	return status;
}

/**
 * PmPinCtrlSetParam() - Set PIN configuration parameter value
 * @ipiMask	IPI mask of the master that initiated the request
 * @pinId	ID of the PIN
 * @paramId	ID of the PIN parameter
 * @value	Parameter value to be set
 *
 * @return	XST_INVALID_PARAM if an argument is not valid
 *		XST_SUCCESS otherwise
 *
 * @note	There is a hardware bug in bank1 ctrl5 - bits 25:14 are shifted
 *		by 14 places to the right (to bitfields 11:0), and bits 13:0 are
 *		shifted to the left by 12 places (to bitfields 25:12) by
 *		hardware. Since we want users to not be aware of this bug, the
 *		fix is added in the function below.
 *		E.g. writing 0x00803FFF bank1_ctrl5 will result in 0x03FFF200:
 * bits: 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
 * val:   0  0  1  0  0  0  0  0  0  0  0  0  1  1  1  1 1 1 1 1 1 1 1 1 1 1
 * val`:  1  1  1  1  1  1  1  1  1  1  1  1  1  1  0  0 1 0 0 0 0 0 0 0 0 0
 *
 * val is the written value, val` is the actual value that will be configured by
 * hardware. Therefore, if a user writes to configure pin 13, the hardware will
 * actually configure pin 25 (13 + 12). To get pin 13 configured we need to
 * write as if we're configuring pin 1 (13 - 12), so hardware will configure
 * bit 13 (1 + 12). If a user wants to configure pin 4, we need to write as if
 * we're configuring bit[18] to actually get the pin 4 configured.
 */
int PmPinCtrlSetParam(const u32 pinId, const u32 paramId, const u32 value)
{
	int status = XST_INVALID_PARAM;
	u32 addr, val, shift;

	if (0U != (PM_PIN_PARAM_RO & pmPinParams[paramId].flags)) {
		goto done;
	}
	if (0U != (PM_PIN_PARAM_2_BITS & pmPinParams[paramId].flags)) {
		if (value > 3U) {
			goto done;
		}
	} else {
		if (value > 1U) {
			goto done;
		}
	}

	status = XST_SUCCESS;
	shift = pinId % PM_PIN_PARAM_PER_REG;
	addr = PM_PIN_PARAM_GET_ADDR(pinId, pmPinParams[paramId].offset);

	/* Workaround the hardware bug in bank1_ctrl5 */
	if (IOU_SLCR_BANK1_CTRL5 == addr) {
		FIX_BANK1_CTRL5(shift);
	}

	if (0U == (PM_PIN_PARAM_2_BITS & pmPinParams[paramId].flags)) {
		XPfw_RMW32(addr, 1 << shift, value << shift);
		/* When setting pull up/down we need to enable pull as well */
		if (paramId == PINCTRL_CONFIG_PULL_CTRL) {
			addr = PM_PIN_PARAM_GET_ADDR(pinId,
				pmPinParams[PINCTRL_CONFIG_PULL_CTRL].offset);

			/* Workaround the hardware bug in bank1_ctrl5 */
			if (addr == IOU_SLCR_BANK1_CTRL5) {
				FIX_BANK1_CTRL5(shift);
			}
			XPfw_RMW32(addr, 1 << shift, value << shift);
		}
	} else {
		/* Write value[0] at address + 4 and value[1] at address */
		XPfw_RMW32(addr + 4U, 1 << shift, (value & 0x1U) << shift);
		XPfw_RMW32(addr, 1 << shift, ((value & 0x2U) >> 1U) << shift);
	}

done:
	return status;
}