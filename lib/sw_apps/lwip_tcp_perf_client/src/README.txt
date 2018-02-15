LwIP TCP Perf Client
--------------------

The LwIP TCP Perf client application creates TCP client using LwIP stack.
This client connects to TCP server (running on Linux Host machine using
Iperf 2.0.5) with IP address (default 192.168.1.100) provided in application.
The TCP server should be running on host machine to serve this client.
Once client connects with server, then application will start data
transfer and performance will be measured.
Connection details and data transfer statistics will be displayed by client
on serial console.

Following TCP client options can be changed in file tcp_perf_client.h,
1) INTERIM_REPORT_INTERVAL: time interval (in secs) for intermediate report
display interval. (default 5 secs)
2) TCP_CONN_PORT: Port to be used for TCP connection. (default 5001)
3) TCP_TIME_INTERVAL: time interval (in secs) for which TCP client will run.
(default 300 secs)
4) TCP_SERVER_IP_ADDRESS: Server IP address to which client will be connected.
(default 192.168.1.100)

If LWIP_DHCP enabled then board should get IP address from DHCP server.
If DHCP timeout happens or LWIP_DHCP is disabled then, the program assigns the
following IP settings to the board:
IP Address: 192.168.1.10
Netmask   : 255.255.255.0
Gateway   : 192.168.1.1
MAC address:  00:0a:35:00:01:02

These settings can be changed in the file main.c.

The TCP client connection and statistics logic is present in the file
tcp_perf_client.c

Running the LwIP TCP client example
-----------------------------------

First run Iperf server on host machine using below command
$ iperf -s -i 5 -w 2M

Now, download and run the TCP client application on the board.
