// Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
#include <max9295.h>
#include "dct_assert.h"

#define SER_ADDR 0x40

struct serializer_driver max9295_instance[MAX_SER_COUNT] = {
	{
		.name = "max9295-0",
		.link_lane = LINK_A,
		.i2c_addr = 0x40,
		.bus_num = 0x1,
		.alias_addr = 0x0,
		.broadcast_addr = 0x0,
		.enable = 1,
		.alias_en = 0,
		.broadcast_en = 0,
		.ser_state = in_deinit,
		.init_array = Ser0_init,
		.init_array_len = (sizeof(Ser0_init)) / (sizeof(RegI2CT)),
		.init_serializer = max9295_init,
		.deinit_serializer = max9295_deinit,
		.set_alias_addr = max9295_set_alias,
		.enable_link = max9295_enable_link,
		.disable_link = max9295_disable_link,
		.reset_links = max9295_reset_links,
		.phy_lanes_enable = max9295_phy_lane_enable,
		.set_virt_ch_map = max9295_set_virt_id_map,
		.set_sensor_addr_map = max9295_set_sensor_addr_map,
		.set_broadcast_addr = max9295_set_broadcast_addr,
		.send_broadcast = max9295_send_broadcast,
		.i2c_write = max9295_i2c_write,
		.i2c_read = max9295_i2c_read,
	},
	{
		.name = "max9295-1",
		.link_lane = LINK_B,
		.i2c_addr = 0x40,
		.bus_num = 0x1,
		.alias_addr = 0x0,
		.broadcast_addr = 0x0,
		.enable = 1,
		.alias_en = 0,
		.broadcast_en = 0,
		.ser_state = in_deinit,
		.init_array = Ser1_init,
		.init_array_len = (sizeof(Ser1_init)) / (sizeof(RegI2CT)),
		.init_serializer = max9295_init,
		.deinit_serializer = max9295_deinit,
		.set_alias_addr = max9295_set_alias,
		.enable_link = max9295_enable_link,
		.disable_link = max9295_disable_link,
		.reset_links = max9295_reset_links,
		.phy_lanes_enable = max9295_phy_lane_enable,
		.set_virt_ch_map = max9295_set_virt_id_map,
		.set_sensor_addr_map = max9295_set_sensor_addr_map,
		.set_broadcast_addr = max9295_set_broadcast_addr,
		.send_broadcast = max9295_send_broadcast,
		.i2c_write = max9295_i2c_write,
		.i2c_read = max9295_i2c_read,
	},
	{
		.name = "max9295-2",
		.link_lane = LINK_A,
		.i2c_addr = 0x40,
		.bus_num = 0x1,
		.alias_addr = 0x0,
		.broadcast_addr = 0x0,
		.enable = 1,
		.alias_en = 0,
		.broadcast_en = 0,
		.ser_state = in_deinit,
		.init_array = Ser2_init,
		.init_array_len = (sizeof(Ser2_init)) / (sizeof(RegI2CT)),
		.init_serializer = max9295_init,
		.deinit_serializer = max9295_deinit,
		.set_alias_addr = max9295_set_alias,
		.enable_link = max9295_enable_link,
		.disable_link = max9295_disable_link,
		.reset_links = max9295_reset_links,
		.phy_lanes_enable = max9295_phy_lane_enable,
		.set_virt_ch_map = max9295_set_virt_id_map,
		.set_sensor_addr_map = max9295_set_sensor_addr_map,
		.set_broadcast_addr = max9295_set_broadcast_addr,
		.send_broadcast = max9295_send_broadcast,
		.i2c_write = max9295_i2c_write,
		.i2c_read = max9295_i2c_read,
	},
	{
		.name = "max9295-3",
		.link_lane = LINK_B,
		.i2c_addr = 0x40,
		.bus_num = 0x1,
		.alias_addr = 0x0,
		.broadcast_addr = 0x0,
		.enable = 1,
		.alias_en = 0,
		.broadcast_en = 0,
		.ser_state = in_deinit,
		.init_array = Ser3_init,
		.init_array_len = (sizeof(Ser3_init)) / (sizeof(RegI2CT)),
		.init_serializer = max9295_init,
		.deinit_serializer = max9295_deinit,
		.set_alias_addr = max9295_set_alias,
		.enable_link = max9295_enable_link,
		.disable_link = max9295_disable_link,
		.reset_links = max9295_reset_links,
		.phy_lanes_enable = max9295_phy_lane_enable,
		.set_virt_ch_map = max9295_set_virt_id_map,
		.set_sensor_addr_map = max9295_set_sensor_addr_map,
		.set_broadcast_addr = max9295_set_broadcast_addr,
		.send_broadcast = max9295_send_broadcast,
		.i2c_write = max9295_i2c_write,
		.i2c_read = max9295_i2c_read,
	},
	{
		.name = "max9295-4",
		.link_lane = LINK_A,
		.i2c_addr = 0x40,
		.bus_num = 0x1,
		.alias_addr = 0x0,
		.broadcast_addr = 0x0,
		.enable = 1,
		.alias_en = 0,
		.broadcast_en = 0,
		.ser_state = in_deinit,
		.init_array = Ser4_init,
		.init_array_len = (sizeof(Ser4_init)) / (sizeof(RegI2CT)),
		.init_serializer = max9295_init,
		.deinit_serializer = max9295_deinit,
		.set_alias_addr = max9295_set_alias,
		.enable_link = max9295_enable_link,
		.disable_link = max9295_disable_link,
		.reset_links = max9295_reset_links,
		.phy_lanes_enable = max9295_phy_lane_enable,
		.set_virt_ch_map = max9295_set_virt_id_map,
		.set_sensor_addr_map = max9295_set_sensor_addr_map,
		.set_broadcast_addr = max9295_set_broadcast_addr,
		.send_broadcast = max9295_send_broadcast,
		.i2c_write = max9295_i2c_write,
		.i2c_read = max9295_i2c_read,
	},
	{
		.name = "max9295-5",
		.link_lane = LINK_B,
		.i2c_addr = 0x40,
		.bus_num = 0x1,
		.alias_addr = 0x0,
		.broadcast_addr = 0x0,
		.enable = 1,
		.alias_en = 0,
		.broadcast_en = 0,
		.ser_state = in_deinit,
		.init_array = Ser5_init,
		.init_array_len = (sizeof(Ser5_init)) / (sizeof(RegI2CT)),
		.init_serializer = max9295_init,
		.deinit_serializer = max9295_deinit,
		.set_alias_addr = max9295_set_alias,
		.enable_link = max9295_enable_link,
		.disable_link = max9295_disable_link,
		.reset_links = max9295_reset_links,
		.phy_lanes_enable = max9295_phy_lane_enable,
		.set_virt_ch_map = max9295_set_virt_id_map,
		.set_sensor_addr_map = max9295_set_sensor_addr_map,
		.set_broadcast_addr = max9295_set_broadcast_addr,
		.send_broadcast = max9295_send_broadcast,
		.i2c_write = max9295_i2c_write,
		.i2c_read = max9295_i2c_read,
	},
	{
		.name = "max9295-6",
		.link_lane = LINK_A,
		.i2c_addr = 0x40,
		.bus_num = 0x1,
		.alias_addr = 0x0,
		.broadcast_addr = 0x0,
		.enable = 1,
		.alias_en = 0,
		.broadcast_en = 0,
		.ser_state = in_deinit,
		.init_array = Ser6_init,
		.init_array_len = (sizeof(Ser6_init)) / (sizeof(RegI2CT)),
		.init_serializer = max9295_init,
		.deinit_serializer = max9295_deinit,
		.set_alias_addr = max9295_set_alias,
		.enable_link = max9295_enable_link,
		.disable_link = max9295_disable_link,
		.reset_links = max9295_reset_links,
		.phy_lanes_enable = max9295_phy_lane_enable,
		.set_virt_ch_map = max9295_set_virt_id_map,
		.set_sensor_addr_map = max9295_set_sensor_addr_map,
		.set_broadcast_addr = max9295_set_broadcast_addr,
		.send_broadcast = max9295_send_broadcast,
		.i2c_write = max9295_i2c_write,
		.i2c_read = max9295_i2c_read,
	},
	{
		.name = "max9295-7",
		.link_lane = LINK_B,
		.i2c_addr = 0x40,
		.bus_num = 0x1,
		.alias_addr = 0x0,
		.broadcast_addr = 0x0,
		.enable = 1,
		.alias_en = 0,
		.broadcast_en = 0,
		.ser_state = in_deinit,
		.init_array = Ser7_init,
		.init_array_len = (sizeof(Ser7_init)) / (sizeof(RegI2CT)),
		.init_serializer = max9295_init,
		.deinit_serializer = max9295_deinit,
		.set_alias_addr = max9295_set_alias,
		.enable_link = max9295_enable_link,
		.disable_link = max9295_disable_link,
		.reset_links = max9295_reset_links,
		.phy_lanes_enable = max9295_phy_lane_enable,
		.set_virt_ch_map = max9295_set_virt_id_map,
		.set_sensor_addr_map = max9295_set_sensor_addr_map,
		.set_broadcast_addr = max9295_set_broadcast_addr,
		.send_broadcast = max9295_send_broadcast,
		.i2c_write = max9295_i2c_write,
		.i2c_read = max9295_i2c_read,
	},
	{
		.name = "max9295-8",
		.link_lane = LINK_A,
		.i2c_addr = 0x40,
		.bus_num = 0x1,
		.alias_addr = 0x0,
		.broadcast_addr = 0x0,
		.enable = 1,
		.alias_en = 0,
		.broadcast_en = 0,
		.ser_state = in_deinit,
		.init_array = Ser8_init,
		.init_array_len = (sizeof(Ser8_init)) / (sizeof(RegI2CT)),
		.init_serializer = max9295_init,
		.deinit_serializer = max9295_deinit,
		.set_alias_addr = max9295_set_alias,
		.enable_link = max9295_enable_link,
		.disable_link = max9295_disable_link,
		.reset_links = max9295_reset_links,
		.phy_lanes_enable = max9295_phy_lane_enable,
		.set_virt_ch_map = max9295_set_virt_id_map,
		.set_sensor_addr_map = max9295_set_sensor_addr_map,
		.set_broadcast_addr = max9295_set_broadcast_addr,
		.send_broadcast = max9295_send_broadcast,
		.i2c_write = max9295_i2c_write,
		.i2c_read = max9295_i2c_read,
	},
	{
		.name = "max9295-9",
		.link_lane = LINK_B,
		.i2c_addr = 0x40,
		.bus_num = 0x1,
		.alias_addr = 0x0,
		.broadcast_addr = 0x0,
		.enable = 1,
		.alias_en = 0,
		.broadcast_en = 0,
		.ser_state = in_deinit,
		.init_array = Ser9_init,
		.init_array_len = (sizeof(Ser9_init)) / (sizeof(RegI2CT)),
		.init_serializer = max9295_init,
		.deinit_serializer = max9295_deinit,
		.set_alias_addr = max9295_set_alias,
		.enable_link = max9295_enable_link,
		.disable_link = max9295_disable_link,
		.reset_links = max9295_reset_links,
		.phy_lanes_enable = max9295_phy_lane_enable,
		.set_virt_ch_map = max9295_set_virt_id_map,
		.set_sensor_addr_map = max9295_set_sensor_addr_map,
		.set_broadcast_addr = max9295_set_broadcast_addr,
		.send_broadcast = max9295_send_broadcast,
		.i2c_write = max9295_i2c_write,
		.i2c_read = max9295_i2c_read,
	},
	{
		.name = "max9295-10",
		.link_lane = LINK_A,
		.i2c_addr = 0x40,
		.bus_num = 0x1,
		.alias_addr = 0x0,
		.broadcast_addr = 0x0,
		.enable = 1,
		.alias_en = 0,
		.broadcast_en = 0,
		.ser_state = in_deinit,
		.init_array = Ser10_init,
		.init_array_len = (sizeof(Ser10_init)) / (sizeof(RegI2CT)),
		.init_serializer = max9295_init,
		.deinit_serializer = max9295_deinit,
		.set_alias_addr = max9295_set_alias,
		.enable_link = max9295_enable_link,
		.disable_link = max9295_disable_link,
		.reset_links = max9295_reset_links,
		.phy_lanes_enable = max9295_phy_lane_enable,
		.set_virt_ch_map = max9295_set_virt_id_map,
		.set_sensor_addr_map = max9295_set_sensor_addr_map,
		.set_broadcast_addr = max9295_set_broadcast_addr,
		.send_broadcast = max9295_send_broadcast,
		.i2c_write = max9295_i2c_write,
		.i2c_read = max9295_i2c_read,
	},
	{
		.name = "max9295-11",
		.link_lane = LINK_B,
		.i2c_addr = 0x40,
		.bus_num = 0x1,
		.alias_addr = 0x0,
		.broadcast_addr = 0x0,
		.enable = 1,
		.alias_en = 0,
		.broadcast_en = 0,
		.ser_state = in_deinit,
		.init_array = Ser11_init,
		.init_array_len = (sizeof(Ser11_init)) / (sizeof(RegI2CT)),
		.init_serializer = max9295_init,
		.deinit_serializer = max9295_deinit,
		.set_alias_addr = max9295_set_alias,
		.enable_link = max9295_enable_link,
		.disable_link = max9295_disable_link,
		.reset_links = max9295_reset_links,
		.phy_lanes_enable = max9295_phy_lane_enable,
		.set_virt_ch_map = max9295_set_virt_id_map,
		.set_sensor_addr_map = max9295_set_sensor_addr_map,
		.set_broadcast_addr = max9295_set_broadcast_addr,
		.send_broadcast = max9295_send_broadcast,
		.i2c_write = max9295_i2c_write,
		.i2c_read = max9295_i2c_read,
	},
	{
		.name = "max9295-2",
		.link_lane = LINK_A,
		.i2c_addr = 0x40,
		.bus_num = 0x1,
		.alias_addr = 0x0,
		.broadcast_addr = 0x0,
		.enable = 1,
		.alias_en = 0,
		.broadcast_en = 0,
		.ser_state = in_deinit,
		.init_array = Ser12_init,
		.init_array_len = (sizeof(Ser12_init)) / (sizeof(RegI2CT)),
		.init_serializer = max9295_init,
		.deinit_serializer = max9295_deinit,
		.set_alias_addr = max9295_set_alias,
		.enable_link = max9295_enable_link,
		.disable_link = max9295_disable_link,
		.reset_links = max9295_reset_links,
		.phy_lanes_enable = max9295_phy_lane_enable,
		.set_virt_ch_map = max9295_set_virt_id_map,
		.set_sensor_addr_map = max9295_set_sensor_addr_map,
		.set_broadcast_addr = max9295_set_broadcast_addr,
		.send_broadcast = max9295_send_broadcast,
		.i2c_write = max9295_i2c_write,
		.i2c_read = max9295_i2c_read,
	},
};

