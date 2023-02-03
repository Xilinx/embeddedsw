/*
 * Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <metal/sys.h>
#include <metal/device.h>
#include <metal/io.h>
#include <metal/irq.h>
#include "metal-test.h"

/*
	test snippet in device tree amba subnode:
		shm0: shm@0 {
		 compatible = "shm_uio";
		 reg = <0x0 3ed80000 0x0 0x1000
			0x0 3fd80000 0x0 0x1000>;
		};
*/

#define	OFFSET	0x10
#define	SHM_DEV	"3ed80000.shm"
#define	BUS	"platform"
#define	NUM_REGIONS	2
#define TESTVAL	0xfeedbeef


/* Test for linux device having multiple, accessible IO regions */
static int io_test(void) {
	int ret = 0, i;
	struct metal_device *dev;
	struct metal_io_region *io[2];

	ret = metal_device_open(BUS, SHM_DEV, &dev);
	if (ret) {
		perror("metal_device_open(\"BUS\", \"SHM_DEV\", dev)\");");
		goto done;
	}

	for (i = 0; i < NUM_REGIONS; ++i) {
		io[i] = metal_device_io_region(dev, i);
		if (io[i] == NULL) {
			printf("Failed to map IO region (%d)\n", i);
			ret = -EINVAL;
			goto cleanup;
		}

		/* write in some test value that differs between each region */
		metal_io_write32(io[i], OFFSET, TESTVAL + 1);
	}

	for (i = 0; i < NUM_REGIONS; ++i) {
		if (metal_io_read32(io[i], OFFSET) != (long unsigned int)(TESTVAL+1)) {
			ret = -EINVAL;
			break;
		}
	}

cleanup:
	metal_device_close(dev);

done:
	return ret;
}
METAL_ADD_TEST(io_test);
