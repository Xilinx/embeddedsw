rm -f web.img

# Create image file of 64K
dd if=/dev/zero of=web.img bs=512 count=128

# Format image with FAT
/sbin/mkfs.vfat -F 12 web.img

# Mount it
mkdir /tmp/fs
chmod 666 web.img
sudo mount -t vfat -o loop,rw web.img /tmp/fs/

# Copy webpages
cp -r  * /tmp/fs/
sudo umount /tmp/fs
rm -R /tmp/fs
chmod 444 web.img
