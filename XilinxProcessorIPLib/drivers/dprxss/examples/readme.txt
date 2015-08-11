There are 4 DisplayPort Receiver Subsystem examples include in this directory:
1) xdprxss_selftest_example.c : This self test example will call self test
   functions of each DisplayPort RX Subsystem's sub-cores to verify that the sub-cores
   are programmed.

2) xdprxss_mst_example.c : This multi-stream transport (MST) example shows how to
   configure driver for MST/SST based upon system configuration parameters.
   For this example, interrupt controller and timer controller must be present.

3) xdprxss_intr_example.c : This interrupt example shows how to set up the interrupt
   system and specify the interrupt handlers for when a DisplayPort RX Subsystem
   interrupt event occurs. An interrupt controller with a connection to the DisplayPort
   RX Subsystem interrupt signal needs to exist in the hardware system.

4) xdprxss_debug_example.c : The debug example setup MST/SST mode based on system confi-
   guration parameters and prints debug information at the end.

Additionally, in order to be able to use the interrupt, MST/SST, and debug examples,
the user will need to implement following functions:
1) DpRxSs_PlatformInit : This function needs to do all hardware system
   initialization.
2) DpRxSs_VideoPhyInit : These function need to configure the Video Phy using video phy
   driver functions. For information, refer video phy driver documentation.
3) DpTxSs_Setup : These function need to configure the DisplayPort TX Subsystem using
   DPTX Subsystem driver functions. For information, refer DPTXSS driver documentation.

Note: All example functions start with DpRxSs_*, while all DisplayPort RX Subsystem
driver functions start with XDpRxSs_*.
