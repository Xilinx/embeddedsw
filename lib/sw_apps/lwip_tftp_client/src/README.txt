lwIP tftp client
----------------

The lwIP tftp client application gets/puts files from/to tftp server running on host machine.

The program assigns the following settings to the board if DHCP is disabled or
DHCP request times out:
IP Address: 192.168.1.10
Netmask   : 255.255.255.0
Gateway   : 192.168.1.1
MAC address:  00:0a:35:00:01:02
These settings can be changed in the file main.c.

The main tftp client logic is present in the file tftp_client.c.
TFTP client application will put and get 3 files from TFTP server(default: 10.10.70.101).
TFTP server ip can be changed from tftp_common.h.

Running the tftp client example
-------------------------------

To test the tftp client application tftp server should be running on the host machine.

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
