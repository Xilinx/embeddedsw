// Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
#ifndef __HAL_API_H__
#define __HAL_API_H__

#include <stdbool.h>
//#include "return_codes.h"
#include <xiicps.h>

#include <xil_printf.h>
#include <xil_types.h>
//#include "isi.h"
//#include "oslayer.h"
#include <dct_assert.h>

typedef int RESULT;
typedef void *HalHandle_t;
typedef uint16_t TickType_t;

typedef enum HalDevType_e {
	HAL_DEV_ISP = 0,
	HAL_DEV_MEM,
	HAL_DEV_I2C,
	HAL_DEV_MIPI,
	HAL_DEV_VIDEO_IN,
	HAL_DEV_MSS,
} HalDevType_t;

typedef enum HalMapMemType_s {
	HAL_MAPMEM_READWRITE = 0,
	HAL_MAPMEM_READONLY = 1,
	HAL_MAPMEM_WRITEONLY = 2
} HalMapMemType_t;

typedef struct HalIrqCtx_s {
	u32 misRegAddress;
	u32 icrRegAddress;
	u32 misValue;
} HalIrqCtx_t;

/******************************************************************************
 * HalOpen()
 *****************************************************************************/
/**
 *  @brief  Opens the low level driver.
 *
 *  @return           Handle of the low level driver; NULL on failure
 *
 ******************************************************************************/

RESULT HalOpen();

/******************************************************************************
 * HalClose()
******************************************************************************/
/**
 *  @brief  Closes the low level driver
 *
 *  @return           Status of operation
 *
 ******************************************************************************/
RESULT HalClose ();

/******************************************************************************
 * HalAddRef()
******************************************************************************/
/**
 *  @brief  Tell HAL about another user of the low level driver
 *
 *  @return           Status of operation
 *
 ******************************************************************************/
RESULT HalAddRef (HalHandle_t HalHandle, uint8_t devType);

/******************************************************************************
 * HalDelRef()
******************************************************************************/
/**
 *  @brief  Tell HAL about departed user of the low level driver
 *
 *  @return           Status of operation
 *
 ******************************************************************************/
RESULT HalDelRef ();

/******************************************************************************
 * HalNumOfCams()
******************************************************************************/
/**
 *  @brief  Returns the number of connected cameras
 *
 *  @param  no        Select negative edge of incoming PClk to sample camera data.
 *
 *  @return           Status of operation
 *
 ******************************************************************************/
RESULT HalNumOfCams (u32 *no);

/******************************************************************************
 * HalSetCamConfig()
******************************************************************************/
/**
 *  @brief  Sets the configuration of the given CAM devices
 *
 *  @param  dev_mask        Mask of CAM devices to set the configuration for.
 *
 *  @param  power_lowact    CAM power on signal is low-active
 *
 *  @param  reset_lowact    CAM reset signal is low-active
 *
 *  @param  pclk_negedge    Select negative edge of incoming PClk to sample camera data
 *
 *  @return                 Status of operation
 *
 ******************************************************************************/
RESULT HalSetCamConfig (u32 dev_mask, bool power_lowact, bool reset_lowact, bool pclk_negedge);

/******************************************************************************
 * HalSetCamPhyConfig()
******************************************************************************/
/**
 *  @brief  Sets the configuration of the given CAM PHY devices
 *
 *  @param  dev_mask        Mask of CAM PHY devices to set the configuration for.
 *
 *  @param  power_lowact    CAM PHY power on signal is low-active.
 *
 *  @param  reset_lowact    CAM PHY reset signal is low-active.
 *
 *  @return                 Status of operation
 *
 ******************************************************************************/
RESULT HalSetCamPhyConfig(u32 dev_mask, bool power_lowact, bool reset_lowact);

/******************************************************************************
 * HalSetReset()
 ******************************************************************************/
/**
 *  @brief  Enables/Disables reset of the given devices
 *
 *  @param  dev_mask        Mask of devices to change reset state
 *
 *  @param  activate        Enable or disable reset of the specified devices.
 *
 *  @return                 Status of operation
 *
 ******************************************************************************/
RESULT HalSetReset (u32 dev_mask, bool activate);

/******************************************************************************
 * HalSetPower()
******************************************************************************/
/**
 *  @brief  Enables/Disables power of the given devices
 *
 *  @param  dev_mask        Mask of devices to change power state
 *
 *  @param  activate        Enable or disable power of the specified devices.
 *
 *  @return                 Status of operation
 *
 ******************************************************************************/
RESULT HalSetPower (u32 dev_mask, bool activate);

/******************************************************************************
 * HalSetClock()
******************************************************************************/
/**
 *  @brief  Enables and sets/disables clock of the given devices
 *
 *  @param  dev_mask        Mask of devices to change clock settings for
 *
 *  @param  frequency       Sets frequency of given clocks in Hz steps.
 *
 *  @return                 Status of operation
 *
 ******************************************************************************/
RESULT HalSetClock (u32 dev_mask, u32 frequency);

