// Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
#include <max9296.h>
#include "dct_assert.h"
//#include "cpu_info.h"
#if defined (AMP_COMMON_I2C_CONTROLLER) || defined (AMP_MULTIPLE_I2C_CONTROLLER)
	#include "spinlock_address_map.h"
	extern u32 *FMC_INIT_STATUS_REG;
	extern u32 *SHARED_DES_ARRAY_SRUCT;
#endif

extern struct serializer_driver max9295_instance[];
extern struct sensor_driver sensor_instance[];

void Remapping_I2C_addressess(desInterface *);

#define NUM_INTERATION 3

IsiFmc_t g_fmc_single = {
	.FmcName = "xylon_fmc",
	.pIsiCreateFmcIss = xylon_IsiCreateFmcIss,
	.pIsiIsiFmcSetup = xylon_Fmc_Setup,
	.pIsiDeserSetup = xylon_Deser_setup,
	.pIsiDeserEnable = xylon_Deser_Enable,
	.pIsiDeserDisable = xylon_Deser_Disable,
	.serializer_array = {
		&max9295_instance[IN_PIPE_0],
		&max9295_instance[IN_PIPE_1],
		&max9295_instance[IN_PIPE_2],
		&max9295_instance[IN_PIPE_3],
		&max9295_instance[IN_PIPE_4],
		&max9295_instance[IN_PIPE_5],
		&max9295_instance[IN_PIPE_6],
		&max9295_instance[IN_PIPE_7],
		&max9295_instance[IN_PIPE_8],
		&max9295_instance[IN_PIPE_9],
		&max9295_instance[IN_PIPE_10],
		&max9295_instance[IN_PIPE_11],
		&max9295_instance[IN_PIPE_12]
	},
	.sensor_array = {
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	.accessiic_array = {
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
};

static volatile u32 fmcinitDone = 0;

desInterface des_arr[DS_MAX] = {
	{
		DS1_DEFAULT_ADDRESS,
		DS1_ALIAS_ADDRESS,
		PDB_DES_DES1,
		in_deinit,
		NO_LINK,
		{/*Link A*/SERIALIZER_DEFAULT_ADDR, SERIALIZER_0_ALIAS_ADDR, SENSOR_DEFAULT_ADDRSS, SENSOR_0_ALIAS_ADDR}, //mipi-0
		{/*Link B*/SERIALIZER_DEFAULT_ADDR, SERIALIZER_1_ALIAS_ADDR, SENSOR_DEFAULT_ADDRSS, SENSOR_1_ALIAS_ADDR}  //mipi-1
	},
	{
		DS2_DEFAULT_ADDRESS,
		DS2_ALIAS_ADDRESS,
		PDB_DES_DES2,
		in_deinit,
		NO_LINK,//LINK_REVERSE_SPLITTER,//,//LINK_REVERSE_SPLITTER,	 //NO_LINK, /*LINK_A*/
		{/*Link A*/SERIALIZER_DEFAULT_ADDR, SERIALIZER_2_ALIAS_ADDR, SENSOR_DEFAULT_ADDRSS, SENSOR_2_ALIAS_ADDR}, //mipi-2
		{/*Link B*/SERIALIZER_DEFAULT_ADDR, SERIALIZER_3_ALIAS_ADDR, SENSOR_DEFAULT_ADDRSS, SENSOR_3_ALIAS_ADDR}  //mipi-3
	},
	{
		DS3_DEFAULT_ADDRESS,
		DS3_ALIAS_ADDRESS,
		PDB_DES_DES3,
		in_deinit,
		NO_LINK,//NO_LINK,//NO_LINK,//LINK_B/*NO_LINK*/,
		{/*Link A*/SERIALIZER_DEFAULT_ADDR, SERIALIZER_4_ALIAS_ADDR, SENSOR_DEFAULT_ADDRSS, SENSOR_4_ALIAS_ADDR}, //mipi-4
		{/*Link B*/SERIALIZER_DEFAULT_ADDR, SERIALIZER_5_ALIAS_ADDR, SENSOR_DEFAULT_ADDRSS, SENSOR_5_ALIAS_ADDR}  //mipi-5
	},
	{
		DS4_DEFAULT_ADDRESS,
		DS4_ALIAS_ADDRESS,
		PDB_DES_DES4,
		in_deinit,
		NO_LINK,
		{/*Link A*/SERIALIZER_DEFAULT_ADDR, SERIALIZER_6_ALIAS_ADDR, SENSOR_DEFAULT_ADDRSS, SENSOR_6_ALIAS_ADDR}, //mipi-6
		{/*Link B*/SERIALIZER_DEFAULT_ADDR, SERIALIZER_7_ALIAS_ADDR, SENSOR_DEFAULT_ADDRSS, SENSOR_7_ALIAS_ADDR}  //mipi-7
	},
	{
		DS5_DEFAULT_ADDRESS,
		DS5_ALIAS_ADDRESS,
		PDB_DES_DES5,
		in_deinit,
		NO_LINK,
		{/*Link A*/SERIALIZER_DEFAULT_ADDR, SERIALIZER_8_ALIAS_ADDR, SENSOR_DEFAULT_ADDRSS, SENSOR_8_ALIAS_ADDR}, //mipi-8
		{/*Link B*/SERIALIZER_DEFAULT_ADDR, SERIALIZER_9_ALIAS_ADDR, SENSOR_DEFAULT_ADDRSS, SENSOR_9_ALIAS_ADDR}  //mipi-9
	},
	{
		DS6_DEFAULT_ADDRESS,
		DS6_ALIAS_ADDRESS,
		PDB_DES_DES6,
		in_deinit,
		NO_LINK,
		{/*Link A*/SERIALIZER_DEFAULT_ADDR, SERIALIZER_10_ALIAS_ADDR, SENSOR_DEFAULT_ADDRSS, SENSOR_10_ALIAS_ADDR}, //mipi-10
		{/*Link B*/SERIALIZER_DEFAULT_ADDR, SERIALIZER_11_ALIAS_ADDR, SENSOR_DEFAULT_ADDRSS, SENSOR_11_ALIAS_ADDR}  //mipi-11
	},
	{
		DS2_DEFAULT_ADDRESS,
		DS2_ALIAS_ADDRESS,
		PDB_DES_DES7,
		in_deinit,
		NO_LINK,//LINK_REVERSE_SPLITTER,//,//LINK_REVERSE_SPLITTER,	 //NO_LINK, /*LINK_A*/
		{/*Link A*/SERIALIZER_DEFAULT_ADDR, SERIALIZER_2_ALIAS_ADDR, SENSOR_OX5B_ADDRESS, SENSOR_2_ALIAS_ADDR}, //mipi-2
		{/*Link B*/SERIALIZER_DEFAULT_ADDR, SERIALIZER_3_ALIAS_ADDR, SENSOR_DEFAULT_ADDRSS, SENSOR_3_ALIAS_ADDR}  //mipi-3
	},
};

void on_board_topology()
{
	u8 i = 0;

	xil_printf("\n\r	On-Board Sensor Status==>");
	xil_printf("\n\r	De-serializer	Sensor on link-a  Sensor on link-b \n\r ");

	for (i = 0; i < DS_MAX; i++) {
		if (des_arr[i].link_type == LINK_REVERSE_SPLITTER)
			xil_printf("\n\r	%d				Y  					Y \n\r ", i + 1);

		else if (des_arr[i].link_type == NO_LINK)
			xil_printf("\n\r	%d				N  					N \n\r ", i + 1);

		else if (des_arr[i].link_type == LINK_A)
			xil_printf("\n\r	%d				Y  					N \n\r ", i + 1);

		else
			xil_printf("\n\r	%d				N  					Y \n\r ", i + 1);
	}
}

/****************************************************************************
 * @brief	xylon_IsiCreateFmcIss
 *
 * @return	None.
 *
 ****************************************************************************/
static int xylon_IsiCreateFmcIss()
{
	int result = 0;
	Xylon_Context_t *pXylonCtx = 0;

	if (!pXylonCtx)
		return -2;
	memset(pXylonCtx, 0, sizeof(Xylon_Context_t));
#if 0
	pXylonCtx->num_of_serailizer = pConfig->num_of_serailizer;
	pXylonCtx->num_of_deserializer = pConfig->num_of_deserializer;
	pXylonCtx->SlaveAddress = pConfig->SlaveAddr;
#endif
	return (result);
}

/****************************************************************************
 * @brief	xylon_Fmc_Setup
 *
 * @return	.
 *
 ****************************************************************************/
int xylon_Fmc_Setup(int des_arr_id)
{
	int Status = XST_SUCCESS;

	u16 expander_addr = 0;
	u32 register_addr = 0;
	u16 bytes_read = 1;
	u8 read_data[2] = {0};
	u8 wr_data[2];
	const TickType_t xDelay = 500; // portTICK_PERIOD_MS;


#if defined (AMP_COMMON_I2C_CONTROLLER) || defined (AMP_MULTIPLE_I2C_CONTROLLER)
	if (fmcinitDone == 0 && get_cpu_id() == CORE_0) {
#else
	if (fmcinitDone == 0) {
#endif*/
		xil_printf("********************************************************\r\n");
		xil_printf("FMC Init Started...\r\n");

		//voltage adjustment
		register_addr = 0x0;
		u8 potent_addr = 0x2f;

		wr_data[0] = 0x04;
		Status = HalWriteI2CReg(IIC_DEVICE_ID, potent_addr, register_addr, 0x1, wr_data[0], 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		//usleep(100);
		vTaskDelay(xDelay);

		Status = HalReadI2CReg(IIC_DEVICE_ID, potent_addr, register_addr, 0x1, read_data, bytes_read);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

#if defined (READ_I2C_REG)
		xil_printf("potentiometer : Data Read is:[0]:0x%x\r\n", read_data[0]);
#endif
		//ignore LDAC
		register_addr = 0x60;
		u16 ldac_addr = 0x4c;
		wr_data[0] = 0xff;

		Status = HalWriteI2CReg(IIC_DEVICE_ID, ldac_addr, register_addr, 0x1, wr_data[0], 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		Status = HalReadI2CReg(IIC_DEVICE_ID, ldac_addr, register_addr, 0x1, read_data, bytes_read);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

#if defined (READ_I2C_REG)
		xil_printf("ldac : Data Read is:[0]:0x%x\r\n", read_data[0]);
#endif

		//DAC 6Gbps
		register_addr = 0x3f;
		wr_data[0] = 0x00;

		Status = HalWriteI2CReg(IIC_DEVICE_ID, ldac_addr, register_addr, 0x1, wr_data[0], 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		Status = HalReadI2CReg(IIC_DEVICE_ID, ldac_addr, register_addr, 0x1, read_data, bytes_read);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

#if defined (READ_I2C_REG)
		xil_printf("DAC mode 6Gbps done : \r\n");
		xil_printf("Data Read is:[0]:0x%x\r\n", read_data[0]);
#endif
		usleep(500 * 1000);

		//set expander pins as output
		/*
		 * Move below call out
		 * Pre_serdes_config
		 */
		/*
		 * Set PortB  expander pins to output
		 *
		 */

		register_addr = 0x1;
		expander_addr = 0x40 >> 1;
		wr_data[0] = 0x00;

		/*
		 * Read the Direction register and modify specific bit
		 */
		Status = HalWriteI2CReg(IIC_DEVICE_ID, expander_addr, register_addr, 0x1, wr_data[0], 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		usleep(800 * 1000);

#if defined (READ_I2C_REG)
		Status = HalReadI2CReg(IIC_DEVICE_ID, expander_addr, register_addr, 0x1, read_data, bytes_read);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		xil_printf("expander_Reg :IODIRB is:[0]:0x%x\r\n", read_data[0]);
		xil_printf("expander_addr : Data Read is:[0]:0x%x\r\n", read_data[0]);
#endif
		/*
		 * Enable CAM_SUPLY_EN by setting it to 0
		 *
		 */
		register_addr = 0x15;

		/*
		 * Switch off CAM_SUPPLY_EN
		 */
		Status = HalReadI2CReg(IIC_DEVICE_ID, expander_addr, register_addr, 0x1, read_data, bytes_read);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		wr_data[0] = read_data[0] | ((1 << CAM_SUPPLY_EN));
		//xil_printf("expander : OutB: read_data 0x%x  wr_data[0]=%x \n",read_data[0],wr_data[0]);
		Status = HalWriteI2CReg(IIC_DEVICE_ID, expander_addr, register_addr, 0x1, wr_data[0], 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		sleep(1);
		sleep(1);
		sleep(1);
		sleep(1);

		Status = HalReadI2CReg(IIC_DEVICE_ID, expander_addr, register_addr, 0x1, read_data, bytes_read);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		wr_data[0] = read_data[0] & (~(1 << CAM_SUPPLY_EN));
		//xil_printf("expander : OutB: read_data 0x%x  wr_data[0]=%x \n",read_data[0],wr_data[0]);
		Status = HalWriteI2CReg(IIC_DEVICE_ID, expander_addr, register_addr, 0x1, wr_data[0], 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		sleep(1);

		Status = HalReadI2CReg(IIC_DEVICE_ID, expander_addr, register_addr, 0x1, read_data, bytes_read);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		//xil_printf("expander: OutB:0x%x\n",read_data[0]);
		/*
		 * Here we change I2c address for DS2,4 & 6
		 */

		/*
		 * Here we re-map
		 * I2c address for DS2,4 & 6
		 * I2c Addresses of all serializer and sensor
		 */

		xylon_Deser_Disable(PDB_DES_DES1);
		xylon_Deser_Disable(PDB_DES_DES2);
		xylon_Deser_Disable(PDB_DES_DES3);
		xylon_Deser_Disable(PDB_DES_DES4);
		xylon_Deser_Disable(PDB_DES_DES5);
		xylon_Deser_Disable(PDB_DES_DES6);

		xil_printf("Remapping I2c Address on FMC started...\r\n");
#if 0
		Remapping_I2C_addressess(&des_arr[DS_ONE]);
		Remapping_I2C_addressess(&des_arr[DS_TWO]);
		Remapping_I2C_addressess(&des_arr[DS_THREE]);
		Remapping_I2C_addressess(&des_arr[DS_FOUR]);
		Remapping_I2C_addressess(&des_arr[DS_FIVE]);
		Remapping_I2C_addressess(&des_arr[DS_SIX]);
		Remapping_I2C_addressess(&des_arr[DS_SEVEN]);
		xil_printf("Remapping I2c Address on FMC Done...\r\n");
#endif

		fmcinitDone = 1;
#if defined (AMP_COMMON_I2C_CONTROLLER) || defined(AMP_MULTIPLE_I2C_CONTROLLER)

		/*
		 * Make FMC INIT Status to 1
		 */

		u32 *ptr;
		ptr = (u32*)(FMC_INIT_STATUS_REG);
		*ptr = SH_VAR_INIT;

		/* Copy des_arry structure to Shared memory
		 *  May be  __attribute__ ((aligned (4))) ,alginment required for des_arr
		 *  As des_arr is being copied to Non-Cachable region
		 * Need to check
		 *
		 */

		/*
		 * To make things simple for first iteration ,changes done here are based on following assumption.
		 *
		 * FMC init and Re-mapping of Alias address of De-serializer ,Serializer and sensor will be done
		 * by Core 0 code.
		 * Once this stuff is done by core 0 , it writes this structure inti  shared memory region.
		 */
		byte_memcpy(SHARED_DES_ARRAY_SRUCT, des_arr, sizeof(des_arr));

		/*
		 * Secondary Cores, on there boot-up sequence skips the FMC init Re-mapping of Alias address of De-serializer ,Serializer and sensor
		 * But as this info is required by them at runtime .
		 * So Secondary core [1-n] copies this info from the Shared memory and update there des_arr structure.
		 *  This Copying is happening in else of part
		 */
#endif

		xil_printf("FMC Init Done\r\n");
		xil_printf("********************************************************\r\n");

		on_board_topology();
	} else {

#if defined (AMP_COMMON_I2C_CONTROLLER) || defined(AMP_MULTIPLE_I2C_CONTROLLER)
		if (get_cpu_id() == CORE_0)
			xil_printf("FMC Already Init Done by Core-%x \r\n", get_cpu_id());
		else {

			if (fmcinitDone == 0) {
				/*
				 * fmcinitDone check is required as this prevent re-loading of des_arr
				 * on Secondary cores .
				 */
				u32 *ptr;
				ptr = (u32*)(FMC_INIT_STATUS_REG);
				*ptr = SH_VAR_INIT;

				while (*ptr == SH_VAR_UNINIT);

				byte_memcpy(des_arr, SHARED_DES_ARRAY_SRUCT, sizeof(des_arr));
				fmcinitDone = 1;
				xil_printf("Des-arr structure loaded for Core-%x \r\n", get_cpu_id());
				on_board_topology();
			} else
				xil_printf("Des-arr structure Already loaded for Core-%x \r\n", get_cpu_id());
		}
#else
		xil_printf("FMC Already Init...\r\n");
#endif
	}

	Remapping_I2C_addressess(&des_arr[des_arr_id]);

	return Status;
}

void enable_link(u8 Deser_addr, u8 link_type)
{
	int Status = XST_SUCCESS;

	const TickType_t xDelay = 500; // portTICK_PERIOD_MS;
	u8 read_data[2] = {0};
	u8 wr_data[2];
	u16 reg_addr;

	reg_addr = CTRL0_REG;
	Status = HalReadI2CReg(IIC_DEVICE_ID, Deser_addr, reg_addr, 0x2, read_data, 1);
	if (Status != XST_SUCCESS)
		DCT_ASSERT(0);

	/*Disable Auto-Link*/
	read_data[0] = read_data[0] & (~(1 << AUTO_LINK));

	/*Select Link*/
	read_data[0] = read_data[0] & (~(LINK_MASK));
	read_data[0] = read_data[0] | (link_type);

	wr_data[0] = read_data[0];

	Status = HalWriteI2CReg(IIC_DEVICE_ID, Deser_addr, reg_addr, 0x2, wr_data[0], 1);
	if (Status != XST_SUCCESS)
		DCT_ASSERT(0);

	Status = HalReadI2CReg(IIC_DEVICE_ID, Deser_addr, reg_addr, 0x2, read_data, 1);
	if (Status != XST_SUCCESS)
		DCT_ASSERT(0);

#if defined (READ_I2C_REG)
	xil_printf("CTRL0 =%x \n\r", read_data[0]);
#endif
	/*
	 * Do Reset
	 */
	read_data[0] = read_data[0] | (1 << RESET_ONE_SHOT);
	wr_data[0] = read_data[0];

	Status = HalWriteI2CReg(IIC_DEVICE_ID, Deser_addr, reg_addr, 0x2, wr_data[0], 1);
	if (Status != XST_SUCCESS)
		DCT_ASSERT(0);
}

/*****************************************************************************/
/**
* @brief
* This function Probes the serializers /sensor connected on board
*
* @param
* @param
* @param
* @param
*
* @return	None.
*
* @note
*
 ****************************************************************************/
void probe_if_links(u8 Deser_addr, desInterface *desIface)
{
	int Status = XST_SUCCESS;

	u8 read_data[2] = {0};
	const TickType_t xDelay2 = 50; // portTICK_PERIOD_MS;
	u16 reg_addr;

	u8 dev_a = FALSE;
	u8 dev_b = FALSE;

	/*
	 * Enable link A
	 * Apply re-shot
	 * Wait for lock if no lock after sometime,
	 * its clear no device present
	 */
	u8 itr_num = 1;
	enable_link(Deser_addr, LINK_A);

	while (1) {
#if defined (READ_I2C_REG)
		xil_printf("\n\rProbing sensor on link-A of Des%d", desIface->Port_DES_index + 1);
#endif
		vTaskDelay(xDelay2);

		reg_addr = CTRL3_REG;
		Status = HalReadI2CReg(IIC_DEVICE_ID, Deser_addr, reg_addr, 0x2, read_data, 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

#if defined (READ_I2C_REG)
		xil_printf("CTRL3_REG =%x \n\r", read_data[0]);
#endif
		if ((read_data[0] & (1 << LOCKED)) == (1 << LOCKED)) {
			dev_a = true;
			break;
		}

		if (itr_num == NUM_INTERATION)
			break;

		itr_num++;
	}

	/*
	 * Enable link B
	 * Apply re-shot
	 * Wait for lock if no lock after sometime,
	 * its clear no device present
	 */
	itr_num = 1;
	enable_link(Deser_addr, LINK_B);

	while (1) {
#if defined (READ_I2C_REG)
		xil_printf("\n\rProbing sensor on link-B of Des%d", desIface->Port_DES_index + 1);
#endif
		vTaskDelay(xDelay2);

		reg_addr = CTRL3_REG;
		Status = HalReadI2CReg(IIC_DEVICE_ID, Deser_addr, reg_addr, 0x2, read_data, 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

#if defined (READ_I2C_REG)
		xil_printf("CTRL3_REG =%x \n\r", read_data[0]);
#endif
		if ((read_data[0] & (1 << LOCKED)) == (1 << LOCKED)) {
			dev_b = TRUE;
			break;
		}

		if (itr_num == NUM_INTERATION)
			break;

		itr_num++;
	}

	if (dev_a == TRUE && dev_b == TRUE) {
		/*
		 * Sensor on both the link
		 * Enable Reverse splitter mode
		 */
		desIface->link_type = LINK_REVERSE_SPLITTER;
	} else if (dev_a == FALSE && dev_b == FALSE) {
		/*
		 * Sensor not present on any link
		 * NO_LINK
		 */
		desIface->link_type = NO_LINK;
	} else if (dev_a == TRUE && dev_b == FALSE) {
		/*
		 * Sensor present only on link-a
		 *
		 */
		desIface->link_type = LINK_A;
	} else {
		/*
		 * Sensor present only on link-b
		 *
		 */
		desIface->link_type = LINK_B;
	}
#if defined (READ_I2C_REG)
	xil_printf("\n\r");
#endif
}

/*****************************************************************************/
/**
* @brief
* This function Remaps Default I2C device addresses  to new Virtual address
* for De-Serializer ,Serializer and sensor as defined in isi.h
*
* @param
* @param
* @param
* @param
*
* @return	None.
*
* @note
*
 ****************************************************************************/
void Remapping_I2C_addressess(desInterface *desIface)
{
	int Status = XST_SUCCESS;
	u16 expander_addr = 0;
	u32 register_addr = 0;
	u16 bytes_read = 1;
	u8 read_data[2] = {0};
	const TickType_t xDelay = 500; // portTICK_PERIOD_MS;

	u8 wr_data[2];
	u8 Deser_addr;
	u16 reg_addr;
	u8 ser_addr;
	const TickType_t xDelay2 = 100;
	u8 idx;
	dslink *link_ptr;

	Deser_addr = (desIface->des_actual_addr) >> 1 ; //des_addr>>1;

	if (desIface->Port_DES_index == PDB_DES_DES7)
		xylon_Deser_Enable(PDB_DES_DES2);

	else
		xylon_Deser_Enable(desIface->Port_DES_index);

	/*
	 * Below code only for DES1 .DES 3 & DES 5
	 *
	 *
	 */
	if (desIface->Port_DES_index == 0 || desIface->Port_DES_index == 2
	    || desIface->Port_DES_index == 4) {

		//Program Reg0 with Virtual Address
		reg_addr = DEV_ADDR_REG;
		wr_data[0] = desIface->des_alias_addr;
		Status = HalWriteI2CReg(IIC_DEVICE_ID, Deser_addr, reg_addr, 0x2, wr_data[0], 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		//Load Deser_addr
		Deser_addr = (desIface->des_alias_addr) >> 1;

		Status = HalReadI2CReg(IIC_DEVICE_ID, Deser_addr, reg_addr, 0x2, read_data, 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		if (desIface->des_alias_addr != (read_data[0] & 0xFE)) {
			xil_printf("Mismatch in Address set desIface->des_alias_addr-[0x%x] != actual device address-[0x%x] \r\n",
				   desIface->des_alias_addr, read_data[0]);
			DCT_ASSERT(0);
		}
	}

	probe_if_links(Deser_addr, desIface);

	if (desIface->link_type == NO_LINK) {
#if defined (READ_I2C_REG)
		xil_printf("No Sensor Available on link a/B on Deserializer-[0x%x] \n\r so not re-mapping Address of Serializer and Sensor\r\n",
			   desIface->des_alias_addr);
#endif
		if (desIface->Port_DES_index == PDB_DES_DES7)
			xylon_Deser_Disable(PDB_DES_DES2);

		else
			xylon_Deser_Disable(desIface->Port_DES_index);

		return;
	}
#if defined (READ_I2C_REG)
	xil_printf("Re-mapping Serializer & Sensor Addresses at Deserializer-[0x%x]\r\n",
		   desIface->des_alias_addr);
#endif
#if 1
	if (desIface->link_type == LINK_REVERSE_SPLITTER) {

		xil_printf("Configure De-serializer in reverse splitter mode \r\n");
#if defined (READ_I2C_REG)
		xil_printf("Configure Link-A in reverse splitter mode \r\n");
#endif
		link_ptr = & (desIface->link_a);
		Deser_addr = (desIface->des_alias_addr) >> 1;
		reg_addr = 0x10;

		Status = HalReadI2CReg(IIC_DEVICE_ID, Deser_addr, reg_addr, 0x2, read_data, 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		/*Disable Auto-Link*/
		read_data[0] = read_data[0] & (~(1 << AUTO_LINK));
		/*Select LinkA*/
		read_data[0] = read_data[0] & (~(LINK_MASK));
		read_data[0] = read_data[0] | LINK_A; //LINK_DUAL;//LINK_A;
		wr_data[0] = read_data[0];

		Status = HalWriteI2CReg(IIC_DEVICE_ID, Deser_addr, reg_addr, 0x2, wr_data[0], 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		Status = HalReadI2CReg(IIC_DEVICE_ID, Deser_addr, reg_addr, 0x2, read_data, 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		/*
		 * Do Reset
		 */
		read_data[0] = read_data[0] | (1 << RESET_ONE_SHOT);
		wr_data[0] = read_data[0];

		Status = HalWriteI2CReg(IIC_DEVICE_ID, Deser_addr, reg_addr, 0x2, wr_data[0], 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		/*
		 * Poll the Link or add delay
		 */
		while (1) {
			xil_printf("Poll for Lock...on link-A \r\n");
			vTaskDelay(xDelay2);
			reg_addr = CTRL3_REG;

			Status = HalReadI2CReg(IIC_DEVICE_ID, Deser_addr, reg_addr, 0x2, read_data, 1);
			if (Status != XST_SUCCESS)
				DCT_ASSERT(0);

			xil_printf("CTRL3_REG =%x \r\n", read_data[0]);
			if ((read_data[0] & (1 << LOCKED)) == (1 << LOCKED))
				break;
		}

		ser_addr = (link_ptr->serializer_default_addr) >> 1; //SERIALIZER_DEFAULT_ADDR)>>1;
		reg_addr = 0x00;
		wr_data[0] = link_ptr->serializer_alias_addr;// //SERIALIZER_3_ALIAS_ADDR;

		Status = HalReadI2CReg(IIC_DEVICE_ID, ser_addr, reg_addr, 0x2, read_data, 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		Status = HalWriteI2CReg(IIC_DEVICE_ID, ser_addr, reg_addr, 0x2, wr_data[0], 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		xil_printf("Serializer on link-A of Deserializer-[0x%x] is configured to Virtual Address-[0x%x]\r\n",
			   desIface->des_alias_addr, link_ptr->serializer_alias_addr);

		/*
		 * Modifying sensor Address
		 */
		ser_addr = (link_ptr->serializer_alias_addr) >> 1 ;
		reg_addr = 0x00;

		Status = HalReadI2CReg(IIC_DEVICE_ID, ser_addr, reg_addr, 0x2, read_data, 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

#if defined (READ_I2C_REG)
		xil_printf(" Just re verifying  serializer new addr :%x\r\n", read_data[0]);
#endif
		reg_addr = 0x7B;
		wr_data[0] = 0x11;

		Status = HalWriteI2CReg(IIC_DEVICE_ID, ser_addr, reg_addr, 0x2, wr_data[0], 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		reg_addr = 0x83;
		wr_data[0] = 0x11; //sensor_alias_addr;//SENSOR_3_ALIAS_ADDR;

		Status = HalWriteI2CReg(IIC_DEVICE_ID, ser_addr, reg_addr, 0x2, wr_data[0], 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		reg_addr = 0x8b;
		wr_data[0] = 0x11; //sensor_alias_addr;//SENSOR_3_ALIAS_ADDR;

		Status = HalWriteI2CReg(IIC_DEVICE_ID, ser_addr, reg_addr, 0x2, wr_data[0], 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		/*
		 * Configuring sensor Virtual Address
		 */
		reg_addr = I2C_2;
		wr_data[0] = link_ptr->sensor_alias_addr; //sensor_alias_addr;//SENSOR_3_ALIAS_ADDR;

		Status = HalWriteI2CReg(IIC_DEVICE_ID, ser_addr, reg_addr, 0x2, wr_data[0], 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		reg_addr = I2C_3;
		wr_data[0] = link_ptr->sensor_default_addr;// sensor_default_addr;//SENSOR_DEFAULT_ADDRSS;

		Status = HalWriteI2CReg(IIC_DEVICE_ID, ser_addr, reg_addr, 0x2, wr_data[0], 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

#if defined (READ_I2C_REG)
		reg_addr = I2C_2;

		Status = HalReadI2CReg(IIC_DEVICE_ID, ser_addr, reg_addr, 0x2, read_data, 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		xil_printf(" I2c_2 :%x\r\n", read_data[0]);

		reg_addr = I2c_3;

		Status = HalReadI2CReg(IIC_DEVICE_ID, ser_addr, reg_addr, 0x2, read_data, 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		xil_printf(" I2c_3 :%x\r\n", read_data[0]);
#endif
		xil_printf("Sensor on Serializer-[0x%x] is configured to sensor Virtual Address-[0x%x]\r\n",
			   link_ptr->serializer_alias_addr, link_ptr->sensor_alias_addr);

		/*
		 * Now configure , Link-B
		 *
		 */
#if defined (READ_I2C_REG)
		xil_printf("Configure Link-B in reverse splitter mode \r\n");
#endif
		link_ptr = & (desIface->link_b);
		Deser_addr = (desIface->des_alias_addr) >> 1;
		reg_addr = 0x10;

		Status = HalReadI2CReg(IIC_DEVICE_ID, Deser_addr, reg_addr, 0x2, read_data, 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		read_data[0] = read_data[0] & (~(1 << AUTO_LINK));
		read_data[0] = read_data[0] & (~(LINK_MASK));
		read_data[0] = read_data[0] | LINK_REVERSE_SPLITTER;; //LINK_DUAL;//LINK_B;//;
		wr_data[0] = read_data[0];

		Status = HalWriteI2CReg(IIC_DEVICE_ID, Deser_addr, reg_addr, 0x2, wr_data[0], 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		Status = HalReadI2CReg(IIC_DEVICE_ID, Deser_addr, reg_addr, 0x2, read_data, 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		/*
		 * Do Reset
		 */
		read_data[0] = read_data[0] | (1 << RESET_ONE_SHOT);
		wr_data[0] = read_data[0];

		Status = HalWriteI2CReg(IIC_DEVICE_ID, Deser_addr, reg_addr, 0x2, wr_data[0], 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		/*
		 * Poll the Link or add delay
		 */
		while (1) {
			xil_printf("Poll for Lock...on link-B \r\n");
			vTaskDelay(xDelay2);
			reg_addr = CTRL3_REG;

			Status = HalReadI2CReg(IIC_DEVICE_ID, Deser_addr, reg_addr, 0x2, read_data, 1);
			if (Status != XST_SUCCESS)
				DCT_ASSERT(0);

			xil_printf("CTRL3_REG =%x \r\n", read_data[0]);

			if ((read_data[0] & (1 << LOCKED)) == (1 << LOCKED))
				break;
		}

		ser_addr = (link_ptr->serializer_default_addr) >> 1; //SERIALIZER_DEFAULT_ADDR)>>1;
		reg_addr = 0x00;
		wr_data[0] = link_ptr->serializer_alias_addr;// //SERIALIZER_3_ALIAS_ADDR;

		Status = HalReadI2CReg(IIC_DEVICE_ID, ser_addr, reg_addr, 0x2, read_data, 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		Status = HalWriteI2CReg(IIC_DEVICE_ID, ser_addr, reg_addr, 0x2, wr_data[0], 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		xil_printf("Serializer on link-B of Deserializer-[0x%x] is configured to Virtual Address-[0x%x]\r\n",
			   desIface->des_alias_addr, link_ptr->serializer_alias_addr);

		/*
		 * Modifying sensor Address
		 */
		ser_addr = (link_ptr->serializer_alias_addr) >> 1 ;
		reg_addr = 0x00;

		Status = HalReadI2CReg(IIC_DEVICE_ID, ser_addr, reg_addr, 0x2, read_data, 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

#if defined (READ_I2C_REG)
		xil_printf(" Just re verifying  serializer new addr:%x\r\n", read_data[0]);
#endif
		reg_addr = I2C_2;
		wr_data[0] = link_ptr->sensor_alias_addr;

		Status = HalWriteI2CReg(IIC_DEVICE_ID, ser_addr, reg_addr, 0x2, wr_data[0], 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		reg_addr = I2C_3;
		wr_data[0] = link_ptr->sensor_default_addr;

		Status = HalWriteI2CReg(IIC_DEVICE_ID, ser_addr, reg_addr, 0x2, wr_data[0], 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

#if defined (READ_I2C_REG)
		reg_addr = I2C_2;

		Status = HalReadI2CReg(IIC_DEVICE_ID, ser_addr, reg_addr, 0x2, read_data, 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		xil_printf(" I2c_2 :%x\r\n", read_data[0]);

		reg_addr = I2c_3;

		Status = HalReadI2CReg(IIC_DEVICE_ID, ser_addr, reg_addr, 0x2, read_data, 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		xil_printf(" I2c_3 :%x\r\n", read_data[0]);
#endif
		xil_printf("Sensor on Serializer-[0x%x] is configured to sensor Virtual Address-[0x%x]\r\n",
			   link_ptr->serializer_alias_addr, link_ptr->sensor_alias_addr);
	} else {
		if (desIface->link_type == LINK_A)
			link_ptr = & (desIface->link_a);

		else
			link_ptr = & (desIface->link_b);

		reg_addr = CTRL0_REG;
		Status = HalReadI2CReg(IIC_DEVICE_ID, Deser_addr, reg_addr, 0x2, read_data, 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		/*Disable Auto-Link*/
		read_data[0] = read_data[0] & (~(1 << AUTO_LINK));

		/*Select Link*/
		read_data[0] = read_data[0] & (~(LINK_MASK));
		read_data[0] = read_data[0] | (desIface->link_type);
		wr_data[0] = read_data[0];

		Status = HalWriteI2CReg(IIC_DEVICE_ID, Deser_addr, reg_addr, 0x2, wr_data[0], 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		Status = HalReadI2CReg(IIC_DEVICE_ID, Deser_addr, reg_addr, 0x2, read_data, 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

#if defined (READ_I2C_REG)
		xil_printf("CTRL0 =%x \r\n", read_data[0]);
#endif
		/*
		 * Do Reset
		 */
		read_data[0] = read_data[0] | (1 << RESET_ONE_SHOT);
		wr_data[0] = read_data[0];

		Status = HalWriteI2CReg(IIC_DEVICE_ID, Deser_addr, reg_addr, 0x2, wr_data[0], 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		/*
		 * Poll the Link or add delay
		 */
		while (1) {
#if defined (READ_I2C_REG)
			xil_printf("Poll for Lock...on link-%c\r\n", (desIface->link_type == LINK_A) ? 'A' : 'B');
#endif

			vTaskDelay(xDelay2);
			reg_addr = CTRL3_REG;

			Status = HalReadI2CReg(IIC_DEVICE_ID, Deser_addr, reg_addr, 0x2, read_data, 1);
			if (Status != XST_SUCCESS)
				DCT_ASSERT(0);
#if defined (READ_I2C_REG)
			xil_printf("CTRL3_REG =%x \r\n", read_data[0]);
#endif
			if ((read_data[0] & (1 << LOCKED)) == (1 << LOCKED))
				break;
		}

		ser_addr = (link_ptr->serializer_default_addr) >>
			   1; //serializer_default_addr>>1;//SERIALIZER_DEFAULT_ADDR>>1;
		reg_addr = 0x00;
		wr_data[0] = link_ptr->serializer_alias_addr; //serializer_alias_addr;//SERIALIZER_3_ALIAS_ADDR;

#if defined (READ_I2C_REG)
		Status = HalReadI2CReg(IIC_DEVICE_ID, ser_addr, reg_addr, 0x2, read_data, 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		xil_printf(" Read Data:%x\r\n", read_data[0]);
#endif
		Status = HalWriteI2CReg(IIC_DEVICE_ID, ser_addr, reg_addr, 0x2, wr_data[0], 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);
#if defined (READ_I2C_REG)
		xil_printf("Serializer on link-%c of Deserializer-[0x%x] is configured to Virtual Address-[0x%x]\r\n",
			   (desIface->link_type == LINK_A) ? 'A' : 'B', desIface->des_alias_addr,
			   link_ptr->serializer_alias_addr);
#endif
		ser_addr = (link_ptr->serializer_alias_addr) >> 1
			   ; //serializer_alias_addr>>1;//SERIALIZER_3_ALIAS_ADDR>>1;

#if defined (READ_I2C_REG)
		reg_addr = 0x00;

		Status = HalReadI2CReg(IIC_DEVICE_ID, ser_addr, reg_addr, 0x2, read_data, 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		xil_printf(" Read Data:%x\r\n", read_data[0]);
#endif
		reg_addr = I2C_2;
		wr_data[0] = link_ptr->sensor_alias_addr; //sensor_alias_addr;//SENSOR_3_ALIAS_ADDR;

		Status = HalWriteI2CReg(IIC_DEVICE_ID, ser_addr, reg_addr, 0x2, wr_data[0], 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		reg_addr = I2C_3;
		wr_data[0] = link_ptr->sensor_default_addr;// sensor_default_addr;//SENSOR_DEFAULT_ADDRSS;

		Status = HalWriteI2CReg(IIC_DEVICE_ID, ser_addr, reg_addr, 0x2, wr_data[0], 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

#if defined (READ_I2C_REG)
		reg_addr = I2C_2;

		Status = HalReadI2CReg(IIC_DEVICE_ID, ser_addr, reg_addr, 0x2, read_data, 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		xil_printf(" I2c_2 :%x\r\n", read_data[0]);

		reg_addr = I2c_3;

		Status = HalReadI2CReg(IIC_DEVICE_ID, ser_addr, reg_addr, 0x2, read_data, 1);
		if (Status != XST_SUCCESS)
			DCT_ASSERT(0);

		xil_printf(" I2c_3 :%x\r\n", read_data[0]);
#endif
#if defined (READ_I2C_REG)
		xil_printf("Sensor on Serializer-[0x%x] is configured to sensor Virtual Address-[0x%x]\r\n",
			   link_ptr->serializer_alias_addr, link_ptr->sensor_alias_addr);
#endif
	}
#endif
	return Status;
}


u32 get_des_array(desInterface *des, RegI2CT **Des_arr)
{
	int len;

	if (des->Port_DES_index == PDB_DES_DES1) {
		*Des_arr = Des1_init;
		len = (sizeof(Des1_init)) / (sizeof(RegI2CT));
	} else if (des->Port_DES_index == PDB_DES_DES2) {
		*Des_arr = Des2_init;
		len = (sizeof(Des2_init)) / (sizeof(RegI2CT));
		//	*Des_arr=Des2_revserse_splitter_init;
		//	len=(sizeof(Des2_revserse_splitter_init))/(sizeof(RegI2CT));
	} else if (des->Port_DES_index == PDB_DES_DES3) {
		*Des_arr = Des3_init;
		len = (sizeof(Des3_init)) / (sizeof(RegI2CT));
	} else if (des->Port_DES_index == PDB_DES_DES4) {
		*Des_arr = Des4_init;
		len = (sizeof(Des4_init)) / (sizeof(RegI2CT));
	} else if (des->Port_DES_index == PDB_DES_DES5) {
		*Des_arr = Des5_init;
		len = (sizeof(Des5_init)) / (sizeof(RegI2CT));
	} else if (des->Port_DES_index == PDB_DES_DES6) {
		*Des_arr = Des6_init;
		len = (sizeof(Des6_init)) / (sizeof(RegI2CT));
	} else if (des->Port_DES_index == PDB_DES_DES7) {
		*Des_arr = Des7_init;
		len = (sizeof(Des7_init)) / (sizeof(RegI2CT));
	} else
		DCT_ASSERT(0);

	return len;
}

/****************************************************************************
 * @brief	xylon_Deser_setup
 *
 * @return	None.
 *
 ****************************************************************************/
static int xylon_Deser_setup(desInterface *des)
{
	int Status = XST_SUCCESS;

	u8 wr_data[4] = {0};
	u8 rd_data[4] = {0};

	const TickType_t xDelay2 = 200;
	u32 len, j;
	RegI2CT *Deserializer_initialization;

	if (des->des_state == in_deinit) {
		/*init deserializer */
		u16 Deser_addr = (des->des_alias_addr) >>
				 1;   /* This valus get passed to this function as config parameter and enable accordingly */

#if !defined (READ_I2C_REG)
		xil_printf("Initializing De-serializer at Virtual Address = 0x%x...", Deser_addr);
#endif
		len = get_des_array(des, &Deserializer_initialization);

		for (j = 0; j < len; j++) {
			if ((Deserializer_initialization + j)->addr == MAX929X_TABLE_END)
				break;

			if ((Deserializer_initialization + j)->addr == MAX929X_TABLE_WAIT) {
				usleep((Deserializer_initialization + j)->val * 1000);
				continue;
			}

			wr_data[0] = ((Deserializer_initialization + j)->val);

			Status = HalWriteI2CReg(IIC_DEVICE_ID, Deser_addr, (Deserializer_initialization + j)->addr, 0x2,
						wr_data[0], 1);
			if (Status != XST_SUCCESS)
				DCT_ASSERT(0);

			Status = HalReadI2CReg(IIC_DEVICE_ID, Deser_addr, (Deserializer_initialization + j)->addr, 0x2,
					       rd_data, 1);
			if (Status != XST_SUCCESS)
				DCT_ASSERT(0);

#if defined (READ_I2C_REG)
			xil_printf("[%s] [%d] Address 0x%x -> value 0x%x. \n", __func__, __LINE__,
				   (Deserializer_initialization + j)->addr, rd_data[0]);
#endif
			if (j == 0) {
				//	vTaskDelay( xDelay2 );
				sleep(5);
			}
		}

		des->des_state = in_init;
		des->des_state = in_running;
#if !defined (READ_I2C_REG)
		xil_printf("\n\rInitialization Done...\n\r");
#endif
	} else
		xil_printf("De-Serializer Already in Running state(%d) \n\r", des->des_state);

	return Status;
}

static int xylon_Deser_Enable(u8 pos)
{
	int Status = XST_SUCCESS;
	u16 expander_addr = 0;
	u32 register_addr = 0;
	u16 bytes_read = 1;
	u8 read_data[2] = {0};
	u8 wr_data[2];

#if defined (READ_I2C_REG)
	xil_printf("Try expander settngs\r\n");
#endif
	expander_addr = 0x40 >> 1;
	wr_data[0] = 0x00;

	/*
	 * No need of setting Direction ,already in FMC_INIT
	 * All ports are set as out
	 *
	 */
	register_addr = 0x15;

	Status = HalReadI2CReg(IIC_DEVICE_ID, expander_addr, register_addr, 0x1, read_data, bytes_read);
	if (Status != XST_SUCCESS)
		DCT_ASSERT(0);

	wr_data[0] = read_data[0] & (~(1 << pos));

	Status = HalWriteI2CReg(IIC_DEVICE_ID, expander_addr, register_addr, 0x1, wr_data[0], 1);
	if (Status != XST_SUCCESS)
		DCT_ASSERT(0);

	sleep(1);
	sleep(1);

	wr_data[0] = read_data[0] | (1 << pos);

	Status = HalWriteI2CReg(IIC_DEVICE_ID, expander_addr, register_addr, 0x1, wr_data[0], 1);
	if (Status != XST_SUCCESS)
		DCT_ASSERT(0);

	sleep(1);

	Status = HalReadI2CReg(IIC_DEVICE_ID, expander_addr, register_addr, 0x1, read_data, bytes_read);
	if (Status != XST_SUCCESS)
		DCT_ASSERT(0);

#if defined (READ_I2C_REG)
	xil_printf("Enabled De-Serializer-%d \n", pos + 1);
#endif
#if defined (READ_I2C_REG)
	xil_printf("expander:Register OutB=0x%x\n", read_data[0]);
#endif
	return 0;
}

static int xylon_Deser_Disable(u8 pos)
{
	int Status = XST_SUCCESS;
	u16 expander_addr = 0;
	u32 register_addr = 0;
	u16 bytes_read = 1;
	u8 read_data[2] = {0};
	u8 wr_data[2];

	expander_addr = 0x40 >> 1;
	wr_data[0] = 0x00;

	/*
	 * No need of setting Direction ,already in FMC_INIT
	 * All ports are set as out
	 *
	 */
	register_addr = 0x15;

	Status = HalReadI2CReg(IIC_DEVICE_ID, expander_addr, register_addr, 0x1, read_data, bytes_read);
	if (Status != XST_SUCCESS)
		DCT_ASSERT(0);

	wr_data[0] = read_data[0] & (~(1 << pos));

	Status = HalWriteI2CReg(IIC_DEVICE_ID, expander_addr, register_addr, 0x1, wr_data[0], 1);
	if (Status != XST_SUCCESS)
		DCT_ASSERT(0);

	sleep(1);

	Status = HalReadI2CReg(IIC_DEVICE_ID, expander_addr, register_addr, 0x1, read_data, bytes_read);
	if (Status != XST_SUCCESS)
		DCT_ASSERT(0);

#if defined (READ_I2C_REG)
	xil_printf("Disabled DeSerializer-%d \r\n", pos + 1);
#endif
	return 0;
}
