/** \page example Examples
You can refer to the below stated example applications for more details which gives an idea
about display port.

The 2 DisplayPort RX examples are as follows.

@section ex1 xdp_rx_sst_dp159_example.c
This interrupt with timer example shows how to set up both the interrupt
system with interrupt handlers and how to override the default sleep/delay
functionality for MicroBlaze. A timer needs to exist in the hardware system
and will be used for sleep/delay functionality inside of a callback function.
The default behavior in MicroBlaze for sleep/delay is to call MB_Sleep from
microblaze_sleep.h, which has only millisecond accuracy. For ARM/Zynq
SoC systems, the supplied callback function will be ignored - the usleep
function will be called since the SoC has a timer built-in. The programming
sequence for the DP159 retimer is provided. This example uses the EmacPs's
interrupt driven DMA packet transfer mode to send and receive frames.

For details, see xdp_rx_sst_dp159_example.c.

@section ex2 xdp_rx_mst_example.c
This multi-stream transport (MST) example uses interrupts to detect events
coming from the TX. The example sets up a topology that the RX will respond
with, including LINK_ADDRESS reply and I2C and DPCD address spaces for the
downstream ports.

For details, see xdp_rx_mst_example.c.

@note All example functions start with Dprx_*, while all driver functions start with XDp_*.

The 5 DisplayPort TX examples are as follows.

@section ex3 xdp_tx_audio_example.c
This audio example, apart from training the main link and outputting video,
illustrates the sequence required for setting up audio in the DisplayPort
TX. This example requires that an audio source, such as a S/PDIF instance
be present and connected to the DisplayPort TX in the hardware system,
and for the audio enable configuration parameter to be set for the
instantiated DisplayPort TX. For audio to output, the user will need to
implement the following functions:
   a) Dptx_ConfigureAudioSrc : This function needs to configure the audio source
      to output to the DisplayPort TX.
   b) Dptx_AudioSendInfoFrame : This function needs to set up and write an audio
      info frame as per user requirements.

For details, see xdp_tx_audio_example.c.

@section ex4 xdp_tx_intr_example.c
This interrupt example shows how to set up the interrupt system and
specify the interrupt handlers for when a DisplayPort interrupt event
occurs. An interrupt controller with a connection to the DisplayPort
interrupt signal needs to exist in the hardware system.

For details, see xdp_tx_intr_example.c.

@section ex5 xdp_tx_mst_example.c
This multi-stream transport (MST) example shows how to use the driver's
MST capabilities. Streams can be directed at sinks using two methods:
   a) After topology discover has created a sink list, streams may be assigned
      to sinks using the index in the sink list.
   b) The streams may be assigned to sinks directly without using topology
      discovery if their relative addresses (and total number of DisplayPort
      links) from the DisplayPort source is known beforehand.

For details, see xdp_tx_mst_example.c.

@section ex6 xdp_tx_poll_example.c
This interrupt example shows how to poll the DisplayPort TX instance's
registers for DisplayPort interrupt events.

For details, see xdp_tx_poll_example.c.

@section ex7 xdp_tx_timer_example.c
This timer example shows how to override the default sleep/delay functionality
for MicroBlaze. A timer needs to exist in the hardware system and will be
used for sleep/delay functionality inside of a callback function. The
default behavior in MicroBlaze for sleep/delay is tocall MB_Sleep from
microblaze_sleep.h, which has only millisecond accuracy. For ARM/Zynq SoC
systems, the supplied callback function will be ignored - the usleep function
will be called since the SoC has a timer built-in.

For details, see xdp_tx_timer_example.c.

@section ex8 xdp_selftest_example.c
This self test example will perform a sanity check on the state of the DisplayPort
TX/RX instance. It may be called prior to usage of the core or after a reset to
ensure that (a subset of) the registers hold their default values.

For details, see xdp_selftest_example.c.

@section ex9 xdp_tx_example_common.c
This example performs a self test on the DisplayPort TX core by training the
main link at the maximum common capabilities between the TX and RX and
checking the lane status.

For details, see xdp_tx_example_common.c.

@section ex10 xdp_tx_example_common.h
This is the header file which is common for all examples.

For details, see xdp_tx_example_common.h.

@subsection IMPORTANT NOTES
 - Each of these examples are meant to be used in conjunction with
   xdp_tx_example_common.[ch] which holds common functionality for all examples.
   After importing the examples, these files will need to be manually copied into
   the example src/ directory.
   This code shows how to train the main link and set up a video stream.

 - Additionally, in order to be able to use the interrupt, polling, and timer
   examples, the user will need to implement and link the following functions:
   1) Dptx_InitPlatform : This function needs to do all hardware system
      initialization.
   2) Dptx_StreamSrc* : These function need to configure the source of the stream
      (pattern generators, video input, etc.) such that a video stream, with
      timings and video attributes that correspond to the main stream attributes
     (MSA) configuration, is received by the DisplayPort Tx. The examples call
      this function from the Dptx_Run->Dptx_StartVideoStream functions in
      xdp_tx_example_common.c.

@note  All example functions start with Dptx_*, while all driver functions start with XDp_*.
*/