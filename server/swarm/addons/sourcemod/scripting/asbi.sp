#include <sourcemod>
#include <string>

#define INDEX_USER 0
#define INDEX_STEAMID 1
#define INDEX_NICKNAME 2
#define INDEX_MARINES 3
#define INDEX_ACTIVE_MARINE 4
#define INDEX_ALIENS_KILLED 5
#define INDEX_HURT 6
#define INDEX_DEAD 7
#define INDEX_DISCONNECTED 8
#define SIZE 9

new Handle:userMap;
new Handle:nickNameList;
new Handle:steamIDList;

new missionSuccessful;

#define STEAMID_LENGTH 32
#define ACHIEVEMENT_LENGTH 256
#define MAPNAME_LENGTH 100

new Handle:databaseHandle = INVALID_HANDLE;

new Handle:conVarASWSkill;

new Handle:copyList;

new Handle:mapTimer = INVALID_HANDLE;
new secondCount;

new bool:usedFlamerDuringMission;
#define CLASS_ASW_GRENADE_LAUNCHER 89
new bool:usedGrenLauncherDuringMission;
new bool:usedAdrenalineDuringMission;

new Handle:cvar_sv_cheats;
new Handle:cvar_asw_god;
new bool:cheated;
new bool:cheatCmd;
new bool:cheatCmdUnset;

#define DEBUG_ACHIEVES false


new String:validMaps[][] = { 
	"asi-jac1-landingbay_01", 
	"asi-jac1-landingbay_02", 
	"asi-jac2-deima" ,
	"asi-jac3-rydberg",
	"asi-jac4-residential",
	"asi-jac6-sewerjunction",
	"asi-jac7-timorstation",
	"arctic-infiltration",
	"tft-desertoutpost",
	"tft-abandonedmaintenance",
	"tft-spaceport",
	"mission1",
	"mission2",
	"mission3",
	"as_sci1_bridge",
	"as_sci2_sewer",
	"as_sci3_maintenance",
	"as_sci4_vent",
	"as_sci5_complex",
	"dc1-omega_city",
	"dc2-breaking_an_entry",
	"dc3-search_and_rescue",
	"ocs_1",
	"ocs_2",
	"ocs_3",
	"beginning",
	"underground",
	"middelsech",
	"ending",
	"xen-cathalu-labs"
};


#define CAMPAIGN_COUNT 6
#define CAMPAIGN_MAXMAPS 10
new String:campaigns[CAMPAIGN_COUNT][CAMPAIGN_MAXMAPS+1][] = {
	{ "jacobsrest", 
		"asi-jac1-landingbay_01", 
		"asi-jac1-landingbay_02", 
		"asi-jac2-deima" ,
		"asi-jac3-rydberg",
		"asi-jac4-residential",
		"asi-jac6-sewerjunction",
		"asi-jac7-timorstation",
		"",
		"",
		""
	},
	{ "tearsfortarnor1", 
		"tft-desertoutpost",
		"tft-abandonedmaintenance",
		"tft-spaceport",
		"",
		"",
		"",
		"",
		"",
		"",
		""
	},
	{ "paranoia", 
		"mission1",
		"mission2",
		"mission3",
		"",
		"",
		"",
		"",
		"",
		"",
		""
	},
	{ "lanasescape", 
		"as_sci1_bridge",
		"as_sci2_sewer",
		"as_sci3_maintenance",
		"as_sci4_vent",
		"as_sci5_complex",
		"",
		"",
		"",
		"",
		""
	},
	{ "operationcleansweep", 
		"ocs_1",
		"ocs_2",
		"ocs_3",
		"",
		"",
		"",
		"",
		"",
		"",
		""
	},
	{ "thebeginning", 
		"beginning",
		"underground",
		"middelsech",
		"ending",
		"",
		"",
		"",
		"",
		"",
		""
	}

}

#define DEFINED_SPEEDRUNS 7
new String:speedruns[DEFINED_SPEEDRUNS][2][] = {
	{ "asi-jac1-landingbay_01", 	"85" },
	{ "asi-jac1-landingbay_02", 	"170" },
	{ "asi-jac2-deima" ,		"150" },
	{ "asi-jac3-rydberg",		"190" },
	{ "asi-jac4-residential",	"150" },
	{ "asi-jac6-sewerjunction",	"90" },
	{ "asi-jac7-timorstation",	"265" }
}


public isValidMap() {
	new String:mapName[MAPNAME_LENGTH]; GetCurrentMap(mapName, sizeof(mapName));

	
	for (new i=0;i<sizeof(validMaps);i++) {
		if (strlen(validMaps[i]) > 0 && StrEqual(mapName, validMaps[i], false)) {
			return true;
		}
	}
	return false;
}


