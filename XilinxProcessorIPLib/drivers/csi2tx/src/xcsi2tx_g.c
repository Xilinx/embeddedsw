#include "xcsi2tx.h"

XCsi2Tx_Config XCsi2Tx_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,mipi-csi2-tx-ctrl-1.0", /* compatible */
		0x0, /* reg */
		0x0, /* xlnx,csi2-tx-lanes */
		0x0, /* xlnx,csi2-tx-en-active-lanes */
		0x0 /* xlnx,csi2-tx-en-reg-based-fe-gen */
	},
	 {
		 NULL
	}
};