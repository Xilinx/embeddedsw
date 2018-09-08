/** \page example Examples
You can refer to the below stated example applications for more details which gives an idea
about dprxss.

@section ex1 xdprxss_selftest_example.c
This self test example will call self test functions of each DisplayPort
RX Subsystem's sub-cores to verify that the sub-cores are programmed.

For details, see xdprxss_selftest_example.c.

@section ex2 xdprxss_mst_example.c
This multi-stream transport (MST) example shows how to configure driver
for MST/SST based upon system configuration parameters. For this example,
interrupt controller and timer controller must be present.

For details, see xdprxss_mst_example.c.

@section ex3 xdprxss_intr_example.c
This interrupt example shows how to set up the interrupt system and
specify the interrupt handlers for when a DisplayPort RX Subsystem
interrupt event occurs. An interrupt controller with a connection to
the DisplayPort RX Subsystem interrupt signal needs to exist in the
hardware system.

For details, see xdprxss_intr_example.c.

@section ex4 xdprxss_debug_example.c
The debug example setup MST/SST mode based on system configuration
parameters and prints debug information at the end.

For details, see xdprxss_debug_example.c.

@section ex5 xdprxss_hdcp_example.c
The HDCP example setup SST mode and enables the HDCP after RX has started.

For details, see xdprxss_hdcp_example.c.

@subsection IMPORTANT NOTES
 - Additionally, in order to be able to use the interrupt, MST/SST, and debug
   examples, the user will need to implement following functions:
   1) DpRxSs_PlatformInit : This function needs to do all hardware system
      initialization.
   2) DpRxSs_VideoPhyInit : These function need to configure the Video Phy using video phy
      driver functions. For information, refer video phy driver documentation.
   3) DpRxSs_Setup : These function need to configure the DisplayPort RX Subsystem using
      DPRX Subsystem driver functions. For information, refer DPRXSS driver documentation.

@note All example functions start with DpRxSs_*, while all DisplayPort RX Subsystem driver functions start with XDpRxSs_*.
*/