public OnPluginStart() {
	HookEvent("alien_hurt", AlienHurt);

	HookEvent("mission_success", MissionSuccess);

	HookEvent("marine_hurt", MarineHurt);
	HookEvent("marine_infested", MarineInfested);
	HookEvent("entity_killed", EntityKilled); 

	HookEvent("marine_selected", MarineSelected);
	HookEvent("alien_died", AlienDied); 

	HookEvent("recommend_hold_position", CustomEvent);
	
	userMap = CreateArray(SIZE, 0);
	nickNameList = CreateArray(MAX_NAME_LENGTH, 0);
	steamIDList = CreateArray(STEAMID_LENGTH, 0);

	conVarASWSkill = FindConVar("asw_skill");

	copyList = CreateArray(512, 0);

	cvar_sv_cheats = FindConVar("sv_cheats");
	cvar_asw_god = FindConVar("asw_god");
	HookConVarChange(cvar_sv_cheats,cheatsChanged);
	HookConVarChange(cvar_asw_god,cheatsChanged);
	
	AddCommandListener(cheatCommand, "ai_disable");
	AddCommandListener(cheatCommandEnd, "ai_enable");
}

public OnMapStart() {
	ClearArray(userMap);
	ClearArray(nickNameList);
	ClearArray(steamIDList);

	missionSuccessful = false;
	usedFlamerDuringMission = false;
	usedGrenLauncherDuringMission = false;
	usedAdrenalineDuringMission = false;
	updateCheated();

	if (databaseHandle == INVALID_HANDLE) {
		SQL_TConnect(OnSQLConnect, "statsDB");
	}

	secondCount = 0;
}


public Action:cheatCommand(client, const String:command[], argc) {
	cheatCmd = true;
	cheatCmdUnset = false;
	if (!cheated) {
		updateCheated();
	}
	
	return Plugin_Continue;
}
public Action:cheatCommandEnd(client, const String:command[], argc) {
	cheatCmdUnset = true;
	
	return Plugin_Continue;
}

public cheatsChanged(Handle:convar, const String:oldValue[], const String:newValue[]) {
	if (!cheated) {
		updateCheated();
	}
}
public updateCheated() {
	cheated = GetConVarBool(cvar_sv_cheats) || GetConVarBool(cvar_asw_god) || cheatCmd;
	if (cheated) {
		SendChatToAll("cheat cvars set - disabling stats");
	}
	if (cheatCmdUnset) {
		cheatCmd = false;
	}
	
}


public OnSQLConnect(Handle:owner, Handle:hndl, const String:error[], any:data) {
	if (hndl == INVALID_HANDLE) {
		LogError("Database failure: %s", error);
	} else {
		databaseHandle = hndl;
	}
}

public OnMapEnd() {
	if (isValidMap() && (!cheated || DEBUG_ACHIEVES)) {
		if (databaseHandle != INVALID_HANDLE) {
			SQLInsertMapStats();
		
			for (new userindex = 0; userindex < GetArraySize(userMap); userindex++) {
				//only write for users that had a marine selected at one point
				new Handle: marineList = GetArrayCell(userMap, userindex, INDEX_MARINES);

				if (GetArraySize(marineList) > 0) {
					SQLInsertUserName(userindex);
					SQLInsertPlayerStats(userindex);
			
					SQLInsertPlayerMap(userindex);
					SQLInsertTimes(userindex);
				}
			}
		}
	}



	if (mapTimer != INVALID_HANDLE) {
		KillTimer(mapTimer);
		mapTimer = INVALID_HANDLE;
	}
}

public SQLSelectAchievements() {
	if (cheated && !DEBUG_ACHIEVES) {
		SendChatToAll("cheat cvars set - no achievements available");
		return;
	}
	if (DEBUG_ACHIEVES || cheated) {	//cheated used here only to avoid a warning
		SendChatToAll("debugging mode enabled - cheat cvars ignored");
	}

	if (databaseHandle != INVALID_HANDLE) {
		new String:mapName[MAPNAME_LENGTH]; GetCurrentMap(mapName, sizeof(mapName));


		new difficulty = GetConVarInt(conVarASWSkill);
		new copyID = GetArraySize(copyList);	//copyID is the same for all workers, because the workers delete one set of data when they start

		for (new userindex = 0; userindex < GetArraySize(userMap); userindex++) {
			if (GetArrayCell(userMap, userindex, INDEX_DISCONNECTED) == true) {
				continue;
			}

			new Handle: marineList = GetArrayCell(userMap, userindex, INDEX_MARINES);
			if (GetArraySize(marineList) == 0) {
				//player has not actually played on this map
				continue;
			}

			new String:steamid[STEAMID_LENGTH]; GetArrayString(steamIDList, GetArrayCell(userMap, userindex, INDEX_STEAMID), steamid, sizeof(steamid));
			new String:nickname[MAX_NAME_LENGTH]; GetArrayString(nickNameList, GetArrayCell(userMap, userindex, INDEX_NICKNAME), nickname, sizeof(nickname));

			PushArrayCell(copyList, CloneArray(userMap));
			PushArrayCell(copyList, difficulty);
			PushArrayString(copyList, mapName);
			PushArrayString(copyList, steamid);
			PushArrayString(copyList, nickname);
			PushArrayCell(copyList, userindex);


			new String:query[256+sizeof(steamid)];
			Format(query, sizeof(query), "SELECT steamid, name, achievement FROM achievements NATURAL JOIN usernames WHERE steamid='%s' AND (difficulty=%d OR difficulty=-1)", 
				steamid, difficulty);

			SQL_TQuery(databaseHandle, onSQLSelectAchievements, query, copyID);
		}
	}
}

