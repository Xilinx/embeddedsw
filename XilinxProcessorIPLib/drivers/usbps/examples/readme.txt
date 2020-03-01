/** \page example Examples
You can refer to the below stated example applications for more details on how to use usbps driver.

@section ex1 xusbps_ch9.c
Contains an example on how to use the XUsbps driver directly.
This example contains the implementation of the chapter 9 code.

For details, see xusbps_ch9.c.

@section ex2 xusbps_ch9.h
This headerfile contains definitions used in the chapter 9 code.

For details, see xusbps_ch9.h.

@section ex3 xusbps_ch9_storage.c
Contains an example on how to use the XUsbps driver directly.
This example contains the implementation of the storage specific
chapter 9 code.

For details, see xusbps_ch9_storage.c.

@section ex4 xusbps_ch9_storage.h
This headerfile contains definitions used in the chapter 9 code.

For details, see xusbps_ch9_storage.h.

@section ex5 xusbps_class_storage.c
Contains an example on how to use the XUsbps driver directly.
This example contains the contains the implementation of the
storage class code.

For details, see xusbps_class_storage.c.

@section ex6 xusbps_class_storage.h
This headerfile contains definitions used in the chapter 9 code.

For details, see xusbps_class_storage.h.

@section ex7 xusbps_intr_example.c
Contains an example on how to use the XUsbps driver directly.
This example shows the usage of the USB driver with the USB controller
in DEVICE mode.

For details, see xusbps_intr_example.c.

@section ex8 xusb_ch9_audio.c
This is the USB Audio example which contains USB Audio Class related
Chapter 9 functions.

For details, see xusbps_ch9_audio.c.

@section ex9 xusbps_ch9_audio.h
This headerfile contains the constants, type definitions, variables and
function prototypes used in the USB Audio Class related Chapter 9 code.

For details, see xusbps_ch9_audio.h.

@section ex10 xusbps_class_audio.c
Contains an example on how to use the Usb driver directly.
This is the USB Audio example which contains USB Audio Class related
functions.

For details, see xusbps_class_audio.c.

@section ex11 xusbps_class_audio.h
This headerfile contains the constants, type definitions, variables and
function prototypes used in the USB Audio Class related code.

For details, see xusbps_class_audio.h.

@section ex12 xusbps_audio_example.c
This example contains Audio device application which provides a reference
to create a new Isochronous Transfer related application.

 - For Audio example, UAC2 is selected by default. The constant UAC1 has to be
defined to use Audio Class 1.0 device. If UAC1 is defined, the constant
MICROPHONE is used to select Microphone device. Undefine MICROPHONE for
UAC1 Speaker configuration. By default Microphone Specification is selected.
 - For Audio example, if UAC2 specification is selected, device will playback
the data which is provided to it from host. Use following commands to
play/record audio:
	Find alsa hardware info using "aplay/arecord -l".
		Play: "aplay -D hw:X,X <44100Hz_STERIO_FILE>.wav"
		Record: "arecord -D hw:X,X -f S16_LE -r 44100 -c 2 <OUTPUT_FILE>.wav"
	If UAC1 Microphone specification is selected, device will playback the
	dummy data from data.h file. Use following command to record audio:
		"arecord -D hw:X,X -f S16_LE -r 8000 -c 1 <OUTPUT_FILE>.wav"
	If Speaker specification is selected then use following command to play
	the audio:
		"aplay -D hw:X,X <8000Hz_MONO_FILE>.wav"

For details, see xusbps_audio_example.c.

@section ex13 xusbps_audiodata.h
This headerfile contains the dummy audio data which is to be transfered to host
when UAC1 Microphone specification is selected in USB Audio example.

For details, see xusbps_audiodata.h.
*/
