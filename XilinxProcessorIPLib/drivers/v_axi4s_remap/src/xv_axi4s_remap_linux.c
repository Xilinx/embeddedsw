// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_axi4s_remap_linux.c
 * @addtogroup v_axi4s_remap Overview
 */

#ifdef __linux__

/***************************** Include Files *********************************/
#include "xv_axi4s_remap.h"

/************************** Constant Definitions *****************************/
/** Maximum size for UIO device path strings */
#define MAX_UIO_PATH_SIZE       256
/** Maximum size for UIO device name strings */
#define MAX_UIO_NAME_SIZE       64
/** Maximum number of memory maps per UIO device */
#define MAX_UIO_MAPS            5
/** Invalid UIO address marker */
#define UIO_INVALID_ADDR        0

/**************************** Type Definitions ********************************/
/**
 * UIO memory map information structure
 */
typedef struct {
    u32 addr;  /**< Physical address of the memory region */
    u32 size;  /**< Size of the memory region in bytes */
} XV_axi4s_remap_uio_map;

/**
 * UIO device information structure
 */
typedef struct {
    int  uio_fd;                            /**< UIO device file descriptor */
    int  uio_num;                           /**< UIO device number */
    char name[ MAX_UIO_NAME_SIZE ];         /**< UIO device name */
    char version[ MAX_UIO_NAME_SIZE ];      /**< UIO driver version */
    XV_axi4s_remap_uio_map maps[ MAX_UIO_MAPS ]; /**< Memory map information */
} XV_axi4s_remap_uio_info;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int line_from_file(char* filename, char* linebuf);
static int uio_info_read_name(XV_axi4s_remap_uio_info* info);
static int uio_info_read_version(XV_axi4s_remap_uio_info* info);
static int uio_info_read_map_addr(XV_axi4s_remap_uio_info* info, int n);
static int uio_info_read_map_size(XV_axi4s_remap_uio_info* info, int n);

/************************** Variable Definitions *****************************/
/** Static UIO device information */
static XV_axi4s_remap_uio_info uio_info;

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*                                                                           */
/*                       FUNCTION IMPLEMENTATIONS                            */
/*                                                                           */
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/*****************************************************************************/
/**
 * @brief Read a single line from a file
 *
 * This helper function reads the first line from a file and stores it in the
 * provided buffer. It removes any trailing newline characters.
 *
 * @param  filename Pointer to the filename string to read from
 * @param  linebuf Buffer to store the read line (must be at least MAX_UIO_NAME_SIZE)
 *
 * @return 0 on success
 *         -1 if file cannot be opened
 *         -2 if file read fails
 *
 * @note The linebuf must be pre-allocated by the caller
 *
 ******************************************************************************/
static int line_from_file(char* filename, char* linebuf) {
    char* s;
    int i;
    FILE* fp = fopen(filename, "r");
    if (!fp) return -1;
    s = fgets(linebuf, MAX_UIO_NAME_SIZE, fp);
    fclose(fp);
    if (!s) return -2;
    for (i=0; (*s)&&(i<MAX_UIO_NAME_SIZE); i++) {
        if (*s == '\n') *s = 0;
        s++;
    }
    return 0;
}

/*****************************************************************************/
/**
 * @brief Read UIO device name from sysfs
 *
 * This function reads the name of the UIO device from the Linux sysfs
 * filesystem and stores it in the uio_info structure.
 *
 * @param  info Pointer to UIO device information structure
 *
 * @return 0 on success, negative value on failure
 *
 * @note Accesses /sys/class/uio/uioN/name
 *
 *******************************************************************************/
static int uio_info_read_name(XV_axi4s_remap_uio_info* info) {
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/name", info->uio_num);
    return line_from_file(file, info->name);
}

/*****************************************************************************/
/**
 * @brief Read UIO driver version from sysfs
 *
 * This function reads the version information of the UIO driver from the
 * Linux sysfs filesystem and stores it in the uio_info structure.
 *
 * @param  info Pointer to UIO device information structure
 *
 * @return 0 on success, negative value on failure
 *
 * @note Accesses /sys/class/uio/uioN/version
 *
 *******************************************************************************/
static int uio_info_read_version(XV_axi4s_remap_uio_info* info) {
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/version", info->uio_num);
    return line_from_file(file, info->version);
}

/*****************************************************************************/
/**
 * @brief Read physical address of UIO memory map from sysfs
 *
 * This function reads the physical base address of a specific memory map
 * region from the Linux sysfs filesystem.
 *
 * @param  info Pointer to UIO device information structure
 * @param  n Memory map index (0 to MAX_UIO_MAPS-1)
 *
 * @return 0 on success
 *         -1 if file cannot be opened
 *         -2 if file read/parse fails
 *
 * @note Accesses /sys/class/uio/uioN/maps/mapM/addr
 *
 *******************************************************************************/