public onSQLSelectAchievements(Handle:owner, Handle:hndl, const String:error[], any:data) {
	if(hndl == INVALID_HANDLE) {
		LogError(error);
		return;
	}


	new index = data;
	SQL_LockDatabase(databaseHandle);	//does this work? is this necessary? :|
	new Handle:userMapCopy = GetArrayCell(copyList, index); RemoveFromArray(copyList, index);
	new difficulty = GetArrayCell(copyList, index); RemoveFromArray(copyList, index);
	new String:mapName[MAPNAME_LENGTH]; GetArrayString(copyList, index, mapName, sizeof(mapName)); RemoveFromArray(copyList, index);
	new String:steamid[STEAMID_LENGTH]; GetArrayString(copyList, index, steamid, sizeof(steamid)); RemoveFromArray(copyList, index);
	new String:nickname[MAX_NAME_LENGTH]; GetArrayString(copyList, index, nickname, sizeof(nickname)); RemoveFromArray(copyList, index);
	new userindex = GetArrayCell(copyList, index); RemoveFromArray(copyList, index);
	SQL_UnlockDatabase(databaseHandle);


	new Handle:achievements;
	achievements = CreateArray(ACHIEVEMENT_LENGTH, 0);


	while (SQL_FetchRow(hndl)) {
		new String:achievement[ACHIEVEMENT_LENGTH]; SQL_FetchString(hndl, 2, achievement, sizeof(achievement));
		PushArrayString(achievements, achievement);
	}


	//nofire-nogl-nostim-
	modAchievementFire(userMapCopy, achievements, steamid, nickname, mapName, difficulty);

	
	//veteran must come after mission achievements
	achievement_Veteran(achievements, steamid, nickname, difficulty);
	achievement_Perfect(userindex, achievements, steamid, nickname, userMapCopy, difficulty);
	achievement_HordeKiller(userindex, achievements, steamid, nickname, userMapCopy, difficulty);
	achievement_SickBastard(userindex, achievements, steamid, nickname, userMapCopy, difficulty);


	CloseHandle(userMapCopy);
	CloseHandle(achievements);
}

public modAchievementFire(Handle:userMapCopy, Handle:achievements, String:steamid[], String:nickname[], String:mapName[], difficulty) {
	if (!usedFlamerDuringMission) {
		modAchievementNoGL(userMapCopy, achievements, steamid, nickname, mapName, difficulty, "noflamer-", "NoFlamer ", true);
	}
	modAchievementNoGL(userMapCopy, achievements, steamid, nickname, mapName, difficulty, "", "", usedFlamerDuringMission);
}
public modAchievementNoGL(Handle:userMapCopy, Handle:achievements, String:steamid[], String:nickname[], String:mapName[], difficulty, String:mod[], String:modName[], print) {
	if (!usedGrenLauncherDuringMission) {
		new String:doneMod[256];
		Format(doneMod, sizeof(doneMod), "%snogl-", mod);

		new String:doneModName[256];
		Format(doneModName, sizeof(doneModName), "%sNoGL ", modName);
		
		modAchievementStim(userMapCopy, achievements, steamid, nickname, mapName, difficulty, doneMod, doneModName, print);
	}
	modAchievementStim(userMapCopy, achievements, steamid, nickname, mapName, difficulty, mod, modName, usedGrenLauncherDuringMission && print);
}
public modAchievementStim(Handle:userMapCopy, Handle:achievements, String:steamid[], String:nickname[], String:mapName[], difficulty, String:mod[], String:modName[], print) {
	if (!usedAdrenalineDuringMission) {
		new String:doneMod[256];
		Format(doneMod, sizeof(doneMod), "%snostim-", mod);

		new String:doneModName[256];
		Format(doneModName, sizeof(doneModName), "%sNoStim ", modName);
		
		modAchievement(userMapCopy, achievements, steamid, nickname, mapName, difficulty, doneMod, doneModName, print);
	}
	modAchievement(userMapCopy, achievements, steamid, nickname, mapName, difficulty, mod, modName, usedAdrenalineDuringMission && print);
}

