There are 4 examples included in this directory:
1) xdptx_intr_example.c : This interrupt example shows how to set up the
   interrupt system and specify the interrupt handlers for when a DisplayPort
   interrupt event occurs. An interrupt controller with a connection to the
   DisplayPort interrupt signal needs to exist in the hardware system.

2) xdptx_poll_example.c : This interrupt example shows how to poll the
   DisplayPort TX instance's registers for DisplayPort interrupt events.

3) xdptx_timer_example.c : This timer example shows how to override the default
   sleep/delay functionality for MicroBlaze. A timer needs to exist in the
   hardware system and will be used for sleep/delay functionality inside of a
   callback function. The default behavior in MicroBlaze for sleep/delay is to
   call MB_Sleep from microblaze_sleep.h, which has only millisecond accuracy.
   For ARM/Zynq SoC systems, the supplied callback function will be ignored -
   the usleep function will be called since the SoC has a timer built-in.

4) xdptx_selftest_example.c : This self test example will perform a sanity check
   on the state of the DisplayPort TX instance. It may be called prior to usage
   of the core or after a reset to ensure that (a subset of) the registers hold
   the default values.

Each of these examples are meant to be used in conjunction with
xdptx_example_common.[ch] which holds common functionality for all examples.
After importing the examples, these files will need to be manually copied into
the example src/ directory.
This code shows how to train the main link and set up the 

Additionally, in order to be able to use the interrupt, polling, and timer
examples, the user will need to implement and link the following functions:
1) Dptx_InitPlatform : This function needs to do all hardware system
   initialization. This function is invoked when calling 
2) Dptx_ConfigureStreamSrc : This function needs to configure the source of the
   stream (pattern generators, video input, etc.) such that a video stream, with
   timings and video attributes that correspond to the main stream attributes
   (MSA) configuration, is received by the DisplayPort Tx. The examples call
   this function from the Dptx_Run->Dptx_StartVideoStream functions in
   xdptx_example_common.c.

Note: All example functions start with Dptx_*, while all driver functions start
with XDptx_*.
