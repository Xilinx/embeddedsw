# This scripts automates the process of PetaLinux project creation.
PRESENT=$(pwd)

if [ "$1" == "master" ]; then

	# Change directory to the master PetaLinux project directory
	cd "$2"

	#Matrix multiply userspace application
	petalinux-create -t apps --template c --name mat_mul_demo --enable

	cp $OPENAMP/apps/samples/master/linux/userspace/matrix_multiply/mat_mul_demo.c ./components/apps/mat_mul_demo/mat_mul_demo.c

	cp $OPENAMP/libs/system/zc702evk/linux/scripts/makefiles/mat_mul_demo/Makefile ./components/apps/mat_mul_demo/Makefile

	#Echo Test userspace application
	petalinux-create -t apps --template c --name echo_test --enable

	cp $OPENAMP/apps/tests/master/linux/userspace/echo_test/echo_test.c ./components/apps/echo_test/echo_test.c

	#Proxy application
	petalinux-create -t apps --template c --name proxy_app --enable

	cp $OPENAMP/proxy/master/linux/userspace/proxy_app.c ./components/apps/proxy_app/proxy_app.c

	cp $OPENAMP/proxy/master/linux/userspace/proxy_app.h ./components/apps/proxy_app/proxy_app.h

	#Zynq Remoteproc driver
	petalinux-create -t modules -n zynq_remoteproc_driver --enable

	cp $OPENAMP/apps/samples/master/linux/kernelspace/zynq_remoteproc_driver/zynq_remoteproc_driver.c ./components/modules/zynq_remoteproc_driver/zynq_remoteproc_driver.c

	cp $OPENAMP/apps/samples/master/linux/kernelspace/zynq_remoteproc_driver/remoteproc_internal.h ./components/modules/zynq_remoteproc_driver/remoteproc_internal.h


	#Matrix multiply kernel-space module

	petalinux-create -t modules -n rpmsg_mat_mul_kern_app --enable

	cp $OPENAMP/apps/samples/master/linux/kernelspace/rpmsg_mat_mul_kern_app/rpmsg_mat_mul_kern_app.c ./components/modules/rpmsg_mat_mul_kern_app/rpmsg_mat_mul_kern_app.c

	#Echo Test kernel-space module

	petalinux-create -t modules -n rpmsg_echo_test_kern_app --enable

	cp $OPENAMP/apps/tests/master/linux/kernelspace/rpmsg_echo_test_kern_app/rpmsg_echo_test_kern_app.c ./components/modules/rpmsg_echo_test_kern_app/rpmsg_echo_test_kern_app.c


	#RPMSG User Device Driver module

	petalinux-create -t modules -n rpmsg_user_dev_driver --enable

	cp $OPENAMP/apps/samples/master/linux/kernelspace/rpmsg_user_dev_driver/rpmsg_user_dev_driver.c ./components/modules/rpmsg_user_dev_driver/rpmsg_user_dev_driver.c


	#Proxy Device Driver module

	petalinux-create -t modules -n rpmsg_proxy_dev_driver --enable

	cp $OPENAMP/proxy/master/linux/kernelspace/rpmsg_proxy_dev_driver.c ./components/modules/rpmsg_proxy_dev_driver/rpmsg_proxy_dev_driver.c


	#Firmware Installation

	#Bare-metal Matrix Multiply Sample

	petalinux-create -t apps --template install -n mat_mul_baremetal_fw --enable

	cp $OPENAMP/apps/firmware/zc702evk/baremetal/matrix_multiply/firmware ./components/apps/mat_mul_baremetal_fw/data/firmware

	cp $OPENAMP/libs/system/zc702evk/linux/scripts/makefiles/mat_mul_baremetal_fw/Makefile ./components/apps/mat_mul_baremetal_fw/Makefile

	#Bare-metal Echo Test

	petalinux-create -t apps --template install -n echo_test_baremetal_fw --enable

	cp $OPENAMP/apps/firmware/zc702evk/baremetal/echo_test/firmware ./components/apps/echo_test_baremetal_fw/data/firmware

	cp $OPENAMP/libs/system/zc702evk/linux/scripts/makefiles/echo_test_baremetal_fw/Makefile ./components/apps/echo_test_baremetal_fw/Makefile

	#Baremetal RPC Demo

	petalinux-create -t apps --template install -n rpc_demo_baremetal_fw --enable

	cp $OPENAMP/apps/firmware/zc702evk/baremetal/rpc_demo/firmware ./components/apps/rpc_demo_baremetal_fw/data/firmware

	cp $OPENAMP/libs/system/zc702evk/linux/scripts/makefiles/rpc_demo_baremetal_fw/Makefile ./components/apps/rpc_demo_baremetal_fw/Makefile

