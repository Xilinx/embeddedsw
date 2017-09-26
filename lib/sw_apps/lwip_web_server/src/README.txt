lwIP Webserver
--------------

The lwIP webserver application starts a webserver at port 80.

By default, the program assigns the following settings to the board:
IP Address : 192.168.1.10
Netmask    : 255.255.255.0
Gateway    : 192.168.1.1
MAC address: 00:0a:35:00:01:02
These settings can be changed in the file main.c.

The main webserver logic is present in the file webserver.c.

platform.c implements certain processor and platform dependent functions.

Setting RAMFS parameters
------------------------

As the webserver uses RAM based FAT FS to store webpages, it's important to set
correct RAMFS parameters according to your h/w. By default, xilffs library takes
following value for ramfs_size and ramfs_start_addr:

ramfs_size - 3145728
ramfs_start_addr - 0x10000000

These values should work for most Zynq and ZynqMP solutions, but are likely to
be different for MicroBlaze. Please set appropriate parameters in xilffs section
of MSS file for your h/w, if they should be different.

Creating FAT image on Linux
---------------------------

This requires root (sudo) access on the Linux host

Following commands can be used on terminal to create FAT image to be used with
webserver application:

# create image file of 3MB
dd if=/dev/zero of=example.img bs=512 count=6144

# format image with FAT
/sbin/mkfs.vfat example.img

# mount it
mkdir /tmp/fs
sudo mount -t vfat -o loop,rw example.img /tmp/fs/

# copy your webpages
sudo cp -r  webpages_dir/* /tmp/fs/
sudo umount /tmp/fs

Running the webserver example
-----------------------------

The application expects webpages to be stored in RAM based FAT FS. Please
tune RAM FS start address and size according to the h/w design and size of
webpages in MSS file. Please load FAT FS image file in the RAM at the same
address as the one specified in the MSS file while running the application.

To connect and test the webserver, download the program and FAT image file
on the board, and then run the program. The server can be accessed using the
URL - http://<board-ip>; e.g. http://192.168.1.10
