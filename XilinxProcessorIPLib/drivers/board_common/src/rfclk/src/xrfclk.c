/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xrfclk.c
* @addtogroup xrfclk_v1_3
* @{
*
* Contains the API of the XRFclk middleware.
* See xrfclk.h for a detailed description of the device and driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     07/21/19 Initial version
* 1.1   dc     11/21/19 Remove xil dependencies from linux build
*       dc     11/25/19 update LMX and LMK configs
*       dc     12/05/19 adjust LMX and LMK configs to a rftool needs
* 1.2   dc     22/01/20 add version and list of LMK frequencies
*       dc     03/05/20 add protection for shared i2c1 MUX
* 1.3   dc     03/10/20 update LMK/LMX config for MTS
* </pre>
*
******************************************************************************/

#include "xrfclk.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifdef __BAREMETAL__

#include "xil_assert.h"
#include "xil_types.h"
#include "xstatus.h"
#include "platform.h"
#include "xil_printf.h"
#include "xiicps.h"
#define PRINTF xil_printf /* Print macro for BM */
XIicPs Iic1;
#define FD_BRIDGE (&Iic1) /* I2C1 driver descriptor address */
#define FD_I2C1 (&Iic1) /* I2C1 driver descriptor address */

/* MUX Selext GPIO definitions */
#ifdef XPS_BOARD_ZCU111
XIicPs Iic0;
#define FD_I2C0 (&Iic0) /* I2C0 driver descriptor address */
#endif

#define I2C_SLEEP_US 1000U /* I2C sleep period */

#else /* LINUX build */

/* User Headers */
#include <errno.h>
#include <assert.h>

/* Kernel Headers */
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#define Xil_AssertNonvoid(a) assert(a) /* use linux assert */
#define PRINTF printf /* Print macro for Linux driver */
#define RFCLK_I2C1_DEVICE_PATH "/dev/i2c-1" /* i2c1 device path */
#define RFCLK_I2C_I2C2SPI_BRIDGE_DEVICE_PATH                                   \
	"/dev/i2c-20" /* i2c2spi bridge device */
int fd_i2c1;
int fd_bridge;
#define FD_BRIDGE fd_bridge /* Bridge driver descriptor alias */
#define FD_I2C1 fd_i2c1 /* I2C1 driver descriptor alias */

/* MUX Selext GPIO definitions */
#ifdef XPS_BOARD_ZCU111
#define RFCLK_I2C0_DEVICE_PATH "/dev/i2c-0" /* i2c0 device path */
int fd_i2c0;
#define FD_I2C0 fd_i2c0 /* I2C0 driver file descriptor alias */
#else
char GPIO_MUX_SEL0[12]; /* gpio MUX_SEL_0 */
char GPIO_MUX_SEL1[12]; /* gpio MUX_SEL_1 */
#endif

#endif

/* logging macro */
#define LOG(str)                                                               \
	PRINTF("RFCLK error in %s: %s", __func__, str) /* Logging macro */

#define SELECT_SPI_SDO(X) (1 << (3 - X)) /* Value which routs MUX */
#define RF_DATA_READ_BIT 0X80 /* Bit which indicates read */
#define LMX_RESET_VAL 2 /* Reset value for LMX */
#ifdef XPS_BOARD_ZCU111
#define LMK_RESET_VAL 0x20000 /* Reset value for LMK04208 */
#define I2C_IO_EXPANDER_CONFIG_REG_ADDR 0X07 /* Config register address */
#define I2C_IO_EXPANDER_GPIO_OUTPUT_REG_ADDR 0X03 /* Output GPIO address */
#else
#define LMK_RESET_VAL 0X80 /* Reset value for LMK04828 */
#endif

#define LMK_ADDRESS_STEP 8 /* Address step for different port */
#define LMK_PORT_ID_MAX 7 /* Number of output ports */
#define LMK_PORT_STATE_MAX 0xff /* Number of states on output port */
#define LMK_DCLKOUTX_DIV_MAX 31 /* Max CLKOUTX divider value */
#define LMK_DCLKOUTX_MUX_MAX 3 /* Max CLKOUTX MUX value */
#define LMK_SDCLKOUTY_MUX_MAX 1 /* Max CLKOUTY MUX value */
#define LMK_SYSREF_DIV_MIN 8 /* Min SYSREF divider value */
#define LMK_SYSREF_DIV_MAX 8191 /* Max SYSREF divider value */

#define LMX_ADDRESS_SHIFT 16 /* LMX address shift value */
#define LMX_ADDRESS_BITFIELD 0X7F /* LMX address bitfield */
#define LMK_ADDRESS_SHIFT 8 /* LMK address shift value */
#define LMK_ADDRESS_BITFIELD 0X1FFF /* LMK address bitfield */

#define I2C_ADDR_BUS_SWITCH 0X74 /* Bus switch i2c address */
#define I2C_ADDR_I2C2SPI_BRIDGE 0X2F /* I2C2SPI bridge i2c address */
#define I2C_ADDR_I2C_IO_EXPANDER 0X20 /* I2C io expander i2c address */
#define I2C_SWITCH_SELECT_I2C2SPI_BRIDGE (1 << 5) /* Switch value for bridge */
#define I2C_MUX_SEL_0 (1 << 1) /* MUX_SEL0 GPIO bit */
#define I2C_MUX_SEL_1 (1 << 2) /* MUX_SEL1 GPIO bit */
#define NUM_IIC_RETRIES (5) /* Number of IIC retries */
#define DELAY_100uS (100) /* Number for 1ms delay */

#define LMX_MUXOUT_REG_ADDR 0 /* LMX MUXOUT reg. address */
#define LMX_MUXOUT_REG_VAL 0 /* LMX MUXOUT reg. value */
#define LMK_MUXOUT_REG_ADDR 0X15F /* LMK MUXOUT reg. address */
#define LMK_MUXOUT_REG_VAL 0X3B /* LMK MUXOUT reg. value */
#define MUXOUT_LD_SEL_BIT 4 /* MUXOUT select bit */
#define LMK_CFG_OUTPUT_PORT0_ADDR 0X107 /* LMK config port0 address */
#define LMK_DCLKOUTX_DIV_PORT0_ADDR 0X100 /* CLKOUTX port0 divider address */
#define LMK_DCLKOUTX_MUX_PORT0_ADDR 0X103 /* CLKOUTX port 0 MUX address */
#define LMK_SDCLKOUTY_MUX_PORT0_ADDR 0X104 /* CLKOUTY port 0 MUX address */
#define LMK_SYSREF_DIV_MSB_PORT0_ADDR 0X13A /* MSB divider port 0 address */
#define LMK_SYSREF_DIV_LSB_PORT0_ADDR 0X13B /* LSB divider port 0 address */

u32 MuxOutRegStorage[RFCLK_CHIP_NUM];

#ifdef XPS_BOARD_ZCU111
static int XRFClk_I2cIoExpanderConfig();
#else
#ifdef __BAREMETAL__
#define GPIO_DEVICE_ADDR                                                       \
	XPAR_PS_SUBSYSTEM_AXI_GPIO_SPI_MUX_DEVICE_ID /* GPIO base address */
#endif
#define GPIO_DATA_REG 0 /* GPIO data register offset address */
#define GPIO_CONTROL_REG 4 /* GPIO control register offset address */
#endif

#ifdef __BAREMETAL__
static int XRFClk_I2CWrData(XIicPs *Iic, u8 Addr, u8 *Val, u8 Len);
static int XRFClk_I2CRdData(XIicPs *Iic, u8 Addr, u8 *Val, u8 Len);
#else
static int XRFClk_I2CWrData(int File, u8 Addr, u8 *Val, u8 Len);
static int XRFClk_I2CRdData(int File, u8 Addr, u8 *Val, u8 Len);
#endif

#include "xrfclk_LMK_conf.h"
#include "xrfclk_LMX_conf.h"

/****************************************************************************/
/**
*
* This function is used to open and configure i2c drivers.
*
* @param	None
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
static int XRFClk_InitI2C(void)
{
#ifdef __BAREMETAL__
	XIicPs_Config *Config_iic;
	int Status;
	u32 ClkRate = 100000;

#ifdef XPS_BOARD_ZCU111
	/* i2c0 */
	Config_iic = XIicPs_LookupConfig(0);
	if (NULL == Config_iic) {
		return XST_FAILURE;
	}

	Status = XIicPs_CfgInitialize(&Iic0, Config_iic,
				      Config_iic->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XIicPs_SetSClk(&Iic0, ClkRate);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif

	/* i2c1 */
	Config_iic = XIicPs_LookupConfig(1);
	if (NULL == Config_iic) {
		return XST_FAILURE;
	}

	Status = XIicPs_CfgInitialize(&Iic1, Config_iic,
				      Config_iic->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XIicPs_SetSClk(&Iic1, ClkRate);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#else
#ifdef XPS_BOARD_ZCU111
	/* i2c0 */
	fd_i2c0 = open(RFCLK_I2C0_DEVICE_PATH, O_RDWR);
	if (fd_i2c0 < 0) {
		LOG("i2c0 open");
		return XST_FAILURE;
	}
#endif

	/* i2c1 */
	fd_i2c1 = open(RFCLK_I2C1_DEVICE_PATH, O_RDWR);
	if (fd_i2c1 < 0) {
		LOG("i2c1 open");
		return XST_FAILURE;
	}
	fd_bridge = open(RFCLK_I2C_I2C2SPI_BRIDGE_DEVICE_PATH, O_RDWR);
	if (fd_bridge < 0) {
		LOG("i2c2spi bridge open");
		return XST_FAILURE;
	}
#endif
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function is used to open and configure MUX_SEL GPIO drivers.
*
* @param	None
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
static u32 XRFClk_InitGPIO(void)
{
#ifdef XPS_BOARD_ZCU111
	if (XST_FAILURE == XRFClk_I2cIoExpanderConfig()) {
		LOG("config gpio expander");
		return XST_FAILURE;
	}
#elif defined __BAREMETAL__
	Xil_Out32(GPIO_DEVICE_ADDR + GPIO_CONTROL_REG, 0xfffffffC);
#else
	int expfd;
	int dirfd;
	char gpio_dirpath[50];
	ssize_t unused __attribute__((unused));

	expfd = open("/sys/class/gpio/export", O_WRONLY);
	if (expfd < 0) {
		LOG("gpio open");
		return XST_FAILURE;
	}
	unused = write(expfd, GPIO_MUX_SEL0, 4);
	unused = write(expfd, GPIO_MUX_SEL1, 4);

	/* Update the direction of the GPIO to be an output */
	sprintf(gpio_dirpath, "/sys/class/gpio/gpio%s/direction",
		GPIO_MUX_SEL0);
	dirfd = open(gpio_dirpath, O_RDWR);
	if (dirfd < 0) {
		close(expfd);
		LOG("gpio set direction");
		return XST_FAILURE;
	}
	unused = write(dirfd, "out", 4);
	close(dirfd);
	sprintf(gpio_dirpath, "/sys/class/gpio/gpio%s/direction",
		GPIO_MUX_SEL1);
	dirfd = open(gpio_dirpath, O_RDWR);
	if (dirfd < 0) {
		close(expfd);
		LOG("gpio write direction");
		return XST_FAILURE;
	}
	unused = write(dirfd, "out", 4);
	close(dirfd);
	close(expfd);
#endif
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function is used to configure SPI.
*
* @param	None
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
static u32 XRFClk_InitSPI()
{
	u8 tx[2] = { 0xf0, 0x03 };
	if (XST_SUCCESS !=
	    XRFClk_I2CWrData(FD_BRIDGE, I2C_ADDR_I2C2SPI_BRIDGE, tx, 2))
		return XST_FAILURE;
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function is used to set MUX_SEL GPIO pins to connect appropriate SPI
* SDO.
*
* @param	None
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
static u32 XRFClk_MUX_SPI_SDO_GPIOPin(u8 ChipId)
{
#ifdef XPS_BOARD_ZCU111
	u8 rx[2];
	u32 ret;
	u8 tx[4];

	tx[0] = I2C_IO_EXPANDER_GPIO_OUTPUT_REG_ADDR;

	/* Select who to connect to */
	ret = XRFClk_I2CWrData(FD_I2C0, I2C_ADDR_I2C_IO_EXPANDER, tx, 1);
	if (ret == XST_FAILURE) {
		LOG("set gpio expander");
		return XST_FAILURE;
	}

	ret = XRFClk_I2CRdData(FD_I2C0, I2C_ADDR_I2C_IO_EXPANDER, rx, 1);
	if (ret == XST_FAILURE) {
		LOG("read gpio expandor status");
		return XST_FAILURE;
	}

	/* Select who to connect */
	tx[1] = (rx[0] & ~0x6) | (ChipId << 1);
	ret = XRFClk_I2CWrData(FD_I2C0, I2C_ADDR_I2C_IO_EXPANDER, tx, 2);
	if (ret == XST_FAILURE) {
		LOG("set new gpio expander state");
		return XST_FAILURE;
	}
#elif defined(__BAREMETAL__)
	/* Select MUX */
	Xil_Out32(GPIO_DEVICE_ADDR, ChipId);
#else
	int valfd;
	char gpio_valpath[50];
	ssize_t unused __attribute__((unused));

	sprintf(gpio_valpath, "/sys/class/gpio/gpio%s/value", GPIO_MUX_SEL0);
	valfd = open(gpio_valpath, O_RDWR);
	if (valfd < 0) {
		LOG("open MUX_SEL0 gpio");
		return XST_FAILURE;
	}
	if (ChipId & 1)
		unused = write(valfd, "1", 2);
	else
		unused = write(valfd, "0", 2);
	close(valfd);

	sprintf(gpio_valpath, "/sys/class/gpio/gpio%s/value", GPIO_MUX_SEL1);
	valfd = open(gpio_valpath, O_RDWR);
	if (valfd < 0) {
		LOG("open MUX_SEL1 gpio");
		return XST_FAILURE;
	}
	if (ChipId & 2)
		unused = write(valfd, "1", 2);
	else
		unused = write(valfd, "0", 2);
	close(valfd);
#endif
	return XST_SUCCESS;
}

#ifdef __BAREMETAL__
/****************************************************************************/
/**
*
* This function is HAL API for I2c read.
*
* @param	IIc I2c port Id.
* @param	Addr address to be read.
* @param	Val read value.
* @param	Len data length.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
static int XRFClk_I2CRdData_sub(XIicPs *Iic, u8 Addr, u8 *Val, u8 Len)
{
	int Status;
	Status = XIicPs_MasterRecvPolled(Iic, Val, Len, Addr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while (XIicPs_BusIsBusy(Iic))
		;
	usleep(I2C_SLEEP_US);
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function is HAL API for I2c write.
*
* @param	IIc I2c port Id.
* @param	Addr address to be written to.
* @param	Val value to write.
* @param	Len data length.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
static int XRFClk_I2CWrData_sub(XIicPs *Iic, u8 Addr, u8 *Val, u8 Len)
{
	int Status;
	Status = XIicPs_MasterSendPolled(Iic, Val, Len, Addr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while (XIicPs_BusIsBusy(Iic))
		;
	usleep(I2C_SLEEP_US);
	return XST_SUCCESS;
}
#else
/****************************************************************************/
/**
*
* This function is HAL API for I2c read.
*
* @param	File descriptor for the i2c driver.
* @param	Addr address to be read.
* @param	Val read value.
* @param	Len data length.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
static int XRFClk_I2CRdData_sub(int File, u8 Addr, u8 *Val, u8 Len)
{
	int ret = XST_SUCCESS;
	struct i2c_rdwr_ioctl_data packets;
	struct i2c_msg messages;
	messages.addr = Addr;
	messages.flags = I2C_M_RD;
	messages.len = Len;
	messages.buf = Val;
	packets.msgs = &messages;
	packets.nmsgs = 1;
	if (ioctl(File, I2C_RDWR, &packets) < 0) {
		ret = XST_FAILURE;
	}
	return ret;
}

/****************************************************************************/
/**
*
* This function is HAL API for I2c write.
*
* @param	File descriptor for the i2c driver.
* @param	Addr address to be written to.
* @param	Val value to write.
* @param	Len data length.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
static int XRFClk_I2CWrData_sub(int File, u8 Addr, u8 *Val, u8 Len)
{
	int ret = XST_SUCCESS;
	struct i2c_rdwr_ioctl_data packets;
	struct i2c_msg messages;
	messages.addr = Addr;
	messages.flags = 0;
	messages.len = Len;
	messages.buf = Val;
	packets.msgs = &messages;
	packets.nmsgs = 1;
	if (ioctl(File, I2C_RDWR, &packets) < 0) {
		ret = XST_FAILURE;
	}
	return ret;
}
#endif

/****************************************************************************/
/**
*
* This function is used to enable clk104 buses on I2C1 bus switch.
*
* @param	ChipId indicates the RF clock chip Id.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
static int XRFClk_GetBusSwitchI2C1(u8 *val)
{
	s32 ret;

	/* Read I2C bus switch configuration */
	ret = XRFClk_I2CRdData_sub(FD_I2C1, I2C_ADDR_BUS_SWITCH, val, 1);
	if (ret == XST_FAILURE) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}

static int XRFClk_SetBusSwitchI2C1()
{
	u8 val;
	s32 ret;

	/* Enable i2c2spi bridge */
	val = I2C_SWITCH_SELECT_I2C2SPI_BRIDGE;
	/* Write new I2C bus switch configuration */
	ret = XRFClk_I2CWrData_sub(FD_I2C1, I2C_ADDR_BUS_SWITCH, &val, 1);
	if (ret == XST_FAILURE) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function is HAL API for I2c read. If attempt failes function will
* repeate IIC write protocol again for DELAY_100uS microseconds multiplied by
* a loop index number. The procedure will be repeated NUM_IIC_RETRIES times.
*
* @param	File descriptor for the i2c driver.
* @param	Addr address to be read.
* @param	Val read value.
* @param	Len data length.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
#ifdef __BAREMETAL__
static int XRFClk_I2CRdData(XIicPs *Iic, u8 Addr, u8 *Val, u8 Len)
#else
static int XRFClk_I2CRdData(int Iic, u8 Addr, u8 *Val, u8 Len)
#endif
{
	u8 mux = 0;
	u32 i;

	for (i = 0; i < NUM_IIC_RETRIES; i++) {
		/* Set MUX */
		if (XST_FAILURE == XRFClk_SetBusSwitchI2C1()) {
			continue;
		}

		/* Read Register */
		if (XST_FAILURE == XRFClk_I2CRdData_sub(Iic, Addr, Val, Len)) {
			continue;
		}

		/* Read MUX status */
		if (XST_FAILURE == XRFClk_GetBusSwitchI2C1(&mux)) {
			continue;
		}

		/* Check is MUX as expected */
		if (mux == I2C_SWITCH_SELECT_I2C2SPI_BRIDGE) {
			break;
		} else {
			PRINTF("warrning: i2c1 MUX status change");
			/* Add delay before the next attempt */
			usleep(DELAY_100uS * (i + 1));
		}
	}
	if (i < NUM_IIC_RETRIES) {
		return XST_SUCCESS;
	} else {
		return XST_FAILURE;
	}
}

/****************************************************************************/
/**
*
* This function is HAL API for I2c write. If attempt failes function will
* repeate IIC write protocol again for DELAY_100uS microseconds multiplied by
* a loop index number. The procedure will be repeated NUM_IIC_RETRIES times.
*
* @param	Descriptor for the i2c driver.
* @param	Addr address to be written to.
* @param	Val value to write.
* @param	Len data length.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
#ifdef __BAREMETAL__
static int XRFClk_I2CWrData(XIicPs *Iic, u8 Addr, u8 *Val, u8 Len)
#else
static int XRFClk_I2CWrData(int Iic, u8 Addr, u8 *Val, u8 Len)
#endif
{
	u8 mux = 0;
	u32 i;

	for (i = 0; i < NUM_IIC_RETRIES; i++) {
		/* Set MUX */
		if (XST_FAILURE == XRFClk_SetBusSwitchI2C1()) {
			continue;
		}

		/* Read Register */
		if (XST_FAILURE == XRFClk_I2CWrData_sub(Iic, Addr, Val, Len)) {
			continue;
		}

		/* Read MUX status */
		if (XST_FAILURE == XRFClk_GetBusSwitchI2C1(&mux)) {
			continue;
		}

		/* Check is MUX as expected */
		if (mux == I2C_SWITCH_SELECT_I2C2SPI_BRIDGE) {
			break;
		} else {
			PRINTF("warrning: i2c1 MUX status change");
			/* Add delay before the next attempt */
			usleep(DELAY_100uS * (i + 1));
		}
	}
	if (i < NUM_IIC_RETRIES) {
		return XST_SUCCESS;
	} else {
		return XST_FAILURE;
	}
}

/****************************************************************************/
/**
*
* This function is used to connect MUX for read from the selected chip. The
* function also reconfigure the MUXOUT pin to act as a SPI SDO. The previous
* setting has been saved.
*
* @param	ChipId indicates the RF clock chip Id.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
static int XRFClk_MuxSPISDO(u8 ChipId)
{
	u32 ret;
	u8 tx[4];

#ifdef XPS_BOARD_ZCU111
	Xil_AssertNonvoid(ChipId != RFCLK_LMK);
#endif

	/* Set the Output MUX pin on a RFclk chip to SPI SDO */
	tx[0] = SELECT_SPI_SDO(ChipId);
	tx[1] = (MuxOutRegStorage[ChipId] >> 16) & 0xff;
	tx[2] = (MuxOutRegStorage[ChipId] >> 8) & 0xff;

	if (ChipId == RFCLK_LMK)
		tx[3] = LMK_MUXOUT_REG_VAL;
	else
		tx[3] = (MuxOutRegStorage[ChipId] & 0xff) & ~MUXOUT_LD_SEL_BIT;

	ret = XRFClk_I2CWrData(FD_BRIDGE, I2C_ADDR_I2C2SPI_BRIDGE, tx, 4);
	if (XST_FAILURE == ret) {
		LOG("write I2C2SPI bridge");
		return XST_FAILURE;
	}

	/* Set the MUX_SEL GPIOs to connect appropriate SPI SDO */
	if (XST_FAILURE == XRFClk_MUX_SPI_SDO_GPIOPin(ChipId)) {
		LOG("write gpio to MUX to clk104 chip");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function is used to set MUXOUT pin to the original state after read
* completion. Usually the pin is set to indicate a lock state of a PLL.
*
* @param	ChipId indicates the RF clock chip Id.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
static u32 XRFClk_MuxSPISDORevert(u32 ChipId)
{
#ifdef XPS_BOARD_ZCU111
	Xil_AssertNonvoid(ChipId != RFCLK_LMK);
#endif

	u32 ret;
	u8 tx[4];
	tx[0] = SELECT_SPI_SDO(ChipId);
	tx[1] = (MuxOutRegStorage[ChipId] >> 16) & 0xff;
	tx[2] = (MuxOutRegStorage[ChipId] >> 8) & 0xff;
	tx[3] = MuxOutRegStorage[ChipId] & 0xff;

	ret = XRFClk_I2CWrData(FD_BRIDGE, I2C_ADDR_I2C2SPI_BRIDGE, tx, 4);
	if (XST_FAILURE == ret) {
		LOG("revert back a bridge MUX");
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}

#ifdef XPS_BOARD_ZCU111
/****************************************************************************/
/**
*
* This function is used to enable GPIO_SEL on I2C expander for ZCU111.
*
* @param	ChipId indicates the RF clock chip Id.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
static int XRFClk_I2cIoExpanderConfig()
{
	u8 tx[3] = { I2C_IO_EXPANDER_CONFIG_REG_ADDR };
	u8 rx;
	int ret;

	ret = XRFClk_I2CRdData(FD_I2C0, I2C_ADDR_I2C_IO_EXPANDER, &rx, 1);
	if (ret == XST_FAILURE) {
		LOG("read expander data");
		return XST_FAILURE;
	}

	/* Set MUX_SEL_0/1 to output */
	tx[1] = rx & ~(I2C_MUX_SEL_0 | I2C_MUX_SEL_1);
	ret = XRFClk_I2CWrData(FD_I2C0, I2C_ADDR_I2C_IO_EXPANDER, tx, 2);
	if (ret == XST_FAILURE) {
		LOG("write expander data");
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}
/****************************************************************************/
/**
*
* This function is used to write to LMK04208 register.
*
* @param	ChipId indicates the RF clock chip Id.
* @param	d data to be written..
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
static u32 XRFClk_WriteRegToLMK04208(u32 ChipId, u32 d)
{
	Xil_AssertNonvoid(ChipId == RFCLK_LMK);
	u8 tx[5] = { SELECT_SPI_SDO(ChipId), (d >> 24) & 0xff, (d >> 16) & 0xff,
		     (d >> 8) & 0xff, d & 0xff };
	int ret;

	/* Write register */
	ret = XRFClk_I2CWrData(FD_BRIDGE, I2C_ADDR_I2C2SPI_BRIDGE, tx, 5);
	if (ret == XST_FAILURE) {
		LOG("write to LMK");
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}
#endif

/****************************************************************************/
/****************            A P I   section              *******************/
/****************************************************************************/

/****************************************************************************/
/**
*
* This function is used to write a register on one of LMX2594 or LMX04828.
*
* @param	ChipId indicates the RF clock chip Id.
* @param	d = {D2, D1, D0}
*	Where [D0,D1,D2] bits are:
*		LMK04282:
*			bit [23] - 1-bit command field (R/W)
*			bits [22:21] - 2-bit multi-byte field (W1, W0)
*			bits [20:8] - 13-bit address field (A12 to A0)
*			bits [7-0]- 8-bit data field (D7 to D0).
*		LMX2594:
*			bit [23] - 1-bit command field (R/W)
*			bits [22:16] - 7-bit address field (A6 to A0)
*			bits [15-0]- 16-bit data field (D15 to D0).
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
u32 XRFClk_WriteReg(u32 ChipId, u32 d)
{
	Xil_AssertNonvoid(ChipId < RFCLK_CHIP_NUM);

	u8 tx[4] = { SELECT_SPI_SDO(ChipId), (d >> 16) & 0xff, (d >> 8) & 0xff,
		     d & 0xff };

	/* Check is this the register which controls output Mux control */
	if (ChipId == RFCLK_LMK) {
#ifdef XPS_BOARD_ZCU111
		return XRFClk_WriteRegToLMK04208(ChipId, d);
#else
		if (((d >> LMK_ADDRESS_SHIFT) & LMK_ADDRESS_BITFIELD) ==
		    LMK_MUXOUT_REG_ADDR)
			MuxOutRegStorage[ChipId] = d;
#endif
	} else {
		if (((d >> LMX_ADDRESS_SHIFT) & LMX_ADDRESS_BITFIELD) ==
		    LMX_MUXOUT_REG_ADDR)
			MuxOutRegStorage[ChipId] = d;
	}
	/* Write register */
	return XRFClk_I2CWrData(FD_BRIDGE, I2C_ADDR_I2C2SPI_BRIDGE, tx, 4);
}

/****************************************************************************/
/**
*
* This function is used to read a register from one of LMX2594 or LMX04828.
*
* @param	ChipId indicates the RF clock chip Id.
* @param	d = {D2, D1, D0}
*	Where [D0,D1,D2] bits are:
*		LMK04282:
*			bit [23] - 1-bit command field (R/W)
*			bits [22:21] - 2-bit multi-byte field (W1, W0)
*			bits [20:8] - 13-bit address field (A12 to A0)
*			bits [7-0]- 8-bit data field (D7 to D0).
*		LMX2594:
*			bit [23] - 1-bit command field (R/W)
*			bits [22:16] - 7-bit address field (A6 to A0)
*			bits [15-0]- 16-bit data field (D15 to D0).
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
u32 XRFClk_ReadReg(u32 ChipId, u32 *d)
{
	Xil_AssertNonvoid(ChipId < RFCLK_CHIP_NUM);
	Xil_AssertNonvoid(d != NULL);

	u8 data[3] = { 0xff & (*d >> 16), 0xff & (*d >> 8), 0xff & (*d) };
	u8 tx[4] = { SELECT_SPI_SDO(ChipId), data[0], data[1], data[2] };

#ifdef XPS_BOARD_ZCU111
	if (ChipId == RFCLK_LMK) {
		LOG("read LMK not supported");
		return XST_FAILURE;
	}
#endif

	/* Setup environment for read */
	if (XRFClk_MuxSPISDO(ChipId) == XST_FAILURE) {
		LOG("Setup SPISDO for read");
		return XST_FAILURE;
	}
	/* Read register */
	tx[1] = data[0] | RF_DATA_READ_BIT;
	if (XST_FAILURE ==
	    XRFClk_I2CWrData(FD_BRIDGE, I2C_ADDR_I2C2SPI_BRIDGE, tx, 4)) {
		LOG("set bridge for read");
		return XST_FAILURE;
	}
	if (XST_FAILURE ==
	    XRFClk_I2CRdData(FD_BRIDGE, I2C_ADDR_I2C2SPI_BRIDGE, data, 3)) {
		LOG("read register");
		return XST_FAILURE;
	}
	/* Revert the environment */
	if (XRFClk_MuxSPISDORevert(ChipId) == XST_FAILURE) {
		LOG("revert the environment");
		return XST_FAILURE;
	}
	*d = (data[0] << 16) + (data[1] << 8) + data[2];
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function is used to initialize RFCLK devices on i2c1-bus:
* i2c1 bus switch, i2c2spi bridge and MUX_SELx GPIOs.
*
* @param	none
*
* @return	GpioId gpio ID for Linux build, n/a for baremetal build.
*
* @note		None
*
****************************************************************************/
#if defined __BAREMETAL__ || defined XPS_BOARD_ZCU111
u32 XRFClk_Init()
{
#else
u32 XRFClk_Init(int GpioId)
{
	sprintf(GPIO_MUX_SEL0, "%d", GpioId);
	sprintf(GPIO_MUX_SEL1, "%d", GpioId + 1);

#endif
	if (XST_FAILURE == XRFClk_InitI2C()) {
		LOG("i2c init");
		return XST_FAILURE;
	}
	if (XST_FAILURE == XRFClk_InitGPIO()) {
		LOG("gpio init");
		return XST_FAILURE;
	}
	if (XST_FAILURE == XRFClk_InitSPI()) {
		LOG("spi init");
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function is used to close RFCLK devices.
*
* @param	none
*
* @return	none
*
* @note		None
*
****************************************************************************/
void XRFClk_Close(void)
{
#ifndef __BAREMETAL__
#ifdef XPS_BOARD_ZCU111
	close(FD_I2C0);
#endif
	close(FD_BRIDGE);
	close(FD_I2C1);
#endif
}

/****************************************************************************/
/**
*
* This function is used to reset one of LMX2594 or LMK04828.
*
* @param	ChipId indicates the RF clock chip Id.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
u32 XRFClk_ResetChip(u32 ChipId)
{
	Xil_AssertNonvoid(ChipId < RFCLK_CHIP_NUM);

	u32 val = LMX_RESET_VAL;

	if (ChipId == RFCLK_LMK)
		val = LMK_RESET_VAL;

	if (XST_SUCCESS != XRFClk_WriteReg(ChipId, val)) {
		LOG("reset chip");
		return XST_FAILURE;
	}
	if (XST_SUCCESS != XRFClk_WriteReg(ChipId, 0)) {
		LOG("undo reset");
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function is used to set config on LMK.
*
* @param	ConfigId indicates the config Id.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
static u32 XRFClk_SetConfigLMK(u32 ConfigId)
{
	for (int i = 0; i < LMK_COUNT; i++) {
		if (XST_SUCCESS !=
		    XRFClk_WriteReg(RFCLK_LMK, LMK_CKin[ConfigId][i])) {
			LOG("write reg in LMK");
			return XST_FAILURE;
		}
	}
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function is used to set config on LMX.
*
* @param	ChipId indicates the RF clock chip Id.
* @param	ConfigId indicates the config Id.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
static u32 XRFClk_SetConfigLMX(u32 ChipID, u32 ConfigId)
{
	for (int i = 0; i < LMX2594_COUNT; i++) {
		if (XST_SUCCESS !=
		    XRFClk_WriteReg(ChipID, LMX2594[ConfigId][i])) {
			LOG("write reg in LMX");
			return XST_FAILURE;
		}
	}
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function is used to set a full configuration on one of LMX2594 or
* LMX04828 for the requested frequency.where the register settings is
* provided from the selected hard coded data.
*
* @param	ChipId indicates the RF clock chip Id.
* @param	ConfigId indicates the RF clock chip configuration Id.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
u32 XRFClk_SetConfigOnOneChipFromConfigId(u32 ChipId, u32 ConfigId)
{
	Xil_AssertNonvoid(ChipId < RFCLK_CHIP_NUM);

	if (ChipId == RFCLK_LMK) {
		Xil_AssertNonvoid(ConfigId <
				  sizeof(LMK_CKin) / sizeof(LMK_CKin[0]));
		return XRFClk_SetConfigLMK(ConfigId);
	}
	Xil_AssertNonvoid(ConfigId < sizeof(LMX2594) / sizeof(LMX2594[0]));
	return XRFClk_SetConfigLMX(ChipId, ConfigId);
}

/****************************************************************************/
/**
*
* This function is used to set the full configuration data on one of
* LMX2594 or LMX04828. The all register values are passed as a pointer
* CfgData, Len defines a number of data ready for writing.
*
* @param	ChipId indicates the RF clock chip Id.
* @param	CfgData indicates the configuration for all registers.
* @param	Len indicates a number of data.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
u32 XRFClk_SetConfigOnOneChip(u32 ChipId, u32 *CfgData, u32 Len)
{
	Xil_AssertNonvoid(ChipId < RFCLK_CHIP_NUM);
	Xil_AssertNonvoid(CfgData != NULL);
	Xil_AssertNonvoid(Len < 256);

	u32 *d = CfgData;
	for (u32 i = 0; i < Len; i++, d++) {
		if (XST_SUCCESS != XRFClk_WriteReg(ChipId, *d)) {
			LOG("write reg");
			return XST_FAILURE;
		}
	}
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function is used to set a full configuration on all LMX2594
* and LMK04828 or LMK04208 for the requested frequency.
*
* @param	ConfigId_LMK indicates the LMK configuration Id.
* @param	ConfigId_RF1 indicates the LMX RF1 configuration Id.
* @param	ConfigId_RF2 indicates the LMX RF2 configuration Id.
* @param	ConfigId_RF3 indicates the LMX RF3 configuration Id.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
u32 XRFClk_SetConfigOnAllChipsFromConfigId(u32 ConfigId_LMK, u32 ConfigId_1,
#ifdef XPS_BOARD_ZCU111
					   u32 ConfigId_2, u32 ConfigId_3)
#else
					   u32 ConfigId_2)
#endif
{
	Xil_AssertNonvoid(ConfigId_LMK <
			  sizeof(LMK_CKin) / sizeof(LMK_CKin[0]));
	Xil_AssertNonvoid(ConfigId_1 < sizeof(LMX2594) / sizeof(LMX2594[0]));
	Xil_AssertNonvoid(ConfigId_2 < sizeof(LMX2594) / sizeof(LMX2594[0]));

	if (XST_SUCCESS !=
	    XRFClk_SetConfigOnOneChipFromConfigId(RFCLK_LMK, ConfigId_LMK)) {
		LOG("set config to LMK");
		return XST_FAILURE;
	}
	if (XST_SUCCESS != XRFClk_SetConfigOnOneChipFromConfigId(
				   RFCLK_LMX2594_1, ConfigId_1)) {
		LOG("set config to LMX1");
		return XST_FAILURE;
	}
	if (XST_SUCCESS != XRFClk_SetConfigOnOneChipFromConfigId(
				   RFCLK_LMX2594_2, ConfigId_2)) {
		LOG("set config to LMX2");
		return XST_FAILURE;
	}
#ifdef XPS_BOARD_ZCU111
	Xil_AssertNonvoid(ConfigId_3 < sizeof(LMX2594) / sizeof(LMX2594[0]));
	if (XST_SUCCESS != XRFClk_SetConfigOnOneChipFromConfigId(
				   RFCLK_LMX2594_3, ConfigId_3)) {
		LOG("set config to LMX3");
		return XST_FAILURE;
	}
#endif
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function is used to get config from LMK.
*
* @param	ChipId indicates the RF clock chip Id.
* @param	in pointer to array of data read from the chip registers.
* @param	out pointer to array of data read from the chip registers.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
static u32 XRFClk_getConfig_fromLMK(u32 ChipId, u32 *in, u32 *out)
{
#ifdef XPS_BOARD_ZCU111
	(void)ChipId;
	(void)in;
	(void)out;
	LOG("not supported");
	return XST_FAILURE;
#else
	u8 data[3];
	u8 tx[4];
	tx[0] = SELECT_SPI_SDO(ChipId);

	for (int i = 0; i < LMK_COUNT; i++) {
		data[0] = 0xff & (in[i] >> 16);
		data[1] = 0xff & (in[i] >> 8);
		tx[1] = data[0];
		tx[2] = data[1];

		if ((((in[i]) >> 8) & 0xffff) == LMK_MUXOUT_REG_ADDR) {
			out[i] = MuxOutRegStorage[ChipId];
			continue;
		}

		/* Read register */
		tx[1] = data[0] | RF_DATA_READ_BIT;
		if (XST_FAILURE == XRFClk_I2CWrData(FD_BRIDGE,
						    I2C_ADDR_I2C2SPI_BRIDGE, tx,
						    4)) {
			LOG("write reg");
			return XST_FAILURE;
		}
		if (XST_FAILURE == XRFClk_I2CRdData(FD_BRIDGE,
						    I2C_ADDR_I2C2SPI_BRIDGE,
						    data, 3)) {
			LOG("read reg");
			return XST_FAILURE;
		}

		out[i] = (data[0] << 16) + (data[1] << 8) + data[2];
	}
#endif
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function is used to get config from LMK.
*
* @param	ChipId indicates the RF clock chip Id.
* @param	d pointer to array of data read from the chip registers.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
static u32 XRFClk_getConfig_fromLMX(u32 ChipId, u32 *d)
{
	u8 data[3] = { 0, 0, 0 };
	u8 tx[4] = { SELECT_SPI_SDO(ChipId), data[0], data[1], data[2] };
	for (int i = 0; i < LMX2594_COUNT; i++, d++) {
		data[0] = i;

		if (data[0] == LMX_MUXOUT_REG_ADDR) {
			*d = MuxOutRegStorage[ChipId];
			continue;
		}

		/* Read register */
		tx[1] = data[0] | RF_DATA_READ_BIT;
		if (XST_FAILURE == XRFClk_I2CWrData(FD_BRIDGE,
						    I2C_ADDR_I2C2SPI_BRIDGE, tx,
						    4)) {
			LOG("write reg");
			return XST_FAILURE;
		}
		if (XST_FAILURE == XRFClk_I2CRdData(FD_BRIDGE,
						    I2C_ADDR_I2C2SPI_BRIDGE,
						    data, 3)) {
			LOG("read reg");
			return XST_FAILURE;
		}

		*d = (data[0] << 16) + (data[1] << 8) + data[2];
	}
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function is used to read the full configuration data from one of
* LMX2594 or LMX04828.
*
* @param	ChipId indicates the RF clock chip Id.
* @param	CfgData the array of the RF clock chip configuration data.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
u32 XRFClk_GetConfigFromOneChip(u32 ChipId, u32 *CfgData)
{
	Xil_AssertNonvoid(ChipId < RFCLK_CHIP_NUM);
	Xil_AssertNonvoid(CfgData != NULL);
#ifdef XPS_BOARD_ZCU111
	Xil_AssertNonvoid(ChipId != RFCLK_LMK);
#endif

	u32 ret = XST_SUCCESS;
	u32 *d = CfgData;

	/* Setup environment for read */
	if (XRFClk_MuxSPISDO(ChipId) == XST_FAILURE) {
		LOG("mux SPISDO");
		return XST_FAILURE;
	}

	if (ChipId == RFCLK_LMK)
		ret = XRFClk_getConfig_fromLMK(ChipId, (u32 *const)LMK_CKin, d);
	else
		ret = XRFClk_getConfig_fromLMX(ChipId, d);

	/* Revert the environment */
	if (XRFClk_MuxSPISDORevert(ChipId) == XST_FAILURE) {
		LOG("revert SPISDO");
		ret = XST_FAILURE;
	}

	return ret;
}

/****************************************************************************/
/**
*
* This function is used to enable or disable the specified output port on
* LMK04828.
*
* @param	PortId indicates a LMK04828 port id, [1:7].
* @param	State indicates the state of the port, see chapter 9.7.2.7
*		in lmk04828.pdf datasheet, [0:7]
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
u32 XRFClk_ControlOutputPortLMK(u32 PortId, u32 State)
{
	Xil_AssertNonvoid((PortId <= LMK_PORT_ID_MAX) && (PortId > 0));
	Xil_AssertNonvoid(State <= 0xff);

	u32 ret = XST_SUCCESS;
#ifdef XPS_BOARD_ZCU111
	(void)PortId;
	(void)State;
	LOG("not supported");
	ret = XST_FAILURE;
#else

	u32 Addr = (LMK_CFG_OUTPUT_PORT0_ADDR + LMK_ADDRESS_STEP * (PortId - 1))
		   << LMK_ADDRESS_SHIFT;
	u8 data = State;
	if (XST_SUCCESS != XRFClk_WriteReg(RFCLK_LMK, Addr + data)) {
		LOG("write reg");
		ret = XST_FAILURE;
	}
#endif

	return ret;
}

/****************************************************************************/
/**
*
* This function is used to configure a clock divider and the output port MUX
* state on LMK04828. For the details consult tables 16, 18, 19 and 25 in
* the lmk04828.pdf datasheet.
*
* @param	PortId indicates a LMK04828 port id, [1:7].
* @param	DCLKoutX_DIV sets divider for the clock output [0:31].
* @param	DCLKoutX_MUX selects input to the device clk buffer [0:3].
* @param	SDCLKoutY_MUX sets input to SDCLKoutY [0:1].
* @param	SYSREF_DIV sets SYSREF output divider [8:8191]
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
u32 XRFClk_ConfigOutputDividerAndMUXOnLMK(u32 PortId, u32 DCLKoutX_DIV,
					  u32 DCLKoutX_MUX, u32 SDCLKoutY_MUX,
					  u32 SYSREF_DIV)
{
	u32 ret = XST_SUCCESS;
#ifdef XPS_BOARD_ZCU111
	(void)PortId;
	(void)DCLKoutX_DIV;
	(void)DCLKoutX_MUX;
	(void)SDCLKoutY_MUX;
	(void)SYSREF_DIV;
	LOG("not supported");
	ret = XST_FAILURE;
#else
	Xil_AssertNonvoid((PortId <= LMK_PORT_ID_MAX) && (PortId > 0));
	Xil_AssertNonvoid(DCLKoutX_DIV <= LMK_DCLKOUTX_DIV_MAX);
	Xil_AssertNonvoid(DCLKoutX_MUX <= LMK_DCLKOUTX_MUX_MAX);
	Xil_AssertNonvoid(SDCLKoutY_MUX <= LMK_SDCLKOUTY_MUX_MAX);
	Xil_AssertNonvoid((SYSREF_DIV >= LMK_SYSREF_DIV_MIN) &&
			  (SYSREF_DIV <= LMK_SYSREF_DIV_MAX));

	u32 Addr;
	u8 data;
	if ((PortId >= LMK_PORT_ID_MAX) || (PortId == 0) ||
	    (DCLKoutX_DIV > LMK_DCLKOUTX_DIV_MAX) ||
	    (DCLKoutX_MUX > LMK_DCLKOUTX_MUX_MAX) ||
	    (SDCLKoutY_MUX > LMK_SDCLKOUTY_MUX_MAX) ||
	    (SYSREF_DIV < LMK_SYSREF_DIV_MIN) ||
	    (SYSREF_DIV > LMK_SYSREF_DIV_MAX)) {
		LOG("wrong LMK settings");
		return XST_FAILURE;
	}

	/* Set DCLKoutX_DIV */
	Addr = (LMK_DCLKOUTX_DIV_PORT0_ADDR + LMK_ADDRESS_STEP * PortId)
	       << LMK_ADDRESS_SHIFT;
	data = DCLKoutX_DIV;
	if (XST_SUCCESS != XRFClk_WriteReg(RFCLK_LMK, Addr + data)) {
		LOG("Set DCLKoutX_DIV");
		return XST_FAILURE;
	}

	/* Set DCLKoutX_MUX */
	Addr = (LMK_DCLKOUTX_MUX_PORT0_ADDR + LMK_ADDRESS_STEP * PortId)
	       << LMK_ADDRESS_SHIFT;
	data = DCLKoutX_MUX;
	if (XST_SUCCESS != XRFClk_WriteReg(RFCLK_LMK, Addr + data)) {
		LOG("Set DCLKoutX_MUX");
		return XST_FAILURE;
	}

	/* Set SDCLKoutY_MUX */
	Addr = (LMK_SDCLKOUTY_MUX_PORT0_ADDR + LMK_ADDRESS_STEP * PortId)
	       << LMK_ADDRESS_SHIFT;
	data = DCLKoutX_MUX;
	if (XST_SUCCESS != XRFClk_WriteReg(RFCLK_LMK, Addr + data)) {
		LOG("Set DCLKoutY_DIV");
		return XST_FAILURE;
	}

	/* Set SYSREF_DIV */
	Addr = LMK_SYSREF_DIV_MSB_PORT0_ADDR << LMK_ADDRESS_SHIFT;
	data = (SYSREF_DIV >> 8) & 0xff;
	if (XST_SUCCESS != XRFClk_WriteReg(RFCLK_LMK, Addr + data)) {
		LOG("Set SYSREF_DIV");
		return XST_FAILURE;
	}
	Addr = LMK_SYSREF_DIV_LSB_PORT0_ADDR << LMK_ADDRESS_SHIFT;
	data = SYSREF_DIV & 0xff;
	if (XST_SUCCESS != XRFClk_WriteReg(RFCLK_LMK, Addr + data)) {
		LOG("update SYSREF_DIV");
		return XST_FAILURE;
	}
#endif
	return ret;
}
/** @} */
