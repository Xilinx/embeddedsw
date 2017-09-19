FreeRTOS lwIP tftp client
-------------------------

The FreeRTOS lwIP tftp client application gets/puts files from/to tftp server running on host machine.

By default, the program assigns the following settings to the board:
IP Address: 192.168.1.10
Netmask   : 255.255.255.0
Gateway   : 192.168.1.1
MAC address:  00:0a:35:00:01:02
These settings can be changed in the file main.c.

The main tftp client logic is present in the file tftp_client.c.
TFTP client application will put and get 3 files from TFTP server(default: 10.10.70.101).
TFTP server ip can be changed from tftp_common.h.

platform.c implements certain processor and platform dependent functions.
The file platform_config.h is generated based on the hardware design.
It makes two assumptions:
  - The timer has its interrupt line connected to the interrupt controller.
  - All the ethernet peripherals (xps_ethernetlite or xps_ll_temac) accessible from
    the processor can be used with lwIP.

Running the tftp client example
-------------------------------

To test the tftp client application tftp server should be running on the host machine.
And sample1.txt, sample2.txt and sample3.txt should be there on tftp server.

Log on board:
TFTP_WRQ: tftp get sample1.txt
TFTP_WRQ: Transfer completed

TFTP_WRQ: tftp get sample2.txt
TFTP_WRQ: Transfer completed

TFTP_WRQ: tftp get sample3.txt
TFTP_WRQ: Transfer completed

TFTP_RRQ: tftp get sample1.txt
TFTP_RRQ: Transfer completed

TFTP_RRQ: tftp get sample2.txt
TFTP_RRQ: Transfer completed

TFTP_RRQ: tftp get sample3.txt
TFTP_RRQ: Transfer completed