public modAchievement(Handle:userMapCopy, Handle: achievements, String:steamid[], String:nickname[], String:mapName[], difficulty, String:mod[], String:modName[], print) {
	new String:doneMod[256];
	new String:doneModName[256];
	
	new sound = executedSoundly(userMapCopy);
	if (sound) {
		// ****
		// sound mission/speedrun
		// ****

		Format(doneMod, sizeof(doneMod), "%snodeaths", mod);
		Format(doneModName, sizeof(doneModName), "%sSound Execution", modName);
		achievement_Mission(achievements, steamid, nickname, mapName, difficulty, doneMod, doneModName, print);

		
		Format(doneMod, sizeof(doneMod), "%ssound-", mod);
		Format(doneModName, sizeof(doneModName), "%sSound ", modName);
		achievement_SpeedRun(achievements, steamid, nickname, mapName, difficulty, doneMod, doneModName, print);
	}
	
	// ****
	// mission/speedrun
	// ****
	Format(doneMod, sizeof(doneMod), "%smission", mod);
	Format(doneModName, sizeof(doneModName), "%sMission Complete", modName);
	achievement_Mission(achievements, steamid, nickname, mapName, difficulty, doneMod, doneModName, !sound && print);
	
	
	achievement_SpeedRun(achievements, steamid, nickname, mapName, difficulty, mod, doneMod, !sound && print);

	
	
	// ****
	// campaigns
	// ****

	
	Format(doneMod, sizeof(doneMod), "%smission", mod);
	Format(doneModName, sizeof(doneModName), "%sCampaign complete", modName);
	new String:doneCampaignMod[256];
	Format(doneCampaignMod, sizeof(doneCampaignMod), "%scampaign", mod);
	achievement_Campaign(achievements, steamid, nickname, difficulty, doneMod, doneModName, doneCampaignMod, print);
	
	
	Format(doneMod, sizeof(doneMod), "%snodeaths", mod);
	Format(doneModName, sizeof(doneModName), "%sOutstanding Execution", modName);
	achievement_Campaign(achievements, steamid, nickname, difficulty, doneMod, doneModName, "", print);
	
	
	Format(doneMod, sizeof(doneMod), "%sspeedrun", mod);
	Format(doneModName, sizeof(doneModName), "%sSpeedrun Mastery", modName);
	achievement_Campaign(achievements, steamid, nickname, difficulty, doneMod, doneModName, "", print);
	
	
	Format(doneMod, sizeof(doneMod), "%ssound-speedrun", mod);
	Format(doneModName, sizeof(doneModName), "%sOutstanding Speedrun Mastery", modName);
	achievement_Campaign(achievements, steamid, nickname, difficulty, doneMod, doneModName, "", print);
}

public putAchievement(Handle:achievements, String:steamid[], difficulty, String:achievement[]) {
	new String:query[256+STEAMID_LENGTH+ACHIEVEMENT_LENGTH];
	Format(query, sizeof(query), "INSERT INTO achievements VALUES ('%s', %d, '%s', now())", 
		steamid, difficulty, achievement);

	if (isValidMap()) {
		SQL_TQuery(databaseHandle, onSQLInsertAchievement, query);

		PushArrayString(achievements, achievement);
	}
}
public onSQLInsertAchievement(Handle:owner, Handle:hndl, const String:error[], any:data) {
	if (hndl == INVALID_HANDLE) {
		LogError("Query InsertAchievement failed! %s", error);
	}
}

public printAchievement(String:name[], difficulty, String:achievement[]) {
	new String:difficultyName[16];
	switch (difficulty) {
		case 1: difficultyName = "Easy";
		case 2: difficultyName = "Normal";
		case 3: difficultyName = "Hard";
		case 4: difficultyName = "Insane";
		case 5: difficultyName = "Brutal";
	}

	new String:message[256+STEAMID_LENGTH+ACHIEVEMENT_LENGTH];
	if (difficulty == -1) {
		Format(message, sizeof(message), "%s achieved: %s", 
			name, achievement);
	} else {
		Format(message, sizeof(message), "%s achieved: %s on %s", 
			name, achievement, difficultyName);
	}

	SendChatToAll(message);

}

SendChatToAll(String:message[]) {
	for (new i = 1; i <= MaxClients; i++) {
		if (!IsClientInGame(i) || IsFakeClient(i)) {
			continue;
		}
		PrintToChat(i, "%s", message);
	}
}


achievement_Perfect(userindex, Handle:achievements, String:steamid[], String:nickname[], Handle:userMapCopy, difficulty){
	new timesHurt = GetArrayCell(userMapCopy, userindex, INDEX_HURT);
	new String:achievement[32] = "perfect";
	
	if (timesHurt == 0) {
		if (FindStringInArray(achievements, achievement) == -1) {
			putAchievement(achievements, steamid, difficulty, achievement);
			printAchievement(nickname, difficulty, "Perfect");
		}
	}
}

