/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of Mentor Graphics Corporation nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**************************************************************************
 * FILE NAME
 *
 *       firmware.c
 *
 * COMPONENT
 *
 *         OpenAMP stack.
 *
 * DESCRIPTION
 *
 *
 **************************************************************************/

#include <string.h>
#include "openamp/firmware.h"

/**
 * config_get_firmware
 *
 * Searches the given firmware in firmware table list and provides
 * it to caller.
 *
 * @param fw_name    - name of the firmware
 * @param start_addr - pointer t hold start address of firmware
 * @param size       - pointer to hold size of firmware
 *
 * returns -  status of function execution
 *
 */

extern struct firmware_info fw_table[];
extern int fw_table_size;

int config_get_firmware(char *fw_name, uintptr_t *start_addr,
			unsigned int *size)
{
	int idx;
	for (idx = 0; idx < fw_table_size; idx++) {
		if (!strncmp((char *)fw_table[idx].name, fw_name, sizeof(fw_table[idx].name))) {
			*start_addr = fw_table[idx].start_addr;
			*size =
			    fw_table[idx].end_addr - fw_table[idx].start_addr +
			    1;
			return 0;
		}
	}
	return -1;
}
