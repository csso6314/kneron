@ECHO OFF
REM "DFU binary file convert"
SET BIN_IN=%1
SET BIN_OUT=fw_scpu.bin
SET ENC_BIN_OUT=fw_scpu_enc.bin

SET UTILS_PATH=..\..\..\..\utils

ECHO Generating DFU binary [%BIN_IN% -^> %BIN_OUT%]...
%UTILS_PATH%\dfu\gen_dfu_binary_for_win.exe -scpu .\Objects\%BIN_IN% .\Objects\%BIN_OUT%

copy .\Objects\%BIN_OUT% ..\..\..\..\..\..\..\res\firmware\KL720
