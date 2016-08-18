# open-amp
This repository is the home for the Open Asymmetric Multi Processing (OpenAMP)
framework project. The OpenAMP framework provides software components that
enable development of software applications for Asymmetric Multiprocessing
(AMP) systems. The framework provides the following key capabilities.

1. Provides Life Cycle Management, and Inter Processor Communication
   capabilities for management of remote compute resources and their associated
   software contexts
2. Provides a stand alone library usable with RTOS and Baremetal software
   environments
3. Compatibility with upstream Linux remoteproc and rpmsg components
4. Following AMP configurations supported
	a. Linux master/Generic(Baremetal) remote
	b. Generic(Baremetal) master/Linux remote
5. Proxy infrastructure and supplied demos showcase ability of proxy on master
   to handle printf, scanf, open, close, read, write calls from Bare metal
   based remote contexts.

## OpenAMP Source Structure
```
|- lib/
|  |- common/     # common helper functions
|  |- virtio/     # virtio implemnetation
|  |- rpmsg/      # rpmsg implementation
|  |- remoteproc/ # remoteproc implementation
|  |  |- drivers  # remoteproc drivers
|  |- proxy/      # implement one processor access device on the
|  |              # other processor with file operations
|  |- system/     # os specific implementation
|  |  |  |- generic/ # E.g. generic system (that is baremetal)
|  |  |  |  |- machine/ # machine specific implmentation for the system
|  |- include/     # header files
|  |  |- machine/ # machine specific header files
|  |  |- system/  # system specific header files
|- apps/        # demontrastion/testing applicaitons
|  |- machine/  # common files for machine can be shared by applications
|               # It is up to each app to decide whether to use these files.
|  |- system/   # common files for system can be shared by applicaitons
|               # It is up to each app to decide whether to use these files.
|- obsolete     # It is used to build libs which may also required when
|               # building the apps. It will be removed in future since
|               # user can specify which libs to use when compiling the apps.
|- cmake        # CMake files
```

OpenAMP library libopen_amp is composed of the following directorys in `lib/`:
*   `common/`
*   `virtio/`
*   `rpmsg/`
*   `remoteproc/`
*   `proxy/`
*   `system/`

## OpenAMP Compilation
OpenAMP uses CMake for library and demonstration applicaiton compilation.

###  Example to compile Zynq MP SoC R5 generic(baremetal) remote:
```
$ mkdir build
$ cd build/
$ cmake ../open-amp -DCMAKE_TOOLCHAIN_FILE=zynqmp_r5_generic -DWITH_OBSOLETE=ON -DWITH_APPS=ON
$ make DESTDIR=$(pwd) install
```
The OpenAMP library will be generated to `build/usr/local/lib` directory, headers will be generated to
`build/usr/local/include` directory, and the applications executables will be generated to
`build/usr/local/bin` directory.

*   `-DWITH_OBSOLETE=ON` is to build the `libxil.a`.
*   `-DWITH_APPS=ON` is to build the demonstration applications.

###  Example to compile Zynq A9 remote:
```
$ mkdir build
$ cd build/
$ cmake ../open-amp -DCMAKE_TOOLCHAIN_FILE=zynq7_generic -DWITH_OBSOLETE=on -DWITH_APPS=ON
$ make DESTDIR=$(pwd) install
```

## Supported System and Machines
For now, it supports:
* Zynq generic slave
* Zynq generic master
* ZynqMP R5 generic slave

## Known Limitations:
1. In rpc_demo.c(the remote demonstration application that showcases usage of
   rpmsg retargetting infrastructure),  the bindings for the flag input
   parameter in open() system call has been redefined. The GCC tool library
   bindings for this input argument is different between arm-xilinx/none-eabi, and
   arm-linux-eabi toolchains. For this reason, redefinition is required for
   compatibility with proxy on Linux master.

For using the framework please refer to the documents present in the /docs folder.
Subscribe to the open-amp mailing list at https://groups.google.com/group/open-amp.
