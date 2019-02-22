@echo off
call %comspec% /k "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\VsDevCmd.bat"
set PATH=%PATH%;C:\Program Files\Git
git-bash.exe 

REM ===========================================
REM $ cat ~/.minttyrc
REM Locale=zh_CN
REM Charset=GBK
REM ===========================================
