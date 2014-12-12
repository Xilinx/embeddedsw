/*
 * xedid_print.h
 *
 *  Created on: Nov 9, 2014
 *      Author: andreis
 */

#ifndef XEDID_PRINT_H_
#define XEDID_PRINT_H_

#include "xdptx.h"

u32 Edid_PrintDecodeBase(u8 *EdidRaw);
u32 Edid_PrintDecodeAll(XDptx *InstancePtr, u8 Lct, u8 *Rad);

#endif /* XEDID_PRINT_H_ */
