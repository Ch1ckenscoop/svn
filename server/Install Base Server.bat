cd %~dp0
DEL ClientRegistry.blob
DEL InstallRecord.blob

HLDSUpdateTool -command update -game alienswarm -dir "%~dp0" -verify_all