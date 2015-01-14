/*
 * xedid_print.h
 *
 *  Created on: Nov 9, 2014
 *      Author: andreis
 */

#ifndef XVIDC_EDID_PRINT_H_
#define XVIDC_EDID_PRINT_H_

#include "xdp.h"
#include "xvidc_edid.h"

u32 Edid_PrintDecodeBase(u8 *EdidRaw);
void Edid_Print_Supported_VideoModeTable(u8 *EdidRaw);

#endif /* XVIDC_EDID_PRINT_H_ */
