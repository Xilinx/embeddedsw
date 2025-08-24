// Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
#ifndef __XYLON_MAX_9295_H__
#define __XYLON_MAX_9295_H__

#include <isi_fmc.h>
int max9295_init(struct serializer_driver *);
int max9295_deinit(struct serializer_driver *);

int max9295_set_alias(struct serializer_driver *, int link_number, int i2c_addr, int alias_addr);
int max9295_enable_link(struct serializer_driver *, int link_mode);
int max9295_disable_link(struct serializer_driver *, int link_mode);
int max9295_reset_links(struct serializer_driver *);

int max9295_phy_lane_enable(struct serializer_driver *, int phy_count);
int max9295_set_virt_id_map(struct serializer_driver *, struct map_struct *map, int map_count);
int max9295_set_sensor_addr_map(struct serializer_driver *, struct map_struct *map, int map_count);
int max9295_set_broadcast_addr(struct serializer_driver *, int broadcast_addr);
int max9295_send_broadcast(struct serializer_driver *, char *data, int size);

int max9295_i2c_write(struct serializer_driver *, char *data, int size);
int max9295_i2c_read(struct serializer_driver *, char *data, int size);


#if 1
#define MAX929X_TABLE_END		(0xffff)
#define MAX929X_TABLE_WAIT		(0xfffe)
#define MAX929X_TABLE_WAIT_MS		(210)

RegI2CT Ser0_init[] = {
};

RegI2CT Ser1_init[] = {
};

RegI2CT Ser2_init[] = {
#if 0
	{0x0053, 0x10},		//Video	x
	{0x0057, 0x11},		//Video Y
	{0x005B, 0x12},		//Video z
	{0x005F, 0x13},		//Video W

	//Color BPP (VIDEO_TX1)
	{0x0101, 0x4C},		//VID TX X
	{0x0109, 0x0C},		//VID TX Y
	{0x0111, 0x4C},		//VID TX z
	{0x0119, 0x4C},		//VID TX U

	//PHY Mappings
	{0x0330, 0x00},
	{0x0331, 0x33},
	{0x0332, 0xe0},
	{0x0333, 0x04},
	{0x0119, 0x4C},

	//Designated datatype to route to video pipeline (FRONTTOP_12 ...)
	{0x0314, 0x00},
	//{0x0316, 0x5E},  YUV
	{0x0316, 0x00},
	{0x0318, 0x6C},
	{0x031A, 0x00},
	{0x02be, 0x90}, // VC and DT destination map 1

	//Enable CSI port B, CSI port B selection for video pipeline Y (FRONTTOP_0)
	{0x0308, 0x7F},

	//Start video pipe Y from CSI port B (FRONTTOP_9)
	{0x0311, 0x25},

	//Video transmit enable for pipeline Y
	{0x0002, 0xF3},
	//	{0x0002, 0x23},
	{0x02be, 0x90}, /* RESET Sensor */
	{MAX929X_TABLE_WAIT, MAX929X_TABLE_WAIT_MS},
	{MAX929X_TABLE_END, 0}
#endif
	/*
	 * Set stream id on pipe for routing incoming stream,Y with 1 id
	 */
	{0x0053, 0x10},
	{0x0057, 0x11},
	{0x005B, 0x12},
	{0x005F, 0x13},

	//Color BPP (VIDEO_TX1)
	{0x0101, 0x4C},
	{0x0109, 0x0C},
	{0x0111, 0x4C},
	{0x0119, 0x4C},
	{0x0330, 0x00},
	{0x0331, 0x33},
	{0x0332, 0xe0},
	{0x0333, 0x04},
	{0x0119, 0x4C},

	//Designated datatype to route to video pipeline (FRONTTOP_12 ...)
	{0x0314, 0x00},

	//{0x0316, 0x5E},  YUV
	{0x0316, 0x00},
	{0x0318, 0x00},
	{0x031A, 0x00},
	{0x02be, 0x90}, // VC and DT destination map 1

	//Enable CSI port B, CSI port B selection for video pipeline Y (FRONTTOP_0)
	{0x0308, 0x7F},

	//Start video pipe Y from CSI port B (FRONTTOP_9)
	//{0x0311, 0x25},
	{0x0311, 0x20},

	//Video transmit enable for pipeline Y
	{0x0002, 0x23},
	//{0x0002, 0x20},

	{0x02be, 0x90}, /* RESET Sensor */
	{MAX929X_TABLE_WAIT, MAX929X_TABLE_WAIT_MS},
	{MAX929X_TABLE_END, 0}
};
#endif

