/** \page example Examples
You can refer to the below stated example applications for more details which gives an idea
about dptxss.

@section ex1 xdptxss_selftest_example.c
This self test example will call self test functions of
each DisplayPort TX Subsystem's sub-cores to verify that
the sub-cores are programmed.

For details, see xdptxss_selftest_example.c.

@section ex2 xdptxss_mst_example.c
This multi-stream transport (MST) example shows how to use
the drivers's MST capabilities. This example reads the RX
device MST capability. Upon MST/SST capability, example sets
the transport mode.

For details, see xdptxss_mst_example.c.

@section ex3 xdptxss_intr_example.c
This interrupt example shows how to set up the interrupt system
and specify the interrupt handlers for when a DisplayPort TX Subsystem
interrupt event occurs. An interrupt controller with a connection to
the DisplayPort TX Subsystem interrupt signal needs to exist in the
hardware system.

For details, see xdptxss_intr_example.c.

@section ex4 xdptxss_debug_example.c
The debug example set up MST/SST mode based on RX capability
and prints debug information at the end. This example check
whether DisplayPort TX Subsystem is programmed in SST/MST.
Upon MST/SST, reads EDID information and prints.

For details, see xdptxss_debug_example.c.

@section ex5 xdptxss_hdcp_example.c
The HDCP example setup SST mode and enables the HDCP after RX has started.

For details, see xdptxss_hdcp_example.c.

@subsection IMPORTANT NOTES
 - Additionally, in order to be able to use the interrupt, MST/SST, and
   debug examples, the user will need to implement following functions:
   1) DpTxSs_PlatformInit : This function needs to do all hardware system
      initialization.
   2) DpTxSs_StreamSrc* : These function need to configure the source of the
      stream(pattern generators, video input, etc.) such that a video stream,
	  with timings and video attributes that correspond to the main stream
	  attributes(MSA) configuration, is received by the DisplayPort TX Subsystem.

@note All example functions start with DpTxSs_*, while all DisplayPort TX Subsystem driver functions start with XDpTxSs_*.