achievement_Mission(Handle:achievements, String:steamid[], String:nickname[], String:mapName[], difficulty, String:achievementNamePrefix[]="", String:achievementTextPrefix[]="", print=true){
	new String:achievement[ACHIEVEMENT_LENGTH];
	Format(achievement, sizeof(achievement), "%s %s", achievementNamePrefix, mapName);
	for (new i=0;i<sizeof(achievement); i++) {
		achievement[i] = CharToLower(achievement[i]);
	}

	new String:shortMap[100];
	if (StrContains(mapName, "asi-jac", false) == 0) {
		for (new i=9 /*length(asi-jacN-)*/;i<strlen(mapName);i++) {
			shortMap[i-9] = mapName[i];
		}
	} else {
		strcopy(shortMap, sizeof(shortMap), mapName);
	}

	if (FindStringInArray(achievements, achievement) == -1) {
		putAchievement(achievements, steamid, difficulty, achievement);

		if (print) {
			new String:msg[100];
			Format(msg, sizeof(msg), "%s (%s)", achievementTextPrefix, shortMap);
			printAchievement(nickname, difficulty, msg);
		}
	}
}

bool:executedSoundly(Handle:userMapCopy) {
	for (new i=0;i<GetArraySize(userMapCopy);i++) {
		new dead = GetArrayCell(userMapCopy, i, INDEX_DEAD);
		if (dead != 0) {
			return false;
		}
	}
	if (GetArraySize(userMapCopy) < 4 && !DEBUG_ACHIEVES) {
		return false;
	}
	return true;
}

//must be called after achievement_Mission
achievement_Campaign(Handle:achievements, String:steamid[], String:nickname[], difficulty, String:mapAchievementName[], String:achievementText[], String:campaignAchievementText[]="", print=true){
	for (new campaignCtr = 0; campaignCtr < CAMPAIGN_COUNT; campaignCtr++) {
		new String:achievement[ACHIEVEMENT_LENGTH];
		if (StrEqual(campaignAchievementText, "")) {
			Format(achievement, sizeof(achievement), "%s-campaign %s", mapAchievementName, campaigns[campaignCtr][0]);
		} else {
			Format(achievement, sizeof(achievement), "%s %s", campaignAchievementText, campaigns[campaignCtr][0]);
		}

		if (FindStringInArray(achievements, achievement) == -1) {
			new bool:newlyAchieved = true;

			for (new mapCtr = 1; mapCtr < CAMPAIGN_MAXMAPS; mapCtr++) {
				if (strlen(campaigns[campaignCtr][mapCtr]) > 0) {
					new String:mapAchievement[ACHIEVEMENT_LENGTH];
					Format(mapAchievement, sizeof(mapAchievement), "%s %s", mapAchievementName, campaigns[campaignCtr][mapCtr]);
					if (FindStringInArray(achievements, mapAchievement) == -1) {
						newlyAchieved = false;
					}
				}
			}

			if (newlyAchieved == true) {
				putAchievement(achievements, steamid, difficulty, achievement);
				if (print) {
					printAchievement(nickname, difficulty, achievementText);
				}
			}
		}
	}
}


achievement_HordeKiller(userindex, Handle:achievements, String:steamid[], String:nickname[], Handle:userMapCopy, difficulty){
	new aliensKilled = GetArrayCell(userMapCopy, userindex, INDEX_ALIENS_KILLED);
	new String:achievement[32] = "hordekiller";
	
	if (aliensKilled >= 200) {
		if (FindStringInArray(achievements, achievement) == -1) {
			putAchievement(achievements, steamid, difficulty, achievement);
			printAchievement(nickname, difficulty, "Horde Killer (200 kills)");
		}
	}
}

achievement_SickBastard(userindex, Handle:achievements, String:steamid[], String:nickname[], Handle:userMapCopy, difficulty){
	new aliensKilled = GetArrayCell(userMapCopy, userindex, INDEX_ALIENS_KILLED);
	new String:achievement[32] = "sickbastard";
	
	if (aliensKilled >= 350) {
		if (FindStringInArray(achievements, achievement) == -1) {
			putAchievement(achievements, steamid, difficulty, achievement);
			printAchievement(nickname, difficulty, "Sick Bastard (350 kills)");
		}
	}
}

achievement_Veteran(Handle:achievements, String:steamid[], String:nickname[], difficulty){
	new String:achievement[ACHIEVEMENT_LENGTH] = "veteran";

	if (FindStringInArray(achievements, achievement) == -1) {
		new ctr = 0;
		for (new i=0;i<GetArraySize(achievements);i++) {
			new String:existingAchievement[ACHIEVEMENT_LENGTH]; 
			GetArrayString(achievements, i, existingAchievement, sizeof(existingAchievement));

			if (StrContains(existingAchievement, "mission ")==0) {	//"mission XXX" achievement
				ctr++;
			}
		}
		if (ctr >= 3) {
			putAchievement(achievements, steamid, difficulty, achievement);
			printAchievement(nickname, difficulty, "Veteran (3 missions complete)");
		}
	}
}


