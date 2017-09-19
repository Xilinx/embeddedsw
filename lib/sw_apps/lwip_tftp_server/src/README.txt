lwIP tftp server
----------------

The lwIP tftp server application starts a tftp server at port 69.

The program assigns the following settings to the board if DHCP is disabled or
DHCP request times out:
IP Address: 192.168.1.10
Netmask   : 255.255.255.0
Gateway   : 192.168.1.1
MAC address:  00:0a:35:00:01:02
These settings can be changed in the file main.c.

The main tftp server logic is present in the file tftp_server.c.

Running the tftp server example
-------------------------------

To connect and test the tftp server.

Run tftp client on host machine:
$ tftp 192.168.1.10
$ tftp > put sample1.txt
Sent 60 bytes in 0.0 seconds

Logs on board:
TFTP WRQ (write request): sample1.txt
TFTP_WRQ: Transfer completed
