// Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
#include <hal_ps_i2c.h>

XIicPs g_Iic[3];

int Xil_IICWritepolled16(XIicPs *Config, u8 *sdata, u32 size, u16 regoffset,
			 u8 i2c_addrs)
{
	int Status;
	u8 senddata[4];

	senddata[0] = (regoffset >> 8) & 0xFF;
	senddata[1] = regoffset & 0xFF;
	for (int i = 1; i <= size; i++)

		senddata[i + 1] = sdata[i - 1];
	xil_printf("Sending Data:[0x%x] to offset[0x%x],size:%d\n", senddata[2],
		   regoffset, size);
	Status = XIicPs_MasterSendPolled(Config, senddata, size + 2, i2c_addrs);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed writepolled iic: %d\r\n", Status);
		return XST_FAILURE;
	}

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(Config)) {
		/* NOP */
	}
	return XST_SUCCESS;
}

int Xil_IICWritepolled(XIicPs *Config, u8 *sdata, u32 size, u16 regoffset,
		       u8 i2c_addrs)
{
	int Status;
	u8 senddata[6];
	int i = 0;
	senddata[0] = regoffset & 0xFF;

	for (i = 1; i <= size; i++)

		senddata[i] = sdata[i - 1];

	xil_printf("Sending Data:[0x%x] to offset[0x%x],size:%d,i2c_addr:0x%x\n",
		   senddata[1], senddata[0], size, i2c_addrs);
	Status = XIicPs_MasterSendPolled(Config, senddata, size + 1, i2c_addrs);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed writepolled iic\r\n");
		return XST_FAILURE;
	}

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(Config)) {
		/* NOP */
	}
	return XST_SUCCESS;
}

int Xil_IICReadpolled(XIicPs *Config, u8 *RecvBuffer, u16 offset, s32 readsize,
		      u8 i2c_addrs)
{
	int Status;
	u8 SendData[2];

	SendData[0] = offset & 0xFF; //C5

	xil_printf("read Data:slaveaddr:0x%x,readsize:%d\n", i2c_addrs, readsize);
	xil_printf("offset:0x%x\n", offset);
	Status = XIicPs_MasterSendPolled(Config, SendData, 1, i2c_addrs);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(Config)) {
		/* NOP */
	}

	Status = XIicPs_MasterRecvPolled(Config, RecvBuffer, readsize, i2c_addrs);
	if (Status != XST_SUCCESS) {
		xil_printf("XIicPs_MasterRecvPolled failed:\n");
		return XST_FAILURE;
	}
	xil_printf("wait for data\n");
	while (XIicPs_BusIsBusy(Config)) {
		/* NOP */
	}

	for (int Index = 0; Index < readsize; Index++) {

		/* Aardvark as slave can only set 64 bytes for output */
		xil_printf("RECAPI:recv buffer 0x%x \n", RecvBuffer[Index]);
	}
	return Status;

}

int Xil_IICReadpolled_16bit(XIicPs *Config, u8 *RecvBuffer, u16 offset,
			    s32 readsize, u8 i2c_addrs)
{
	int Status;
	u8 SendData[2];

	SendData[0] = (offset >> 8) & 0xFF; //2
	SendData[1] = offset & 0xFF; //C5

	xil_printf("read Data:slaveaddr:0x%x,readsize:%d\n", i2c_addrs, readsize);
	xil_printf("offset:0x%x\n", offset);
	Status = XIicPs_MasterSendPolled(Config, SendData, 2, i2c_addrs);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(Config)) {
		/* NOP */
	}

	Status = XIicPs_MasterRecvPolled(Config, RecvBuffer, readsize, i2c_addrs);
	if (Status != XST_SUCCESS) {
		xil_printf("XIicPs_MasterRecvPolled failed:\n");
		return XST_FAILURE;
	}
	xil_printf("wait for data\n");
	while (XIicPs_BusIsBusy(Config)) {
		/* NOP */
	}

	for (int Index = 0; Index < readsize; Index++) {

		/* Aardvark as slave can only set 64 bytes for output */
		xil_printf("RECAPI:recv buffer 0x%x \n", RecvBuffer[Index]);

	}
	return Status;

}

int IicPsInit(u16 DeviceId)
{
	int Status;
	XIicPs_Config *Config;

	/*
	 * Initialize the IIC driver so that it's ready to use
	 * Look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XIicPs_LookupConfig(DeviceId);
	if (NULL == Config)
		return XST_FAILURE;
	xil_printf("Config->BaseAddress:0x%x\n", Config->BaseAddress);
	Status = XIicPs_CfgInitialize(&g_Iic[DeviceId], Config,
				      Config->BaseAddress);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;
	xil_printf("instance in PSinit:0x%x\n", g_Iic[DeviceId]);
	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XIicPs_SelfTest(&g_Iic[DeviceId]);
	if (Status != XST_SUCCESS) {
		xil_printf("XIicps selftest failed\r\n");
		return XST_FAILURE;
	}

	/*
	 * Set the IIC serial clock rate.
	 */

	XIicPs_SetSClk(&g_Iic[DeviceId], IIC_SCLK_RATE);

	return XST_SUCCESS;
}