achievement_SpeedRun(Handle:achievements, String:steamid[], String:nickname[], String:mapName[], difficulty, String:achievementNamePrefix[]="", String:achievementTextPrefix[]="", print=true){
	new String:achievement[ACHIEVEMENT_LENGTH];
	Format(achievement, sizeof(achievement), "%sspeedrun %s", achievementNamePrefix, mapName);
	for (new i=0;i<sizeof(achievement); i++) {
		achievement[i] = CharToLower(achievement[i]);
	}

	new String:shortMap[100];
	if (StrContains(mapName, "asi-jac", false) == 0) {
		for (new i=9 /*length(asi-jacN-)*/;i<strlen(mapName);i++) {
			shortMap[i-9] = mapName[i];
		}
	} else {
		strcopy(shortMap, sizeof(shortMap), mapName);
	}

	if (FindStringInArray(achievements, achievement) == -1) {
		for (new i=0;i<DEFINED_SPEEDRUNS;i++) {
			new String:mapAchievement[100];
			Format(mapAchievement, sizeof(mapAchievement), "%sspeedrun %s", achievementNamePrefix, speedruns[i][0]);
			if (StrEqual(achievement, mapAchievement)) {
				if (secondCount <= StringToInt(speedruns[i][1])) {
					putAchievement(achievements, steamid, difficulty, achievement);
			
					if (print) {
						new String:msg[100];
						Format(msg, sizeof(msg), "%sSpeedrun (%s)", achievementTextPrefix, shortMap);
						printAchievement(nickname, difficulty, msg);
					}
				}
				return;
			}
		}
	}
}


public SQLInsertPlayerMap(userindex) {
	new String:steamid[STEAMID_LENGTH]; GetArrayString(steamIDList, GetArrayCell(userMap, userindex, INDEX_STEAMID), steamid, sizeof(steamid));
	new difficulty = GetConVarInt(conVarASWSkill);
	new String:mapName[MAPNAME_LENGTH]; GetCurrentMap(mapName, sizeof(mapName));
	new wins = missionSuccessful?1:0;
	new losses = missionSuccessful?0:1;


	new String:query[256+sizeof(steamid)];
	Format(query, sizeof(query), "INSERT INTO playermaps VALUES ('%s', %d, '%s', %d, %d) ON DUPLICATE KEY UPDATE wins=wins+%d, losses=losses+%d", 
		steamid, difficulty, mapName, wins, losses, wins, losses);

	SQL_TQuery(databaseHandle, onSQLInsertPlayerMap, query);
}

public onSQLInsertPlayerMap(Handle:owner, Handle:hndl, const String:error[], any:data) {
	if (hndl == INVALID_HANDLE) {
		LogError("Query InsertPlayerMap failed! %s", error);
	}
}

public SQLInsertTimes(userindex) {
	if (!missionSuccessful) {
		return;
	}

	new String:steamid[STEAMID_LENGTH]; GetArrayString(steamIDList, GetArrayCell(userMap, userindex, INDEX_STEAMID), steamid, sizeof(steamid));
	new difficulty = GetConVarInt(conVarASWSkill);
	new String:mapName[MAPNAME_LENGTH]; GetCurrentMap(mapName, sizeof(mapName));
	new missionTime = secondCount;

	new noflamer=!usedFlamerDuringMission;
	new sound=executedSoundly(userMap);

	new String:query[256+sizeof(steamid)];
	Format(query, sizeof(query), "INSERT INTO maptimes VALUES ('%s', %d, '%s', %d, now(), %d, %d) ON DUPLICATE KEY UPDATE time=CASE WHEN seconds>%d THEN NOW() ELSE time END, seconds=LEAST(seconds, %d);", 
		steamid, difficulty, mapName, missionTime, noflamer, sound, missionTime, missionTime);

	SQL_TQuery(databaseHandle, onSQLInsertTimes, query);
}

public onSQLInsertTimes(Handle:owner, Handle:hndl, const String:error[], any:data) {
	if (hndl == INVALID_HANDLE) {
		LogError("Query InsertTimes failed! %s", error);
	}
}


public SQLInsertPlayerStats(userindex) {
	new String:steamid[STEAMID_LENGTH]; GetArrayString(steamIDList, GetArrayCell(userMap, userindex, INDEX_STEAMID), steamid, sizeof(steamid));
	new difficulty = GetConVarInt(conVarASWSkill);
	new kills = GetArrayCell(userMap, userindex, INDEX_ALIENS_KILLED);
	new deaths = GetArrayCell(userMap, userindex, INDEX_DEAD)!=0 ? 1:0;
	new wins = missionSuccessful?1:0;
	new losses = missionSuccessful?0:1;



	new String:query[256+sizeof(steamid)];
	Format(query, sizeof(query), "INSERT INTO playerstats VALUES ('%s', %d, %d, %d, %d, %d) ON DUPLICATE KEY UPDATE kills=kills+%d, deaths=deaths+%d, wins=wins+%d, losses=losses+%d", 
		steamid, difficulty, kills, deaths, wins, losses, kills, deaths, wins, losses);

	SQL_TQuery(databaseHandle, onSQLInsertPlayerStats, query);
}
public onSQLInsertPlayerStats(Handle:owner, Handle:hndl, const String:error[], any:data) {
	if (hndl == INVALID_HANDLE) {
		LogError("Query InsertPlayerStats failed! %s", error);
	}
}


