DEL "%~dp0swarm\consolelog.log"

START srcds.exe +con_logfile "consolelog.log" -port 27015 +map lobby -game swarm -console