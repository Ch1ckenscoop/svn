@echo off
if EXIST "..\..\..\game\swarm\bin\.\server.dll" for /f "delims=" %%A in ('attrib "..\..\..\game\swarm\bin\.\server.dll"') do set valveTmpIsReadOnly="%%A"
set valveTmpIsReadOnlyLetter=%valveTmpIsReadOnly:~6,1%
if "%valveTmpIsReadOnlyLetter%"=="R" del /q "e:\Ch1ckensCoop_source\game\server\Release_swarm\"server.dll
if exist ..\..\devtools\bin\vpccrccheck.exe ..\..\devtools\bin\vpccrccheck.exe -crc2 test_swarm.vcproj
if ERRORLEVEL 1 exit 1

if errorlevel 1 goto VCReportError
goto VCEnd
:VCReportError
echo Project : error PRJ0019: A tool returned an error code from "Performing Pre-Build Event..."
exit 1
:VCEnd