public SQLInsertUserName(userindex) {
	new String:steamid[STEAMID_LENGTH]; GetArrayString(steamIDList, GetArrayCell(userMap, userindex, INDEX_STEAMID), steamid, sizeof(steamid));


	new String:nickname[MAX_NAME_LENGTH]; GetArrayString(nickNameList, GetArrayCell(userMap, userindex, INDEX_NICKNAME), nickname, sizeof(nickname));
	new String:escapedNick[2*MAX_NAME_LENGTH+1];
	SQL_EscapeString(databaseHandle, nickname, escapedNick, sizeof(escapedNick));


	new String:query[100+sizeof(steamid)+2*sizeof(escapedNick)];
	Format(query, sizeof(query), "INSERT INTO usernames VALUES ('%s', '%s') ON DUPLICATE KEY UPDATE name='%s'", 
		steamid, escapedNick, escapedNick);

	SQL_TQuery(databaseHandle, onSQLInsertUserName, query);
}
public onSQLInsertUserName(Handle:owner, Handle:hndl, const String:error[], any:data) {
	if (hndl == INVALID_HANDLE) {
		LogError("Query InsertUserName failed! %s", error);
	}
}


public SQLInsertMapStats() {
	new String:mapName[MAPNAME_LENGTH];
	new difficulty;
	new String:isSuccess[10];
	new marinesKilled;
	new playerCount;
	new aliensKilled;


	GetCurrentMap(mapName, sizeof(mapName));
	difficulty = GetConVarInt(conVarASWSkill);
	isSuccess = missionSuccessful?"true":"false";
	playerCount = GetArraySize(userMap);
	for (new i=0; i<playerCount; i++){
		marinesKilled += GetArrayCell(userMap, i, INDEX_DEAD);
		aliensKilled += GetArrayCell(userMap, i, INDEX_ALIENS_KILLED);


		new Handle: marineList = GetArrayCell(userMap, i, INDEX_MARINES);
		if (GetArraySize(marineList) == 0) {
			//no marines means this player wasn't actually active on this map
			playerCount--;
		}
	}


	if (playerCount == 0 && aliensKilled == 0) {
		//don't write empty stats
		return;
	}

	new String:query[256];
	Format(query, sizeof(query), "INSERT INTO mapstats VALUES ('%s', %d, %s, now(), %d, %d, %d)", 
		mapName, difficulty, isSuccess, marinesKilled, playerCount, aliensKilled );

	SQL_TQuery(databaseHandle, onSQLInsertMapStat, query);
}
public onSQLInsertMapStat(Handle:owner, Handle:hndl, const String:error[], any:data) {
	if (hndl == INVALID_HANDLE) {
		LogError("Query InsertMapStat failed! %s", error);
	}
}

public Action:MissionSuccess(Handle:event, const String:name[], bool:dontBroadcast) {
	missionSuccessful = true;
	KillTimer(mapTimer);
	mapTimer = INVALID_HANDLE;

	SQLSelectAchievements();
}

public Action:MarineSelected(Handle:event, const String:name[], bool:dontBroadcast)
{
	new userid = GetEventInt(event, "userid");
	new newMarine = GetEventInt(event, "new_marine");
	
	if (mapTimer == INVALID_HANDLE) {
		mapTimer = CreateTimer(1.0, SecondCounter, _, 1)
	}

	addMarineToUserAndSetActive(userid, newMarine);
}

public Action:SecondCounter(Handle:timer) {
	secondCount++;
	return Plugin_Continue;
}


public OnClientDisconnect(client) {
	if (IsClientInGame(client) && !missionSuccessful) {
		new userid = GetClientUserId(client);
		new index = getUserIndexByUserID(userid);
		SetArrayCell(userMap, index, true, INDEX_DISCONNECTED);
		SetArrayCell(userMap, index, -1, INDEX_ACTIVE_MARINE);
	}
}


