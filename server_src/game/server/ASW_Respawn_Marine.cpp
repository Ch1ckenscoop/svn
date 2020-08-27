#include "cbase.h"
#include "ASW_Respawn_Marine.h"
#include "convar.h"
#include "asw_gamerules.h"
#include "asw_game_resource.h"
#include "asw_player.h"
#include "asw_marine_resource.h"
#include "asw_marine.h"

//Ch1ckensCoop: Enable an easy way to respawn marines during the game. Just for fun ;)

//softcopy:
CASW_Respawn_Marine g_Respawn_Marine;
CASW_Respawn_Marine* ASWRespawnMarine() { return &g_Respawn_Marine; }

CASW_Respawn_Marine::CASW_Respawn_Marine(void)
{
	InitMarineName();	//softcopy:
}

CASW_Respawn_Marine::~CASW_Respawn_Marine(void)
{
}

void RespawnMarineX(const char *marineName)
{
	const int numMarineResources = ASWGameResource()->GetMaxMarineResources();
	CASW_Marine *pAliveMarine = NULL;
	for ( int i=0; i < numMarineResources ; i++ )
	{
		CASW_Marine_Resource *pMR = ASWGameResource()->GetMarineResource( i );
		CASW_Marine *pMarine = pMR ? pMR->GetMarineEntity() : NULL;
		if ( pMarine && pMR->GetHealthPercent() > 0 )
		{
			pAliveMarine = pMarine;
			break;
		}
	}

	for ( int i=0; i < numMarineResources ; i++ )
	{
		CASW_Marine_Resource *pMR = ASWGameResource()->GetMarineResource( i );

		if (pMR && !stricmp(pMR->GetProfile()->GetShortName(), marineName))	//softcopy: fixed crash and stricmp to work again
		//if (pMR->GetProfile()->GetShortName() == marineName)
		{
			if ( pMR && pMR->GetHealthPercent() <= 0 ) // if marine exists, is dead
			{
				ASWGameRules()->Resurrect( pMR, pAliveMarine );
				DevMsg("Respawned marine: %s\n", pMR->GetProfile()->GetShortName());
				return; // don't do two in a frame
			}
		}
	}
	//Msg("Marine %s doesn't exist in this game.\n", marineName);	//softcopy:
}

void RespawnMarine(const CCommand &command)
{
	//softcopy: respawn a marine/all marines  
	/*
	if (stricmp(command.ArgS(), "crash"))
		RespawnMarineX("#asw_name_crash");
	if (stricmp(command.ArgS(), "vegas"))
		RespawnMarineX("#asw_name_vegas");
	if (stricmp(command.ArgS(), "wildcat"))
		RespawnMarineX("#asw_name_wildcat");
	if (stricmp(command.ArgS(), "wolfe"))
		RespawnMarineX("#asw_name_wolfe");
	if (stricmp(command.ArgS(), "sarge"))
		RespawnMarineX("#asw_name_sarge");
	if (stricmp(command.ArgS(), "jaeger"))
		RespawnMarineX("#asw_name_jaeger");
	if (stricmp(command.ArgS(), "faith"))
		RespawnMarineX("#asw_name_faith");
	if (stricmp(command.ArgS(), "bastille"))
		RespawnMarineX("#asw_name_bastille");
	*/
	for (int i=0; i < CASW_Respawn_Marine::MARINE_INDEX_COUNT; i++)
	{
		const CASW_Respawn_Marine::MarineInfo *pMI = ASWRespawnMarine()->GetMarineInfo(i);

		if (pMI && (!stricmp(command.ArgS(), pMI->m_MarineName) || !stricmp(command.ArgS(), "")))
			RespawnMarineX(pMI->m_szMarineClassName);
	}

}

static int RespawnAutoComplete(char const *partial, char commands[ COMMAND_COMPLETION_MAXITEMS ][ COMMAND_COMPLETION_ITEM_LENGTH ] )
{
	strcpy( commands[0], "hello" );
	strcpy( commands[1], "goodbye" );
	return 2;
}

static ConCommand asw_respawn_marine("asw_respawn_marine", RespawnMarine, "Respawns a marine by their marine's name.", FCVAR_CHEAT);

//softcopy:
const CASW_Respawn_Marine::MarineInfo *CASW_Respawn_Marine::GetMarineInfo(int index)
{
	if (index < 0 || index >= MARINE_INDEX_COUNT)
		return NULL;

	return &m_MarineInfoArray[index];
}
void CASW_Respawn_Marine::InitMarineName()
{
	m_MarineInfoArray[CRASH_INDEX].m_MarineName = "crash";
	m_MarineInfoArray[VEGAS_INDEX].m_MarineName = "vegas";
	m_MarineInfoArray[WILDCAT_INDEX].m_MarineName = "wildcat";
	m_MarineInfoArray[WOLFE_INDEX].m_MarineName = "wolfe";
	m_MarineInfoArray[SARGE_INDEX].m_MarineName = "sarge";
	m_MarineInfoArray[JAEGER_INDEX].m_MarineName = "jaeger";
	m_MarineInfoArray[FAITH_INDEX].m_MarineName = "faith";
	m_MarineInfoArray[BASTILLE_INDEX].m_MarineName = "bastille";

	m_MarineInfoArray[CRASH_INDEX].m_szMarineClassName = "#asw_name_crash";
	m_MarineInfoArray[VEGAS_INDEX].m_szMarineClassName = "#asw_name_vegas";
	m_MarineInfoArray[WILDCAT_INDEX].m_szMarineClassName = "#asw_name_wildcat";
	m_MarineInfoArray[WOLFE_INDEX].m_szMarineClassName = "#asw_name_wolfe";
	m_MarineInfoArray[SARGE_INDEX].m_szMarineClassName = "#asw_name_sarge";
	m_MarineInfoArray[JAEGER_INDEX].m_szMarineClassName = "#asw_name_jaeger";
	m_MarineInfoArray[FAITH_INDEX].m_szMarineClassName = "#asw_name_faith";
	m_MarineInfoArray[BASTILLE_INDEX].m_szMarineClassName = "#asw_name_bastille";
}
