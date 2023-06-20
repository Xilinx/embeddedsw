Steps to Lookup Essential Bit Data from user design:
----------------------------------------------------
Follow the steps given below to find out whether a particular bit is essential or not.

1. Creating golden EBD data from user design
============================================
	1.1 Generate uncompressed *.ebd_cfi file from vivado using commands below:
			set_property bitstream.seu.essentialbits yes [current_design]
			set_property bitstream.general.compress false [current_design]
			write_device_image -raw -force -readback typeN

			where N refers to Specific block type.
		        by default choose block type 0.
			e.g:write_device_image -raw -force -readback type0

	1.2 Download "generate_ebd_golden_data" folder from XilSEM example folder
		(Path" https://github.com/Xilinx/embeddedsw/tree/master/lib/sw_services/xilsem/examples)
		to your local path

	1.3 cd to your local path where the folder "generate_ebd_golden_data" is present

		1.3.1 Copy *.ebd_cfi generated from Vivado (step 1.1) to this folder

		1.3.2 Locate "run.sh" script in the folder and change permissions of the script file by running following command from the terminal:
			chmod u+x run.sh

		1.3.3 Run "run.sh" using the following command to generate "xsem_edbgoldendata.c":
			bash run.sh

2. Build and run R5 example for EBD lookup
==========================================
	2.1 Open Vitis tool and import XilSEM example "xsem_ebd_example" from R5 application
	2.2 This folder contains a source file "xsem_ebdgoldendata.c". This needs to be replaced
	    with the file which is generated from your design (Refer Step 1.3.3)
	2.2 Define a DDR memory section "sem_ebddata" in your R5 linker script as below:
			.sem_ebddata : {
				KEEP (*(.sem_ebddata))
			} > axi_noc_0_C0_DDR_LOW0
		The above section is used for storing structured EBD golden data in DDR memory.
	2.3 Build and run your application.
