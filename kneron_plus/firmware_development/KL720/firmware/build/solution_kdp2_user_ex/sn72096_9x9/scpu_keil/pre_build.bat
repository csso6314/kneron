@ECHO OFF
ECHO "Generate Git Version Header File"
SET GIT_VERSION_FILE= ..\..\..\..\include\git_info.h
DEL %GIT_VERSION_FILE%
git rev-parse --is-inside-work-tree >>NUL
IF %ERRORLEVEL% EQU 0 (
    ECHO #ifndef __GIT_INFO_H__ > %GIT_VERSION_FILE%
    git log -1 --abbrev=8 --pretty=format:"#define IMG_FW_GIT_VERSION "0x"%%h" >> %GIT_VERSION_FILE%
    ECHO. >> %GIT_VERSION_FILE%
    ECHO #endif >> %GIT_VERSION_FILE%
    git log -1 --pretty=tformat:"Create git_info.h="0x"%%h done"
) ELSE (
    ECHO #ifndef __GIT_INFO_H__ > %GIT_VERSION_FILE%
    ECHO #define IMG_FW_GIT_VERSION 0xFFFFFFFF>> %GIT_VERSION_FILE%
    ECHO #endif >> %GIT_VERSION_FILE%
    ECHO Create git_info.h=0xFFFFFFFF done
)
ECHO.