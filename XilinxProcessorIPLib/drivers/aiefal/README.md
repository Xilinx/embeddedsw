# xaiefal

## Overview

Xilinx AI engine functional abstraction layer (FAL) provides common user APIs
for runtime AI engine tiles resources management.

## Build Proejct
We use `cmake` to build the project.

### Generate Documentation
The project uses doxygen for documentation. Follow these instructions to
generate documentation.
```
mkdir build
cd build
cmake ../ -DWITH_DOC=on
make
```

### Includes to build `libxaiengine` library
Use `git submodule` to sync the `aienginev2` driver submodule. Use the following
cmake commands to configure the build:
```
# For Linux to run on target:
mkdir build
cd build
cmake ../ -DCMAKE_TOOLCHAIN_FILE=versal-linux -DWITH_BUILD_XAIEDRV=on
```

After running `cmake` to configure the build, can go to the build directory to
compile:
```
cd build
make VERBOSE=1
```

### Not build `libxaiengine` library
If excludes building `libxaiengine` library, user will need to provide where
`libxaiengine` is as this library depends on the AI engine driver library.
```
# For Linux to run on target:
mkdir build
cd build
cmake ../ -DCMAKE_TOOLCHAIN_FILE=versal-linux -DCMAKE_LIBRARY_PATH=<DIR_TO_XAIENGINE_LIB> -DCMAKE_INCLUDE_PATH=<DIR_TO_XAIENGINE_HEADER>
```

After running `cmake` to configure the build, can go to the build directory to
compile:
```
cd build
make VERBOSE=1
```

### Build Unit Tests
We use `CppUTest` for unit testing. Use CMake option `-DWITH_TESTS=ON` to turn
on building the unit testing.

You can specify to use your external cpputest directory with the following
option `-DWITH_TESTS=ON -DCPPUTEST_DIR=<compiled_cpputest_dir>`

Here is the repo for cpputest:
`https://github.com/cpputest/cpputest.git`

The testing executable will be in `<BUILD_DIR>/tests/`.
It will not run the tests by default during build. If you want to run the tests
in the end of the build, you can use CMake option `-DWITH_TESTS_EXEC=ON`.

#### Build with Coverage Enable
Add the following cmake options along with `-DWITH_TESTS=ON -DWITH_TESTS_EXEC=ON`:
`-DCMAKE_BUILD_TYPE=Debug -DCODE_COVERAGE=on` to enable coverage.

The generated coverage information file will be in:
`<build_dir>/tests/utests/`.