static s32 MuxInitChannel(XIicPs IicInstance, u16 MuxIicAddr, u8 WriteBuffer)
{
	u8 Buffer = 0;
	s32 Status = 0;
	/*
	 * 	 * Wait until bus is idle to start another transfer.
	 * 	 	 */
	while (XIicPs_BusIsBusy(&IicInstance))
		;

	/*
	 * 	 * Send the Data.
	 * 	 	 */
	Status = XIicPs_MasterSendPolled(&IicInstance, &WriteBuffer, 1, MuxIicAddr);
	if (Status != XST_SUCCESS) {
		xil_printf("failed to send data to MUX!!\r\n");
		return XST_FAILURE;
	}

	/*
	 * 	 * Wait until bus is idle to start another transfer.
	 * 	 	 */
	while (XIicPs_BusIsBusy(&IicInstance))
		;

	/*
	 * 	 * Receive the Data.
	 * 	 	 */
	Status = XIicPs_MasterRecvPolled(&IicInstance, &Buffer, 1, MuxIicAddr);
	if (Status != XST_SUCCESS) {
		xil_printf("Fail to read data mux\r\n");
		return XST_FAILURE;
	} else
		xil_printf("Read data from Mux:0x%x\r\n:", Buffer);

	/*
	 * 	 * Wait until bus is idle to start another transfer.
	 * 	 	 */
	while (XIicPs_BusIsBusy(&IicInstance))
		;

	return XST_SUCCESS;
}

int i2cPs_write32(XIicPs *iic_instance, u8 chipAddress, u32 addr, u32 data)
{
	u8 outbuf[8];
	int Status;

	outbuf[0] = addr >> 24;
	outbuf[1] = (addr >> 16) & 0xff;
	outbuf[2] = (addr >> 8) & 0xff;
	outbuf[3] = addr & 0xff;
	/* fixme endian */
	outbuf[4] = (data >> 24) & 0xff;
	outbuf[5] = (data >> 16) & 0xff;
	outbuf[6] = (data >> 8) & 0xff;
	outbuf[7] = (data >> 0) & 0xff;

	Status = XIicPs_MasterSendPolled(iic_instance, outbuf, 4 + 4, chipAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed writepolled iic\r\n");
		return XST_FAILURE;
	}

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(iic_instance)) {
		/* NOP */
	}
	return XST_SUCCESS;
}

RESULT Xil_psI2c_HalI2cInit(uint8_t bus_num)
{
	int Status = XST_SUCCESS;

	u16 i2c_slv_addr_mux = 0x74;

	u8 read_data[2] = { 0, 0 };

	Status = IicPsInit(IIC_DEVICE_ID);

	if (Status == XST_SUCCESS)
		xil_printf("IICPS init done\r\n");

	else
		xil_printf("IICPS init failed\r\n");

	/* To init FMC channel on the I2C mux */

	Status = MuxInitChannel(g_Iic[IIC_DEVICE_ID], i2c_slv_addr_mux,
				0x6);
	if (Status == XST_SUCCESS)
		xil_printf("succesfully Init Mux\r\n");

	xil_printf("Muxer Data Read is:[0]:0x%x\n", read_data[0]);

	return Status;
}

RESULT Xil_psI2c_HalI2cReadReg(u8 bus_num, u8 slave_addr, u32 reg_address,
			       u8 reg_addr_size, void *preg_value, u8 datacount)
{
	int status;
	XIicPs *iic_instance = &(g_Iic[bus_num]);
	xil_printf("Inside hal read i2c register \r\n");

	if (reg_addr_size == 1)
		status = Xil_IICReadpolled(iic_instance, preg_value, reg_addr_size, reg_address, slave_addr);
	else if (reg_addr_size == 2) {
		xil_printf("reg addr size is 2:0x%x\r\n", reg_address);
		status = Xil_IICReadpolled_16bit(iic_instance, preg_value, reg_addr_size, reg_address, slave_addr);
	} else
		xil_printf("will support only reg_addr_size = 1 and 2\n");
	return status;
}

RESULT Xil_psI2c_HalI2cWriteReg(u8 bus_num, u8 slave_addr, u16 reg_address, u8 reg_addr_size,
				u8 reg_value, u32 databytes)
{
	int status;
	XIicPs *iic_instance = &(g_Iic[bus_num]);
	xil_printf("Inside hal write i2c register \r\n");
	if (reg_addr_size == 1)
		status = Xil_IICWritepolled(iic_instance, &reg_value, reg_addr_size, reg_address, slave_addr);
	else if (reg_addr_size == 2) {
		xil_printf("reg addr size is 2:0x%x\r\n", reg_address);
		status = Xil_IICWritepolled16(iic_instance, &reg_value, reg_addr_size, reg_address, slave_addr);
	} else
		xil_printf("will support only reg_addr_size = 1 and 2\n");

	return status;
}

static HalI2cApiOps_t HalPsI2cApiOps = {
	.pHalI2cInit = Xil_psI2c_HalI2cInit,
	// .I2cDeInit        = Xil_psI2c_HalI2cDeInit,
	.pHalI2cReadReg = Xil_psI2c_HalI2cReadReg,
	.pHalI2cWriteReg = Xil_psI2c_HalI2cWriteReg,
};

HalI2cApiOps_t *HalPsI2cGetOps(void)
{
	return &HalPsI2cApiOps;
}
