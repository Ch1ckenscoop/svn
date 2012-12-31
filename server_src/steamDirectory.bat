@echo off

setlocal enabledelayedexpansion

rem Set the steamDirectory variable to the Steam directory.

for /f "tokens=2*" %%A in ('reg query "HKEY_CURRENT_USER\Software\Valve\Steam" /v "SteamPath" 2^>nul') do (
	
	set steamDirectory=%%B
	
	rem Replace slashes with backslashes.
	
	set steamDirectory=!steamDirectory:/=\!
	
)

if not defined steamDirectory (
	
	echo Couldn't find Steam.
	
	exit /b 1
	
)

rem Run the commands given to us.

cmd /k %*