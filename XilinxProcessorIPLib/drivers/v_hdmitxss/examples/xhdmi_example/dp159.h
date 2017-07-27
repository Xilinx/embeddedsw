#ifndef DP159_H		 /* prevent circular inclusions */
#define DP159_H		 /* by using protection macros */

#include "xil_types.h"
#include "xvphy.h"

// Function prototypes
u32 i2c_dp159(XVphy *VphyPtr, u8 QuadId, u64 TxLineRate);
u32 i2c_dp159_write(u8 dev, u8 addr, u8 dat);
void i2c_dp159_dump(void);

#endif
