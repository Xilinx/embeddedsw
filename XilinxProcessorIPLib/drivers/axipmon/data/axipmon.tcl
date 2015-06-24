###############################################################################
#
# Copyright (C) 2012 - 2014 Xilinx, Inc.  All rights reserved.
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
# Use of the Software is limited solely to applications:
# (a) running on a Xilinx device, or
# (b) that interact with a Xilinx device through a bus or interconnect.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Except as contained in this notice, the name of the Xilinx shall not be used
# in advertising or otherwise to promote the sale, use or other dealings in
# this Software without prior written authorization from Xilinx.
#
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ----------------------------------------------------
# 6.1     adk    16/04/14  Added two new parameters(C_S_AXI4_BASEADDR,
#			   C_S_AXI4_HIGHADDR)
###############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
  ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XAxiPmon" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_GLOBAL_COUNT_WIDTH" "C_METRICS_SAMPLE_COUNT_WIDTH" "C_ENABLE_EVENT_COUNT" "C_NUM_MONITOR_SLOTS" "C_NUM_OF_COUNTERS" "C_HAVE_SAMPLED_METRIC_CNT" "C_ENABLE_EVENT_LOG" "C_FIFO_AXIS_DEPTH" "C_FIFO_AXIS_TDATA_WIDTH" "C_FIFO_AXIS_TID_WIDTH" "C_METRIC_COUNT_SCALE" "C_ENABLE_ADVANCED" "C_ENABLE_PROFILE" "C_ENABLE_TRACE" "C_S_AXI4_BASEADDR" "C_S_AXI4_HIGHADDR"
  ::hsi::utils::define_config_file  $drv_handle "xaxipmon_g.c" "XAxiPmon" "DEVICE_ID" "C_BASEADDR" "C_GLOBAL_COUNT_WIDTH" "C_METRICS_SAMPLE_COUNT_WIDTH" "C_ENABLE_EVENT_COUNT" "C_NUM_MONITOR_SLOTS" "C_NUM_OF_COUNTERS" "C_HAVE_SAMPLED_METRIC_CNT" "C_ENABLE_EVENT_LOG" "C_FIFO_AXIS_DEPTH" "C_FIFO_AXIS_TDATA_WIDTH" "C_FIFO_AXIS_TID_WIDTH" "C_METRIC_COUNT_SCALE" "C_ENABLE_ADVANCED" "C_ENABLE_PROFILE" "C_ENABLE_TRACE"
  ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "AxiPmon" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_GLOBAL_COUNT_WIDTH" "C_METRICS_SAMPLE_COUNT_WIDTH" "C_ENABLE_EVENT_COUNT" "C_NUM_MONITOR_SLOTS" "C_NUM_OF_COUNTERS" "C_HAVE_SAMPLED_METRIC_CNT" "C_ENABLE_EVENT_LOG" "C_FIFO_AXIS_DEPTH" "C_FIFO_AXIS_TDATA_WIDTH" "C_FIFO_AXIS_TID_WIDTH" "C_METRIC_COUNT_SCALE" "C_ENABLE_ADVANCED" "C_ENABLE_PROFILE" "C_ENABLE_TRACE" "C_S_AXI4_BASEADDR" "C_S_AXI4_HIGHADDR"
}