static int uio_info_read_map_addr(XV_axi4s_remap_uio_info* info, int n) {
    int ret;
    char file[ MAX_UIO_PATH_SIZE ];
    info->maps[n].addr = UIO_INVALID_ADDR;
    sprintf(file, "/sys/class/uio/uio%d/maps/map%d/addr", info->uio_num, n);
    FILE* fp = fopen(file, "r");
    if (!fp) return -1;
    ret = fscanf(fp, "0x%x", &info->maps[n].addr);
    fclose(fp);
    if (ret < 0) return -2;
    return 0;
}

/*****************************************************************************/
/**
 * @brief Read size of UIO memory map from sysfs
 *
 * This function reads the size of a specific memory map region from the
 * Linux sysfs filesystem.
 *
 * @param  info Pointer to UIO device information structure
 * @param  n Memory map index (0 to MAX_UIO_MAPS-1)
 *
 * @return 0 on success
 *         -1 if file cannot be opened
 *         -2 if file read/parse fails
 *
 * @note Accesses /sys/class/uio/uioN/maps/mapM/size
 *
 *******************************************************************************/
static int uio_info_read_map_size(XV_axi4s_remap_uio_info* info, int n) {
    int ret;
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/maps/map%d/size", info->uio_num, n);
    FILE* fp = fopen(file, "r");
    if (!fp) return -1;
    ret = fscanf(fp, "0x%x", &info->maps[n].size);
    fclose(fp);
    if (ret < 0) return -2;
    return 0;
}

/*****************************************************************************/
/**
 * @brief Initialize AXI4-Stream Remap driver instance on Linux
 *
 * This function initializes the AXI4-Stream Remap driver by locating the
 * corresponding UIO device in the Linux system, opening the device file,
 * and mapping the control register space into user space memory. This allows
 * user-space applications to access the hardware registers.
 *
 * @param  InstancePtr Pointer to the driver instance structure to initialize
 * @param  InstanceName String name of the UIO device to locate and initialize
 *
 * @return XST_SUCCESS if initialization completed successfully
 *         XST_DEVICE_NOT_FOUND if UIO device not found in system
 *         XST_OPEN_DEVICE_FAILED if device file cannot be opened
 *
 * @note The control interface must be mapped to /dev/uioN map0
 * @note InstancePtr->IsReady is set to XIL_COMPONENT_IS_READY on success
 *
 *******************************************************************************/
int XV_axi4s_remap_Initialize(XV_axi4s_remap *InstancePtr, const char* InstanceName) {
	XV_axi4s_remap_uio_info *InfoPtr = &uio_info;
	struct dirent **namelist;
    int i, n;
    char* s;
    char file[ MAX_UIO_PATH_SIZE ];
    char name[ MAX_UIO_NAME_SIZE ];
    int flag = 0;

    assert(InstancePtr != NULL);

    n = scandir("/sys/class/uio", &namelist, 0, alphasort);
    if (n < 0)  return XST_DEVICE_NOT_FOUND;
    for (i = 0;  i < n; i++) {
	strcpy(file, "/sys/class/uio/");
	strcat(file, namelist[i]->d_name);
	strcat(file, "/name");
        if ((line_from_file(file, name) == 0) && (strcmp(name, InstanceName) == 0)) {
            flag = 1;
            s = namelist[i]->d_name;
            s += 3; // "uio"
            InfoPtr->uio_num = atoi(s);
            break;
        }
    }
    if (flag == 0)  return XST_DEVICE_NOT_FOUND;

    uio_info_read_name(InfoPtr);
    uio_info_read_version(InfoPtr);
    for (n = 0; n < MAX_UIO_MAPS; ++n) {
        uio_info_read_map_addr(InfoPtr, n);
        uio_info_read_map_size(InfoPtr, n);
    }

    sprintf(file, "/dev/uio%d", InfoPtr->uio_num);
    if ((InfoPtr->uio_fd = open(file, O_RDWR)) < 0) {
        return XST_OPEN_DEVICE_FAILED;
    }

    // NOTE: slave interface 'Ctrl' should be mapped to uioX/map0
    InstancePtr->BaseAddress = (u32)mmap(NULL, InfoPtr->maps[0].size, PROT_READ|PROT_WRITE, MAP_SHARED, InfoPtr->uio_fd, 0 * getpagesize());
    assert(InstancePtr->BaseAddress);

    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief Release AXI4-Stream Remap driver resources on Linux
 *
 * This function releases resources allocated during initialization by
 * unmapping the control register space from user memory and closing the
 * UIO device file descriptor.
 *
 * @param  InstancePtr Pointer to the driver instance structure to release
 *
 * @return XST_SUCCESS if release completed successfully
 *
 * @note The instance must have been successfully initialized before calling
 *       this function (IsReady == XIL_COMPONENT_IS_READY)
 *
 *******************************************************************************/
int XV_axi4s_remap_Release(XV_axi4s_remap *InstancePtr) {
	XV_axi4s_remap_uio_info *InfoPtr = &uio_info;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    munmap((void*)InstancePtr->BaseAddress, InfoPtr->maps[0].size);

    close(InfoPtr->uio_fd);

    return XST_SUCCESS;
}

#endif
