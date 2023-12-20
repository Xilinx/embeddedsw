#include "xcsi2txss.h"

XCsi2TxSs_Config XCsi2TxSs_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,mipi-csi2-tx-subsystem-2.2", /* compatible */
		0x44a00000, /* reg */
		0x0, /* xlnx,highaddr */
		0x0, /* xlnx,csi2-tx-lanes */
		0x0, /* xlnx,csi2-tx-data-type */
		0x0, /* xlnx,csi2x-tx-pixel-mode */
		0x0, /* xlnx,csi2-tx-line-bufr-depth */
		0x0, /* xlnx,dphy-linerate */
		0x0, /* xlnx,dphy-en-reg-if */
		0x0, /* xlnx,en-reg-based-fe-gen */
		0x0, /* csi2-tx-present */
		0x0, /* csi2-tx-connected */
		0x0, /* mipi-dphy-present */
		0x0, /* mipi-dphy-connected */
		0xffff, /* interrupts */
		0xffff /* interrupt-parent */
	},
	 {
		 NULL
	}
};