/****************************************************************************
 * @brief	max9295 init
 *
 * @return	None.
 *
 ****************************************************************************/
int max9295_init(struct serializer_driver *ser_inst)
{
	int Status = XST_SUCCESS;

	if (ser_inst->ser_state == in_deinit) {

		u8 ser_addr = ser_inst->alias_addr >> 1;
		u8 wr_data[4] = {0};
		RegI2CT *Serializer_initialization;
		u32 len;

#if defined (READ_I2C_REG)
		u8 rd_data[4] = { 0 };
#endif
		sleep(1);

		Serializer_initialization = ser_inst->init_array;
		len = ser_inst->init_array_len;

#if !defined (READ_I2C_REG)
		xil_printf("Initializing Serializer(%s) at Virtual Address = 0x%x on Serial Link %c...\n\r",
			   ser_inst->name, ser_addr, (ser_inst->link_lane == LINK_A) ? 'A' : 'B');
#endif
		for (int j = 0; j < len; j++) {
			if ((Serializer_initialization + j)->addr == MAX929X_TABLE_END)
				break;

			if ((Serializer_initialization + j)->addr == MAX929X_TABLE_WAIT) {
				usleep((Serializer_initialization + j)->val * 1000);
				continue;
			}

			wr_data[0] = ((Serializer_initialization + j)->val);

			Status = HalWriteI2CReg(IIC_DEVICE_ID, ser_addr, (Serializer_initialization + j)->addr, 0x2,
						wr_data[0], 1);
			if (Status != XST_SUCCESS)
				DCT_ASSERT(0);

#if defined (READ_I2C_REG)
			Status = HalReadI2CReg(IIC_DEVICE_ID, 0x62, (Serializer_initialization + j)->addr, 0x2, rd_data, 1);
			if (Status != XST_SUCCESS)
				DCT_ASSERT(0);

			xil_printf("%s Address=%x Read Data:%x\n", __func__, (Serializer_initialization + j)->addr,
				   rd_data[0]);
#endif
		}

		ser_inst->ser_state = in_init;
		ser_inst->ser_state = in_running;

#if !defined (READ_I2C_REG)
		xil_printf("Initialization Done...\n\r");
#endif
		sleep(1);
	} else
		xil_printf("Serializer Already in Running state(%d) \n\r", ser_inst->ser_state);

	return Status;
}

