/******************************************************************************
* Copyright (C) 2021-2022 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfesi570.c
*
* Contains the APIs which can be used to set si570 oscillator.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     03/08/21 Initial version
* 1.1   dc     07/13/21 Update to common latency requirements
* 1.2   dc     11/19/21 Update doxygen documentation
* </pre>
*
******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#if defined __BAREMETAL__

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_printf.h"
#include "xiicps.h"
/* Definitions */
XIicPs Iic1;
#define FD_DEVICE (&Iic1) /* I2C1 driver descriptor address */
#define FD_I2C1 (&Iic1) /* I2C1 driver descriptor address */
#define I2C_SLEEP_US 1000U /* I2C sleep period */

#else

/* User Headers */
#include <errno.h>
#include <assert.h>
/* Kernel Headers */
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
/* Typedefs */
typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long int u64;
typedef int s32;
/* Definitions */
#define XST_SUCCESS 0L
#define XST_FAILURE 1L
#define Xil_AssertNonvoid(a) assert(a) /* use linux assert */
#define Xil_AssertVoid(a) assert(a) /* use linux assert */
#define XDFESI570_I2C1_DEVICE "/dev/i2c-1" /* I2C1 device path */
#define XDFESI570_I2C1_MGT_DEVICE "/dev/i2c-9" /* MGT oscillator device */
int fd_i2c1;
int fd_device;
#define FD_DEVICE fd_device /* Bridge driver descriptor alias */
#define FD_I2C1 fd_i2c1 /* I2C1 driver descriptor alias */

#endif

#define LOG(str) printf("issue in %s: %s", __func__, str) /* Logging macro */
#define RF_DATA_READ_BIT 0X80U /* Bit which indicates read */
#define I2C_ADDR_BUS_SWITCH 0X74U /* Bus switch I2C address */
#define I2C_DATA_LENGTH_BUS_SWITCH 1U /* i2c data length */
#define I2C_ADDR_I2C_MGT_SI570 0X5DU /* MGT oscillator I2C Address */
#define NUM_IIC_RETRIES (5) /* Number of IIC retries */
#define DELAY_100uS (100) /* Number for 1ms delay */

#define XDFESI570_NOT_USED 0xFFU /* Not used HS_DIV */
#define XDFESI570_HS_DIV_MAX_ID 8U /* Array size of HS_DIV values */
#define XDFESI570_DCO_MAX (5670.0) /* DCO maximum frequency in MHz */
#define XDFESI570_DCO_MIN (4850.0) /* DCO minimum frequency in MHz */

#define XDFESI570_N1_HS_DIV_REG 7U /* First register to read */
#define XDFESI570_RFREQ_LSB_REG 12U /* Last register to read */
#define XDFESI570_CLKOUT_DIVIDER_MAX 128U /* Maximum divider value */
#define XDFESI570_CONTROL_REG 0x87U /* Register address 135 */
#define XDFESI570_CONTROL_APPLY_FREQ 0x40U /* Apply new frequency */
#define XDFESI570_FREEZE_DCO_REG 0x89U /* Register address 137 */
#define XDFESI570_FREEZE_DCO 0x10U /* Freeze oscillator */
#define XDFESI570_UNFREEZE_DCO 0U /* Un-freeze oscillator */

/****************************************************************************/
/**
*
* This function is used to open and configure I2C drivers.
*
* @param None
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
int XDfeSi570_InitI2C(void)
{
#ifdef __BAREMETAL__
	XIicPs_Config *Config_iic;
	int Status;
	u32 ClkRate = 100000U;

	/* I2C1 */
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
	/* I2C1 */
	fd_i2c1 = open(XDFESI570_I2C1_DEVICE, O_RDWR);
	if (fd_i2c1 < 0) {
		Xil_AssertNonvoid("i2c1 open");
		return XST_FAILURE;
	}
	fd_device = open(XDFESI570_I2C1_MGT_DEVICE, O_RDWR);
	if (fd_device < 0) {
		Xil_AssertNonvoid("i2c device open");
		return XST_FAILURE;
	}
#endif
	return XST_SUCCESS;
}

