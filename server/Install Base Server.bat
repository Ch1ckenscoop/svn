cd %~dp0
DEL ClientRegistry.blob
DEL InstallRecord.blob

HLDSUpdateTool -game alienswarm -command update -dir "%~dp0"