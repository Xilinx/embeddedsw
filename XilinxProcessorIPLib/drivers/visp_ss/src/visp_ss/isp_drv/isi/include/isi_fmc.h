// Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
#ifndef __ISI_FMC_H_
#define __ISI_FMC_H_

#include "xparameters.h"
#include "xgpiops.h"
#include "xiicps.h"
#include "xiicps.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xiicps.h"

#include "hal_i2c.h"

typedef enum ts {
	in_deinit = 0,
	in_progress,
	in_init,
	in_running,
	in_stop
} transition_state;

typedef struct dslink_t {
	u8 serializer_default_addr;
	u8 serializer_alias_addr;
	u8 sensor_default_addr;
	u8 sensor_alias_addr;
} dslink;

typedef struct desInterface_t {
	u8 des_actual_addr;
	u8 des_alias_addr;
	u8 Port_DES_index;
	transition_state des_state;
	u8 link_type;
	dslink link_a;
	dslink link_b;
} desInterface;


typedef struct fmc_addrconfig_s {
	u8 des_actual_addr;
	u8 des_alias_addr;
	dslink link_a;
	dslink link_b;
} fmc_config;

typedef struct IsiFmcInstanceConfig_s {
	int num_of_serailizer;
	int num_of_deserializer;
	int SlaveAddr;
	int deserializer_slave_address;
	//IsiFmc_t *pFmc;		// to update FMC- function pointer structure ops
} IsiFmcInstanceConfig_t;

typedef int (*IsiCreateFmcIss_t) ();
typedef int (*IsiFmcSetup) ();
typedef int (*IsiDeserSetup) (desInterface *);
typedef int (*IsiDeserEnable) (u8);
typedef int (*IsiDeserDisable) (u8);

#if 1
	#define IN_PIPE_0		0
	#define IN_PIPE_1		1
	#define IN_PIPE_2		2
	#define IN_PIPE_3		3
	#define IN_PIPE_4		4
	#define IN_PIPE_5		5
	#define IN_PIPE_6		6
	#define IN_PIPE_7		7
	#define IN_PIPE_8		8
	#define IN_PIPE_9		9
	#define IN_PIPE_10		10
	#define IN_PIPE_11		11
	#define IN_PIPE_12		12
	#define IN_PIPE_LAST	IN_PIPE_12

	#define MAPPING_INPIPE_TO_DES_ID(pipeId) (pipeId/2)

	//Internal definition
	#define SENSOR_ID0      0     //IN_PIPE_5
	#define SENSOR_ID1      1     //IN_PIPE_2
	#define SENSOR_ID2      2     //IN_PIPE_9
	#define SENSOR_ID3      3     //IN_PIPE_10

	#define MAX_SER_COUNT IN_PIPE_LAST+1+1
	#define MAX_SENSOR_COUNT IN_PIPE_LAST+1+1
#endif

struct IsiFmc_s {
	char	FmcName[30];                       /**< name of the FMC */
	IsiCreateFmcIss_t	pIsiCreateFmcIss;                  /**< create a FMC handle */
	IsiFmcSetup	pIsiIsiFmcSetup;                   /** FMC setup */
	IsiDeserSetup	pIsiDeserSetup;                    /**< Deser Setup */
	IsiDeserEnable	pIsiDeserEnable;
	IsiDeserDisable	pIsiDeserDisable;
	struct serializer_driver	*serializer_array[MAX_SER_COUNT];
	struct sensor_driver	*sensor_array[MAX_SENSOR_COUNT];
	struct accessIIC *accessiic_array[MAX_SENSOR_COUNT];
};

typedef struct IsiFmc_s IsiFmc_t;

typedef struct {
	u16 addr;   // 16-bit address of register
	u8 val;     // 8-bit value to write in register
} RegI2CT;

/*
 * ox03f10 I2C operation related structure
 */
typedef struct {
	u16 address;
	u8 value;
} reg_8;

#define I2C_2		0x0042
#define I2C_3		0x0043
#define I2C_4		0x0044
#define I2C_5		0x0045
#define SERIALIZER_ADDR	0x80
#define IMAGER_ADDR	0x20
#define FPGA_ADDR	0x88