#ifdef __BAREMETAL__
/****************************************************************************/
/**
*
* This function is HAL API for I2C read.
*
* @param Iic I2C port Id.
* @param Addr Address to be read.
* @param Val Read value.
* @param Len Data length.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
static int XDfeSi570_I2CRdData_sub(XIicPs *Iic, u8 Addr, u8 *Val, u8 Len)
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
* This function is HAL API for I2C write.
*
* @param Iic I2C port Id.
* @param Addr Address to be written to.
* @param Val Value to write.
* @param Len Data length.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
static int XDfeSi570_I2CWrData_sub(XIicPs *Iic, u8 Addr, u8 *Val, u8 Len)
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
* This function is HAL API for I2C read.
*
* @param File Descriptor for the I2C driver.
* @param Addr Address to be read.
* @param Val Read value.
* @param Len Data length.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
static int XDfeSi570_I2CRdData_sub(int File, u8 Addr, u8 *Val, u8 Len)
{
	int Return = XST_SUCCESS;
	struct i2c_rdwr_ioctl_data Packets;
	struct i2c_msg Messages;
	Messages.addr = Addr;
	Messages.flags = I2C_M_RD;
	Messages.len = Len;
	Messages.buf = Val;
	Packets.msgs = &Messages;
	Packets.nmsgs = 1U;
	if (ioctl(File, I2C_RDWR, &Packets) < 0) {
		Return = XST_FAILURE;
	}
	return Return;
}

/****************************************************************************/
/**
*
* This function is HAL API for I2C write.
*
* @param File Descriptor for the I2C driver.
* @param Addr Address to be written to.
* @param Val Value to write.
* @param Len Data length.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
static int XDfeSi570_I2CWrData_sub(int File, u8 Addr, u8 *Val, u8 Len)
{
	int Return = XST_SUCCESS;
	struct i2c_rdwr_ioctl_data Packets;
	struct i2c_msg Messages;
	Messages.addr = Addr;
	Messages.flags = 0;
	Messages.len = Len;
	Messages.buf = Val;
	Packets.msgs = &Messages;
	Packets.nmsgs = 1U;
	if (ioctl(File, I2C_RDWR, &Packets) < 0) {
		Return = XST_FAILURE;
	}
	return Return;
}
#endif

/****************************************************************************/
/**
*
* This function is used to read a status on I2C1 bus switch.
*
* @param BusSwitchStatus Returned status.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
static int XDfeSi570_GetBusSwitchI2C1(u8 *BusSwitchStatus)
{
	s32 Return;

	/* Read I2C bus switch configuration */
	Return = XDfeSi570_I2CRdData_sub(FD_I2C1, I2C_ADDR_BUS_SWITCH,
					 BusSwitchStatus,
					 I2C_DATA_LENGTH_BUS_SWITCH);
	if (Return == XST_FAILURE) {
		Xil_AssertNonvoid("get a bus switch status failed");
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function is used to set new status on I2C1 bus switch.
*
* @param BusSwitchStatus Mux Bus status.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
static int XDfeSi570_SetBusSwitchI2C1(u8 BusSwitchStatus)
{
	s32 Return;

	/* Write new I2C bus switch configuration */
	Return = XDfeSi570_I2CWrData_sub(FD_I2C1, I2C_ADDR_BUS_SWITCH,
					 &BusSwitchStatus,
					 I2C_DATA_LENGTH_BUS_SWITCH);
	if (Return == XST_FAILURE) {
		Xil_AssertNonvoid("set a bus switch failed");
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function is HAL API for I2C read. If attempt fails function will
* repeat IIC write protocol again for DELAY_100uS microseconds multiplied by
* a loop index number. The procedure will be repeated NUM_IIC_RETRIES times.
*
* @param Iic File descriptor for the I2C driver.
* @param Addr Address to be read.
* @param Val Read value.
* @param Len Data length.
* @param BusSwitchStatus New Bus Switch setting.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
#ifdef __BAREMETAL__
static int XDfeSi570_I2CRdData(XIicPs *Iic, u8 Addr, u8 *Val, u8 Len,
			       u8 BusSwitchStatus)
#else
static int XDfeSi570_I2CRdData(int Iic, u8 Addr, u8 *Val, u8 Len,
			       u8 BusSwitchStatus)
#endif
{
	u8 Mux = 0;
	u32 Index;

	for (Index = 0; Index < NUM_IIC_RETRIES; Index++) {
		/* Set MUX */
		if (XST_FAILURE ==
		    XDfeSi570_SetBusSwitchI2C1(BusSwitchStatus)) {
			continue;
		}

		/* Read Register */
		if (XST_FAILURE ==
		    XDfeSi570_I2CRdData_sub(Iic, Addr, Val, Len)) {
			continue;
		}

		/* Read MUX status */
		if (XST_FAILURE == XDfeSi570_GetBusSwitchI2C1(&Mux)) {
			continue;
		}

		/* Check is MUX as expected */
		if (Mux == BusSwitchStatus) {
			break;
		} else {
			LOG("i2c1 MUX status");
			/* Add delay before the next attempt */
			usleep(DELAY_100uS * (Index + 1));
		}
	}
	if (Index < NUM_IIC_RETRIES) {
		return XST_SUCCESS;
	} else {
		Xil_AssertNonvoid("i2c1 read data failed");
		return XST_FAILURE;
	}
}

/****************************************************************************/
/**
*
* This function is HAL API for I2C write. If attempt fails function will
* repeat IIC write protocol again for DELAY_100uS microseconds multiplied by
* a loop index number. The procedure will be repeated NUM_IIC_RETRIES times.
*
* @param Iic Descriptor for the I2C driver.
* @param Addr Address to be written to.
* @param Val Value to write.
* @param Len Data length.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
#ifdef __BAREMETAL__
static int XDfeSi570_I2CWrData(XIicPs *Iic, u8 Addr, u8 *Val, u8 Len,
			       u8 BusSwitchStatus)
#else
static int XDfeSi570_I2CWrData(int Iic, u8 Addr, u8 *Val, u8 Len,
			       u8 BusSwitchStatus)
#endif
{
	u8 Mux = 0;
	u32 Index;

	for (Index = 0; Index < NUM_IIC_RETRIES; Index++) {
		/* Set MUX to enable MGT bus  */
		if (XST_FAILURE ==
		    XDfeSi570_SetBusSwitchI2C1(BusSwitchStatus)) {
			continue;
		}

		/* Read Register */
		if (XST_FAILURE ==
		    XDfeSi570_I2CWrData_sub(Iic, Addr, Val, Len)) {
			continue;
		}

		/* Read MUX status */
		if (XST_FAILURE == XDfeSi570_GetBusSwitchI2C1(&Mux)) {
			continue;
		}

		/* Check is MUX as expected */
		if (Mux == BusSwitchStatus) {
			break;
		} else {
			LOG("i2c1 MUX status");
			/* Add delay before the next attempt */
			usleep(DELAY_100uS * (Index + 1));
		}
	}
	if (Index < NUM_IIC_RETRIES) {
		return XST_SUCCESS;
	} else {
		Xil_AssertNonvoid("i2c1 write data failed");
		return XST_FAILURE;
	}
}

/****************************************************************************/
/**
*
* This function reads current state of si570, calculates parameters for si570
* MGT oscillator new frequency, writes them to si570
*
* @param CurrentFrequency MGT oscillator current frequency.
* @param NewFrequency MGT new frequency.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
u32 XDfeSi570_SetFrequency(double CurrentFrequency, double NewFrequency)
{
	const u8 XDfeSi570_HS_DIV[XDFESI570_HS_DIV_MAX_ID] = {
		4U, 5U, 6U, 7U, XDFESI570_NOT_USED, 9U, XDFESI570_NOT_USED, 11U
	};

	u8 Addr = XDFESI570_CONTROL_REG;
	u8 MuxAddr = 0x8U;
	u8 Tx = 1U;
	u8 Rx[20];
	u64 Rx_new;
	u32 Index = 0;
	u64 RFREQ;
	u64 RFREQ_new;
	double RFREQ_f;
	double f_RFREQ_new;
	u32 N;
	u32 N_new;
	u32 HS_DIV;
	double f_xtal;
	double f_dco;

	/* Set a control register address (0x87=135) and set recall NVM to RAM */
	XDfeSi570_I2CWrData(FD_DEVICE, I2C_ADDR_I2C_MGT_SI570, &Addr, 1U,
			    MuxAddr);
	XDfeSi570_I2CWrData(FD_DEVICE, I2C_ADDR_I2C_MGT_SI570, &Tx, 1U,
			    MuxAddr);

	/* Set a si570 register address and read data */
	for (Addr = XDFESI570_N1_HS_DIV_REG; Addr <= XDFESI570_RFREQ_LSB_REG;
	     Addr++) {
		XDfeSi570_I2CWrData(FD_DEVICE, I2C_ADDR_I2C_MGT_SI570, &Addr,
				    1U, MuxAddr);
		XDfeSi570_I2CRdData(FD_DEVICE, I2C_ADDR_I2C_MGT_SI570,
				    &Rx[Index], 1U, MuxAddr);
		Index++;
	}

	/* Prepare parameters for calculation */
	RFREQ = (((u64)(Rx[1] & 0x3fU)) << 32U) + (Rx[2] << 24U) +
		(Rx[3] << 16U) + (Rx[4] << 8U) + Rx[5];
	RFREQ_f = (double)((double)RFREQ / (1U << 28U));
	N = (((Rx[0] & 0x1fU) << 2U) | ((Rx[1] & 0xc0U) >> 6U)) + 1U;
	HS_DIV = XDfeSi570_HS_DIV[(Rx[0] & 0xE0U) >> 5U];
	f_xtal = (CurrentFrequency * N * HS_DIV) / RFREQ_f;

	/* Search N and HS_DIV to be in range [4.85GHz,5.67GHz] */
	for (N_new = 1U; N_new <= XDFESI570_CLKOUT_DIVIDER_MAX; N_new++) {
		/* valid are all even numbers and 1 */
		if ((N_new != 1U) && (N_new != 2U * (N_new / 2U))) {
			continue;
		}
		for (Index = 0; Index < XDFESI570_HS_DIV_MAX_ID; Index++) {
			HS_DIV = XDfeSi570_HS_DIV[Index];
			if (HS_DIV == XDFESI570_NOT_USED) {
				continue;
			}
			f_dco = NewFrequency * N_new * HS_DIV;
			if ((XDFESI570_DCO_MIN < f_dco) &&
			    (f_dco < XDFESI570_DCO_MAX)) {
				goto exit_loop;
			}
		}
		N_new += 2U;
	}
	LOG("Not available setting");
	return XST_FAILURE;

exit_loop:
	/* Find RFREQ for the chosen N and HS_DIV */
	f_RFREQ_new = f_dco / f_xtal;
	RFREQ_new = (u64)(f_RFREQ_new * (1U << 28U));

	/* Sort RFREQ into u8 array */
	Rx_new = RFREQ_new + ((u64)(N_new - 1U) << 38U);
	for (Index = 0; Index < 6U; Index++) {
		Rx[Index] = Rx_new & 0xffU;
		Rx_new = Rx_new >> 8U;
	}

	/* Freeze oscillator in register address (0x89=137) */
	Addr = XDFESI570_FREEZE_DCO_REG;
	Tx = XDFESI570_FREEZE_DCO;
	XDfeSi570_I2CWrData(FD_DEVICE, I2C_ADDR_I2C_MGT_SI570, &Addr, 1U,
			    MuxAddr);
	XDfeSi570_I2CWrData(FD_DEVICE, I2C_ADDR_I2C_MGT_SI570, &Tx, 1U,
			    MuxAddr);

	/* Set a si570 register address and write data */
	Index = 0;
	for (Addr = XDFESI570_N1_HS_DIV_REG; Addr <= XDFESI570_RFREQ_LSB_REG;
	     Addr++) {
		XDfeSi570_I2CWrData(FD_DEVICE, I2C_ADDR_I2C_MGT_SI570, &Addr,
				    1U, MuxAddr);
		XDfeSi570_I2CWrData(FD_DEVICE, I2C_ADDR_I2C_MGT_SI570,
				    &Rx[Index], 1U, MuxAddr);
		Index++;
	}

	/* UnFreeze oscillator in register address (0x89=137) */
	Addr = XDFESI570_FREEZE_DCO_REG;
	Tx = XDFESI570_UNFREEZE_DCO;
	XDfeSi570_I2CWrData(FD_DEVICE, I2C_ADDR_I2C_MGT_SI570, &Addr, 1U,
			    MuxAddr);
	XDfeSi570_I2CWrData(FD_DEVICE, I2C_ADDR_I2C_MGT_SI570, &Tx, 1U,
			    MuxAddr);

	/* New frequency applied in register address (0x87=135) */
	Addr = XDFESI570_CONTROL_REG;
	Tx = XDFESI570_CONTROL_APPLY_FREQ;
	XDfeSi570_I2CWrData(FD_DEVICE, I2C_ADDR_I2C_MGT_SI570, &Addr, 1U,
			    MuxAddr);
	XDfeSi570_I2CWrData(FD_DEVICE, I2C_ADDR_I2C_MGT_SI570, &Tx, 1U,
			    MuxAddr);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function resets PL and make it use the new clock.
*
* @param	None
*
* @return	None
*
* @note		None
*
****************************************************************************/
void XDfeSi570_SetPL()
{
#ifdef __BAREMETAL__
	Xil_Out32(0xFD615000U, 0U);
	Xil_In32(0xac001000U);
	Xil_In32(0xac001008U);
	Xil_Out32(0xac001000U, 0U);
	Xil_In32(0xac001008U);
#else
	(void)system("devmem 0xFD615000 w 0");
	(void)system("devmem 0xac001000");
	(void)system("devmem 0xac001008");
	(void)system("devmem 0xac001000 32 0");
	(void)system("devmem 0xac001008");
#endif
}

/****************************************************************************/
/**
*
* This function: initialize I2C bus, reads current state of si570, calculates
* parameters for si570 MGT oscillator new frequency, writes them to si570 and
* resets PL to use the new frequency.
*
* @param CurrentFrequency MGT oscillator current frequency.
* @param NewFrequency MGT oscillator new frequency.
*
* @return	None
*
* @note		None
*
****************************************************************************/
int XDfeSi570_SetMgtOscillator(double CurrentFrequency, double NewFrequency)
{
	if (XST_FAILURE == XDfeSi570_InitI2C()) {
		return XST_FAILURE;
	}
	if (XDfeSi570_SetFrequency(CurrentFrequency, NewFrequency)) {
		return XST_FAILURE;
	} else {
		XDfeSi570_SetPL();
	}
	return XST_SUCCESS;
}

/** @} */