public getUserIndexByUserID(key) {
	new Handle:array = userMap;

	new size = GetArraySize(array);
	for (new i = 0; i < size; i++) {
		new userID = GetArrayCell(array, i, INDEX_USER);
		if (userID == key) {
			return i;
		}
	}

	PushArrayCell(array, key)
	new Handle:marineList = CreateArray(1, 0);


	new client = GetClientOfUserId(key);
	new String:steamID[STEAMID_LENGTH]; GetClientAuthString(client, steamID, sizeof(steamID))
	PushArrayString(steamIDList, steamID);
	SetArrayCell(array, size, GetArraySize(steamIDList)-1, INDEX_STEAMID);

	new String:nickName[MAX_NAME_LENGTH]; GetClientName(client, nickName, sizeof(nickName))
	PushArrayString(nickNameList, nickName);
	SetArrayCell(array, size, GetArraySize(nickNameList)-1, INDEX_NICKNAME);

	SetArrayCell(array, size, marineList, INDEX_MARINES);
	SetArrayCell(array, size, 0, INDEX_ACTIVE_MARINE);
	SetArrayCell(array, size, 0, INDEX_ALIENS_KILLED);
	SetArrayCell(array, size, 0, INDEX_HURT);
	SetArrayCell(array, size, 0, INDEX_DEAD);
	SetArrayCell(array, size, false, INDEX_DISCONNECTED);

	return size;
}

public addMarineToUserAndSetActive(userid, marine) {
	new index = getUserIndexByUserID(userid);

	new Handle: marineList = GetArrayCell(userMap, index, INDEX_MARINES);
	new found = FindValueInArray(marineList, marine);
	if (found == -1) {
		PushArrayCell(marineList, marine);
	}

	SetArrayCell(userMap, index, marine, INDEX_ACTIVE_MARINE);
	SetArrayCell(userMap, index, false, INDEX_DISCONNECTED);//can't select a marine while being disconnected.. I think
}


public getUserIndexByMarineID(key) {
	new Handle:array = userMap;

	new size = GetArraySize(array);
	for (new i = 0; i < size; i++) {
		new Handle:marineList = GetArrayCell(array, i, INDEX_MARINES);

		new found = FindValueInArray(marineList, key);
		if (found != -1) {
			return i;
		}
	}


	return -1;
}

public Action:AlienDied(Handle:event, const String:name[], bool:dontBroadcast)
{
	//new alien = GetEventInt(event, "alien");
	new marine = GetEventInt(event, "marine");
	//new weapon = GetEventInt(event, "weapon");

	new index = getUserIndexByMarineID(marine);
	if (index != -1) {
		if (GetArrayCell(userMap, index, INDEX_ACTIVE_MARINE) == marine) {
			new killCount = GetArrayCell(userMap, index, INDEX_ALIENS_KILLED);
			SetArrayCell(userMap, index, killCount+1, INDEX_ALIENS_KILLED);
		} else {
			//killed by unselected bot
		}
	} else {
		//killed by bot or other
	}
}



public Action:EntityKilled(Handle:event, const String:name[], bool:dontBroadcast)
{
	new entindex_killed = GetEventInt(event, "entindex_killed");
	//new entindex_attacker = GetEventInt(event, "entindex_attacker");
	//new entindex_inflictor = GetEventInt(event, "entindex_inflictor");
	//new damagebits = GetEventInt(event, "damagebits");


	new index = getUserIndexByMarineID(entindex_killed);
	if (index != -1) { //a marine was killed
		SetArrayCell(userMap, index, 1, INDEX_DEAD);
	}
}

public Action:MarineHurt(Handle:event, const String:name[], bool:dontBroadcast)
{
	new userid = GetEventInt(event, "userid");
	//new attacker = GetEventInt(event, "attacker");
	new Float:health = GetEventFloat(event, "health");



	if (health != 1) {
		new index = getUserIndexByUserID(userid);
		new hurtCount = GetArrayCell(userMap, index, INDEX_HURT);
		SetArrayCell(userMap, index, hurtCount+1, INDEX_HURT);
	}
}

public Action:MarineInfested(Handle:event, const String:name[], bool:dontBroadcast)
{
	new entindex = GetEventInt(event, "entindex");

	new index = getUserIndexByMarineID(entindex);
	new hurtCount = GetArrayCell(userMap, index, INDEX_HURT);
	SetArrayCell(userMap, index, hurtCount+1, INDEX_HURT);
}


public Action:AlienHurt(Handle:event, const String:name[], bool:dontBroadcast) {
	new alien_setonfire = GetEventBool(event, "alien_setonfire");
	if (alien_setonfire) {
		usedFlamerDuringMission = true;
	}
	
	new weaponID = GetEventInt(event, "weapon");
	if (weaponID == CLASS_ASW_GRENADE_LAUNCHER) {
		usedGrenLauncherDuringMission = true;
	}
}

public Action:CustomEvent(Handle:event, const String:name[], bool:dontBroadcast) {
	new isStimEvent = GetEventBool(event, "stim_event");
	if (isStimEvent) {
		usedAdrenalineDuringMission = true;
	}
}

public Plugin:myinfo =
{
	name = "BI Achievements",
	author = "honk",
	description = "bi - tracker for stats and achievements",
	version = "2011-02-20",
	url = "http://naru.mooo.com/bi-stats/"
}