/******************************************************************************
 * HalReadReg()
******************************************************************************/
/**
 *  @brief  Reads a value from the specified address.
 *
 *  @param  reg_address     Full address (base address + offset) of register to read
 *
 *  @return                 The value read from the register
 *
 ******************************************************************************/
u32 HalReadReg (u32 reg_address);

/******************************************************************************
 * HalWriteReg()
******************************************************************************/
/**
 *  @brief  Writes a value to a given address.
 *
 *  @param  reg_address     Full address (base address + offset) of register to write.
 *
 *  @param  value           Value to write into register
 *
 ******************************************************************************/
void HalWriteReg (u32 reg_address, u32 value);

/******************************************************************************
 * HalReadMaskedReg()
******************************************************************************/
/**
 *  @brief  Reads a value from a specific part of the given address.
 *
 *  @param  reg_address     Full address (base address + offset) of register to read.
 *
 *  @param  reg_mask        Mask to apply to register value after being read
 *
 *  @param  shift_mask      Amount to right shift masked register value prior to being returned
 *
 *  @return                 The masked and shifted value of the specified register that is read
 *
 ******************************************************************************/
u32 HalReadMaskedReg (u32 reg_address, u32 reg_mask, u32 shift_mask);

/******************************************************************************
 * HalWriteMaskedReg()
******************************************************************************/
/**
 *  @brief  Writes a value to a specific part of the given address.
 *
 *  @param  reg_address     Full address (base address + offset) of register to write
 *
 *  @param  reg_mask        Mask to isolate parts of the register that should be written
 *
 *  @param  shift_mask      Amount to left shift the value prior to being modified in the register
 *
 *  @param  value           Value to write into the specified register position
 *
 ******************************************************************************/
void HalWriteMaskedReg (u32 reg_address, u32 reg_mask, u32 shift_mask, u32 value);

/******************************************************************************
 * HalReadSysReg()
******************************************************************************/
/**
 *  @brief  Reads a value from a given system register.
 *
 *  @param  reg_address     Full address (base address + offset) of system register to read
 *
 *  @return                 The value read from the system register
 *
 ******************************************************************************/
u32 HalReadSysReg (u32 reg_address);

/******************************************************************************
 * HalWriteSysReg()
******************************************************************************/
/**
 *  @brief  Writes a value to a given system register
 *
 *  @param  reg_address     Full address (base address + offset) of system register to write
 *
 *  @param  value           Value to write into the register
 *
 ******************************************************************************/
void HalWriteSysReg (u32 reg_address, u32 value);

/******************************************************************************
 * HalAllocMemory()
******************************************************************************/
/**
 *  @brief  Allocates the given amount of hardware memory
 *
 *  @param  byte_size       Amount of memory to allocate
 *
 *  @return                 mem_address - memory block start address in hardware memory; 0 on failure
 *
 ******************************************************************************/
u32 HalAllocMemory (u32 byte_size);

/******************************************************************************
 * HalFreeMemory()
******************************************************************************/
/**
 *  @brief  Frees the given block of hardware memory
 *
 *  @param  mem_address    Start address of memory block in hardware memory
 *
 *  @return					Status of operation
 *
 ******************************************************************************/
RESULT HalFreeMemory (u32 mem_address);

/******************************************************************************
 * HalReadMemory()
******************************************************************************/
/**
 *  @brief  Reads a number of data from the memory to a buffer starting at the given address
 *
 *  @param  mem_address    Source start address in hardware memory
 *
 *  @param  p_read_buffer  Pointer to local memory holding the data being read
 *
 *  @param  byte_size      Amount of data to read
 *
 *  @return				   Status of operation
 *
 ******************************************************************************/
RESULT HalReadMemory (u32 mem_address, u8 *p_read_buffer, u32 byte_size);

/******************************************************************************
 * HalWriteMemory()
******************************************************************************/
/**
 *  @brief  Writes a number of data from the memory to a buffer starting at the given address
 *
 *  @param  mem_address    Target start address in hardware memory
 *
 *  @param  p_write_buffer Pointer to local memory holding the data to be written
 *
 *  @param  byte_size      Amount of data to write
 *
 *  @return				   Status of operation
 *
 ******************************************************************************/
RESULT HalWriteMemory (u32 mem_address, u8 *p_write_buffer, u32 byte_size);

/******************************************************************************
 * HalMapMemory()
******************************************************************************/
/**
 *  @brief  Maps a number of data from the memory into local memory starting at the given address
 *
 *  @param  mem_address    Source start address in hardware memory
 *
 *  @param  byte_size      Amount of data to map
 *
 *  @param  mapping_type   The way the mapping is performed
 *
 *  @param  pp_mapped_buf  Reference to pointer to the mapped local memory
 *
 *  @return                Status of operation
 *
 ******************************************************************************/
RESULT HalMapMemory (u32 mem_address, u32 byte_size, HalMapMemType_t mapping_type,
		     void **pp_mapped_buf);

