/** \page example Examples
You can refer to the below stated example applications for more details on how to use rfdc driver.

@section ex1 xrfdc_selftest_example.c
contains a selftest example for using the rfdc hardware and
RFSoC Data Converter driver.
This example does some writes to the hardware to do some sanity checks
and does a reset to restore the original settings.

For details, see xrfdc_selftest_example.c.

@section ex2 xrfdc_read_write_example.c
Contains an example to use multiple driver "set" APIs to configure the targeted
AMS block.
Subsequently it uses "get" APIs to read back the configurations to ensure
that the desired configurations are applied.
For DAC it sets the following configurations:
MixerSettings, QMCSettings, Write Fabricrate, Decoder mode, Output Current
and Coarse Delay.
For ADC it sets the following configurations:
MixerSettings, QMCSettings, Read Fabricrate and Threshold Settings.
This example shows how to change the configurations for ADC
and DAC using driver functions.
NOTE: The purpose of the example is to show how to use the driver APIs.
For real user scenarios this example will not be relevant.

For details, see xrfdc_read_write_example.c.

@section ex3 xrfdc_intr_example.c
Contains an example to show the interrupts, interrupts are mostly used for error
reporting.
The interrupts do not do any data processing. Since they dont do any data
processing, interrupts are invoked in rare conditions.
The example here attempts to demonstrate users how an error interrupt can be
generated. Also once generated how does the processing happen.
Upon an interrupt, the control reaches to ScuGIC interrupt handler.
From there the control is transferred to the libmetal isr handling which
then calls the driver interrupt handler. Users are expected to register
their callbacks with the driver interrupt framework.
The actual interrupt handling is expected to happen in the user provided
callback.
This example generates ADC fabric interrupts by writing some incorrect
fabric data rate based on the read/write clocks.

For details, see xrfdc_intr_example.c.

*/