else

	# Change directory to the remote PetaLinux project directory
	cd "$2"

	#Matrix multiply user-space application
	petalinux-create -t apps --template c --name mat_mul_demo --enable

	cp $OPENAMP/apps/samples/master/linux/userspace/matrix_multiply/mat_mul_demo.c ./components/apps/mat_mul_demo/mat_mul_demo.c

	cp $OPENAMP/libs/system/zc702evk/linux/scripts/makefiles/mat_mul_demo/Makefile ./components/apps/mat_mul_demo/Makefile

	#Echo Test user-space application

	petalinux-create -t apps --template c --name echo_test --enable

	cp $OPENAMP/apps/tests/master/linux/userspace/echo_test/echo_test.c ./components/apps/echo_test/echo_test.c

	#Create Kernel Modules

	#Zynq RPMSG Driver module

	petalinux-create -t modules -n zynq_rpmsg_driver --enable

	cp $OPENAMP/apps/samples/master/linux/kernelspace/zynq_rpmsg_driver/zynq_rpmsg_driver.c ./components/modules/zynq_rpmsg_driver/zynq_rpmsg_driver.c
	cp $OPENAMP/apps/samples/master/linux/kernelspace/zynq_rpmsg_driver/zynq_rpmsg_internals.h ./components/modules/zynq_rpmsg_driver/zynq_rpmsg_internals.h

	#Matrix multiply kernel-space module

	petalinux-create -t modules -n rpmsg_mat_mul_kern_app --enable

	cp $OPENAMP/apps/samples/master/linux/kernelspace/rpmsg_mat_mul_kern_app/rpmsg_mat_mul_kern_app.c ./components/modules/rpmsg_mat_mul_kern_app/rpmsg_mat_mul_kern_app.c

	#Echo Test kernel-space module

	petalinux-create -t modules -n rpmsg_echo_test_kern_app --enable

	cp $OPENAMP/apps/tests/master/linux/kernelspace/rpmsg_echo_test_kern_app/rpmsg_echo_test_kern_app.c ./components/modules/rpmsg_echo_test_kern_app/rpmsg_echo_test_kern_app.c

	#RPMSG User Device Driver module

	petalinux-create -t modules -n rpmsg_user_dev_driver --enable

	cp $OPENAMP/apps/samples/master/linux/kernelspace/rpmsg_user_dev_driver/rpmsg_user_dev_driver.c ./components/modules/rpmsg_user_dev_driver/rpmsg_user_dev_driver.c

	#Functional Test Driver module

	petalinux-create -t modules -n rpmsg_func_test_kern_app --enable

	cp $OPENAMP/apps/tests/master/linux/kernelspace/rpmsg_func_test_kern_app/rpmsg_func_test_kern_app.c ./components/modules/rpmsg_func_test_kern_app/rpmsg_func_test_kern_app.c

	#DTS File
	cp $OPENAMP/libs/system/zc702evk/linux/patches/linux/petalinux2013.10/system.dts ./subsystems/linux/hw-description/system.dts

fi

# Return
cd $PRESENT