RegI2CT Ser3_init[] = {
	//y with 2 id
	{0x0053, 0x10},	//x
	{0x0057, 0x12},	//y
	{0x005B, 0x11},	//z

	{0x005F, 0x13},
	//Color BPP (VIDEO_TX1)
	{0x0101, 0x4C},
	{0x0109, 0x0C},
	{0x0111, 0x4C},
	{0x0119, 0x4C},

	{0x0330, 0x00},
	{0x0331, 0x33},
	{0x0332, 0xe0},
	{0x0333, 0x04},
	{0x0119, 0x4C},
	//Designated datatype to route to video pipeline (FRONTTOP_12 ...)
	{0x0314, 0x00},
	//{0x0316, 0x5E},  YUV
	{0x0316, 0x00},
	{0x0318, 0x00},
	{0x031A, 0x00},
	{0x02be, 0x90}, // VC and DT destination map 1
	//Enable CSI port B, CSI port B selection for video pipeline Y (FRONTTOP_0)
	{0x0308, 0x7F},

	//Start video pipe Y from CSI port B (FRONTTOP_9)
	//{0x0311, 0x25},
	{0x0311, 0x20},

	//Video transmit enable for pipeline Y
	{0x0002, 0x23},
	//{0x0002, 0x20},

	{0x02be, 0x90}, /* RESET Sensor */
	{MAX929X_TABLE_WAIT, MAX929X_TABLE_WAIT_MS},
	{MAX929X_TABLE_END, 0}
};

RegI2CT Ser4_init[] = {
	/*
	 * Set stream id on pipe for routing incoming stream,Y with 1 id
	 */
	{0x0053, 0x10},
	{0x0057, 0x11},
	{0x005B, 0x12},
	{0x005F, 0x13},

	//Color BPP (VIDEO_TX1)
	{0x0101, 0x4C},
	{0x0109, 0x0C},
	{0x0111, 0x4C},
	{0x0119, 0x4C},

	{0x0330, 0x00},
	{0x0331, 0x33},
	{0x0332, 0xe0},
	{0x0333, 0x04},
	{0x0119, 0x4C},
	//Designated datatype to route to video pipeline (FRONTTOP_12 ...)
	{0x0314, 0x00},
	//{0x0316, 0x5E},  YUV
	{0x0316, 0x00},
	{0x0318, 0x00},
	{0x031A, 0x00},
	{0x02be, 0x90}, // VC and DT destination map 1
	//Enable CSI port B, CSI port B selection for video pipeline Y (FRONTTOP_0)
	{0x0308, 0x7F},

	//Start video pipe Y from CSI port B (FRONTTOP_9)
	//{0x0311, 0x25},
	{0x0311, 0x20},

	//Video transmit enable for pipeline Y
	{0x0002, 0x23},
	//{0x0002, 0x20},

	{0x02be, 0x90}, /* RESET Sensor */
	{MAX929X_TABLE_WAIT, MAX929X_TABLE_WAIT_MS},
	{MAX929X_TABLE_END, 0}
};

