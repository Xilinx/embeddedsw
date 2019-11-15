if not "%HDI_APPROOT%" == "" (
%HDI_APPROOT%\tps\win64\cmake-3.3.2\bin\cmake.exe %* 2>&1
) else (
if "%XILINX_SDK%"== "" (
%XILINX_VITIS%\tps\win64\cmake-3.3.2\bin\cmake.exe %* 2>&1
) else (
%XILINX_SDK%\tps\win64\cmake-3.3.2\bin\cmake.exe %* 2>&1
))
