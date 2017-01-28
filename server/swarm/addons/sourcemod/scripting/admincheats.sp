#pragma semicolon 1

#include <sourcemod>

//softcopy:
//#define PLUGIN_VERSION "0.2"
//#define PLUGIN_VERSION "0.2.1"	//fixed the wrong statement
//#define PLUGIN_VERSION "0.2.2"	//shorten the console information
//#define PLUGIN_VERSION "0.2.3"	//added send the cheat status to player who has issued the cheat command
//#define PLUGIN_VERSION "0.2.4"	//modified for sourcemod 1.7.2
#define PLUGIN_VERSION "0.2.5"		//added show cheats command and weapon name to admin
 
public Plugin:myinfo =
{
	name = "AdminCheats",
	author = "devicenull, updated by Softcopy",
	description = "Allow admins to use cheat commands for SourceMod v1.7.2",
	version = PLUGIN_VERSION,
	url = "http://www.sourcemod.net/"
};

#define MAX_COMMANDS 512

new String:hooked[MAX_COMMANDS][128];
new nextHooked=0;
new Handle:cCheatOverride;
new Handle:ccheatalert;

public OnPluginStart()
{
	cCheatOverride = CreateConVar("sm_admin_cheats_level","z","Level required to execute cheat commands",FCVAR_PLUGIN);
	CreateConVar("sm_admin_cheats_version",PLUGIN_VERSION,"Version Information",FCVAR_REPLICATED);
	ccheatalert = CreateConVar("sm_admin_cheats_alert", "1", "default=1, notify the cheat command success or not.");	//softcopy:

	new String:cmdname[128];
	new bool:iscmd, cmdflags;
	new Handle:cmds = FindFirstConCommand(cmdname,128,iscmd,cmdflags);
	do
	{
		if (cmdflags&FCVAR_CHEAT && iscmd && nextHooked < MAX_COMMANDS)
		{
			RegConsoleCmd(cmdname,cheatcommand);
			SetCommandFlags(cmdname,GetCommandFlags(cmdname)^FCVAR_CHEAT);
			strcopy(hooked[nextHooked++],128,cmdname);
		}
		if (nextHooked >= MAX_COMMANDS)
		{
			LogToGame("[admincheats] WARNING: Too many cheat commands to hook them all, increase MAX_COMMANDS");
			return;
		}
	}
	while (FindNextConCommand(cmds,cmdname,128,iscmd,cmdflags));
	PrintToServer("admincheats hooked %i commands",nextHooked);

}

public OnPluginEnd()
{
	PrintToServer("admincheats unloaded, restoring cheat flags");
	for (new i=0;i<nextHooked;i++)
	{
		SetCommandFlags(hooked[i],GetCommandFlags(hooked[i])|FCVAR_CHEAT);
	}
}

public Action:cheatcommand(client, args)
{
	//softcopy:
	new String:name[128];
	GetClientName(client,name,128);
	new String:steamid[64];
	new String:argstring2[256];
	GetCmdArg(1,argstring2,256);
	
	new String:access[8];
	GetConVarString(cCheatOverride,access,8);
	new String:argstring[256];
	GetCmdArg(0,argstring,256);
	if (client == 0)
	{
		//softcopy:
		//LogAction(0,-1,"CONSOLE ran cheat command '%s' success!",argstring);
		LogAction(0,-1,"CONSOLE ran cheat command '%s %s' success!",argstring, argstring2);
		
		return Plugin_Continue;
	}

	//softcopy:
	GetClientAuthId(client, AuthId_Steam2, steamid, sizeof(steamid));

	if (GetUserFlagBits(client)&ReadFlagString(access) > 0 || GetUserFlagBits(client)&ADMFLAG_ROOT > 0)
	{
		//softcopy:
		//LogClient(client,"ran cheat command '%s' success!",argstring);
		LogClient(client,"ran cheat command '%s %s' success!",argstring, argstring2);
		//doesn't work on SourceMod v1.7.2
		//for (new i=1;i<MaxClients;i++)
		//{
		//	if (IsClientConnected(i) && IsClientInGame(i) && !IsFakeClient(i))
		//	{
		//		//PrintToConsole(i,"%s <%s> ran cheat command '%s'",argstring);
		//	}
		//}
		//PrintToServer("%s ran cheat command '%s' success!", name, argstring);
		if (GetConVarInt(ccheatalert) == 1)
		{
			PrintToChat(client,"You ran cheat command '%s %s' success!",argstring, argstring2);
		}
		PrintToServer("%s<%s> ran cheat command '%s %s' success!", name, steamid, argstring, argstring2);

		return Plugin_Continue;
	}
	//softcopy:
	//LogClient(client,"was prevented from running cheat command '%s'",argstring);
	LogClient(client,"was denied to run cheat command '%s %s'",argstring, argstring2);
	if (GetConVarInt(ccheatalert) == 1)
	{
		PrintToChat(client,"You were denied to run cheat command '%s %s'",argstring, argstring2);
	}
	PrintToServer("%s<%s> was denied to run cheat command '%s %s'", name, steamid, argstring, argstring2);

	return Plugin_Handled;
}

public LogClient(client,String:format[], any:...)
{
	new String:buffer[512];
	VFormat(buffer,512,format,3);
	new String:name[128];
	new String:steamid[64];
	//new String:ip[32];	//softcopy:

	GetClientName(client,name,128);

	//softcopy: deprecated command
	//GetClientAuthString(client,steamid,64);
	//GetClientIP(client,ip,32);
	//LogAction(client,-1,"<%s><%s><%s> %s",name,steamid,ip,buffer);
	GetClientAuthId(client, AuthId_Steam2, steamid, sizeof(steamid));
	LogAction(client,-1,"%s <%s> %s",name, steamid, buffer);
}