int max9295_deinit(struct serializer_driver *ser_inst)
{
	return XST_SUCCESS;
}

int max9295_set_alias(struct serializer_driver *ser_inst, int link_number, int i2c_addr,
		      int alias_addr)
{
	return XST_SUCCESS;
}

int max9295_enable_link(struct serializer_driver *ser_inst, int link_mode)
{
	return XST_SUCCESS;
}

int max9295_disable_link(struct serializer_driver *ser_inst, int link_mode)
{
	return XST_SUCCESS;
}

int max9295_reset_links(struct serializer_driver *ser_inst)
{
	return XST_SUCCESS;
}

int max9295_phy_lane_enable(struct serializer_driver *ser_inst, int phy_count)
{
	return XST_SUCCESS;
}

int max9295_set_virt_id_map(struct serializer_driver *ser_inst, struct map_struct *map,
			    int map_count)
{
	return XST_SUCCESS;
}

int max9295_set_sensor_addr_map(struct serializer_driver *ser_inst, struct map_struct *map,
				int map_count)
{
	return XST_SUCCESS;
}

int max9295_set_broadcast_addr(struct serializer_driver *ser_inst, int broadcast_addr)
{
	return XST_SUCCESS;
}

int max9295_send_broadcast(struct serializer_driver *ser_inst, char *data, int size)
{
	return XST_SUCCESS;
}
int max9295_i2c_write(struct serializer_driver *ser_inst, char *data, int size)
{
	return XST_SUCCESS;
}

int max9295_i2c_read(struct serializer_driver *ser_inst, char *data, int size)
{
	return XST_SUCCESS;
}
