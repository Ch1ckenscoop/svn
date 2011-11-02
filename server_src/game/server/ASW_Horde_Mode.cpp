#include "cbase.h"
#include "ASW_Horde_Mode.h"
#include "convar.h"
#include "asw_spawn_manager.h"

LINK_ENTITY_TO_CLASS( asw_horde_mode, CASW_Horde_Mode );

//Ch1ckensCoop: External convars we need to have control over
extern ConVar asw_horde_override;

//Ch1ckensCoop: Core hordemode settings.
ConVar asw_hordemode("asw_hordemode", "0", FCVAR_CHEAT, "Enables hordemode on the server.");
ConVar asw_hordemode_debug("asw_hordemode_debug", "0", FCVAR_CHEAT, "Shows hordemode debug messages.");
ConVar asw_hordemode_update_mode("asw_hordemode_update_mode", "0", FCVAR_CHEAT, "0 - update after each horde spawn; >0 - Update every x seconds.");
ConVar asw_hordemode_mode("asw_hordemode_mode", "0", FCVAR_CHEAT, "0 - Health settings only; 1 - Static aliens; 2 - Binary + horde_size_max settings");

//Ch1ckensCoop: Hordemode spawning settings
ConVar asw_hordemode_aliens("asw_hordemode_aliens", "0", FCVAR_CHEAT, "Binary flag of allowed aliens.");
ConVar asw_hordemode_aliens_static("asw_hordemode_aliens_static", "0", FCVAR_CHEAT, "Single binary flag of aliens to spawn in static mode.");

//Ch1ckensCoop: Hordemode horde_size_max settings and alien health settings
//Drones
ConVar asw_hordemode_drone_max("asw_hordemode_drone_max", "0", FCVAR_CHEAT, "Maximum drones to spawn.");
ConVar asw_hordemode_drone_min("asw_hordemode_drone_min", "0", FCVAR_CHEAT, "Minimum drones to spawn.");
ConVar asw_hordemode_drone_health_max("asw_hordemode_drone_health_max", "50", FCVAR_CHEAT, "Maximum drone health.");
ConVar asw_hordemode_drone_health_min("asw_hordemode_drone_health_min", "25", FCVAR_CHEAT, "Minimum drone health.");

//Buzzers
ConVar asw_hordemode_buzzer_max("asw_hordemode_buzzer_max", "0", FCVAR_CHEAT, "Maximum buzzers to spawn.");
ConVar asw_hordemode_buzzer_min("asw_hordemode_buzzer_min", "0", FCVAR_CHEAT, "Minimum buzzers to spawn.");
ConVar asw_hordemode_buzzer_health_max("asw_hordemode_buzzer_health_max", "0", FCVAR_CHEAT, "Maximum buzzer health.");
ConVar asw_hordemode_buzzer_health_min("asw_hordemode_buzzer_health_min", "0", FCVAR_CHEAT, "Minimum buzzer health.");

//Parasites
ConVar asw_hordemode_parasite_max("asw_hordemode_parasite_max", "0", FCVAR_CHEAT, "Maximum parasites to spawn.");
ConVar asw_hordemode_parasite_min("asw_hordemode_parasite_min", "0", FCVAR_CHEAT, "Minimum parasites to spawn.");
ConVar asw_hordemode_parasite_health_max("asw_hordemode_parasite_health_max", "0", FCVAR_CHEAT, "Maximum parasite heatlh.");
ConVar asw_hordemode_parasite_health_min("asw_hordemode_parasite_health_min", "0", FCVAR_CHEAT, "Minimum paraiste health.");

//Shieldbugs

//No grubs FFS.

//Drone Jumpers

//Harvesters

//Defanged parasites

//Queen

//Boomers

//Rangers

//Mortars

//Shamen

//Uber Drones

//Beta Drones

//Beta Shieldbugs


CASW_Horde_Mode::CASW_Horde_Mode(void)
{
}

CASW_Horde_Mode::~CASW_Horde_Mode(void)
{
}

void CASW_Horde_Mode::Spawn()
{
	BaseClass::Spawn();
	InitAlienData();
	SetNextThink( gpGlobals->curtime ); // Think now
}

void CASW_Horde_Mode::Think()
{
	if (asw_hordemode.GetBool())
		SetHordeMode(true);
	else
		SetHordeMode(false);
}

void CASW_Horde_Mode::SetHordeMode(bool bEnabled)
{
	if (bEnabled)
	{
		asw_horde_override.SetValue(1);
	}
	else	//Hordemode disabled
	{
		asw_horde_override.SetValue(0);
	}
}

