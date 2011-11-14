#include "cbase.h"
#include "ASW_Client_Effects.h"
#include "asw_director.h"
#include "asw_marine_resource.h"
#include "asw_gamerules.h"

#include "memdbgon.h"

ConVar asw_cfx_enable("asw_cfx_enable", "1", FCVAR_CHEAT, "Enables client effects.");
ConVar asw_cfx_debug("asw_cfx_debug", "0", FCVAR_NONE, "Shows debug messages for client effects.");

ConVar asw_cfx_lce_multi("asw_cfx_lce_multi", "1.0", FCVAR_CHEAT, "Controls local contrast enhancement intensity.");
ConVar asw_cfx_lce_max("asw_cfx_lce_max", "1.0", FCVAR_CHEAT, "Maximum intensity of local contrast enhancement.");
ConVar asw_cfx_lce_deadzone("asw_cfx_lce_deadzone", "0.2", FCVAR_CHEAT, "'Dead zone' where we don't worry about a client's cvar being off by this much.");
ConVar asw_cfx_lce_hurt("asw_cfx_lce_hurt", "35", FCVAR_CHEAT, "Threshold of marine health at which LCE effects start to show.");

//Some defines so we don't have to type out the full convar name everywhere

#define MARINE_HURT asw_cfx_lce_hurt.GetFloat()	//Any health below this is defined as "hurt".
#define FORCE_UPDATE_TIME 1.0f	//After this amount of time, update cvars anyway.

#define LCE	// Local Contrast Enhancement

#define LCE_MAX	asw_cfx_lce_max.GetFloat()						//Maximum intensity LCE can reach.
#define LCE_MULTI asw_cfx_lce_multi.GetFloat()					//Multiplier for LCE intensity.

#define LCE_DEADZONE asw_cfx_lce_deadzone.GetFloat()			//"Dead zone" where we won't worry about a client's cvar being off by this much, so we don't spam them with clientcommands.

#define LCE_ONOFF "mat_local_contrast_enable"					//Enables or disables LCE.
#define LCE_VIGNETTE "mat_local_contrast_scale_override"		//Controls LCE in a vignette'd area (see below). Sort of a stressful, "tunnel vision" effect.
#define LCE_VSTART "mat_local_contrast_vignette_start_override"	//Vignette start.
#define LCE_VEND "mat_local_contrast_vignette_end_override"		//Vignette end.

CASW_Client_Effects g_C_ASW_Effects;
CASW_Client_Effects* ASW_Client_Effects() { return &g_C_ASW_Effects; }

CASW_Client_Effects::CASW_Client_Effects(void)
{
	for (int i = 0; i < ASW_PLAYERINFO_SIZE; i++)
	{/*
	 PlayerInfoArray[i].LCE_isEnabled = true;
	 PlayerInfoArray[i].LCE_vEnd = 0.5f;
	 PlayerInfoArray[i].LCE_vStart = 1.0f;
	 PlayerInfoArray[i].LCE_vStrength = 0.0f;
	 PlayerInfoArray[i].cfx_LastForceUpdate = gpGlobals->curtime;*/

		PlayerInfoArray[i].LCE_isEnabled.m_bValue = true;
		PlayerInfoArray[i].playerWantsDisabled = true;
	}
	
	i_DebugThinks = 0;
}

CASW_Client_Effects::~CASW_Client_Effects(void)
{
	ResetPlayers();
}

void CASW_Client_Effects::ResetPlayers()
{
	for (int i = 0; i < ASW_PLAYERINFO_SIZE; i++)
	{
		PlayerInfoArray[i].pMarine = NULL;
		PlayerInfoArray[i].pPlayer = NULL;

		PlayerInfoArray[i].LCE_isEnabled.m_bValue = true;
		PlayerInfoArray[i].playerWantsDisabled = true;
	}
}

bool CASW_Client_Effects::PlayerAdd(CASW_Player *pPlayer)
{
	if (!pPlayer)
		return false;

	for (int i = 0; i < ASW_PLAYERINFO_SIZE; i++)
	{
		if (PlayerInfoArray[i].pPlayer == NULL)
		{
			PlayerInfoArray[i].pPlayer = pPlayer;

			if (pPlayer->GetSpectatingMarine())
				PlayerInfoArray[i].pMarine = pPlayer->GetSpectatingMarine();

			if (pPlayer->GetMarine())
			{
				if (pPlayer->GetMarine()->GetHealth() >= 1)
					PlayerInfoArray[i].pMarine = pPlayer->GetMarine();
			}



			if (asw_cfx_debug.GetBool())
			{
				Msg("Added player '%s' to CFX array. Contents:\n", pPlayer->GetPlayerName());

				for (int i = 0; i < ASW_PLAYERINFO_SIZE; i++)
				{
					CASW_Player *pPlayer2 = PlayerInfoArray[i].pPlayer;
					if (pPlayer2)
					{
						Msg(" :: %s\n", pPlayer2->GetPlayerName());
					}
					else
					{
						Msg(" :: No player\n");
					}
				}
			}

			return true;
		}
	}

	return false;
}

