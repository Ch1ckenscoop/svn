#include "cbase.h"
#include "ASW_Client_Effects.h"
#include "asw_director.h"
#include "asw_marine_resource.h"

#include "memdbgon.h"

ConVar asw_cfx_enable("asw_cfx_enable", "1", FCVAR_CHEAT, "Enables client effects.");

ConVar asw_cfx_lce_multi("asw_cfx_lce_multi", "1.0", FCVAR_CHEAT, "Controls local contrast enhancement intensity.");
ConVar asw_cfx_lce_max("asw_cfx_lce_max", "1.0", FCVAR_CHEAT, "Maximum intensity of local contrast enhancement.");
ConVar asw_cfx_debug("asw_cfx_debug", "0", FCVAR_NONE, "Shows debug messages for client effects.");

//Some defines so we don't have to type out the full convar name everywhere

#define MARINE_HURT 35.0f	//Any health below this is defined as "hurt".
#define FORCE_UPDATE_TIME 1.0f	//After this amount of time, update cvars anyway.

// LCE - Local Contrast Enhancement

#define LCE_MAX	1.0f

#define LCE_DEADZONE 0.1f										//"Dead zone" where we won't worry about a client's cvar being off by this much, so we don't spam them with clientcommands.

#define LCE_ONOFF "mat_local_contrast_enable"					//Enables or disables LCE.
//#define LCE_OVERALL "mat_local_contrast_edge_scale_override"	//Controls LCE for the entire screen.
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
	}

}

CASW_Client_Effects::~CASW_Client_Effects(void)
{
}

bool CASW_Client_Effects::MarineAdd(CASW_Marine *pMarine)
{
	for (int i = 0; i < ASW_PLAYERINFO_SIZE; i++)
	{
		if (PlayerInfoArray[i].pMarine == NULL)
		{
			PlayerInfoArray[i].pMarine = pMarine;
			PlayerInfoArray[i].pPlayer = pMarine->GetCommander();

			return true;
		}
	}

	return false;
}

void CASW_Client_Effects::MarineRemove(CASW_Marine *pMarine)
{

}

bool CASW_Client_Effects::ShouldUpdateCvar(CFX_Float OldValue, float NewValue, EffectType_t EffectType)
{
	if (EffectType == EFFECT_LCE)
	{
		float delta = OldValue.m_flValue - NewValue;
		float deadZone = LCE_DEADZONE;
		float forceUpdateTime = FORCE_UPDATE_TIME;

		if (delta > deadZone || delta < -deadZone)
			return true;
		
		//If we haven't updated in a while, update anyway.
		if (OldValue.m_flLastUpdate + forceUpdateTime < gpGlobals->curtime)
			return true;
	}
	return false;
}

bool CASW_Client_Effects::SendClientCommand(CASW_Player *pPlayer, const char *Command, float Value)
{
	if (!pPlayer || !stricmp(Command, ""))
		return false;

	char buffer[128];
	char szValue[8];

	Q_snprintf(szValue, sizeof(szValue), " %.3f", Value);

	strcpy(buffer, Command);
	strcat(buffer, szValue);
	engine->ClientCommand(pPlayer->edict(), buffer);
	return true;
}

void CASW_Client_Effects::FrameUpdatePostEntityThink()
{
	if (!asw_cfx_enable.GetBool())
		return;

	for (int i = 0; i < ASW_PLAYERINFO_SIZE; i++)
	{
		CASW_Marine *pMarine = PlayerInfoArray[i].pMarine;
		CASW_Player *pPlayer = PlayerInfoArray[i].pPlayer;
		bool isEnabled = PlayerInfoArray[i].LCE_isEnabled.m_bValue;
		if (pMarine && pPlayer && isEnabled)
		{
			CFX_Float vStrength_old = PlayerInfoArray[i].LCE_vStrength;
			CFX_Float vStart_old = PlayerInfoArray[i].LCE_vStart;
			CFX_Float vEnd_old = PlayerInfoArray[i].LCE_vEnd;

			float vStart_new = 0.5f;	//Static for now.
			float vEnd_new = 1.0f;
			
			float vStrength_max = asw_cfx_lce_max.GetFloat();
			float vStrength_new = 0.0f;
	
			vStrength_new += IsMarineHurt(pMarine);
			vStrength_new += GetMarineIntensity(pMarine);
			vStrength_new *= asw_cfx_lce_multi.GetFloat();
			
			if (vStrength_new > vStrength_max)
				vStrength_new = vStrength_max;	//Make sure we can't get to insanely high intensity values.

			if (ShouldUpdateCvar(vStrength_old, vStrength_new, EFFECT_LCE))
			{
				SendClientCommand(pPlayer, LCE_VIGNETTE, vStrength_new);
				PlayerInfoArray[i].LCE_vStrength.m_flValue = vStrength_new;
				PlayerInfoArray[i].LCE_vStrength.m_flLastUpdate = gpGlobals->curtime;
			}

			if (ShouldUpdateCvar(vStart_old, vStart_new, EFFECT_LCE))
			{
				SendClientCommand(pPlayer, LCE_VSTART, vStart_new);
				PlayerInfoArray[i].LCE_vStart.m_flValue = vStart_new;
				PlayerInfoArray[i].LCE_vStart.m_flLastUpdate = gpGlobals->curtime;
			}

			if (ShouldUpdateCvar(vEnd_old, vEnd_new, EFFECT_LCE))
			{
				SendClientCommand(pPlayer, LCE_VEND, vEnd_new);
				PlayerInfoArray[i].LCE_vEnd.m_flValue = vEnd_new;
				PlayerInfoArray[i].LCE_vEnd.m_flLastUpdate = gpGlobals->curtime;
			}
		}
	}
}

float CASW_Client_Effects::IsMarineHurt(CASW_Marine *pMarine)
{
	int curHealth = pMarine->GetHealth();
	//int maxHealth = pMarine->GetMaxHealth();

	float hurtAmount = 0.0f;
	
	//First we're going to divide their current health by our "hurt" value.
	hurtAmount = curHealth / MARINE_HURT;

	//If marine isn't hurt, return 0.0f.
	if (hurtAmount >= 1.0f)
		return 0.0f;
	
	return 1.0f - hurtAmount;	//Return how hurt the marine is.
}

float CASW_Client_Effects::GetMarineIntensity(CASW_Marine *pMarine)
{
	CASW_Intensity *pIntensity = pMarine->GetMarineResource()->GetIntensity();
	
	if (pIntensity)
	{
		return pIntensity->GetCurrent();
	}
	
	return 0.0f;
}