#define GMSL2
#define YUV_SENSOR


#define SENSOR_3MP 0
#define SENSOR_5MP 1
#define SENSOR_8MP 2

#define RESOLUTION_640P 0
#define RESOLUTION_720P 1
#define RESOLUTION_1080P 2

#define FRAMERATE_10 0
#define FRAMERATE_30 1


#define FORMAT_0 0
#define FORMAT_1 1


typedef struct {
	u16 address;    // 16-bit I2C Address of register to write to
	u16 value;      // 16-bit value
	int delay;        // delay in us to wait after specific writes
} RegWrite;

int i2cPs_write32 (XIicPs *iic_instance, u8 chipAddress, u32 addr, u32 data);

#if 0
enum link_mode_tt {
	LINK_A,
	LINK_B,
	LINK_MAX
};
#endif

struct map_struct {
	int orig;
	int alias;
};

struct serializer_driver {
	char name[30];
	u8 link_lane;
	int i2c_addr;
	int bus_num;
	int alias_addr;
	int broadcast_addr;
	int enable ;
	int alias_en;
	int broadcast_en;
	transition_state ser_state;
	/*
	 * init_array & init_array_len
	 * are filled at runtime
	 */
	RegI2CT *init_array;
	u32 init_array_len;
	int (*init_serializer) (struct serializer_driver *ser_inst);
	int (*deinit_serializer) (struct serializer_driver *ser_inst);
	int (*set_alias_addr) (struct serializer_driver *ser_inst, int link_number, int addr,
			       int alias_addr);
	int (*enable_link) (struct serializer_driver *ser_inst, int link);
	int (*disable_link) (struct serializer_driver *ser_inst, int link);
	int (*reset_links) (struct serializer_driver *ser_inst);
	int (*phy_lanes_enable) (struct serializer_driver *ser_inst, int phy_cnt);
	int (*set_virt_ch_map) (struct serializer_driver *ser_inst, struct map_struct *map, int map_count);
	int (*set_sensor_addr_map) (struct serializer_driver *ser_inst, struct map_struct *map,
				    int map_count);
	int (*set_broadcast_addr) (struct serializer_driver *ser_inst, int addr);
	int (*send_broadcast)(struct serializer_driver *ser_inst, char *data, int size);
	int (*i2c_write)(struct serializer_driver *ser_inst, char *data, int size);
	int (*i2c_read)(struct serializer_driver *ser_inst, char *data, int size);
};

struct accessIIC {
	u8 i2cBusId;
	int (*readIIC) (u8 i2cBusId, u8 slave_addr, uint16_t addr, uint8_t regWidth, uint16_t *pValue,
			uint8_t dataWidth);
	int (*writeIIC) (u8 i2cBusId, u8 slave_addr, uint16_t addr, uint8_t regWidth, uint16_t value,
			 uint8_t dataWidth);
};

struct sensor_driver {
	char name[30];
	u8 pipe_no;
	/*
	 *  init_array & init_array_len
	 *  are filled at runtime
	 */
	reg_8 *init_array;
	u32 init_array_len;
	reg_8 *streamon_array;
	u32 streamon_array_len;
	reg_8 *streamoff_array;
	u32 streamoff_array_len;
	reg_8 *fps_array;
	u32 fps_array_len;
	u8 sensor_alias_addr;
	int (*init_sensor) (struct sensor_driver *);
	int (*deinit_sensor) (struct sensor_driver *);
	int (*stream_on) (struct sensor_driver *);
	int (*stream_off) (struct sensor_driver *);
	transition_state sensor_state;
};


typedef struct Sensor_device_t {
	u32 SensorCfg_Enabled;
	u32 Sensortype;
	u32 Sensor_resolution;
	u32 Sensor_format;
	u32 Sensor_framerate;
} Sensor_device ;

extern IsiFmc_t g_fmc_single;

#define SENSOR_ADDR				0x36

#define LINK_MASK				0x3
#define LINK_A					0x01
#define LINK_B					0x02
#define LINK_DUAL				0x00
#define LINK_REVERSE_SPLITTER			0x03
#define NO_LINK					0x04
#define LINK_MAX				0x02

