cd %~dp0
DEL ClientRegistry.blob
DEL InstallRecord.blob

rem HLDSUpdateTool -command update -game alienswarm -dir "%~dp0" -verify_all