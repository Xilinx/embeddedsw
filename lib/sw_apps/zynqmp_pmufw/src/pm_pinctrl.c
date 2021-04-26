/*
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */


#include "pm_common.h"
#include "pm_slave.h"
#include "pm_periph.h"
#include "pm_usb.h"
#include "pm_requirement.h"
#include "pm_pinctrl.h"

#define PINMUX_FN(name, fn, sel, do)	\
	{ \
		.select = (sel), \
		.fid = (fn), \
		PIN_##do \
	}

#define PIN_NULL	\
		.slaves = NULL, \
		.slavesCnt = 0U

#define PIN_BIND(c)	\
		.slaves = (c), \
		.slavesCnt = ARRAY_SIZE(c)

#define PINMUX(id)	\
	static u8 pinMux##id[]

#define PINMUX_REF(pinmux)	\
	{	\
		.pinMux = (pinmux),	\
		.pinMuxSize = ARRAY_SIZE(pinmux),	\
	}

#define FID(name)	PINCTRL_FUNC_##name

#define DEFINE_PIN(id)	\
	{	\
		.pinMuxArr = pinMux##id,	\
		.owner = 0U,	\
	}


#define PM_PIN_PARAM_RO		(1U << 0U)
#define PM_PIN_PARAM_2_BITS	(1U << 1U)
#define PM_PIN_PARAM_PER_BANK	(1U << 2U)

#define PM_PIN_PARAM_PER_REG	26U

#define IOU_SLCR_BANK0_CTRL0	(IOU_SLCR_BASE + 0x138U)
#define IOU_SLCR_BANK0_CTRL1	(IOU_SLCR_BASE + 0x154U)
#define PM_IOU_SLCR_BANK_OFFSET	(IOU_SLCR_BANK0_CTRL1 - IOU_SLCR_BANK0_CTRL0)

#define PM_PIN_PARAM_GET_ADDR(pinId, regOffset)	\
	(IOU_SLCR_BANK0_CTRL0 + \
	 (PM_IOU_SLCR_BANK_OFFSET * ((pinId) / PM_PIN_PARAM_PER_REG)) + (regOffset))

#define IOU_SLCR_BANK1_CTRL5	(IOU_SLCR_BASE + 164U)

#define FIX_BANK1_CTRL5(shift)	\
	(shift) = (((shift) < 12U) ? ((shift) + 14U) : ((shift) - 12U))

#define SWAP_BITS_BANK1_CTRL5(val)	\
	(val) = (((val) & 0x3FFFU) << 12U) | (((val) >> 14U) & 0xFFFU)

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
 * @pinMuxArr	Array of mux function ids
 * @owner	IPI mask of the master which requested PIN control
 */
