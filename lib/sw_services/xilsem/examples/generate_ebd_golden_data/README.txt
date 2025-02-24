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

	1.3 Generate EBD Golden Data for Mono Devices

		1.3.1 cd to your local path where the folder "generate_ebd_golden_data" is present

		1.3.2 Copy *.ebd_cfi generated from Vivado (step 1.1) to this folder

		1.3.3 Locate "run.sh" script in the folder and change permissions of the script file by running following command from the terminal:
					chmod u+x run.sh

		1.3.4 Run "run.sh" using the following command to generate "xsem_edbgoldendata.c":
			bash run.sh

	1.4 Generate EBD Golden Data for SSIT Devices

		1.4.1 cd to your local path where the folder "generate_ebd_golden_data" is present

		1.4.2 Copy *.ebd_cfi files generated for all SLRs from Vivado (step 1.1) to this folder

		1.4.3 Locate "run_ssit.sh" script in the folder and change permissions of the script file by running following command from the terminal:
					chmod u+x run_ssit.sh

		1.4.4 Run "run_ssit.sh" using the following command to generate "xsem_edbgoldendata<slr_id>.c" for all SLRs in the device:
			bash run_ssit.sh <slr_id>
			slr_id = 0 for SLR0,1 for SLR1,..n for SLRn

2. Build and Run EBD lookup example for Versal Monolithic device
================================================================================
	2.1 Open Vitis tool and import XilSEM example "xsem_ebd_example" from R5/A72 application
	2.2 This folder contains a source file "xsem_ebdgoldendata.c". This needs to be replaced
	    with the file which is generated from your design (Refer Step 1.3)
	2.3 Define a DDR memory section "sem_ebddata" in your R5/A72 linker script as below:
			.sem_ebddata : {
				KEEP (*(.sem_ebddata))
			} > axi_noc_0_C0_DDR_LOW0
		The above section is used for storing structured EBD golden data in DDR memory.
	2.4 Build and run your application.

3. Build and Run EBD lookup example for Versal SSIT device
==========================================================================
	3.1 Open Vitis tool and Import XilSEM example "xsem_ebd_ssit_example" from R5/A72 application
	3.2 This folder contains source files
		xsem_ebdgoldendata0.c for slr0
		xsem_ebdgoldendata1.c for slr1
		Replace these two files with the generated files from your design (Refer steps 1.4)
		Note:If the device has more than 2 SLRs then you need to add xsem_ebdgoldendataX.c for other SLRs
	3.3 Define a DDR memory section "sem_ebddata" in your R5/A72 linker script as below:
			.sem_ebddata : {
				KEEP (*(.sem_ebddata))
			} > axi_noc_0_C0_DDR_LOW0
		The above section is used for storing structured EBD golden data in DDR memory.
	3.4 Build and run your application.
