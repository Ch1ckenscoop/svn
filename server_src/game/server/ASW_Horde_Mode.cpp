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


CASW_Horde_Mode::CASW_Horde_Mode(void)
{
}

CASW_Horde_Mode::~CASW_Horde_Mode(void)
{
}


/*struct ValidAlienClasses
{
	bool testAlien1;
};*/

void CASW_Horde_Mode::Spawn()
{
	BaseClass::Spawn();
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
	//ValidAlienClasses vClasses = GetValidAliens();
}

int CASW_Horde_Mode::GetRandomValidAlien()
{
	//do 
	//{
	//	AlienClassNames;
	//}
	//while (!(asw_hordemode_aliens.GetInt() & [number][flag]) || (alienData[number][max]==0));
	return 0;
	AlienInfoArray
}