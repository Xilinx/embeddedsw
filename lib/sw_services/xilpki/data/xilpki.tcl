###############################################################################
# Copyright (C) 2023, Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
#
# Modification History
#
# Ver   Who   Date      Changes
# ----- ----  --------  -----------------------------------------------
# 1.0   Nava  12/05/22  First release
#
##############################################################################

#---------------------------------------------
# pki_drc
#---------------------------------------------
proc pki_drc {libhandle} {
	# check processor type
	set proc_instance [hsi::get_sw_processor];
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]
	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];

}
