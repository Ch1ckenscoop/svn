@echo off
if exist "..\..\..\game\swarm\bin\." goto ValveStart
mkdir "..\..\..\game\swarm\bin\."
:ValveStart
copy "e:\Ch1ckensCoop_source\game\server\Release_swarm\"server.dll "..\..\..\game\swarm\bin\.\server.dll"
if ERRORLEVEL 1 goto BuildEventFailed
if exist "e:\Ch1ckensCoop_source\game\server\Release_swarm\"server.map copy "e:\Ch1ckensCoop_source\game\server\Release_swarm\"server.map "..\..\..\game\swarm\bin\.\server.map"
copy "e:\Ch1ckensCoop_source\game\server\Release_swarm\"server.pdb "..\..\..\game\swarm\bin\.\server.pdb"
if ERRORLEVEL 1 goto BuildEventFailed
goto BuildEventOK
:BuildEventFailed
echo *** ERROR! PostBuildStep FAILED for Server (Swarm)! EXE or DLL is probably running. ***
del /q "e:\Ch1ckensCoop_source\game\server\Release_swarm\"server.dll
exit 1
:BuildEventOK

if errorlevel 1 goto VCReportError
goto VCEnd
:VCReportError
echo Project : error PRJ0019: A tool returned an error code from "Publishing to ..\..\..\game\swarm\bin\."
exit 1
:VCEnd