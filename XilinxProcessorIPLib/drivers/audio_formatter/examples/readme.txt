/** \page example Examples
You can refer to the below stated example applications for more details on how to use audio formatter driver.

@section xaudioformatter_example.c
Contains an example on how to use the XAudioFormatter driver directly.
This example shows the usage of the driver in interrrupt mode.
The audio is received from external device through i2s receiver and given to
audio formatter through axi stream interface, audio formatter writes the
output to memory through DMA transfer from where another instance of audio
formatter reads it and send to i2s transmitter for playback. In this example we
wait for IOC interrupts from the audio formatter S2MM and MM2S cores and on
receiving both the interrupts along with i2srx and i2stx interrupts we
stop audio formatter and print the test is successfull.

For details, see xaudioformatter_example.c.
*/
