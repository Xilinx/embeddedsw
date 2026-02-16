// ==============================================================
// Copyright (c) 1986 - 2021 Xilinx Inc. All rights reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_demosaic_linux.c
 * @addtogroup v_demosaic Overview
 */

#ifdef __linux__

/***************************** Include Files *********************************/
#include "xv_demosaic.h"

/***************** Macros (Inline Functions) Definitions *********************/
/** Maximum size for UIO device path string */
#define MAX_UIO_PATH_SIZE       256

/** Maximum size for UIO device name string */
#define MAX_UIO_NAME_SIZE       64

/** Maximum number of memory maps per UIO device */
#define MAX_UIO_MAPS            5

/** Invalid address marker for UIO memory mapping */
#define UIO_INVALID_ADDR        0

/**************************** Type Definitions *******************************/
/** UIO memory map structure for Demosaic device */
typedef struct {
    u32 addr;  /**< Physical base address of the memory region */
    u32 size;  /**< Size of the memory region in bytes */
} XV_demosaic_uio_map;

/** UIO device information structure for Demosaic IP */
typedef struct {
    int  uio_fd;                        /**< File descriptor for UIO device */
    int  uio_num;                       /**< UIO device number */
    char name[ MAX_UIO_NAME_SIZE ];     /**< Name of the UIO device */
    char version[ MAX_UIO_NAME_SIZE ];  /**< Version string of the UIO device */
    XV_demosaic_uio_map maps[ MAX_UIO_MAPS ];  /**< Array of memory maps */
} XV_demosaic_uio_info;

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/** Static instance of UIO device information */
static XV_demosaic_uio_info uio_info;

/************************** Function Definitions *****************************/
/**
 * @brief Reads a single line from a file.
 *
 * This helper function opens the specified file, reads a single line into
 * the provided buffer, removes any trailing newline character, and closes
 * the file.
 *
 * @param filename Pointer to the filename string to read from.
 * @param linebuf Buffer to store the line read from the file.
 *
 * @return 0 on success, -1 if file cannot be opened, -2 if read fails.
 *
 * @note The linebuf should be at least MAX_UIO_NAME_SIZE bytes.
 *
 *******************************************************************************/
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
 * @brief Reads the name of the UIO device from sysfs.
 *
 * This function constructs the sysfs path for the UIO device name and
 * reads it into the info structure's name field.
 *
 * @param info Pointer to the UIO info structure to populate.
 *
 * @return 0 on success, negative value on error.
 *
 * @note None
 *
 *******************************************************************************/
static int uio_info_read_name(XV_demosaic_uio_info* info) {
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/name", info->uio_num);
    return line_from_file(file, info->name);
}

/*****************************************************************************/
/**
 * @brief Reads the version of the UIO device from sysfs.
 *
 * This function constructs the sysfs path for the UIO device version and
 * reads it into the info structure's version field.
 *
 * @param info Pointer to the UIO info structure to populate.
 *
 * @return 0 on success, negative value on error.
 *
 * @note None
 *
 *******************************************************************************/
static int uio_info_read_version(XV_demosaic_uio_info* info) {
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/version", info->uio_num);
    return line_from_file(file, info->version);
}

/*****************************************************************************/
/**
 * @brief Reads the physical address of a UIO memory map from sysfs.
 *
 * This function retrieves the physical base address for the specified
 * memory map index from the sysfs interface and stores it in the info
 * structure.
 *
 * @param info Pointer to the UIO info structure to populate.
 * @param n Index of the memory map to read (0 to MAX_UIO_MAPS-1).
 *
 * @return 0 on success, -1 if file cannot be opened, -2 if read fails.
 *
 * @note None
 *
 *******************************************************************************/
static int uio_info_read_map_addr(XV_demosaic_uio_info* info, int n) {
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
 * @brief Reads the size of a UIO memory map from sysfs.
 *
 * This function retrieves the size in bytes for the specified memory map
 * index from the sysfs interface and stores it in the info structure.
 *
 * @param info Pointer to the UIO info structure to populate.
 * @param n Index of the memory map to read (0 to MAX_UIO_MAPS-1).
 *
 * @return 0 on success, -1 if file cannot be opened, -2 if read fails.
 *
 * @note None
 *
 *******************************************************************************/
static int uio_info_read_map_size(XV_demosaic_uio_info* info, int n) {
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
 * @brief Initializes the Demosaic IP instance using Linux UIO.
 *
 * This function looks up the UIO device by name in the /sys/class/uio
 * directory, retrieves device information, opens the device file, and
 * memory maps the control interface into user space. The slave interface
 * 'Ctrl' is expected to be mapped to map0.
 *
 * @param InstancePtr Pointer to the XV_demosaic instance to initialize.
 * @param InstanceName Name of the UIO device to bind to.
 *
 * @return XST_SUCCESS if initialization succeeds, XST_DEVICE_NOT_FOUND if
 *         the device cannot be found, or XST_OPEN_DEVICE_FAILED if the
 *         device cannot be opened.
 *
 * @note The caller must ensure InstancePtr is not NULL.
 *
 *******************************************************************************/
int XV_demosaic_Initialize(XV_demosaic *InstancePtr, const char* InstanceName) {
    XV_demosaic_uio_info *InfoPtr = &uio_info;
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
    InstancePtr->Config.BaseAddress = (u32)mmap(NULL, InfoPtr->maps[0].size, PROT_READ|PROT_WRITE, MAP_SHARED, InfoPtr->uio_fd, 0 * getpagesize());
    assert(InstancePtr->Config.BaseAddress);

    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief Releases resources associated with the Demosaic IP instance.
 *
 * This function unmaps the memory-mapped control interface and closes
 * the UIO device file descriptor, freeing all resources allocated during
 * initialization.
 *
 * @param InstancePtr Pointer to the XV_demosaic instance to release.
 *
 * @return XST_SUCCESS on successful release.
 *
 * @note The instance must have been previously initialized and must be
 *       in the ready state (XIL_COMPONENT_IS_READY).
 *
 *******************************************************************************/
int XV_demosaic_Release(XV_demosaic *InstancePtr) {
    XV_demosaic_uio_info *InfoPtr = &uio_info;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    munmap((void*)InstancePtr->Config.BaseAddress, InfoPtr->maps[0].size);

    close(InfoPtr->uio_fd);

    return XST_SUCCESS;
}

#endif