typedef struct PmMioPin {
	u8* pinMuxArr;
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

static PmPinMuxFn PmPinMuxFunArr[] = {
	PINMUX_FN(Can0,		FID(CAN0),	0x20U,	BIND(pmCan0Slaves)),
	PINMUX_FN(Can1,		FID(CAN1),	0x20U,	BIND(pmCan1Slaves)),
	PINMUX_FN(Eth0,		FID(ETHERNET0),	0x02U,	BIND(pmEth0Slaves)),
	PINMUX_FN(Eth1,		FID(ETHERNET1),	0x02U,	BIND(pmEth1Slaves)),
	PINMUX_FN(Eth2,		FID(ETHERNET2),	0x02U,	BIND(pmEth2Slaves)),
	PINMUX_FN(Eth3,		FID(ETHERNET3),	0x02U,	BIND(pmEth3Slaves)),
	PINMUX_FN(GemTsu,	FID(GEMTSU0),	0x02U,	BIND(pmGemTsuSlaves)),
	PINMUX_FN(Gpio,		FID(GPIO0),	0x00U,	BIND(pmGpioSlaves)),
	PINMUX_FN(I2C0,		FID(I2C0),	0x40U,	BIND(pmI2C0Slaves)),
	PINMUX_FN(I2C1,		FID(I2C1),	0x40U,	BIND(pmI2C1Slaves)),
	PINMUX_FN(Mdio0,	FID(MDIO0),	0x60U,	BIND(pmEth0Slaves)),
	PINMUX_FN(Mdio1,	FID(MDIO1),	0x80U,	BIND(pmEth1Slaves)),
	PINMUX_FN(Mdio2,	FID(MDIO2),	0xA0U,	BIND(pmEth2Slaves)),
	PINMUX_FN(Mdio3,	FID(MDIO3),	0xC0U,	BIND(pmEth3Slaves)),
	PINMUX_FN(QSpi,		FID(QSPI0),	0x02U,	BIND(pmQSpiSlaves)),
	PINMUX_FN(QSpiFbClk,	FID(QSPI_FBCLK),0x02U,	BIND(pmQSpiSlaves)),
	PINMUX_FN(QSpiSS,	FID(QSPI_SS),	0x02U,	BIND(pmQSpiSlaves)),
	PINMUX_FN(Spi0,		FID(SPI0),	0x80U,	BIND(pmSpi0Slaves)),
	PINMUX_FN(Spi1,		FID(SPI1),	0x80U,	BIND(pmSpi1Slaves)),
	PINMUX_FN(Spi0SS,	FID(SPI0_SS),	0x80U,	BIND(pmSpi0Slaves)),
	PINMUX_FN(Spi1SS,	FID(SPI1_SS),	0x80U,	BIND(pmSpi1Slaves)),
	PINMUX_FN(Sdio0,	FID(SDIO0),	0x08U,	BIND(pmSD0Slaves)),
	PINMUX_FN(Sdio0Pc,	FID(SDIO0_PC),	0x08U,	BIND(pmSD0Slaves)),
	PINMUX_FN(Sdio0Cd,	FID(SDIO0_CD),	0x08U,	BIND(pmSD0Slaves)),
	PINMUX_FN(Sdio0Wp,	FID(SDIO0_WP),	0x08U,	BIND(pmSD0Slaves)),
	PINMUX_FN(Sdio1,	FID(SDIO1),	0x10U,	BIND(pmSD1Slaves)),
	PINMUX_FN(Sdio1Pc,	FID(SDIO1_PC),	0x10U,	BIND(pmSD1Slaves)),
	PINMUX_FN(Sdio1Cd,	FID(SDIO1_CD),	0x10U,	BIND(pmSD1Slaves)),
	PINMUX_FN(Sdio1Wp,	FID(SDIO1_WP),	0x10U,	BIND(pmSD1Slaves)),
	PINMUX_FN(Nand,		FID(NAND0),	0x04U,	BIND(pmNandSlaves)),
	PINMUX_FN(NandCe,	FID(NAND0_CE),	0x04U,	BIND(pmNandSlaves)),
	PINMUX_FN(NandRb,	FID(NAND0_RB),	0x04U,	BIND(pmNandSlaves)),
	PINMUX_FN(NandDqs,	FID(NAND0_DQS),	0x04U,	BIND(pmNandSlaves)),
	PINMUX_FN(Ttc0Clk,	FID(TTC0_CLK),	0xA0U,	BIND(pmTtc0Slaves)),
	PINMUX_FN(Ttc0Wav,	FID(TTC0_WAV),	0xA0U,	BIND(pmTtc0Slaves)),
	PINMUX_FN(Ttc1Clk,	FID(TTC1_CLK),	0xA0U,	BIND(pmTtc1Slaves)),
	PINMUX_FN(Ttc1Wav,	FID(TTC1_WAV),	0xA0U,	BIND(pmTtc1Slaves)),
	PINMUX_FN(Ttc2Clk,	FID(TTC2_CLK),	0xA0U,	BIND(pmTtc2Slaves)),
	PINMUX_FN(Ttc2Wav,	FID(TTC2_WAV),	0xA0U,	BIND(pmTtc2Slaves)),
	PINMUX_FN(Ttc3Clk,	FID(TTC3_CLK),	0xA0U,	BIND(pmTtc3Slaves)),
	PINMUX_FN(Ttc3Wav,	FID(TTC3_WAV),	0xA0U,	BIND(pmTtc3Slaves)),
	PINMUX_FN(Uart0,	FID(UART0),	0xC0U,	BIND(pmUart0Slaves)),
	PINMUX_FN(Uart1,	FID(UART1),	0xC0U,	BIND(pmUart1Slaves)),
	PINMUX_FN(Usb0,		FID(USB0),	0x04U,	BIND(pmUsb0Slaves)),
	PINMUX_FN(Usb1,		FID(USB1),	0x04U,	BIND(pmUsb1Slaves)),
	PINMUX_FN(Swdt0Clk,	FID(SWDT0_CLK),	0x60U,	NULL),
	PINMUX_FN(Swdt0Rst,	FID(SWDT0_RST),	0x60U,	NULL),
	PINMUX_FN(Swdt1Clk,	FID(SWDT1_CLK),	0x60U,	BIND(pmSwdt1Slaves)),
	PINMUX_FN(Swdt1Rst,	FID(SWDT1_RST),	0x60U,	BIND(pmSwdt1Slaves)),
	PINMUX_FN(Pmu,		FID(PMU0),	0x08U,	NULL),
	PINMUX_FN(Pcie,		FID(PCIE0),	0x04U,	BIND(pmPcieSlaves)),
	PINMUX_FN(Csu,		FID(CSU0),	0x18U,	NULL),
	PINMUX_FN(Dp,		FID(DPAUX0),	0x18U,	BIND(pmDPSlaves)),
	PINMUX_FN(PJtag,	FID(PJTAG0),	0x60U,	NULL),
	PINMUX_FN(Trace,	FID(TRACE0),	0xE0U,	NULL),
	PINMUX_FN(TraceClk,	FID(TRACE0_CLK),0xE0U,	NULL),
	PINMUX_FN(TestScan,	FID(TESTSCAN0),	0x10U,	NULL),
};

/*
 * Mux select data is defined as follows:
 *
 *	L0_SEL[0],	L0_SEL[1],
 *	L1_SEL[0],	L1_SEL[1],
 *	L2_SEL[0],	L2_SEL[1],	L2_SEL[2],	L2_SEL[3],
 *	L3_SEL[0],	L3_SEL[1],	L3_SEL[2],	L3_SEL[3],
 *	L3_SEL[4],	L3_SEL[5],	L3_SEL[6],	L3_SEL[7],
 *	MAX_FUNCTION
 *
 * Note: L0_SEL[0], L1_SEL[0], and L2_SEL[0] are always reserved. If an element
 * in a pattern (matrix) above is missing an empty place is preserved to be able
 * to easily compare definitions with spec.
 */
PINMUX(0) = {		FID(QSPI0),

					FID(TESTSCAN0),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(PJTAG0),
	FID(SPI0),	FID(TTC3_CLK),	FID(UART1),	FID(TRACE0_CLK),
	MAX_FUNCTION
};
PINMUX(1) = {		FID(QSPI0),

					FID(TESTSCAN0),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(PJTAG0),
	FID(SPI0_SS),	FID(TTC3_WAV),	FID(UART1),	FID(TRACE0_CLK),
	MAX_FUNCTION
};
PINMUX(2) = {		FID(QSPI0),

					FID(TESTSCAN0),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(PJTAG0),
	FID(SPI0_SS),	FID(TTC2_CLK),	FID(UART0),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(3) = {		FID(QSPI0),

					FID(TESTSCAN0),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(PJTAG0),
	FID(SPI0_SS),	FID(TTC2_WAV),	FID(UART0),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(4) = {		FID(QSPI0),

					FID(TESTSCAN0),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(SWDT1_CLK),
	FID(SPI0),	FID(TTC1_CLK),	FID(UART1),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(5) = {		FID(QSPI_SS),

					FID(TESTSCAN0),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(SWDT1_RST),
	FID(SPI0),	FID(TTC1_WAV),	FID(UART1),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(6) = {		FID(QSPI_FBCLK),

					FID(TESTSCAN0),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(SWDT0_CLK),
	FID(SPI1),	FID(TTC0_CLK),	FID(UART0),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(7) = {		FID(QSPI_SS),

					FID(TESTSCAN0),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(SWDT0_RST),
	FID(SPI1_SS),	FID(TTC0_WAV),	FID(UART0),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(8) = {		FID(QSPI0),

					FID(TESTSCAN0),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(SWDT1_CLK),
	FID(SPI1_SS),	FID(TTC3_CLK),	FID(UART1),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(9) = {		FID(QSPI0),
			FID(NAND0_CE),
					FID(TESTSCAN0),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(SWDT1_RST),
	FID(SPI1_SS),	FID(TTC3_WAV),	FID(UART1),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(10) = {		FID(QSPI0),
			FID(NAND0_RB),
					FID(TESTSCAN0),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(SWDT0_CLK),
	FID(SPI1),	FID(TTC2_CLK),	FID(UART0),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(11) = {		FID(QSPI0),
			FID(NAND0_RB),
					FID(TESTSCAN0),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(SWDT0_RST),
	FID(SPI1),	FID(TTC2_WAV),	FID(UART0),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(12) = {		FID(QSPI0),
			FID(NAND0_DQS),
					FID(TESTSCAN0),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(PJTAG0),
	FID(SPI0),	FID(TTC1_CLK),	FID(UART1),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(13) = {
			FID(NAND0),
			FID(SDIO0),	FID(TESTSCAN0),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(PJTAG0),
	FID(SPI0_SS),	FID(TTC1_WAV),	FID(UART1),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(14) = {
			FID(NAND0),
			FID(SDIO0),	FID(TESTSCAN0),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(PJTAG0),
	FID(SPI0_SS),	FID(TTC0_CLK),	FID(UART0),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(15) = {
			FID(NAND0),
			FID(SDIO0),	FID(TESTSCAN0),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(PJTAG0),
	FID(SPI0_SS),	FID(TTC0_WAV),	FID(UART0),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(16) = {
			FID(NAND0),
			FID(SDIO0),	FID(TESTSCAN0),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(SWDT1_CLK),
	FID(SPI0),	FID(TTC3_CLK),	FID(UART1),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(17) = {
			FID(NAND0),
			FID(SDIO0),	FID(TESTSCAN0),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(SWDT1_RST),
	FID(SPI0),	FID(TTC3_WAV),	FID(UART1),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(18) = {
			FID(NAND0),
			FID(SDIO0),	FID(TESTSCAN0),	FID(CSU0),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(SWDT0_CLK),
	FID(SPI1),	FID(TTC2_CLK),	FID(UART0),
	MAX_FUNCTION
};
PINMUX(19) = {
			FID(NAND0),
			FID(SDIO0),	FID(TESTSCAN0),	FID(CSU0),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(SWDT0_RST),
	FID(SPI1_SS),	FID(TTC2_WAV),	FID(UART0),
	MAX_FUNCTION
};
PINMUX(20) = {
			FID(NAND0),
			FID(SDIO0),	FID(TESTSCAN0),	FID(CSU0),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(SWDT1_CLK),
	FID(SPI1_SS),	FID(TTC1_CLK),	FID(UART1),
	MAX_FUNCTION
};
PINMUX(21) = {
			FID(NAND0),
			FID(SDIO0),	FID(TESTSCAN0),	FID(CSU0),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(SWDT1_RST),
	FID(SPI1_SS),	FID(TTC1_WAV),	FID(UART1),
	MAX_FUNCTION
};
PINMUX(22) = {
			FID(NAND0),
			FID(SDIO0),	FID(TESTSCAN0),	FID(CSU0),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(SWDT0_CLK),
	FID(SPI1),	FID(TTC0_CLK),	FID(UART0),
	MAX_FUNCTION
};
PINMUX(23) = {
			FID(NAND0),
			FID(SDIO0_PC),	FID(TESTSCAN0),	FID(CSU0),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(SWDT0_RST),
	FID(SPI1),	FID(TTC0_WAV),	FID(UART0),
	MAX_FUNCTION
};
PINMUX(24) = {
			FID(NAND0),
			FID(SDIO0_CD),	FID(TESTSCAN0),	FID(CSU0),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(SWDT1_CLK),
			FID(TTC3_CLK),	FID(UART1),
	MAX_FUNCTION
};
PINMUX(25) = {
			FID(NAND0),
			FID(SDIO0_WP),	FID(TESTSCAN0),	FID(CSU0),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(SWDT1_RST),
			FID(TTC3_WAV),	FID(UART1),
	MAX_FUNCTION
};
PINMUX(26) = {		FID(ETHERNET0),
			FID(NAND0_CE),
			FID(PMU0),	FID(TESTSCAN0),	FID(CSU0),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(PJTAG0),
	FID(SPI0),	FID(TTC2_CLK),	FID(UART0),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(27) = {		FID(ETHERNET0),
			FID(NAND0_RB),
			FID(PMU0),	FID(TESTSCAN0),	FID(DPAUX0),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(PJTAG0),
	FID(SPI0_SS),	FID(TTC2_WAV),	FID(UART0),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(28) = {		FID(ETHERNET0),
			FID(NAND0_RB),
			FID(PMU0),	FID(TESTSCAN0),	FID(DPAUX0),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(PJTAG0),
	FID(SPI0_SS),	FID(TTC1_CLK),	FID(UART1),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(29) = {		FID(ETHERNET0),
			FID(PCIE0),
			FID(PMU0),	FID(TESTSCAN0),	FID(DPAUX0),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(PJTAG0),
	FID(SPI0_SS),	FID(TTC1_WAV),	FID(UART1),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(30) = {		FID(ETHERNET0),
			FID(PCIE0),
			FID(PMU0),	FID(TESTSCAN0),	FID(DPAUX0),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(SWDT0_CLK),
	FID(SPI0),	FID(TTC0_CLK),	FID(UART0),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(31) = {		FID(ETHERNET0),
			FID(PCIE0),
			FID(PMU0),	FID(TESTSCAN0),	FID(CSU0),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(SWDT0_RST),
	FID(SPI0),	FID(TTC0_WAV),	FID(UART0),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(32) = {		FID(ETHERNET0),
			FID(NAND0_DQS),
			FID(PMU0),	FID(TESTSCAN0),	FID(CSU0),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(SWDT1_CLK),
	FID(SPI1),	FID(TTC3_CLK),	FID(UART1),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(33) = {		FID(ETHERNET0),
			FID(PCIE0),
			FID(PMU0),	FID(TESTSCAN0),	FID(CSU0),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(SWDT1_RST),
	FID(SPI1_SS),	FID(TTC3_WAV),	FID(UART1),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(34) = {		FID(ETHERNET0),
			FID(PCIE0),
			FID(PMU0),	FID(TESTSCAN0),	FID(DPAUX0),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(SWDT0_CLK),
	FID(SPI1_SS),	FID(TTC2_CLK),	FID(UART0),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(35) = {		FID(ETHERNET0),
			FID(PCIE0),
			FID(PMU0),	FID(TESTSCAN0),	FID(DPAUX0),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(SWDT0_RST),
	FID(SPI1_SS),	FID(TTC2_WAV),	FID(UART0),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(36) = {		FID(ETHERNET0),
			FID(PCIE0),
			FID(PMU0),	FID(TESTSCAN0),	FID(DPAUX0),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(SWDT1_CLK),
	FID(SPI1),	FID(TTC1_CLK),	FID(UART1),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(37) = {		FID(ETHERNET0),
			FID(PCIE0),
			FID(PMU0),	FID(TESTSCAN0),	FID(DPAUX0),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(SWDT1_RST),
	FID(SPI1),	FID(TTC1_WAV),	FID(UART1),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(38) = {		FID(ETHERNET1),

			FID(SDIO0),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(PJTAG0),
	FID(SPI0),	FID(TTC0_CLK),	FID(UART0),	FID(TRACE0_CLK),
	MAX_FUNCTION
};
PINMUX(39) = {		FID(ETHERNET1),

			FID(SDIO0_CD),	FID(SDIO1),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(PJTAG0),
	FID(SPI0_SS),	FID(TTC0_WAV),	FID(UART0),	FID(TRACE0_CLK),
	MAX_FUNCTION
};
PINMUX(40) = {		FID(ETHERNET1),

			FID(SDIO0),	FID(SDIO1),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(PJTAG0),
	FID(SPI0_SS),	FID(TTC3_CLK),	FID(UART1),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(41) = {		FID(ETHERNET1),

			FID(SDIO0),	FID(SDIO1),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(PJTAG0),
	FID(SPI0_SS),	FID(TTC3_WAV),	FID(UART1),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(42) = {		FID(ETHERNET1),

			FID(SDIO0),	FID(SDIO1),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(SWDT0_CLK),
	FID(SPI0),	FID(TTC2_CLK),	FID(UART0),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(43) = {		FID(ETHERNET1),

			FID(SDIO0),	FID(SDIO1_PC),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(SWDT0_RST),
	FID(SPI0),	FID(TTC2_WAV),	FID(UART0),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(44) = {		FID(ETHERNET1),

			FID(SDIO0),	FID(SDIO1_WP),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(SWDT1_CLK),
	FID(SPI1),	FID(TTC1_CLK),	FID(UART1),
	MAX_FUNCTION
};
PINMUX(45) = {		FID(ETHERNET1),

			FID(SDIO0),	FID(SDIO1_CD),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(SWDT1_RST),
	FID(SPI1_SS),	FID(TTC1_WAV),	FID(UART1),
	MAX_FUNCTION
};
PINMUX(46) = {		FID(ETHERNET1),

			FID(SDIO0),	FID(SDIO1),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(SWDT0_CLK),
	FID(SPI1_SS),	FID(TTC0_CLK),	FID(UART0),
	MAX_FUNCTION
};
PINMUX(47) = {		FID(ETHERNET1),

			FID(SDIO0),	FID(SDIO1),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(SWDT0_RST),
	FID(SPI1_SS),	FID(TTC0_WAV),	FID(UART0),
	MAX_FUNCTION
};
PINMUX(48) = {		FID(ETHERNET1),

			FID(SDIO0),	FID(SDIO1),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(SWDT1_CLK),
	FID(SPI1),	FID(TTC3_CLK),	FID(UART1),
	MAX_FUNCTION
};
PINMUX(49) = {		FID(ETHERNET1),

			FID(SDIO0_PC),	FID(SDIO1),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(SWDT1_RST),
	FID(SPI1),	FID(TTC3_WAV),	FID(UART1),
	MAX_FUNCTION
};
PINMUX(50) = {		FID(GEMTSU0),

			FID(SDIO0_WP),	FID(SDIO1),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(SWDT0_CLK),
	FID(MDIO1),	FID(TTC2_CLK),	FID(UART0),
	MAX_FUNCTION
};
PINMUX(51) = {		FID(GEMTSU0),

			FID(SDIO0_WP),	FID(SDIO1),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(SWDT0_RST),
	FID(MDIO1),	FID(TTC2_WAV),	FID(UART0),
	MAX_FUNCTION
};
PINMUX(52) = {		FID(ETHERNET2),
			FID(USB0),

	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(PJTAG0),
	FID(SPI0),	FID(TTC1_CLK),	FID(UART1),	FID(TRACE0_CLK),
	MAX_FUNCTION
};
PINMUX(53) = {		FID(ETHERNET2),
			FID(USB0),

	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(PJTAG0),
	FID(SPI0_SS),	FID(TTC1_WAV),	FID(UART1),	FID(TRACE0_CLK),
	MAX_FUNCTION
};
PINMUX(54) = {		FID(ETHERNET2),
			FID(USB0),

	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(PJTAG0),
	FID(SPI0_SS),	FID(TTC0_CLK),	FID(UART0),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(55) = {		FID(ETHERNET2),
			FID(USB0),

	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(PJTAG0),
	FID(SPI0_SS),	FID(TTC0_WAV),	FID(UART0),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(56) = {		FID(ETHERNET2),
			FID(USB0),

	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(SWDT1_CLK),
	FID(SPI0),	FID(TTC3_CLK),	FID(UART1),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(57) = {		FID(ETHERNET2),
			FID(USB0),

	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(SWDT1_RST),
	FID(SPI0),	FID(TTC3_WAV),	FID(UART1),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(58) = {		FID(ETHERNET2),
			FID(USB0),

	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(PJTAG0),
	FID(SPI1),	FID(TTC2_CLK),	FID(UART0),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(59) = {		FID(ETHERNET2),
			FID(USB0),

	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(PJTAG0),
	FID(SPI1_SS),	FID(TTC2_WAV),	FID(UART0),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(60) = {		FID(ETHERNET2),
			FID(USB0),

	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(PJTAG0),
	FID(SPI1_SS),	FID(TTC1_CLK),	FID(UART1),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(61) = {		FID(ETHERNET2),
			FID(USB0),

	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(PJTAG0),
	FID(SPI1_SS),	FID(TTC1_WAV),	FID(UART1),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(62) = {		FID(ETHERNET2),
			FID(USB0),

	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(SWDT0_CLK),
	FID(SPI1),	FID(TTC0_CLK),	FID(UART0),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(63) = {		FID(ETHERNET2),
			FID(USB0),

	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(SWDT0_RST),
	FID(SPI1),	FID(TTC0_WAV),	FID(UART0),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(64) = {		FID(ETHERNET3),
			FID(USB1),
			FID(SDIO0),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(SWDT1_CLK),
	FID(SPI0),	FID(TTC3_CLK),	FID(UART1),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(65) = {		FID(ETHERNET3),
			FID(USB1),
			FID(SDIO0_CD),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(SWDT1_RST),
	FID(SPI0_SS),	FID(TTC3_WAV),	FID(UART1),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(66) = {		FID(ETHERNET3),
			FID(USB1),
			FID(SDIO0),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(SWDT0_CLK),
	FID(SPI0_SS),	FID(TTC2_CLK),	FID(UART0),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(67) = {		FID(ETHERNET3),
			FID(USB1),
			FID(SDIO0),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(SWDT0_RST),
	FID(SPI0_SS),	FID(TTC2_WAV),	FID(UART0),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(68) = {		FID(ETHERNET3),
			FID(USB1),
			FID(SDIO0),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(SWDT1_CLK),
	FID(SPI0),	FID(TTC1_CLK),	FID(UART1),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(69) = {		FID(ETHERNET3),
			FID(USB1),
			FID(SDIO0),	FID(SDIO1_WP),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(SWDT1_RST),
	FID(SPI0),	FID(TTC1_WAV),	FID(UART1),	FID(TRACE0),
	MAX_FUNCTION
};
PINMUX(70) = {		FID(ETHERNET3),
			FID(USB1),
			FID(SDIO0),	FID(SDIO1_PC),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(SWDT0_CLK),
	FID(SPI1),	FID(TTC0_CLK),	FID(UART0),
	MAX_FUNCTION
};
PINMUX(71) = {		FID(ETHERNET3),
			FID(USB1),
			FID(SDIO0),	FID(SDIO1),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(SWDT0_RST),
	FID(SPI1_SS),	FID(TTC0_WAV),	FID(UART0),
	MAX_FUNCTION
};
PINMUX(72) = {		FID(ETHERNET3),
			FID(USB1),
			FID(SDIO0),	FID(SDIO1),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(SWDT1_CLK),
	FID(SPI1_SS),			FID(UART1),
	MAX_FUNCTION
};
PINMUX(73) = {		FID(ETHERNET3),
			FID(USB1),
			FID(SDIO0),	FID(SDIO1),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(SWDT1_RST),
	FID(SPI1_SS),			FID(UART1),
	MAX_FUNCTION
};
PINMUX(74) = {		FID(ETHERNET3),
			FID(USB1),
			FID(SDIO0),	FID(SDIO1),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(SWDT0_CLK),
	FID(SPI1),			FID(UART0),
	MAX_FUNCTION
};
PINMUX(75) = {		FID(ETHERNET3),
			FID(USB1),
			FID(SDIO0_PC),	FID(SDIO1),
	FID(GPIO0),	FID(CAN0),	FID(I2C0),	FID(SWDT0_RST),
	FID(SPI1),			FID(UART0),
	MAX_FUNCTION
};
PINMUX(76) = {

			FID(SDIO0_WP),	FID(SDIO1),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(MDIO0),
	FID(MDIO1),	FID(MDIO2),	FID(MDIO3),
	MAX_FUNCTION
};
PINMUX(77) = {

					FID(SDIO1_CD),
	FID(GPIO0),	FID(CAN1),	FID(I2C1),	FID(MDIO0),
	FID(MDIO1),	FID(MDIO2),	FID(MDIO3),
	MAX_FUNCTION
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

static PmPinParam pmPinParams[PINCTRL_MAX_CONFIG] = {
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
		.flags = PM_PIN_PARAM_RO | PM_PIN_PARAM_PER_BANK,
	},
};

/**
 * PmPinCtrlRequestInt() - Request PIN control
 * @ipiMask	IPI mask of the master
 * @pinId	ID of the pin in question
 *
 * @return	XST_SUCCESS if successfully requested
 *		XST_INVALID_PARAM if pinId argument is not valid
 *		XST_PM_NO_ACCESS if PIN control is already requested by another
 *		master
 */
s32 PmPinCtrlRequestInt(const u32 ipiMask, const u32 pinId)
{
	s32 status = XST_SUCCESS;

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
 * @return	XST_SUCCESS if successfully released
 *		XST_INVALID_PARAM if pinId argument is not valid
 *		XST_FAILURE if PIN control has not been previously requested
 */
s32 PmPinCtrlReleaseInt(const u32 ipiMask, const u32 pinId)
{
	s32 status = XST_SUCCESS;

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
s32 PmPinCtrlGetFunctionInt(const u32 pinId, u32* const fnId)
{
	s32 status = XST_PM_INTERNAL;
	u32 reg, i;
	u8 *pinMuxArr;

	if (pinId >= ARRAY_SIZE(pmPinMuxCtrl)) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	reg = XPfw_Read32(IOU_SLCR_BASE + (4U * pinId));
	pinMuxArr = pmPinMuxCtrl[pinId].pinMuxArr;

	for (i = 0U; pinMuxArr[i] != MAX_FUNCTION; i++) {
		if (PmPinMuxFunArr[pinMuxArr[i]].select == reg) {
			*fnId = PmPinMuxFunArr[pinMuxArr[i]].fid;
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
s32 PmPinCtrlCheckPerms(const u32 ipiMask, const u32 pinId)
{
	s32 status = XST_SUCCESS;

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
s32 PmPinCtrlSetFunctionInt(const PmMaster* const master, const u32 pinId,
			    const u32 fnId)
{
	s32 status = XST_INVALID_PARAM;
	u32 val = 0U;
	u32 i;
	u32 s;
	u8 *pinMuxArr = pmPinMuxCtrl[pinId].pinMuxArr;

	for (i = 0U; pinMuxArr[i] != MAX_FUNCTION; i++) {
		PmPinMuxFn *fn = &PmPinMuxFunArr[pinMuxArr[i]];

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
		XPfw_Write32(IOU_SLCR_BASE + (4U * pinId), val);
	}

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
s32 PmPinCtrlGetParam(const u32 pinId, const u32 paramId, u32* const value)
{
	s32 status = XST_SUCCESS;
	u32 addr, val, shift;

	if ((paramId >= ARRAY_SIZE(pmPinParams)) ||
	    (pinId >= ARRAY_SIZE(pmPinMuxCtrl))) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	if (0U == (PM_PIN_PARAM_PER_BANK & pmPinParams[paramId].flags)) {
		shift = pinId % PM_PIN_PARAM_PER_REG;
	} else {
		shift = 0;
	}
	addr = PM_PIN_PARAM_GET_ADDR(pinId, pmPinParams[paramId].offset);
	val = XPfw_Read32(addr);

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
 * @note	In bank1 ctrl5, pins are not mapped with sequential order.
 *		Pin 38 to 51 are mapped with BIT[0:13], Pin 26 to 37 are mapped
 *		with BIT[14:25].
 */
s32 PmPinCtrlSetParam(const u32 pinId, const u32 paramId, const u32 value)
{
	s32 status = XST_INVALID_PARAM;
	u32 addr, shift;

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
	if (0U == (PM_PIN_PARAM_PER_BANK & pmPinParams[paramId].flags)) {
		shift = pinId % PM_PIN_PARAM_PER_REG;
	} else {
		shift = 0;
	}
	addr = PM_PIN_PARAM_GET_ADDR(pinId, pmPinParams[paramId].offset);

	if (IOU_SLCR_BANK1_CTRL5 == addr) {
		FIX_BANK1_CTRL5(shift);
	}

	if (0U == (PM_PIN_PARAM_2_BITS & pmPinParams[paramId].flags)) {
		XPfw_RMW32(addr, (u32)1 << shift, (u32)value << shift);
		/* When setting pull up/down we need to enable pull as well */
		if (paramId == PINCTRL_CONFIG_PULL_CTRL) {
			addr = PM_PIN_PARAM_GET_ADDR(pinId,
				pmPinParams[PINCTRL_CONFIG_PULL_CTRL].offset);

			if (addr == IOU_SLCR_BANK1_CTRL5) {
				FIX_BANK1_CTRL5(shift);
			}
			XPfw_RMW32(addr, (u32)1 << shift, (u32)value << shift);
		}
	} else {
		/* Write value[0] at address + 4 and value[1] at address */
		XPfw_RMW32(addr + 4U, (u32)1 << shift, (value & 0x1U) << shift);
		XPfw_RMW32(addr, (u32)1 << shift, ((value & 0x2U) >> 1U) << shift);
	}

done:
	return status;
}
