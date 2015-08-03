xusb_example_readme.txt
-------------------------

The examples in this directory are provided to give the user an idea of
how the USB and its driver can be used for Bulk and Interrupt tranfers.


FILES

1. xusb_types.h  - Top level include for all examples.

2. xusb_cp.c     - Top level USB chapter 9 related file for all examples.

3. xusb_cp9.h    - Top level USB chapter9 related include file.

4. xusb_storage.c - Provides the bulk transfer example. It implements a mass
		    storage device.

5. xusb_storage.h - This file is the include file required for mass storage
			device.

6. xusb_keyboard.c - A USB keyboard example using USB for interrupt transfers.

7. xusb_keyboard.h - The include file for the USB keyboard example.

8. xusb_mouse.c - A USB mouse example using USB for interrupt transfers.

9. xusb_mouse.h - This is the include file for the USB mouse example.

10. xusb_dma_intr_storage.c - Provides the bulk transfer example. It implements
		a mass storage device using the dma interrupts such that all the
		SCSI processing is handled in the interrupts.
11. xusb_phy_read_write.c - Provides a simple example for ULPI PHY read/write
				access.
12. xusb_microphone.c - A USB microphone example to demonstrate the isochronous
		transactions.
13. xusb_microphone.h - This is the include file for the USB microphone example.

14. xusb_storage_polled_mode.c - USB mass storage example implemented in polled mode

NOTES

* These examples are independent from one another. All the examples except the
the phy_read_write example should include the xusb_cp9.c file.

* The constants HID_DEVICES, USB_MOUSE, USB_KEYBOARD and MASS_STORAGE_DEVICE
defined in the xusb_types.h file are to be used for testing the examples.

* To run mass storage examples, the constant definition MASS_STORAGE_DEVICE is
to be defined and the constants HID_DEVICES, USB_KEYBOARD and USB_MOUSE are to
be undefined.

* To run keyboard example, the constants HID_DEVICES and USB_KEYBOARD are to
be defined and the constant definitions MASS_STORAGE_DEVICE and USB_MOUSE are to
be undefined.

* To run mouse example, the constants HID_DEVICES and USB_MOUSE are to
be defined and the constant definitions MASS_STORAGE_DEVICE and USB_KEYBOARD are
to be undefined.

* To run the microphone example the constant definition MICROPHONE has to be
defined and the other example constant definitions should be undefined.











