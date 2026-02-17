// ==============================================================
// Copyright (c) 1986 - 2022 Xilinx Inc. All rights reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================
#ifdef __linux__

/**
 * @file xv_warp_init_linux.c
 * @addtogroup v_warp_init Overview
 */

/***************************** Include Files *********************************/
#include "xv_warp_init.h"

/**************************** Type Definitions *******************************/
/**
 * UIO memory map structure
 */
typedef struct {
    u64 addr;  /**< Physical address of the memory map */
    u32 size;  /**< Size of the memory map in bytes */
} XV_warp_init_uio_map;

/**
 * UIO device information structure
 */
typedef struct {
    int  uio_fd;                           /**< UIO device file descriptor */
    int  uio_num;                          /**< UIO device number */
    char name[ MAX_UIO_NAME_SIZE ];        /**< UIO device name */
    char version[ MAX_UIO_NAME_SIZE ];     /**< UIO device version */
    XV_warp_init_uio_map maps[ MAX_UIO_MAPS ];  /**< Array of memory maps */
} XV_warp_init_uio_info;

/***************** Macros (Inline Functions) Definitions *********************/
/** Maximum size for UIO file path strings */
#define MAX_UIO_PATH_SIZE       256

/** Maximum size for UIO name strings */
#define MAX_UIO_NAME_SIZE       64

/** Maximum number of UIO memory maps */
#define MAX_UIO_MAPS            5

/** Invalid address indicator for UIO mapping */
#define UIO_INVALID_ADDR        0

/************************** Variable Definitions *****************************/
/** Static UIO device information instance */
static XV_warp_init_uio_info uio_info;

/************************** Function Definitions *****************************/
/**
 * @brief Read a line from a file
 *
 * This function reads a single line from the specified file and stores it
 * in the provided buffer, removing any trailing newline character.
 *
 * @param  filename Pointer to the filename string to read from
 * @param  linebuf Pointer to the buffer to store the line
 *
 * @return 0 on success, -1 if file cannot be opened, -2 if read fails
 *
 * @note The buffer must be at least MAX_UIO_NAME_SIZE bytes
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
 * @brief Read the UIO device name from sysfs
 *
 * This function reads the name of the UIO device from the sysfs filesystem
 * and stores it in the info structure.
 *
 * @param  info Pointer to the UIO information structure
 *
 * @return 0 on success, negative value on failure
 *
 * @note None
 *
 ******************************************************************************/
static int uio_info_read_name(XV_warp_init_uio_info* info) {
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/name", info->uio_num);
    return line_from_file(file, info->name);
}

/*****************************************************************************/
/**
 * @brief Read the UIO device version from sysfs
 *
 * This function reads the version string of the UIO device from the sysfs
 * filesystem and stores it in the info structure.
 *
 * @param  info Pointer to the UIO information structure
 *
 * @return 0 on success, negative value on failure
 *
 * @note None
 *
 ******************************************************************************/
static int uio_info_read_version(XV_warp_init_uio_info* info) {
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/version", info->uio_num);
    return line_from_file(file, info->version);
}

/*****************************************************************************/
/**
 * @brief Read the physical address of a UIO memory map
 *
 * This function reads the physical address of the specified memory map
 * from the sysfs filesystem and stores it in the info structure.
 *
 * @param  info Pointer to the UIO information structure
 * @param  n Index of the memory map to read
 *
 * @return 0 on success, -1 if file cannot be opened, -2 if read fails
 *
 * @note None
 *
 ******************************************************************************/
static int uio_info_read_map_addr(XV_warp_init_uio_info* info, int n) {
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
 * @brief Read the size of a UIO memory map
 *
 * This function reads the size of the specified memory map from the sysfs
 * filesystem and stores it in the info structure.
 *
 * @param  info Pointer to the UIO information structure
 * @param  n Index of the memory map to read
 *
 * @return 0 on success, -1 if file cannot be opened, -2 if read fails
 *
 * @note None
 *
 ******************************************************************************/
static int uio_info_read_map_size(XV_warp_init_uio_info* info, int n) {
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
 * @brief Initialize the warp init driver instance
 *
 * This function initializes the XV_warp_init driver instance by locating
 * the UIO device with the specified name, opening it, and mapping its
 * control interface into user space memory.
 *
 * @param  InstancePtr Pointer to the XV_warp_init instance
 * @param  InstanceName Name of the UIO device to open
 *
 * @return XST_SUCCESS on success
 *         XST_DEVICE_NOT_FOUND if device is not found
 *         XST_OPEN_DEVICE_FAILED if device cannot be opened
 *
 * @note The InstancePtr must not be NULL and the device name must match
 *       a valid UIO device in /sys/class/uio
 *
 ******************************************************************************/
int XV_warp_init_Initialize(XV_warp_init *InstancePtr, const char* InstanceName) {
	XV_warp_init_uio_info *InfoPtr = &uio_info;
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
    InstancePtr->Ctrl_BaseAddress = (u64)mmap(NULL, InfoPtr->maps[0].size, PROT_READ|PROT_WRITE, MAP_SHARED, InfoPtr->uio_fd, 0 * getpagesize());
    assert(InstancePtr->Ctrl_BaseAddress);

    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief Release the warp init driver instance
 *
 * This function releases resources associated with the XV_warp_init driver
 * instance by unmapping the control interface from user space and closing
 * the UIO device file descriptor.
 *
 * @param  InstancePtr Pointer to the XV_warp_init instance
 *
 * @return XST_SUCCESS on success
 *
 * @note The InstancePtr must not be NULL and must be in the ready state
 *
 ******************************************************************************/
int XV_warp_init_Release(XV_warp_init *InstancePtr) {
	XV_warp_init_uio_info *InfoPtr = &uio_info;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    munmap((void*)InstancePtr->Ctrl_BaseAddress, InfoPtr->maps[0].size);

    close(InfoPtr->uio_fd);

    return XST_SUCCESS;
}

#endif
