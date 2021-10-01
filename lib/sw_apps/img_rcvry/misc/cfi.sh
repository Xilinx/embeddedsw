#******************************************************************************
# Copyright (c) 2020 - 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#******************************************************************************

rm web.img

# Create image file of 96K
dd if=/dev/zero of=web.img bs=512 count=192

# Format image with FAT
/sbin/mkfs.vfat -F 12 web.img

# Mount it
sudo mkdir /tmp/fs
chmod 666 web.img
sudo mount -t vfat -o loop,rw web.img /tmp/fs/

# Copy webpages
sudo cp -r  ./web_pages/* /tmp/fs/
sudo umount /tmp/fs
sudo rm -R /tmp/fs
chmod 444 web.img