#define SPLITTER				0x3
#define AUTO_LINK				0x4
#define RESET_ONE_SHOT			0x5
#define LOCKED					0x3

#define DEV_ADDR_REG			0x00
#define CTRL0_REG				0x10
#define CTRL3_REG				0x13
#define I2c_2					0x42
#define I2c_3					0x43
#define I2c_4					0x44
#define I2c_5					0x45


#define PDB_DES_DES1			0
#define PDB_DES_DES2			1
#define PDB_DES_DES3			2
#define PDB_DES_DES4			3
#define PDB_DES_DES5			4
#define PDB_DES_DES6			5
#define PDB_DES_DES7			6
#define CAM_SUPPLY_EN			7

#if 0
	#define DS1_ADDRESS				0xD0
	#define DS2_ADDRESS				0xD0
	#define DS3_ADDRESS				0x94
	#define DS4_ADDRESS				0x94
	#define DS5_ADDRESS				0xD8
	#define DS6_ADDRESS				0xD8
#endif

#define DS1_DEFAULT_ADDRESS		0xD0
#define DS1_ALIAS_ADDRESS		0xD2

#define DS2_DEFAULT_ADDRESS		0xD0
#define DS2_ALIAS_ADDRESS		0xD0

#define DS3_DEFAULT_ADDRESS		0x94
#define DS3_ALIAS_ADDRESS		0x96

#define DS4_DEFAULT_ADDRESS		0x94
#define DS4_ALIAS_ADDRESS		0x94

#define DS5_DEFAULT_ADDRESS		0xD8
#define DS5_ALIAS_ADDRESS		0XDA

#define DS6_DEFAULT_ADDRESS		0xD8
#define DS6_ALIAS_ADDRESS		0xD8


#define DS_ONE						0
#define DS_TWO						1
#define DS_THREE					2
#define DS_FOUR						3
#define DS_FIVE						4
#define DS_SIX						5
#define DS_SEVEN					6
#define DS_MAX						6+1

#define SERIALIZER_DEFAULT_ADDR		0xC4

#define SERIALIZER_0_ALIAS_ADDR		SERIALIZER_DEFAULT_ADDR
#define SERIALIZER_1_ALIAS_ADDR		0xC6
#define SERIALIZER_2_ALIAS_ADDR		0xC8
#define SERIALIZER_3_ALIAS_ADDR		0xCA
#define SERIALIZER_4_ALIAS_ADDR		0xCC
#define SERIALIZER_5_ALIAS_ADDR		0xCE
#define SERIALIZER_6_ALIAS_ADDR		0x80
#define SERIALIZER_7_ALIAS_ADDR		0x82
#define SERIALIZER_8_ALIAS_ADDR		0x84
#define SERIALIZER_9_ALIAS_ADDR		0x86
#define SERIALIZER_10_ALIAS_ADDR	0x88
#define SERIALIZER_11_ALIAS_ADDR	0x8A

#define SENSOR_OX3F10_ADDRESS		0x6C
#define SENSOR_OX5B_ADDRESS			0x20
#define SENSOR_DEFAULT_ADDRSS		SENSOR_OX3F10_ADDRESS

#define SENSOR_0_ALIAS_ADDR			SENSOR_DEFAULT_ADDRSS
#define SENSOR_1_ALIAS_ADDR			0x30
#define SENSOR_2_ALIAS_ADDR			0x32
#define SENSOR_3_ALIAS_ADDR			0x34
#define SENSOR_4_ALIAS_ADDR			0x36
#define SENSOR_5_ALIAS_ADDR			0x38
#define SENSOR_6_ALIAS_ADDR			0x3A
#define SENSOR_7_ALIAS_ADDR			0x3C
#define SENSOR_8_ALIAS_ADDR			0x3E
#define SENSOR_9_ALIAS_ADDR			0x20
#define SENSOR_10_ALIAS_ADDR		0x22
#define SENSOR_11_ALIAS_ADDR		0x24

#define SENSOR_MAX					12
#define TABLE_WAIT			(0xfffe)
#define TABLE_END			(0xffff)
#endif
