#include "cbase.h"
#include "ASW_Horde_Mode.h"
#include "convar.h"
#include "asw_spawn_manager.h"

#include "memdbgon.h"

//LINK_ENTITY_TO_CLASS( asw_horde_mode, CASW_Horde_Mode );

//Ch1ckensCoop: External convars we need to have control over
extern ConVar asw_horde_class;
extern ConVar asw_horde_override;
extern ConVar asw_horde_size_min;
extern ConVar asw_horde_size_max;

//Ch1ckensCoop: Core hordemode settings.
ConVar asw_hordemode("asw_hordemode", "0", FCVAR_CHEAT | FCVAR_CCOOP, "Enables hordemode on the server.");
ConVar asw_hordemode_debug("asw_hordemode_debug", "0", FCVAR_CHEAT | FCVAR_CCOOP, "Show hordemode debug messages.");
ConVar asw_hordemode_update_mode("asw_hordemode_update_mode", "0", FCVAR_CHEAT | FCVAR_CCOOP, "0 - update after each horde spawn; >0 - Update every x seconds.");
ConVar asw_hordemode_mode("asw_hordemode_mode", "1", FCVAR_CHEAT | FCVAR_CCOOP, "0 = Health settings only; 1 = Binary + horde_size_max settings + health; 2 = Binary + horde_size_max settings");

//Ch1ckensCoop: Hordemode spawning settings
ConVar asw_hordemode_aliens("asw_hordemode_aliens", "8063", FCVAR_CHEAT | FCVAR_CCOOP, "Binary flag of allowed aliens. See asw_hordemode_aliens_list command.");

//Ch1ckensCoop: Hordemode horde_size_max settings and alien health settings

