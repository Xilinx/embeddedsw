###############################################################################
#
# Copyright (C) 2003 - 2016 Xilinx, Inc.  All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
#
#
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ------------------------------------
# 9.0     adk    12/10/13 Updated as per the New Tcl API's
# 9.0     bss    02/20/14 Added kintex 8 and kintexu families
# 10.0	  bss 	 06/24/14 Modified not to generate family.h so that
#		  	  all families are allowed.
##############################################################################
#--------------------------------
# Tcl procedure generate 
#--------------------------------

proc generate {drv_handle} {
    
    # Generate #defines in xparameters.h
    xdefine_include_file $drv_handle "xparameters.h" "XHwIcap" "NUM_INSTANCES" "C_BASEADDR" "C_HIGHADDR" "DEVICE_ID" "C_ICAP_DWIDTH" "C_MODE"
       
    # Generate the _g.c configuration file
    xdefine_config_file $drv_handle "xhwicap_g.c" "XHwIcap"  "DEVICE_ID" "C_BASEADDR" "C_ICAP_DWIDTH" "C_MODE" 
       
    xdefine_canonical_xpars $drv_handle "xparameters.h" "HwIcap" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_ICAP_DWIDTH" "C_MODE"

}