void CASW_Client_Effects::PlayerRemove(CASW_Player *pPlayer)
{
	if (!pPlayer)
		return;

	for (int i = 0; i < ASW_PLAYERINFO_SIZE; i++)
	{
		if (PlayerInfoArray[i].pPlayer == pPlayer)
		{
			if (asw_cfx_debug.GetBool())
				Msg("Removing player '%s' from CFX array.\n", pPlayer->GetPlayerName());
			PlayerInfoArray[i].pMarine = NULL;
			PlayerInfoArray[i].pPlayer = NULL;
		}
	}
}

bool CASW_Client_Effects::ShouldUpdateCvar(CFX_Float OldValue, float NewValue, EffectType_t EffectType)
{
	if (EffectType == EFFECT_LCE)
	{
		float delta = OldValue.m_flValue - NewValue;
		float deadZone = LCE_DEADZONE;
		float forceUpdateTime = FORCE_UPDATE_TIME;

		delta = fabs(delta);	//Get the absolute value.

		if (delta > deadZone)
			return true;
		
		//If we haven't updated in a while, update anyway.
		if (OldValue.m_flLastUpdate + forceUpdateTime < gpGlobals->curtime)
			return true;
	}
	return false;
}

bool CASW_Client_Effects::ShouldUpdateCvar(CFX_Bool OldValue, bool NewValue)
{
	float forceUpdateTime = FORCE_UPDATE_TIME;
	bool bOldValue = OldValue.m_bValue;

	if (NewValue != bOldValue)
		return true;

	//If we haven't updated in a while, update anyway.
	if (OldValue.m_flLastUpdate + forceUpdateTime < gpGlobals->curtime)
		return true;
	return false;
}

bool CASW_Client_Effects::SendClientCommand(edict_t *pPlayerEdict, const char *Command, float Value)
{
	if (!pPlayerEdict || !stricmp(Command, ""))
		return false;

	char buffer[128];
	char szValue[8];

	Q_snprintf(szValue, sizeof(szValue), " %.3f", Value);

	strcpy(buffer, Command);
	strcat(buffer, szValue);

	engine->ClientCommand(pPlayerEdict, buffer);
	return true;
}

bool CASW_Client_Effects::SendClientCommand(edict_t *pPlayerEdict, const char *Command, bool Value)
{
	if (!pPlayerEdict || !stricmp(Command, ""))
		return false;

	char buffer[128];
	char szValue[8];

	Q_snprintf(szValue, sizeof(szValue), " %i", Value);

	strcpy(buffer, Command);
	strcat(buffer, szValue);

	engine->ClientCommand(pPlayerEdict, buffer);
	return true;
}

void CASW_Client_Effects::PlayerSwitched(CASW_Player *pPlayer, CASW_Marine *pMarine_new)
{
	if (!pPlayer || !pMarine_new)
		return;

	for (int i = 0; i < ASW_PLAYERINFO_SIZE; i++)
	{
		if (PlayerInfoArray[i].pPlayer == pPlayer)
		{
			PlayerInfoArray[i].pMarine = pMarine_new;
		}
	}
}

