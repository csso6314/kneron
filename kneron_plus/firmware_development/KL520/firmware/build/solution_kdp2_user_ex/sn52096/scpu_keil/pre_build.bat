REM "prebuild script"
REM "pin for KSR record"
ECHO %CD%>project_path.txt
MOVE project_path.txt ..\..\..\..\..\internal_utils\KneronSWReleaseServer
cd ..\..\..\..\..\internal_utils\KneronSWReleaseServer
KSR_server_handler.exe -r 1