void CASW_Horde_Mode::InitAlienData()
{
	//Alien Classnames
	AlienInfoArray[DRONE_INDEX].pAlienClassName = "asw_drone";
	AlienInfoArray[BUZZER_INDEX].pAlienClassName = "asw_buzzer";
	AlienInfoArray[PARASITE_INDEX].pAlienClassName = "asw_parasite";
	AlienInfoArray[SHIELDBUG_INDEX].pAlienClassName = "asw_shieldbug";
	AlienInfoArray[GRUB_INDEX].pAlienClassName = "asw_grub";
	AlienInfoArray[JUMPER_INDEX].pAlienClassName = "asw_drone_jumper";
	AlienInfoArray[HARVESTER_INDEX].pAlienClassName = "asw_harvester";
	AlienInfoArray[PARASITE_DEFANGED_INDEX].pAlienClassName = "asw_parasite_defanged";
	AlienInfoArray[QUEEN_INDEX].pAlienClassName = "asw_queen";
	AlienInfoArray[BOOMER_INDEX].pAlienClassName = "asw_boomer";
	AlienInfoArray[RANGER_INDEX].pAlienClassName = "asw_ranger";
	AlienInfoArray[MORTAR_INDEX].pAlienClassName = "asw_mortar";
	AlienInfoArray[SHAMEN_INDEX].pAlienClassName = "asw_shamen";
	AlienInfoArray[UBER_INDEX].pAlienClassName = "asw_drone_uber";
	AlienInfoArray[BETA_DRONE_INDEX].pAlienClassName = "asw_drone";
	AlienInfoArray[BETA_SHIELDBUG_INDEX].pAlienClassName = "asw_shieldbug";

	//Alien Flags
	AlienInfoArray[DRONE_INDEX].flag = 1;
	AlienInfoArray[BUZZER_INDEX].flag = 2;
	AlienInfoArray[PARASITE_INDEX].flag = 4;
	AlienInfoArray[SHIELDBUG_INDEX].flag = 8;
	AlienInfoArray[GRUB_INDEX].flag = 16;
	AlienInfoArray[JUMPER_INDEX].flag = 32;
	AlienInfoArray[HARVESTER_INDEX].flag = 64;
	AlienInfoArray[PARASITE_DEFANGED_INDEX].flag = 128;
	AlienInfoArray[QUEEN_INDEX].flag = 256;
	AlienInfoArray[BOOMER_INDEX].flag = 512;
	AlienInfoArray[RANGER_INDEX].flag = 1024;
	AlienInfoArray[MORTAR_INDEX].flag = 2048;
	AlienInfoArray[SHAMEN_INDEX].flag = 4096;
	AlienInfoArray[UBER_INDEX].flag = 8192;
	AlienInfoArray[BETA_DRONE_INDEX].flag = 16384;
	AlienInfoArray[BETA_SHIELDBUG_INDEX].flag = 32768;

	//Beta Alien Flags
	AlienInfoArray[BETA_DRONE_INDEX].betaAlienConVar = ConVarRef("asw_new_drone");
	AlienInfoArray[BETA_SHIELDBUG_INDEX].betaAlienConVar = ConVarRef("asw_old_shieldbug");
	AlienInfoArray[BETA_SHIELDBUG_INDEX].betaAlienCvarReversed = true;

	//Default Alien Healths
	/*
	AlienInfoArray[DRONE_INDEX].defaultHealth = ConVarRef("asw_drone_health").GetInt();
	AlienInfoArray[BUZZER_INDEX].defaultHealth = 2;
	AlienInfoArray[PARASITE_INDEX].defaultHealth = 4;
	AlienInfoArray[SHIELDBUG_INDEX].defaultHealth = 8;
	AlienInfoArray[GRUB_INDEX].defaultHealth = 16;
	AlienInfoArray[JUMPER_INDEX].defaultHealth = 32;
	AlienInfoArray[HARVESTER_INDEX].defaultHealth = 64;
	AlienInfoArray[PARASITE_DEFANGED_INDEX].defaultHealth = 128;
	AlienInfoArray[QUEEN_INDEX].defaultHealth = 256;
	AlienInfoArray[BOOMER_INDEX].defaultHealth = 512;
	AlienInfoArray[RANGER_INDEX].defaultHealth = 1024;
	AlienInfoArray[MORTAR_INDEX].defaultHealth = 2048;
	AlienInfoArray[SHAMEN_INDEX].defaultHealth = 4096;
	AlienInfoArray[UBER_INDEX].defaultHealth = 8192;
	AlienInfoArray[BETA_DRONE_INDEX].defaultHealth = 16384;
	AlienInfoArray[BETA_SHIELDBUG_INDEX].defaultHealth = 32768;
	*/

	//Alien Maximum Healths

	//Alien Minimum Healths


}

int CASW_Horde_Mode::GetRandomValidAlien()
{
	int alienNum = 1;
	do 
	{
		alienNum = RandomInt(0, HIGHEST_INDEX - 1);
	}
	while (!(asw_hordemode_aliens.GetInt() & AlienInfoArray[alienNum].flag) || (AlienInfoArray[alienNum].max.GetInt() == 0));
	return 0;
}