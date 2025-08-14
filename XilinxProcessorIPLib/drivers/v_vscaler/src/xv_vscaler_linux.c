// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifdef __linux__

/***************************** Include Files *********************************/
#include "xv_vscaler.h"

/***************** Macros (Inline Functions) Definitions *********************/
#define MAX_UIO_PATH_SIZE       256
#define MAX_UIO_NAME_SIZE       64
#define MAX_UIO_MAPS            5
#define UIO_INVALID_ADDR        0

/**************************** Type Definitions ******************************/
/**
 * @brief Structure representing a memory map for the XV_vscaler UIO device.
 *
 * This structure holds the base address and size of a mapped region
 * for the XV_vscaler hardware in a Linux environment.
 *
 * @param addr Base address of the mapped region.
 * @param size Size (in bytes) of the mapped region.
 */
typedef struct {
    u32 addr;
    u32 size;
} XV_vscaler_uio_map;


/**
 * @brief Structure holding UIO device information for XV_vscaler.
 *
 * This structure contains file descriptor, device number, device name,
 * version string, and memory map information for the XV_vscaler UIO device.
 */
typedef struct {
    int  uio_fd;                                 /**< File descriptor for /dev/uioX */
    int  uio_num;                                /**< UIO device number (X in uioX) */
    char name[ MAX_UIO_NAME_SIZE ];              /**< Device name string */
    char version[ MAX_UIO_NAME_SIZE ];           /**< Device version string */
    XV_vscaler_uio_map maps[ MAX_UIO_MAPS ];     /**< Array of memory maps */
} XV_vscaler_uio_info;

/***************** Variable Definitions **************************************/
static XV_vscaler_uio_info uio_info;

/************************** Function Implementation *************************/
/**
 * Reads a single line from the specified file into the provided buffer.
 *
 * @param filename Path to the file to read from.
 * @param linebuf Buffer to store the read line.
 * @return 0 on success, -1 if file can't be opened, -2 if line can't be read.
 */
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
/**
 * @brief Reads the name of the UIO device and stores it in the provided info structure.
 *
 * This function constructs the path to the UIO device's name file in the sysfs
 * (e.g., "/sys/class/uio/uioX/name" where X is the UIO device number), reads
 * the device name from the file, and stores it in the 'name' field of the
 * XV_vscaler_uio_info structure.
 *
 * @param info Pointer to an XV_vscaler_uio_info structure containing the UIO device number.
 * @return positive value on success, or a negative value on failure.
 */

static int uio_info_read_name(XV_vscaler_uio_info* info) {
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/name", info->uio_num);
    return line_from_file(file, info->name);
}


/**
 * Reads the version information of a UIO device and stores it in the provided info structure.
 *
 * @param info Pointer to an XV_vscaler_uio_info structure containing the UIO device number.
 * @return Returns the result of the line_from_file function, typically 0 on success or a negative value on failure.
 *
 * This function constructs the path to the version file for the specified UIO device,
 * reads its contents, and stores the version string in the info structure.
 */
static int uio_info_read_version(XV_vscaler_uio_info* info) {
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/version", info->uio_num);
    return line_from_file(file, info->version);
}

/**
 * Reads the physical address of the specified UIO memory map and stores it in the info structure.
 *
 * @param info Pointer to the XV_vscaler_uio_info structure where the map address will be stored.
 * @param n    Index of the memory map to read the address for.
 * @return     0 on success,
 *            -1 if the address file could not be opened,
 *            -2 if the address could not be read from the file.
 *
 * This function constructs the path to the sysfs file containing the physical address
 * of the specified UIO memory map, opens the file, reads the address, and stores it
 * in the corresponding entry of the info->maps array. If any error occurs during
 * file operations or reading, an appropriate negative error code is returned.
 */
static int uio_info_read_map_addr(XV_vscaler_uio_info* info, int n) {
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



/**
 * Reads the size of the specified UIO memory map from the sysfs interface.
 *
 * @param info Pointer to the XV_vscaler_uio_info structure containing UIO device information.
 * @param n    Index of the memory map to read the size for.
 *
 * @return 0 on success,
 *         -1 if the size file could not be opened,
 *         -2 if the size could not be read from the file.
 *
 * This function constructs the path to the sysfs file that contains the size of the
 * specified memory map for the given UIO device, opens the file, reads the size value,
 * and stores it in the corresponding map entry in the info structure.
 */
static int uio_info_read_map_size(XV_vscaler_uio_info* info, int n) {
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
/**
 * Initializes the XV_vscaler instance by searching for the UIO device with the
 * specified name, reading its configuration from sysfs, and mapping its control
 * interface into user space.
 *
 * This function scans the /sys/class/uio directory to find a UIO device whose
 * name matches the provided InstanceName. Once found, it reads the device's
 * name, version, and memory map information, opens the corresponding /dev/uioX
 * device file, and maps the control interface into the process address space.
 *
 * @param InstancePtr Pointer to the XV_vscaler instance to initialize.
 * @param InstanceName Name of the UIO device instance to search for.
 * @return XST_SUCCESS on success, XST_DEVICE_NOT_FOUND if the device is not found,
 *         or XST_OPEN_DEVICE_FAILED if the device file cannot be opened.
 */
int XV_vscaler_Initialize(XV_vscaler *InstancePtr, const char* InstanceName) {
    XV_vscaler_uio_info *InfoPtr = &uio_info;
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

/**
 * Releases resources allocated for the XV_vscaler instance.
 *
 * This function unmaps the memory region associated with the XV_vscaler hardware
 * and closes the corresponding UIO file descriptor. It should be called when the
 * XV_vscaler instance is no longer needed to free system resources.
 *
 * @param InstancePtr Pointer to the XV_vscaler instance to be released.
 *        Must not be NULL and must be initialized.
 *
 * @return
 *     - XST_SUCCESS if the resources were released successfully.
 *
 * @note
 *     The function asserts that InstancePtr is not NULL and that the instance is ready.
 */


int XV_vscaler_Release(XV_vscaler *InstancePtr) {
    XV_vscaler_uio_info *InfoPtr = &uio_info;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    munmap((void*)InstancePtr->Config.BaseAddress, InfoPtr->maps[0].size);

    close(InfoPtr->uio_fd);

    return XST_SUCCESS;
}

#endif
