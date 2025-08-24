#include <string.h>
// Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
	memcpy(p_data, &ATM_ENABLE, sizeof(int));
	packet.payload_size += sizeof(uint32_t);
	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;
	LOGI("%s   %d: after VsiVvdeviceInstanceInit ", __func__, __LINE__);

	result = Send_Command(APU_2_RPU_MB_CMD_SET_ATM, &packet, packet.payload_size + payload_extra_size,
			      dest_cpu_id, src_cpu_id);
	if (RET_SUCCESS != result)
		return RET_FAILURE;
	LOGI("%s   %d: after VsiVvdeviceInstanceInit ", __func__, __LINE__);

	apu_wait_for_ACK(packet.cookie, packet.payload_data);; //replace with wait_response();
	LOGI("%s   %d: after VsiVvdeviceInstanceInit ", __func__, __LINE__);

	return result;
}