RegI2CT Ser5_init[] = {
	//y with 2 id
	{0x0053, 0x10},	//x
	{0x0057, 0x12},	//y
	{0x005B, 0x11},	//z

	{0x005F, 0x13},
	//Color BPP (VIDEO_TX1)
	{0x0101, 0x4C},
	{0x0109, 0x0C},
	{0x0111, 0x4C},
	{0x0119, 0x4C},

	{0x0330, 0x00},
	{0x0331, 0x33},
	{0x0332, 0xe0},
	{0x0333, 0x04},
	{0x0119, 0x4C},
	//Designated datatype to route to video pipeline (FRONTTOP_12 ...)
	{0x0314, 0x00},
	//{0x0316, 0x5E},  YUV
	{0x0316, 0x00},
	{0x0318, 0x00},
	{0x031A, 0x00},
	{0x02be, 0x90}, // VC and DT destination map 1
	//Enable CSI port B, CSI port B selection for video pipeline Y (FRONTTOP_0)
	{0x0308, 0x7F},

	//Start video pipe Y from CSI port B (FRONTTOP_9)
	//{0x0311, 0x25},
	{0x0311, 0x20},

	//Video transmit enable for pipeline Y
	{0x0002, 0x23},
	//{0x0002, 0x20},

	{0x02be, 0x90}, /* RESET Sensor */
	{MAX929X_TABLE_WAIT, MAX929X_TABLE_WAIT_MS},
	{MAX929X_TABLE_END, 0}
};

#if 0
RegI2CT Ser5_init[] = {
#if 1
	//MIPI_5
	/*
	 * Sumit
	 *Doing Register - I2C_2 & I2C_3 Configuration from code itself
	 */
	//	{0x0010,0x22},

	//{I2C_2,  IMAGER_ADDR},
	//{I2C_3,  IMAGER_ADDR},
	// Packet settings (TX3)
	{0x0053, 0x10},		//x
	{0x0057, 0x12},		//y
	{0x005B, 0x11},		//z
	{0x005F, 0x13},		//u
	// Color BPP (VIDEO_TX1)
	{0x0101, 0x4C},
	{0x0109, 0x0C},
	{0x0111, 0x4C},
	{0x0119, 0x4C},

	/* ---------- zwc add -------- */
	{0x0330, 0x00},
	{0x0331, 0x33},
	{0x0332, 0xe0},
	{0x0333, 0x04},

	{0x0119, 0x4C},
	/* ------------------ */
	// Designated datatype to route to video pipeline (FRONTTOP_12 ...)
	{0x0314, 0x00},
	//{0x0316, 0x5E},   // YUV
	{0x0316, 0x00},//y
	{0x0318, 0x6C},//z
	{0x031A, 0x00},
	{0x02be, 0x90},      // VC and DT destination map 1  */

	// Enable CSI port B, CSI port B selection for video pipeline Y (FRONTTOP_0)
	{0x0308, 0x7F},

	// Start video pipe Y from CSI port B (FRONTTOP_9)
	{0x0311, 0x25},
	// Video transmit enable for pipeline Y
	{0x0002, 0xF3},
#endif
};
#endif

RegI2CT Ser6_init[] = {
};

RegI2CT Ser7_init[] = {
};

RegI2CT Ser8_init[] = {
};

RegI2CT Ser9_init[] = {
	//y with 2 id
	{0x0053, 0x10},	//x
	{0x0057, 0x12},	//y
	{0x005B, 0x11},	//z

	{0x005F, 0x13},
	//Color BPP (VIDEO_TX1)
	{0x0101, 0x4C},
	{0x0109, 0x0C},
	{0x0111, 0x4C},
	{0x0119, 0x4C},

	{0x0330, 0x00},
	{0x0331, 0x33},
	{0x0332, 0xe0},
	{0x0333, 0x04},
	{0x0119, 0x4C},
	//Designated datatype to route to video pipeline (FRONTTOP_12 ...)
	{0x0314, 0x00},
	//{0x0316, 0x5E},  YUV
	{0x0316, 0x00},
	{0x0318, 0x00},
	{0x031A, 0x00},
	{0x02be, 0x90}, // VC and DT destination map 1
	//Enable CSI port B, CSI port B selection for video pipeline Y (FRONTTOP_0)
	{0x0308, 0x7F},

	//Start video pipe Y from CSI port B (FRONTTOP_9)
	//{0x0311, 0x25},
	{0x0311, 0x20},

	//Video transmit enable for pipeline Y
	{0x0002, 0x23},
	//{0x0002, 0x20},

	{0x02be, 0x90}, /* RESET Sensor */
	{MAX929X_TABLE_WAIT, MAX929X_TABLE_WAIT_MS},
	{MAX929X_TABLE_END, 0}
};

