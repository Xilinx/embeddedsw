FreeRTOS lwIP Echo Server
----------------------------

The FreeRTOS lwIP Echo server application starts an echo server at port 7. Any data sent to
this port is simply echoed back.

By default, the program assigns the following settings to the board:
IP Address: 192.168.1.10
Netmask   : 255.255.255.0
Gateway   : 192.168.1.1
MAC address:  00:0a:35:00:01:02
These settings can be changed in the file main.c.

The main echo server logic is present in the file echo.c.

Running the Echo Server example
-------------------------------

To connect and test the echo server, download and run the program on the board,
and then issue the following command from your host machine:

$ telnet 192.168.1.10 7
Trying 192.168.1.10...
Connected to 192.168.1.10.
Escape character is '^]'.
hello world
hello world
all messages will be echo'ed back
all messages will be echo'ed back
^]
telnet> quit
Connection closed.
$

References
----------

More details regarding the echo server can be obtained from Xilinx XAPP 1026:
http://www.xilinx.com/support/documentation/application_notes/xapp1026.pdf
