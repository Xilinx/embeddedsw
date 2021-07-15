Building Image Selector from git:

Image Selector has 3 directories.
	1. data - It contains files for Vitis
	2. src  - It contains the Image Selector source files
	3. misc - It contains miscellaneous files required to
		  compile Image Selector.
		  Build for zcu102 board is  supported.

How to compile Image Selectlor:

	1.Go to the src directory "lib/sw_apps/imgsel/src/"
	2.If executables and other artifacts from previous build with other
	  configuration (different processor/state) are present, run
	  make clean to delete them.
	2.Give build command in the following manner.
		a. Go to src directory.
		b. Give the "make" command for default build
		c. make "BOARD=som" for SOM specific build
		d. Make command will trigger the build of imgsel_bsp first and then
			build imgsel.
	3.Below are the examples for compiling for different options
		a. To generate imgsel for zcu102 board:
			make
		b. To generage imgsel for SOM specific board:
			make "BOARD=som"