RegI2CT Ser10_init[] = {
	/*
	* Set stream id on pipe for routing incoming stream,Y with 1 id
	*/
	{0x0053, 0x10},
	{0x0057, 0x11},
	{0x005B, 0x12},
	{0x005F, 0x13},
	//Color BPP (VIDEO_TX1)
	{0x0101, 0x4C},
	{0x0109, 0x0C},
	{0x0111, 0x4C},
	{0x0119, 0x4C},
	{0x0330, 0x00},
	{0x0331, 0x33},
	{0x0332, 0xe0},
	{0x0333, 0x04},
	{0x0119, 0x4C},
	//Designated datatype to route to video pipeline (FRONTTOP_12 ...)
	{0x0314, 0x00},
	//{0x0316, 0x5E},  YUV
	{0x0316, 0x00},
	{0x0318, 0x00},
	{0x031A, 0x00},
	{0x02be, 0x90}, // VC and DT destination map 1
	//Enable CSI port B, CSI port B selection for video pipeline Y (FRONTTOP_0)
	{0x0308, 0x7F},
	//Start video pipe Y from CSI port B (FRONTTOP_9)
	//{0x0311, 0x25},
	{0x0311, 0x20},
	//Video transmit enable for pipeline Y
	{0x0002, 0x23},
	//{0x0002, 0x20},
	{0x02be, 0x90}, /* RESET Sensor */
	{MAX929X_TABLE_WAIT, MAX929X_TABLE_WAIT_MS},
	{MAX929X_TABLE_END, 0}
};

RegI2CT Ser11_init[] = {
};

RegI2CT Ser12_init[] = {
	//y with 2 id
	{0x0053, 0x10},	//x
	{0x0057, 0x11},	//y
	{0x005B, 0x12},	//z

	{0x005F, 0x13},
	//Color BPP (VIDEO_TX1)
	{0x0101, 0x4B},
	{0x0109, 0x0B},  //Changed for RGB-IR (12bit to 10bit)
	{0x0111, 0x4B},
	{0x0119, 0x4B},

	{0x0330, 0x00},
	{0x0331, 0x33},
	{0x0332, 0xe0},
	{0x0333, 0x04},
	{0x0119, 0x4C},
	//Designated datatype to route to video pipeline (FRONTTOP_12 ...)
	{0x0314, 0x00},
	//{0x0316, 0x5E},  YUV
	{0x0316, 0x00},
	{0x0318, 0x00},
	{0x031A, 0x00},
	{0x02be, 0x90}, // VC and DT destination map 1
	//Enable CSI port B, CSI port B selection for video pipeline Y (FRONTTOP_0)
	{0x0308, 0x7F},

	//Start video pipe Y from CSI port B (FRONTTOP_9)
	//{0x0311, 0x25},
	{0x0311, 0x20},

	//Video transmit enable for pipeline Y
	{0x0002, 0x23},
	//{0x0002, 0x20},

	{0x02be, 0x90}, /* RESET Sensor */
	{MAX929X_TABLE_WAIT, MAX929X_TABLE_WAIT_MS},
	{MAX929X_TABLE_END, 0}
};


#endif
