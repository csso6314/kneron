@ECHO OFF
REM "Usage: post_build.bat binary_file [gen_enc]"
REM " ex. post_build.bat main           -> generate fw_ncpu.bin"
REM "     post_build.bat main gen_enc   -> generate both fw_ncpu.bin and fw_ncpu_enc.bin"

SET UTILS_PATH=..\..\..\..\utils
SET BIN_IN=%1
SET BIN_OUT=fw_ncpu.bin
SET ENC_BIN_OUT=fw_ncpu_enc.bin

echo Deleting old binary file...
IF EXIST %BIN_IN% (del %BIN_IN%)
IF EXIST %ENC_BIN_OUT% (del %ENC_BIN_OUT%)
IF EXIST %BIN_OUT% (del %BIN_OUT%)

ECHO Copy built raw file [%BIN_IN%]...
copy bin\vp6_asic\%xt_target%\%BIN_IN% .

call %xt_tools%\Tools\misc\xtensaenv.bat %xt_tools%\..\..\..\builds\RI-2019.2-win32\vp6_asic %xt_tools% %xt_tools%\..\..\..\builds\RI-2019.2-win32\vp6_asic\config vp6_asic

ECHO Run xt-objcopy to generate [%BIN_IN% -^> tmp.bin]...
xt-objcopy.exe -j .ResetVector.text -j .WindowVectors.text -j .Level2InterruptVector.text -j .Level3InterruptVector.text -j .DebugExceptionVector.text -j .NMIExceptionVector.text  -j .KernelExceptionVector.text  -j .UserExceptionVector.text -j .DoubleExceptionVector.text -j .clib.rodata -j .rtos.rodata  -j .rodata -j .text  -j .clib.data -j .rtos.percpu.data -j .data -j .bss -j .debug_pubnames  -j .debug_info  -j .debug_abbrev -j .debug_line -j .xt.prop  -j .xtensa.info -j .comment -j .debug_ranges -O binary .\%BIN_IN% tmp.bin

ECHO Generating DFU binary [tmp.bin -^> %BIN_OUT%]...
%UTILS_PATH%\dfu\gen_dfu_binary_for_win.exe -ncpu tmp.bin %BIN_OUT%

copy %BIN_OUT% ..\..\..\..\..\..\..\res\firmware\KL720
