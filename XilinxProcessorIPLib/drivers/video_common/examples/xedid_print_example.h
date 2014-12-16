/*
 * xedid_print.h
 *
 *  Created on: Nov 9, 2014
 *      Author: andreis
 */

#ifndef XEDID_PRINT_H_
#define XEDID_PRINT_H_

#include "xdptx.h"
#include "xedid.h"

u32 Edid_PrintDecodeBase(u8 *EdidRaw);
void Edid_Print_Supported_VideoModeTable(u8 *EdidRaw);

#endif /* XEDID_PRINT_H_ */