/******************************************************************************
 * HalUnMapMemory()
******************************************************************************/
/**
 *  @brief  Unmaps previously mapped memory from local memory
 *
 *  @param  p_mapped_buf    Pointer to local memory to unmap as returned by HalMapMemory
 *
 *  @return					Status of operation
 *
 ******************************************************************************/
RESULT HalUnMapMemory (void *p_mapped_buf);

/******************************************************************************
 * HalReadI2CMem()
******************************************************************************/
/**
 *  @brief  Reads a number of data from the memory to a buffer starting at the given address
 *
 *  @param  bus_num        Number of bus which is to be used
 *
 *  @param  slave_addr     Address of slave to be accessed
 *
 *  @param  reg_address    Address of register to read
 *
 *  @param  reg_addr_size  Size of reg_address in bytes. Valid range: [0..4] bytes.
 *
 *  @param  p_read_buffer  Pointer to local memory holding the data being read
 *
 *  @param  byte_size      Amount of data to read
 *
 *  @return				   Status of operation
 *
 ******************************************************************************/
RESULT HalReadI2CMem (u8 bus_num, u16 slave_addr, u32 reg_address, u8 reg_addr_size,
		      u8 *p_read_buffer, u32 byte_size);

/******************************************************************************
 * HalWriteI2CMem()
******************************************************************************/
/**
 *  @brief  Writes a number of data from a buffer to the memory starting at the given address
 *
 *  @param  bus_num        Number of bus which is to be used
 *
 *  @param  slave_addr     Address of slave to be accessed
 *
 *  @param  reg_address    Address of register to write
 *
 *  @param  reg_addr_size  Size of reg_address in bytes. Valid range: [0..4] bytes.
 *
 *  @param  p_read_buffer  Pointer to local memory holding the data being written
 *
 *  @param  byte_size      Amount of data to read
 *
 *  @return				   Status of operation
 *
 ******************************************************************************/
RESULT HalWriteI2CMem (u8 bus_num, u16 slave_addr, u32 reg_address, u8 reg_addr_size,
		       u8 *p_write_buffer, u32 byte_size);

/******************************************************************************
 * HalConnectIrq()
******************************************************************************/
/**
 *  @brief  Register an interrupt service routine with the system software
 *
 *  @param  pIrqCtx        Reference of HAL IRQ context structure that represent this connection
 *
 *  @param  int_src        Number of the interrupt source, set to 0 if not needed
 *
 *  @param  IsrFunction    First interrupt routine, (first level handler); set to NULL if not needed
 *
 *  @param  DpcFunction    Second interrupt routine (second level handler)
 *
 *  @param  pContext       Context provided when the interrupt routines are called
 *
 *  @return				   Status of operation
 *
 ******************************************************************************/


/******************************************************************************
 * HalDisconnectIrq()
******************************************************************************/
/**
 *  @brief  Deregister an interrupt service routine from the system software
 *
 *  @param  pIrqCtx        Reference of HAL IRQ context structure that represent this connection
 *
 *  @return				   Status of operation
 *
 ******************************************************************************/
RESULT HalDisconnectIrq (HalIrqCtx_t *pIrqCtx);

/******************************************************************************
 * HalReadI2CReg()
******************************************************************************/
/**
 *  @brief  Reads a number of data from the memory to a buffer starting at the given address. References HalReadI2CMem
 *
 *  @param  bus_num        Number of bus which is to be used
 *
 *  @param  slave_addr     Address of slave to be accessed (supports auto detection of 10bit adresses)
 *
 *  @param  reg_address    Address of register to read. The register data gets read starting with the least significant byte
 *
 *  @param  reg_addr_size  Size of reg_address in bytes. Valid range: [0..4] bytes
 *
 *  @param  preg_value     Pointer to local memory holding the register data being read
 *
 *  @param  reg_size       Size of register in bytes. Valid range: [1..4].
 *
 *  @return				   Status of operation
 *
 ******************************************************************************/
RESULT HalXilReadI2CReg(u8 bus_num, u8 slave_addr, u32 reg_address, u8 reg_addr_size,
			void *preg_value, u8 reg_size);

/******************************************************************************
 * HalWriteI2CReg()
 *****************************************************************************/
/**
 *  @brief  Writes a number of data from a buffer to the memory starting at the given address. References HalWriteI2CMem
 *
 *  @param  bus_num        Number of bus which is to be used
 *
 *  @param  slave_addr     Address of slave to be accessed (supports auto detection of 10bit adresses)
 *
 *  @param  reg_address    Address of register to write. The register data gets written starting with the least significant byte.
 *
 *  @param  reg_addr_size  Size of reg_address in bytes. Valid range: [0..4] bytes
 *
 *  @param  reg_value      Pointer to local memory holding the register data being written
 *
 *  @param  reg_size       Size of register in bytes. Valid range: [1..4].
 *
 *  @return				   Status of operation
 *
 ******************************************************************************/
RESULT HalXilWriteI2CReg(u8 bus_num, u8 slave_addr, u16 reg_address, u8 reg_addr_size, u8 reg_value,
			 u32 reg_size);


#endif