//Drones
ConVar asw_hordemode_drone_max("asw_hordemode_drone_max", "1", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum drones to spawn.");
ConVar asw_hordemode_drone_min("asw_hordemode_drone_min", "1", FCVAR_CHEAT | FCVAR_CCOOP, "Minimum drones to spawn.");
ConVar asw_hordemode_drone_health_max("asw_hordemode_drone_health_max", "50", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum drone health.");
ConVar asw_hordemode_drone_health_min("asw_hordemode_drone_health_min", "25", FCVAR_CHEAT | FCVAR_CCOOP, "Minimum drone health.");

//Buzzers
ConVar asw_hordemode_buzzer_max("asw_hordemode_buzzer_max", "1", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum buzzers to spawn.");
ConVar asw_hordemode_buzzer_min("asw_hordemode_buzzer_min", "1", FCVAR_CHEAT | FCVAR_CCOOP, "Minimum buzzers to spawn.");
ConVar asw_hordemode_buzzer_health_max("asw_hordemode_buzzer_health_max", "35", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum buzzer health.");
ConVar asw_hordemode_buzzer_health_min("asw_hordemode_buzzer_health_min", "10", FCVAR_CHEAT | FCVAR_CCOOP, "Minimum buzzer health.");

//Parasites
ConVar asw_hordemode_parasite_max("asw_hordemode_parasite_max", "1", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum parasites to spawn.");
ConVar asw_hordemode_parasite_min("asw_hordemode_parasite_min", "1", FCVAR_CHEAT | FCVAR_CCOOP, "Minimum parasites to spawn.");
ConVar asw_hordemode_parasite_health_max("asw_hordemode_parasite_health_max", "35", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum parasite heatlh.");
ConVar asw_hordemode_parasite_health_min("asw_hordemode_parasite_health_min", "20", FCVAR_CHEAT | FCVAR_CCOOP, "Minimum parasite health.");

//Shieldbugs
ConVar asw_hordemode_shieldbug_max("asw_hordemode_shieldbug_max", "1", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum shieldbugs to spawn.");
ConVar asw_hordemode_shieldbug_min("asw_hordemode_shieldbug_min", "1", FCVAR_CHEAT | FCVAR_CCOOP, "Minimum shieldbugs to spawn.");
ConVar asw_hordemode_shieldbug_health_max("asw_hordemode_shieldbug_health_max", "1500", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum shieldbug heatlh.");
ConVar asw_hordemode_shieldbug_health_min("asw_hordemode_shieldbug_health_min", "750", FCVAR_CHEAT | FCVAR_CCOOP, "Minimum shieldbug health.");

//No grubs FFS.

//Drone Jumpers
ConVar asw_hordemode_drone_jumper_max("asw_hordemode_drone_jumper_max", "1", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum drone jumpers to spawn.");
ConVar asw_hordemode_drone_jumper_min("asw_hordemode_drone_jumper_min", "1", FCVAR_CHEAT | FCVAR_CCOOP, "Minimum drone jumpers to spawn.");
ConVar asw_hordemode_drone_jumper_health_max("asw_hordemode_drone_jumper_health_max", "45", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum drone jumper heatlh.");
ConVar asw_hordemode_drone_jumper_health_min("asw_hordemode_drone_jumper_health_min", "30", FCVAR_CHEAT, "Minimum drone jumper health.");

//Harvesters
ConVar asw_hordemode_harvester_max("asw_hordemode_harvester_max", "1", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum harvesters to spawn.");
ConVar asw_hordemode_harvester_min("asw_hordemode_harvester_min", "1", FCVAR_CHEAT | FCVAR_CCOOP, "Minimum harvesters to spawn.");
ConVar asw_hordemode_harvester_health_max("asw_hordemode_harvester_health_max", "325", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum harvester heatlh.");
ConVar asw_hordemode_harvester_health_min("asw_hordemode_harvester_health_min", "150", FCVAR_CHEAT | FCVAR_CCOOP, "Minimum harvester health.");

//Defanged parasites
ConVar asw_hordemode_parasite_safe_max("asw_hordemode_parasite_safe_max", "1", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum defanged parasites to spawn.");
ConVar asw_hordemode_parasite_safe_min("asw_hordemode_parasite_safe_min", "1", FCVAR_CHEAT | FCVAR_CCOOP, "Minimum defanged parasites to spawn.");
ConVar asw_hordemode_parasite_safe_health_max("asw_hordemode_parasite_safe_health_max", "20", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum defanged parasite heatlh.");
ConVar asw_hordemode_parasite_safe_health_min("asw_hordemode_parasite_safe_health_min", "10", FCVAR_CHEAT | FCVAR_CCOOP, "Minimum defanged parasite health.");

//Queen
ConVar asw_hordemode_queen_max("asw_hordemode_queen_max", "0", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum queens to spawn.");
ConVar asw_hordemode_queen_min("asw_hordemode_queen_min", "0", FCVAR_CHEAT | FCVAR_CCOOP, "Minimum queens to spawn.");
ConVar asw_hordemode_queen_health_max("asw_hordemode_queen_health_max", "2500", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum queen heatlh.");
ConVar asw_hordemode_queen_health_min("asw_hordemode_queen_health_min", "1000", FCVAR_CHEAT | FCVAR_CCOOP, "Minimum queen health.");

//Boomers
ConVar asw_hordemode_boomer_max("asw_hordemode_boomer_max", "1", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum boomers to spawn.");
ConVar asw_hordemode_boomer_min("asw_hordemode_boomer_min", "1", FCVAR_CHEAT | FCVAR_CCOOP, "Minimum boomers to spawn.");
ConVar asw_hordemode_boomer_health_max("asw_hordemode_boomer_health_max", "900", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum boomer heatlh.");
ConVar asw_hordemode_boomer_health_min("asw_hordemode_boomer_health_min", "750", FCVAR_CHEAT | FCVAR_CCOOP, "Minimum boomer health.");

//Rangers
ConVar asw_hordemode_ranger_max("asw_hordemode_ranger_max", "1", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum rangers to spawn.");
ConVar asw_hordemode_ranger_min("asw_hordemode_ranger_min", "1", FCVAR_CHEAT | FCVAR_CCOOP, "Minimum rangers to spawn.");
ConVar asw_hordemode_ranger_health_max("asw_hordemode_ranger_health_max", "125", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum ranger heatlh.");
ConVar asw_hordemode_ranger_health_min("asw_hordemode_ranger_health_min", "75", FCVAR_CHEAT | FCVAR_CCOOP, "Minimum ranger health.");

//Mortars
ConVar asw_hordemode_mortar_max("asw_hordemode_mortar_max", "1", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum mortars to spawn.");
ConVar asw_hordemode_mortar_min("asw_hordemode_mortar_min", "1", FCVAR_CHEAT | FCVAR_CCOOP, "Minimum mortars to spawn.");
ConVar asw_hordemode_mortar_health_max("asw_hordemode_mortar_health_max", "425", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum mortar heatlh.");
ConVar asw_hordemode_mortar_health_min("asw_hordemode_mortar_health_min", "300", FCVAR_CHEAT, "Minimum mortar health.");

//Shamen
ConVar asw_hordemode_shaman_max("asw_hordemode_shaman_max", "1", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum shamans to spawn.");
ConVar asw_hordemode_shaman_min("asw_hordemode_shaman_min", "1", FCVAR_CHEAT | FCVAR_CCOOP, "Minimum shamans to spawn.");
ConVar asw_hordemode_shaman_health_max("asw_hordemode_shaman_health_max", "80", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum shaman heatlh.");
ConVar asw_hordemode_shaman_health_min("asw_hordemode_shaman_health_min", "45", FCVAR_CHEAT | FCVAR_CCOOP, "Minimum shaman health.");

//Uber Drones
ConVar asw_hordemode_drone_uber_max("asw_hordemode_drone_uber_max", "1", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum uber drones to spawn.");
ConVar asw_hordemode_drone_uber_min("asw_hordemode_drone_uber_min", "1", FCVAR_CHEAT | FCVAR_CCOOP, "Minimum uber drones to spawn.");
ConVar asw_hordemode_drone_uber_health_max("asw_hordemode_drone_uber_health_max", "350", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum uber drone heatlh.");
ConVar asw_hordemode_drone_uber_health_min("asw_hordemode_drone_uber_health_min", "275", FCVAR_CHEAT | FCVAR_CCOOP, "Minimum uber drone health.");

//Beta Drones
ConVar asw_hordemode_drone_beta_max("asw_hordemode_drone_beta_max", "1", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum beta drones to spawn.");
ConVar asw_hordemode_drone_beta_min("asw_hordemode_drone_beta_min", "1", FCVAR_CHEAT | FCVAR_CCOOP, "Minimum beta drones to spawn.");
ConVar asw_hordemode_drone_beta_health_max("asw_hordemode_drone_beta_health_max", "50", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum beta drone heatlh.");
ConVar asw_hordemode_drone_beta_health_min("asw_hordemode_drone_beta_health_min", "25", FCVAR_CHEAT | FCVAR_CCOOP, "Minimum beta drone health.");

//Beta Shieldbugs
ConVar asw_hordemode_shieldbug_beta_max("asw_hordemode_shieldbug_beta_max", "0", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum beta shieldbugs to spawn.");
ConVar asw_hordemode_shieldbug_beta_min("asw_hordemode_shieldbug_beta_min", "0", FCVAR_CHEAT | FCVAR_CCOOP, "Minimum beta shieldbugs to spawn.");
ConVar asw_hordemode_shieldbug_beta_health_max("asw_hordemode_shieldbug_beta_health_max", "1500", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum beta shieldbug heatlh.");
ConVar asw_hordemode_shieldbug_beta_health_min("asw_hordemode_shieldbug_beta_health_min", "750", FCVAR_CHEAT | FCVAR_CCOOP, "Minimum beta shieldbug health.");

//Beta Harvesters
ConVar asw_hordemode_harvester_beta_max("asw_hordemode_harvester_beta_max", "1", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum beta harvesters to spawn.");
ConVar asw_hordemode_harvester_beta_min("asw_hordemode_harvester_beta_min", "1", FCVAR_CHEAT | FCVAR_CCOOP, "Minimum beta harvesters to spawn.");
ConVar asw_hordemode_harvester_beta_health_max("asw_hordemode_harvester_beta_health_max", "325", FCVAR_CHEAT | FCVAR_CCOOP, "Maximum beta harvester heatlh.");
ConVar asw_hordemode_harvester_beta_health_min("asw_hordemode_harvester_beta_health_min", "150", FCVAR_CHEAT | FCVAR_CCOOP, "Minimum beta harvester health.");

//softcopy: hordemode horde_size_max settings and beta aliens health settings
//Beta Parasites   
ConVar asw_hordemode_parasite_beta_max("asw_hordemode_parasite_beta_max", "1", FCVAR_CHEAT, "Maximum beta parasite to spawn.");
ConVar asw_hordemode_parasite_beta_min("asw_hordemode_parasite_beta_min", "1", FCVAR_CHEAT, "Minimum beta parasite  to spawn.");
ConVar asw_hordemode_parasite_beta_health_max("asw_hordemode_parasite_beta_health_max", "35", FCVAR_CHEAT, "Maximum beta parasite  heatlh.");
ConVar asw_hordemode_parasite_beta_health_min("asw_hordemode_parasite_beta_health_min", "20", FCVAR_CHEAT, "Minimum beta parasite  health.");
//Beta Buzzers    
ConVar asw_hordemode_buzzer_beta_max("asw_hordemode_buzzer_beta_max", "1", FCVAR_CHEAT, "Maximum beta buzzer to spawn.");
ConVar asw_hordemode_buzzer_beta_min("asw_hordemode_buzzer_beta_min", "1", FCVAR_CHEAT, "Minimum beta buzzer  to spawn.");
ConVar asw_hordemode_buzzer_beta_health_max("asw_hordemode_buzzer_beta_health_max", "120", FCVAR_CHEAT, "Maximum beta buzzer  heatlh.");
ConVar asw_hordemode_buzzer_beta_health_min("asw_hordemode_buzzer_beta_health_min", "60", FCVAR_CHEAT, "Minimum beta buzzer  health.");
//Beta Mortars    
ConVar asw_hordemode_mortar_beta_max("asw_hordemode_mortar_beta_max", "1", FCVAR_CHEAT, "Maximum beta mortar to spawn.");
ConVar asw_hordemode_mortar_beta_min("asw_hordemode_mortar_beta_min", "1", FCVAR_CHEAT, "Minimum beta mortar  to spawn.");
ConVar asw_hordemode_mortar_beta_health_max("asw_hordemode_mortar_beta_health_max", "425", FCVAR_CHEAT, "Maximum beta mortar  heatlh.");
ConVar asw_hordemode_mortar_beta_health_min("asw_hordemode_mortar_beta_health_min", "300", FCVAR_CHEAT, "Minimum beta mortar  health.");

static CASW_Horde_Mode g_ASWHordeMode;
CASW_Horde_Mode* ASWHordeMode() 
{
	return &g_ASWHordeMode;
}

CASW_Horde_Mode::CASW_Horde_Mode()
{

}

CASW_Horde_Mode::~CASW_Horde_Mode()
{

}

void CASW_Horde_Mode::LevelInitPostEntity()
{
	//Init();
}

bool CASW_Horde_Mode::Init()
{
	//BaseClass::Spawn();

	m_iLastAlienClass = DRONE_INDEX;

	InitAlienData();
	//UpdateHordeMode();
	m_flLastThinkTime = gpGlobals->curtime;

	return true;
}

void CASW_Horde_Mode::HordeFinishedSpawning()
{
	//Ch1ckensCoop: For now, all we have to do is think.
	if (asw_hordemode_debug.GetBool())
		Msg("Horde finished... updating horde mode!\n");

	if (asw_hordemode_update_mode.GetFloat() == 0)
	{
		if (asw_hordemode_mode.GetInt() == 1)
			UpdateHordeMode();
		else if (asw_hordemode_mode.GetInt() == 0)
			RandomizeHealth();
	}
}

void CASW_Horde_Mode::FrameUpdatePostEntityThink()
{
	if (!asw_hordemode.GetBool())
		return;

	asw_horde_override.SetValue(1);

	float thinkRate = asw_hordemode_update_mode.GetFloat();
	if (thinkRate > 0 && m_flLastThinkTime <= gpGlobals->curtime)	//Check if we're actually supposed to be updating on a timed basis.
	{
		UpdateHordeMode();
		m_flLastThinkTime = gpGlobals->curtime + thinkRate;
	}
}

void CASW_Horde_Mode::UpdateHordeMode()
{	
	if (asw_hordemode_debug.GetBool())
		Msg("Updating hordemode!\n");

	//Ch1ckensCoop: Turn off beta if it was on for the previous alien.
	if (m_AlienInfoArray[m_iLastAlienClass].m_bBeta)
	{
		if (!m_AlienInfoArray[m_iLastAlienClass].m_pBetaAlienCvar)
			Warning("Bad beta convar for alien class %s!\n", m_AlienInfoArray[m_iLastAlienClass].m_szAlienClassName);
		//softcopy: cvar 2 = random cvar 1 or 2, no reverse for cvar 2 has set
		if (m_AlienInfoArray[m_iLastAlienClass].m_pBetaAlienCvar->GetFloat() != 2)
		{
			if (!m_AlienInfoArray[m_iLastAlienClass].m_bBetaAlienCvarReversed)
				m_AlienInfoArray[m_iLastAlienClass].m_pBetaAlienCvar->SetValue(1);
			else
				m_AlienInfoArray[m_iLastAlienClass].m_pBetaAlienCvar->SetValue(0);

			if (asw_hordemode_debug.GetBool())
				Msg("Disabled beta spawning for %s.\n", m_AlienInfoArray[m_iLastAlienClass].m_szAlienClassName);
		}
	}

	//Ch1ckensCoop: Select the next random alien
	int alienIndex = GetRandomValidAlien();

	if (asw_hordemode_debug.GetBool())
		Msg("Alien index is %i.\n", alienIndex);

	const char *alienClassName = m_AlienInfoArray[alienIndex].m_szAlienClassName;
	ConVar *alienMax = m_AlienInfoArray[alienIndex].m_pMaxCvar;
	ConVar *alienMin = m_AlienInfoArray[alienIndex].m_pMinCvar;
	ConVar *alienHealthCvar = m_AlienInfoArray[alienIndex].m_pAlienHealthCvar;
	ConVar *alienBetaCvar = m_AlienInfoArray[alienIndex].m_pBetaAlienCvar;
	bool alienBetaCvarReversed = m_AlienInfoArray[alienIndex].m_bBetaAlienCvarReversed;
	bool alienIsBeta = m_AlienInfoArray[alienIndex].m_bBeta;


	asw_horde_size_max.SetValue(alienMax->GetInt());
	asw_horde_size_min.SetValue(alienMin->GetInt());

	int mode = asw_hordemode_mode.GetInt();
	if (mode == 0 || mode == 1)
		RandomizeHealth();
	//softcopy: cvar 2 = random cvar 1 or 2, no reverse for cvar 2 has set
	//if (alienIsBeta && alienBetaCvar)
	if (alienIsBeta && alienBetaCvar && m_AlienInfoArray[alienIndex].m_pBetaAlienCvar->GetFloat() != 2)
	{
		if (!alienBetaCvarReversed)
			alienBetaCvar->SetValue(0);
		else
			alienBetaCvar->SetValue(1);
	}
	//else if (!alienIsBeta && alienBetaCvar)
	else if (!alienIsBeta && alienBetaCvar && m_AlienInfoArray[alienIndex].m_pBetaAlienCvar->GetFloat() != 2)
	{
		if (!alienBetaCvarReversed)
			alienBetaCvar->SetValue(1);
		else
			alienBetaCvar->SetValue(0);
	}

	//Finally, set the classname and we're ready to go!
	asw_horde_class.SetValue(alienClassName);
	if (asw_hordemode_debug.GetBool())
	{
		const char *boolText;
		if (alienIsBeta)
			boolText = "true";
		else
			boolText = "false";

		engine->Con_NPrintf( 19, "Hordemode set alien class to %s, health to %i, isBeta = %s.\n", alienClassName, alienHealthCvar->GetInt(), boolText);
		Msg("Hordemode set alien class to %s, health to %i, isBeta = %s.\n", alienClassName, alienHealthCvar->GetInt(), boolText);
	}
}

int CASW_Horde_Mode::GetRandomValidAlien()
{
	int alienNum = 1;
	int allowedAliens = asw_hordemode_aliens.GetInt();
	int alienMax = 0;
	do 
	{
		alienNum = RandomInt(0, ALIEN_INDEX_COUNT - 1);

		if (asw_hordemode_debug.GetInt() == 2)
			Msg("Hordemode: alienNum = %i\n", alienNum);

		alienMax = m_AlienInfoArray[alienNum].m_pMaxCvar->GetInt();
	}
	while ((allowedAliens & m_AlienInfoArray[alienNum].m_iFlag) == 0 || (alienMax == 0));
	m_iLastAlienClass = alienNum;
	return alienNum;
}

void CASW_Horde_Mode::RandomizeHealth(int alienNum)
{
	if (!(FStrEq(m_AlienInfoArray[alienNum].m_szAlienClassName, "asw_queen") && m_AlienInfoArray[alienNum].m_pMaxCvar->GetInt() == 0))
	{
		m_AlienInfoArray[alienNum].m_pAlienHealthCvar->SetValue(
			RandomInt(
			m_AlienInfoArray[alienNum].m_pHealthMinCvar->GetInt(), 
			m_AlienInfoArray[alienNum].m_pHealthMaxCvar->GetInt())
			);
	}
}

void CASW_Horde_Mode::RandomizeHealth()
{
	for (int i = 0; i < ALIEN_INDEX_COUNT; i++)
	{
		// Is one or more of our convars null?
		if (!m_AlienInfoArray[i].m_pAlienHealthCvar ||
			!m_AlienInfoArray[i].m_pHealthMinCvar ||
			!m_AlienInfoArray[i].m_pHealthMaxCvar)
		{
			Msg("Alien %s has one or more null cvars!\n", m_AlienInfoArray[i].m_szAlienClassName);
		}

		m_AlienInfoArray[i].m_pAlienHealthCvar->SetValue(
			RandomInt(
			m_AlienInfoArray[i].m_pHealthMinCvar->GetInt(), 
			m_AlienInfoArray[i].m_pHealthMaxCvar->GetInt())
			);
	}
}

void CASW_Horde_Mode::InitAlienData()
{
	// Alien Classnames
	m_AlienInfoArray[DRONE_INDEX].m_szAlienClassName = "asw_drone";
	m_AlienInfoArray[BUZZER_INDEX].m_szAlienClassName = "asw_buzzer";
	m_AlienInfoArray[PARASITE_INDEX].m_szAlienClassName = "asw_parasite";
	m_AlienInfoArray[SHIELDBUG_INDEX].m_szAlienClassName = "asw_shieldbug";
	m_AlienInfoArray[JUMPER_INDEX].m_szAlienClassName = "asw_drone_jumper";
	m_AlienInfoArray[HARVESTER_INDEX].m_szAlienClassName = "asw_harvester";
	m_AlienInfoArray[PARASITE_DEFANGED_INDEX].m_szAlienClassName = "asw_parasite_defanged";
	m_AlienInfoArray[QUEEN_INDEX].m_szAlienClassName = "asw_queen";
	m_AlienInfoArray[BOOMER_INDEX].m_szAlienClassName = "asw_boomer";
	m_AlienInfoArray[RANGER_INDEX].m_szAlienClassName = "asw_ranger";
	m_AlienInfoArray[MORTAR_INDEX].m_szAlienClassName = "asw_mortarbug";
	m_AlienInfoArray[SHAMAN_INDEX].m_szAlienClassName = "asw_shaman";
	m_AlienInfoArray[UBER_INDEX].m_szAlienClassName = "asw_drone_uber";
	m_AlienInfoArray[BETA_DRONE_INDEX].m_szAlienClassName = "asw_drone";
	m_AlienInfoArray[BETA_SHIELDBUG_INDEX].m_szAlienClassName = "asw_shieldbug";
	m_AlienInfoArray[BETA_HARVESTER_INDEX].m_szAlienClassName = "asw_harvester";
	//softcopy: 
	m_AlienInfoArray[BETA_BUZZER_INDEX].m_szAlienClassName = "asw_buzzer";
	m_AlienInfoArray[BETA_PARASITE_INDEX].m_szAlienClassName = "asw_parasite";
	m_AlienInfoArray[BETA_MORTAR_INDEX].m_szAlienClassName = "asw_mortarbug";


	// Alien Flags
	m_AlienInfoArray[DRONE_INDEX].m_iFlag = 1;
	m_AlienInfoArray[BUZZER_INDEX].m_iFlag = 2;
	m_AlienInfoArray[PARASITE_INDEX].m_iFlag = 4;
	m_AlienInfoArray[SHIELDBUG_INDEX].m_iFlag = 8;
	m_AlienInfoArray[JUMPER_INDEX].m_iFlag = 16;
	m_AlienInfoArray[HARVESTER_INDEX].m_iFlag = 32;
	m_AlienInfoArray[PARASITE_DEFANGED_INDEX].m_iFlag = 64;
	m_AlienInfoArray[QUEEN_INDEX].m_iFlag = 128;
	m_AlienInfoArray[BOOMER_INDEX].m_iFlag = 256;
	m_AlienInfoArray[RANGER_INDEX].m_iFlag = 512;
	m_AlienInfoArray[MORTAR_INDEX].m_iFlag = 1024;
	m_AlienInfoArray[SHAMAN_INDEX].m_iFlag = 2048;
	m_AlienInfoArray[UBER_INDEX].m_iFlag = 4096;
	m_AlienInfoArray[BETA_DRONE_INDEX].m_iFlag = 8192;
	m_AlienInfoArray[BETA_SHIELDBUG_INDEX].m_iFlag = 16384;
	m_AlienInfoArray[BETA_HARVESTER_INDEX].m_iFlag = 32768;
	//softcopy: beta alien Flags
	m_AlienInfoArray[BETA_BUZZER_INDEX].m_iFlag = 65536;
	m_AlienInfoArray[BETA_PARASITE_INDEX].m_iFlag = 131072;
	m_AlienInfoArray[BETA_MORTAR_INDEX].m_iFlag = 262144;


	// Beta Alien Flags
	m_AlienInfoArray[DRONE_INDEX].m_pBetaAlienCvar = g_pCVar->FindVar("asw_new_drone");
	//softcopy:
	m_AlienInfoArray[BUZZER_INDEX].m_pBetaAlienCvar = g_pCVar->FindVar("asw_old_buzzer");
	m_AlienInfoArray[BUZZER_INDEX].m_bBetaAlienCvarReversed = true;
	m_AlienInfoArray[PARASITE_INDEX].m_pBetaAlienCvar = g_pCVar->FindVar("asw_old_parasite");
	m_AlienInfoArray[PARASITE_INDEX].m_bBetaAlienCvarReversed = true;
	
	m_AlienInfoArray[SHIELDBUG_INDEX].m_pBetaAlienCvar = g_pCVar->FindVar("asw_old_shieldbug");
	m_AlienInfoArray[SHIELDBUG_INDEX].m_bBetaAlienCvarReversed = true;

	m_AlienInfoArray[HARVESTER_INDEX].m_pBetaAlienCvar = g_pCVar->FindVar("asw_harvester_new");
	//softcopy:
	m_AlienInfoArray[MORTAR_INDEX].m_pBetaAlienCvar = g_pCVar->FindVar("asw_old_mortarbug");
	m_AlienInfoArray[MORTAR_INDEX].m_bBetaAlienCvarReversed = true;

	m_AlienInfoArray[BETA_DRONE_INDEX].m_pBetaAlienCvar = g_pCVar->FindVar("asw_new_drone");
	m_AlienInfoArray[BETA_DRONE_INDEX].m_bBeta = true;

	m_AlienInfoArray[BETA_SHIELDBUG_INDEX].m_pBetaAlienCvar = g_pCVar->FindVar("asw_old_shieldbug");
	m_AlienInfoArray[BETA_SHIELDBUG_INDEX].m_bBetaAlienCvarReversed = true;
	m_AlienInfoArray[BETA_SHIELDBUG_INDEX].m_bBeta = true;

	m_AlienInfoArray[BETA_HARVESTER_INDEX].m_pBetaAlienCvar = g_pCVar->FindVar("asw_harvester_new");
	m_AlienInfoArray[BETA_HARVESTER_INDEX].m_bBeta = true;
	//softcopy:
	m_AlienInfoArray[BETA_BUZZER_INDEX].m_pBetaAlienCvar = g_pCVar->FindVar("asw_old_buzzer");
	m_AlienInfoArray[BETA_BUZZER_INDEX].m_bBetaAlienCvarReversed = true;
	m_AlienInfoArray[BETA_BUZZER_INDEX].m_bBeta = true;
	m_AlienInfoArray[BETA_PARASITE_INDEX].m_pBetaAlienCvar = g_pCVar->FindVar("asw_old_parasite");
	m_AlienInfoArray[BETA_PARASITE_INDEX].m_bBetaAlienCvarReversed = true;
	m_AlienInfoArray[BETA_PARASITE_INDEX].m_bBeta = true;
	m_AlienInfoArray[BETA_MORTAR_INDEX].m_pBetaAlienCvar = g_pCVar->FindVar("asw_old_mortarbug");
	m_AlienInfoArray[BETA_MORTAR_INDEX].m_bBetaAlienCvarReversed = true;
	m_AlienInfoArray[BETA_MORTAR_INDEX].m_bBeta = true;
	
	
	// Non beta alien flags
	m_AlienInfoArray[DRONE_INDEX].m_bBeta = false;
	m_AlienInfoArray[BUZZER_INDEX].m_bBeta = false;
	m_AlienInfoArray[PARASITE_INDEX].m_bBeta = false;
	m_AlienInfoArray[SHIELDBUG_INDEX].m_bBeta = false;
	m_AlienInfoArray[JUMPER_INDEX].m_bBeta = false;
	m_AlienInfoArray[HARVESTER_INDEX].m_bBeta = false;
	m_AlienInfoArray[PARASITE_DEFANGED_INDEX].m_bBeta = false;
	m_AlienInfoArray[QUEEN_INDEX].m_bBeta = false;
	m_AlienInfoArray[BOOMER_INDEX].m_bBeta = false;
	m_AlienInfoArray[RANGER_INDEX].m_bBeta = false;
	m_AlienInfoArray[MORTAR_INDEX].m_bBeta = false;
	m_AlienInfoArray[SHAMAN_INDEX].m_bBeta = false;
	m_AlienInfoArray[UBER_INDEX].m_bBeta = false;


	// Alien Maximum Healths
	m_AlienInfoArray[DRONE_INDEX].m_pHealthMaxCvar = g_pCVar->FindVar("asw_hordemode_drone_health_max");
	m_AlienInfoArray[BUZZER_INDEX].m_pHealthMaxCvar = g_pCVar->FindVar("asw_hordemode_buzzer_health_max");
	m_AlienInfoArray[PARASITE_INDEX].m_pHealthMaxCvar = g_pCVar->FindVar("asw_hordemode_parasite_health_max");
	m_AlienInfoArray[SHIELDBUG_INDEX].m_pHealthMaxCvar = g_pCVar->FindVar("asw_hordemode_shieldbug_health_max");
	m_AlienInfoArray[JUMPER_INDEX].m_pHealthMaxCvar = g_pCVar->FindVar("asw_hordemode_drone_jumper_health_max");
	m_AlienInfoArray[HARVESTER_INDEX].m_pHealthMaxCvar = g_pCVar->FindVar("asw_hordemode_harvester_health_max");
	m_AlienInfoArray[PARASITE_DEFANGED_INDEX].m_pHealthMaxCvar = g_pCVar->FindVar("asw_hordemode_parasite_safe_health_max");
	m_AlienInfoArray[QUEEN_INDEX].m_pHealthMaxCvar = g_pCVar->FindVar("asw_hordemode_queen_health_max");
	m_AlienInfoArray[BOOMER_INDEX].m_pHealthMaxCvar = g_pCVar->FindVar("asw_hordemode_boomer_health_max");
	m_AlienInfoArray[RANGER_INDEX].m_pHealthMaxCvar = g_pCVar->FindVar("asw_hordemode_ranger_health_max");
	m_AlienInfoArray[MORTAR_INDEX].m_pHealthMaxCvar = g_pCVar->FindVar("asw_hordemode_mortar_health_max");
	m_AlienInfoArray[SHAMAN_INDEX].m_pHealthMaxCvar = g_pCVar->FindVar("asw_hordemode_shaman_health_max");
	m_AlienInfoArray[UBER_INDEX].m_pHealthMaxCvar = g_pCVar->FindVar("asw_hordemode_drone_uber_health_max");
	m_AlienInfoArray[BETA_DRONE_INDEX].m_pHealthMaxCvar = g_pCVar->FindVar("asw_hordemode_drone_beta_health_max");
	m_AlienInfoArray[BETA_SHIELDBUG_INDEX].m_pHealthMaxCvar = g_pCVar->FindVar("asw_hordemode_shieldbug_beta_health_max");
	m_AlienInfoArray[BETA_HARVESTER_INDEX].m_pHealthMaxCvar = g_pCVar->FindVar("asw_hordemode_harvester_beta_health_max");
	//softcopy:
	m_AlienInfoArray[BETA_BUZZER_INDEX].m_pHealthMaxCvar = g_pCVar->FindVar("asw_hordemode_buzzer_beta_health_max");
	m_AlienInfoArray[BETA_PARASITE_INDEX].m_pHealthMaxCvar = g_pCVar->FindVar("asw_hordemode_parasite_beta_health_max");
	m_AlienInfoArray[BETA_MORTAR_INDEX].m_pHealthMaxCvar = g_pCVar->FindVar("asw_hordemode_mortar_beta_health_max");


	// Alien Minimum Healths
	m_AlienInfoArray[DRONE_INDEX].m_pHealthMinCvar = g_pCVar->FindVar("asw_hordemode_drone_health_min");
	m_AlienInfoArray[BUZZER_INDEX].m_pHealthMinCvar = g_pCVar->FindVar("asw_hordemode_buzzer_health_min");
	m_AlienInfoArray[PARASITE_INDEX].m_pHealthMinCvar = g_pCVar->FindVar("asw_hordemode_parasite_health_min");
	m_AlienInfoArray[SHIELDBUG_INDEX].m_pHealthMinCvar = g_pCVar->FindVar("asw_hordemode_shieldbug_health_min");
	m_AlienInfoArray[JUMPER_INDEX].m_pHealthMinCvar = g_pCVar->FindVar("asw_hordemode_drone_jumper_health_min");
	m_AlienInfoArray[HARVESTER_INDEX].m_pHealthMinCvar = g_pCVar->FindVar("asw_hordemode_harvester_health_min");
	m_AlienInfoArray[PARASITE_DEFANGED_INDEX].m_pHealthMinCvar = g_pCVar->FindVar("asw_hordemode_parasite_safe_health_min");
	m_AlienInfoArray[QUEEN_INDEX].m_pHealthMinCvar = g_pCVar->FindVar("asw_hordemode_queen_health_min");
	m_AlienInfoArray[BOOMER_INDEX].m_pHealthMinCvar = g_pCVar->FindVar("asw_hordemode_boomer_health_min");
	m_AlienInfoArray[RANGER_INDEX].m_pHealthMinCvar = g_pCVar->FindVar("asw_hordemode_ranger_health_min");
	m_AlienInfoArray[MORTAR_INDEX].m_pHealthMinCvar = g_pCVar->FindVar("asw_hordemode_mortar_health_min");
	m_AlienInfoArray[SHAMAN_INDEX].m_pHealthMinCvar = g_pCVar->FindVar("asw_hordemode_shaman_health_min");
	m_AlienInfoArray[UBER_INDEX].m_pHealthMinCvar = g_pCVar->FindVar("asw_hordemode_drone_uber_health_min");
	m_AlienInfoArray[BETA_DRONE_INDEX].m_pHealthMinCvar = g_pCVar->FindVar("asw_hordemode_drone_beta_health_max");
	m_AlienInfoArray[BETA_SHIELDBUG_INDEX].m_pHealthMinCvar = g_pCVar->FindVar("asw_hordemode_shieldbug_beta_health_min");
	m_AlienInfoArray[BETA_HARVESTER_INDEX].m_pHealthMinCvar = g_pCVar->FindVar("asw_hordemode_harvester_beta_health_min");
	//softcopy:
	m_AlienInfoArray[BETA_BUZZER_INDEX].m_pHealthMinCvar = g_pCVar->FindVar("asw_hordemode_buzzer_beta_health_min");
	m_AlienInfoArray[BETA_PARASITE_INDEX].m_pHealthMinCvar = g_pCVar->FindVar("asw_hordemode_parasite_beta_health_min");
	m_AlienInfoArray[BETA_MORTAR_INDEX].m_pHealthMinCvar = g_pCVar->FindVar("asw_hordemode_mortar_beta_health_min");


	// Alien Maximums
	m_AlienInfoArray[DRONE_INDEX].m_pMaxCvar = g_pCVar->FindVar("asw_hordemode_drone_max");
	m_AlienInfoArray[BUZZER_INDEX].m_pMaxCvar = g_pCVar->FindVar("asw_hordemode_buzzer_max");
	m_AlienInfoArray[PARASITE_INDEX].m_pMaxCvar = g_pCVar->FindVar("asw_hordemode_parasite_max");
	m_AlienInfoArray[SHIELDBUG_INDEX].m_pMaxCvar = g_pCVar->FindVar("asw_hordemode_shieldbug_max");
	m_AlienInfoArray[JUMPER_INDEX].m_pMaxCvar = g_pCVar->FindVar("asw_hordemode_drone_jumper_max");
	m_AlienInfoArray[HARVESTER_INDEX].m_pMaxCvar = g_pCVar->FindVar("asw_hordemode_harvester_max");
	m_AlienInfoArray[PARASITE_DEFANGED_INDEX].m_pMaxCvar = g_pCVar->FindVar("asw_hordemode_parasite_safe_max");
	m_AlienInfoArray[QUEEN_INDEX].m_pMaxCvar = g_pCVar->FindVar("asw_hordemode_queen_max");
	m_AlienInfoArray[BOOMER_INDEX].m_pMaxCvar = g_pCVar->FindVar("asw_hordemode_boomer_max");
	m_AlienInfoArray[RANGER_INDEX].m_pMaxCvar = g_pCVar->FindVar("asw_hordemode_ranger_max");
	m_AlienInfoArray[MORTAR_INDEX].m_pMaxCvar = g_pCVar->FindVar("asw_hordemode_mortar_max");
	m_AlienInfoArray[SHAMAN_INDEX].m_pMaxCvar = g_pCVar->FindVar("asw_hordemode_shaman_max");
	m_AlienInfoArray[UBER_INDEX].m_pMaxCvar = g_pCVar->FindVar("asw_hordemode_drone_uber_max");
	m_AlienInfoArray[BETA_DRONE_INDEX].m_pMaxCvar = g_pCVar->FindVar("asw_hordemode_drone_beta_max");
	m_AlienInfoArray[BETA_SHIELDBUG_INDEX].m_pMaxCvar = g_pCVar->FindVar("asw_hordemode_shieldbug_beta_max");
	m_AlienInfoArray[BETA_HARVESTER_INDEX].m_pMaxCvar = g_pCVar->FindVar("asw_hordemode_harvester_beta_max");
	//softcopy:
	m_AlienInfoArray[BETA_BUZZER_INDEX].m_pMaxCvar = g_pCVar->FindVar("asw_hordemode_buzzer_beta_max");
	m_AlienInfoArray[BETA_PARASITE_INDEX].m_pMaxCvar = g_pCVar->FindVar("asw_hordemode_parasite_beta_max");
	m_AlienInfoArray[BETA_MORTAR_INDEX].m_pMaxCvar = g_pCVar->FindVar("asw_hordemode_mortar_beta_max");


	// Alien Minimums
	m_AlienInfoArray[DRONE_INDEX].m_pMinCvar = g_pCVar->FindVar("asw_hordemode_drone_min");
	m_AlienInfoArray[BUZZER_INDEX].m_pMinCvar = g_pCVar->FindVar("asw_hordemode_buzzer_min");
	m_AlienInfoArray[PARASITE_INDEX].m_pMinCvar = g_pCVar->FindVar("asw_hordemode_parasite_min");
	m_AlienInfoArray[SHIELDBUG_INDEX].m_pMinCvar = g_pCVar->FindVar("asw_hordemode_shieldbug_min");
	m_AlienInfoArray[JUMPER_INDEX].m_pMinCvar = g_pCVar->FindVar("asw_hordemode_drone_jumper_min");
	m_AlienInfoArray[HARVESTER_INDEX].m_pMinCvar = g_pCVar->FindVar("asw_hordemode_harvester_min");
	m_AlienInfoArray[PARASITE_DEFANGED_INDEX].m_pMinCvar = g_pCVar->FindVar("asw_hordemode_parasite_safe_min");
	m_AlienInfoArray[QUEEN_INDEX].m_pMinCvar = g_pCVar->FindVar("asw_hordemode_queen_min");
	m_AlienInfoArray[BOOMER_INDEX].m_pMinCvar = g_pCVar->FindVar("asw_hordemode_boomer_min");
	m_AlienInfoArray[RANGER_INDEX].m_pMinCvar = g_pCVar->FindVar("asw_hordemode_ranger_min");
	m_AlienInfoArray[MORTAR_INDEX].m_pMinCvar = g_pCVar->FindVar("asw_hordemode_mortar_min");
	m_AlienInfoArray[SHAMAN_INDEX].m_pMinCvar = g_pCVar->FindVar("asw_hordemode_shaman_min");
	m_AlienInfoArray[UBER_INDEX].m_pMinCvar = g_pCVar->FindVar("asw_hordemode_drone_uber_min");
	m_AlienInfoArray[BETA_DRONE_INDEX].m_pMinCvar = g_pCVar->FindVar("asw_hordemode_drone_beta_min");
	m_AlienInfoArray[BETA_SHIELDBUG_INDEX].m_pMinCvar = g_pCVar->FindVar("asw_hordemode_shieldbug_beta_min");
	m_AlienInfoArray[BETA_HARVESTER_INDEX].m_pMinCvar = g_pCVar->FindVar("asw_hordemode_harvester_beta_min");
	//softcopy:
	m_AlienInfoArray[BETA_BUZZER_INDEX].m_pMinCvar = g_pCVar->FindVar("asw_hordemode_buzzer_beta_min");
	m_AlienInfoArray[BETA_PARASITE_INDEX].m_pMinCvar = g_pCVar->FindVar("asw_hordemode_parasite_beta_min");
	m_AlienInfoArray[BETA_MORTAR_INDEX].m_pMinCvar = g_pCVar->FindVar("asw_hordemode_mortar_beta_min");


	// Alien Health Cvar ConVarRefs
	m_AlienInfoArray[DRONE_INDEX].m_pAlienHealthCvar = g_pCVar->FindVar("asw_drone_health");
	m_AlienInfoArray[BUZZER_INDEX].m_pAlienHealthCvar = g_pCVar->FindVar("sk_asw_buzzer_health");
	m_AlienInfoArray[PARASITE_INDEX].m_pAlienHealthCvar = g_pCVar->FindVar("asw_parasite_health");
	m_AlienInfoArray[SHIELDBUG_INDEX].m_pAlienHealthCvar = g_pCVar->FindVar("asw_shieldbug_health");
	m_AlienInfoArray[JUMPER_INDEX].m_pAlienHealthCvar = g_pCVar->FindVar("asw_drone_jumper_health");
	m_AlienInfoArray[HARVESTER_INDEX].m_pAlienHealthCvar = g_pCVar->FindVar("asw_harvester_health");
	m_AlienInfoArray[PARASITE_DEFANGED_INDEX].m_pAlienHealthCvar = g_pCVar->FindVar("asw_parasite_defanged_health");
	m_AlienInfoArray[QUEEN_INDEX].m_pAlienHealthCvar = g_pCVar->FindVar("asw_queen_override_health");
	m_AlienInfoArray[BOOMER_INDEX].m_pAlienHealthCvar = g_pCVar->FindVar("asw_boomer_health");
	m_AlienInfoArray[RANGER_INDEX].m_pAlienHealthCvar = g_pCVar->FindVar("asw_ranger_health");
	m_AlienInfoArray[MORTAR_INDEX].m_pAlienHealthCvar = g_pCVar->FindVar("asw_mortarbug_health");
	m_AlienInfoArray[SHAMAN_INDEX].m_pAlienHealthCvar = g_pCVar->FindVar("asw_shaman_health");
	m_AlienInfoArray[UBER_INDEX].m_pAlienHealthCvar = g_pCVar->FindVar("asw_drone_uber_health");
	m_AlienInfoArray[BETA_DRONE_INDEX].m_pAlienHealthCvar = g_pCVar->FindVar("asw_drone_health");
	m_AlienInfoArray[BETA_SHIELDBUG_INDEX].m_pAlienHealthCvar = g_pCVar->FindVar("asw_shieldbug_health");
	m_AlienInfoArray[BETA_HARVESTER_INDEX].m_pAlienHealthCvar = g_pCVar->FindVar("asw_harvester_health");
	//softcopy:
	m_AlienInfoArray[BETA_BUZZER_INDEX].m_pAlienHealthCvar = g_pCVar->FindVar("sk_asw_buzzer_beta_health");
	m_AlienInfoArray[BETA_PARASITE_INDEX].m_pAlienHealthCvar = g_pCVar->FindVar("asw_parasite_health");
	m_AlienInfoArray[BETA_MORTAR_INDEX].m_pAlienHealthCvar = g_pCVar->FindVar("asw_mortarbug_health");

}

const CASW_Horde_Mode::AlienInfo *CASW_Horde_Mode::GetAlienInfo(int index)
{
	if (index < 0 || index >= ALIEN_INDEX_COUNT)
		return NULL;

	return &m_AlienInfoArray[index];
}

void ASWHordemodeListAliens( const CCommand &command )
{
	for (int i = 0; i < CASW_Horde_Mode::ALIEN_INDEX_COUNT; i++)
	{
		const CASW_Horde_Mode::AlienInfo *pAlien = ASWHordeMode()->GetAlienInfo(i);

		ConColorMsg(Color(255, 0, 0), "%s\n", pAlien->m_szAlienClassName);
		// Flag
		{
			Msg("	flag: %i\n", pAlien->m_iFlag);
		}

		// Enabled?
		{
			Msg("	enabled:");
			
			if (pAlien->m_iFlag & asw_hordemode_aliens.GetInt())
				ConColorMsg(Color(0, 255, 0), "true");
			else
				ConColorMsg(Color(255, 255, 0), "false");

			Msg("\n");
		}

		// Beta?
		{
			Msg("	beta: ");

			if (pAlien->m_bBeta)
				ConColorMsg(Color(0, 255, 0), "true");
			else
				ConColorMsg(Color(255, 255, 0), "false");

			Msg("\n");
		}

		// Extra newline
		Msg("\n");
	}
}
ConCommand asw_hordemode_aliens_list("asw_hordemode_aliens_list", ASWHordemodeListAliens, "Displays a listing of aliens supported by hordemode.", FCVAR_CCOOP);
