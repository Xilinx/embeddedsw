#include <string.h>
// Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
#include "visp_common.h"
int SetATM(void)
{
	//LOGI(" ATMenable = %d\n",ATM_ENABLE);
	RESULT result = RET_SUCCESS;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = 0xABCD;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;

	/* Send instanceId (0 for now) */
	uint32_t instanceId = 0;
	memcpy(p_data, &instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);

	/* Send high_mem prefix */
	uint32_t high_mem = ATM_HIGH_MEM_PREFIX ;
	memcpy(p_data, &high_mem, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);

	/* Send is_64bit flag */
	uint32_t is_64bit = 1;
	memcpy(p_data, &is_64bit, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SET_ATM, &packet, packet.payload_size + payload_extra_size,
			      dest_cpu_id, src_cpu_id);
	if (RET_SUCCESS != result)
		return RET_FAILURE;

	apu_wait_for_ACK(packet.cookie, packet.payload_data);; //replace with wait_response();

	return result;
}