void CASW_Client_Effects::FrameUpdatePostEntityThink()
{	
	if (!asw_cfx_enable.GetBool())	//Don't think if not enabled
		return;

	// only think when we're in-game
	if ( !ASWGameRules() || ASWGameRules()->GetGameState() != ASW_GS_INGAME )
		return;

	for (int i = 0; i < ASW_PLAYERINFO_SIZE; i++)
	{
		CASW_Marine *pMarine = PlayerInfoArray[i].pMarine;
		CASW_Player *pPlayer = PlayerInfoArray[i].pPlayer;
		//bool isEnabled = PlayerInfoArray[i].LCE_isEnabled.m_bValue;

#ifdef LCE

		if (pMarine && pPlayer && pPlayer->IsConnected())
		{
			edict_t *pPlayerEdict = pPlayer->edict();
			CFX_Float vStrength_old = PlayerInfoArray[i].LCE_vStrength;
			CFX_Float vStart_old = PlayerInfoArray[i].LCE_vStart;
			CFX_Float vEnd_old = PlayerInfoArray[i].LCE_vEnd;
			CFX_Bool vEnabled = PlayerInfoArray[i].LCE_isEnabled;

			float vStart_new = 0.5f;	//Static for now.
			float vEnd_new = 1.0f;
			
			float vStrength_max = LCE_MAX;
			float vStrength_new = 0.0f;

			bool vEnabled_new = PlayerInfoArray[i].playerWantsDisabled;
	
			vStrength_new += IsMarineHurt(pMarine);
			vStrength_new += GetMarineIntensity(pMarine);
			vStrength_new *= LCE_MULTI;
			
			if (vStrength_new > vStrength_max)
				vStrength_new = vStrength_max;	//Make sure we can't get to insanely high intensity values.

			if (ShouldUpdateCvar(vStrength_old, vStrength_new, EFFECT_LCE))
			{
				if (!SendClientCommand(pPlayerEdict, LCE_VIGNETTE, vStrength_new) && asw_cfx_debug.GetBool())
					Msg("Unable to send command %s!\n", LCE_VIGNETTE);

				PlayerInfoArray[i].LCE_vStrength.m_flValue = vStrength_new;
				PlayerInfoArray[i].LCE_vStrength.m_flLastUpdate = gpGlobals->curtime;
			}

			if (ShouldUpdateCvar(vStart_old, vStart_new, EFFECT_LCE))
			{
				if (!SendClientCommand(pPlayerEdict, LCE_VSTART, vStart_new) && asw_cfx_debug.GetBool())
					Msg("Unable to send command %s!\n", LCE_VSTART);

				PlayerInfoArray[i].LCE_vStart.m_flValue = vStart_new;
				PlayerInfoArray[i].LCE_vStart.m_flLastUpdate = gpGlobals->curtime;
			}

			if (ShouldUpdateCvar(vEnd_old, vEnd_new, EFFECT_LCE))
			{
				if (!SendClientCommand(pPlayerEdict, LCE_VEND, vEnd_new) && asw_cfx_debug.GetBool())
					Msg("Unable to send command %s!\n", LCE_VEND);

				PlayerInfoArray[i].LCE_vEnd.m_flValue = vEnd_new;
				PlayerInfoArray[i].LCE_vEnd.m_flLastUpdate = gpGlobals->curtime;
			}

			if (ShouldUpdateCvar(vEnabled, vEnabled_new))
			{
				if (!SendClientCommand(pPlayerEdict, LCE_ONOFF, vEnabled_new) && asw_cfx_debug.GetBool())
					Msg("Unable to send command %s!\n", LCE_ONOFF);

				PlayerInfoArray[i].LCE_isEnabled.m_bValue = vEnabled_new;
				PlayerInfoArray[i].LCE_isEnabled.m_flLastUpdate = gpGlobals->curtime;
			}
		}
#endif	// LCE
	}
	i_DebugThinks++;
	if (asw_cfx_debug.GetInt() == 2)
		Msg("ThinkNum: %i\n", i_DebugThinks);
}

float CASW_Client_Effects::IsMarineHurt(CASW_Marine *pMarine)
{
	if (!pMarine)
		return 0.0f;

	int curHealth = pMarine->GetHealth();
	
	if (curHealth < 1)
		return 1.0f;

	float hurtAmount = 0;
	
	//First we're going to divide their current health by our "hurt" value.
	hurtAmount = curHealth / MARINE_HURT;

	//If marine isn't hurt, return 0.0f.
	if (hurtAmount >= 1.0f)
		return 0.0f;
	
	return 1.0f - hurtAmount;	//Return how hurt the marine is.
}

float CASW_Client_Effects::GetMarineIntensity(CASW_Marine *pMarine)
{
	if (!pMarine)
		return 0.0f;

	CASW_Marine_Resource *pMR = pMarine->GetMarineResource();

	if (!pMR)
		return 0.0f;

	CASW_Intensity *pIntensity = pMR->GetIntensity();
	
	if (pIntensity)
	{
		return pIntensity->GetCurrent();
	}
	
	return 0.0f;
}

void CASW_Client_Effects::ToggleForPlayer(CASW_Player *pPlayer, bool bEnabled)
{
	if (!pPlayer)
		return;

	for (int i = 0; i < ASW_PLAYERINFO_SIZE; i++)
	{
		if (PlayerInfoArray[i].pPlayer == pPlayer)
		{
			if (asw_cfx_debug.GetBool())
				Msg("Disabled cfx by request for player '%s'.\n", pPlayer->GetPlayerName());

			PlayerInfoArray[i].playerWantsDisabled = bEnabled;
		}
	}
}

void DisableEffects_f( const CCommand &args )
{
	if (args.ArgC() == 3)
	{
		int plrIndex = atoi(args.Arg(1));
		bool Enabled = static_cast<bool>( atoi(args.Arg(2)) );
		if (ASW_Client_Effects())
			ASW_Client_Effects()->ToggleForPlayer(ToASW_Player(UTIL_PlayerByIndex(plrIndex)), Enabled);
	}
	else
	{
		Msg("Incorrect syntax! 'cfx_toggle <player index> <1/0>\n");
	}
}
ConCommand cfx_toggle("cfx_toggle", DisableEffects_f, "Disables CFX for a client index. Used by sourcemod.");