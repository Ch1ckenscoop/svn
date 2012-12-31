#include "cbase.h"
#include "ASW_Client_Effects.h"
#include "asw_director.h"
#include "asw_marine_resource.h"
#include "asw_gamerules.h"
#include "client.h"
#include "HypKeyValues.h"

#include "memdbgon.h"

ConVar asw_cfx_enable("asw_cfx_enable", "1", FCVAR_CHEAT, "Enables client effects.");
ConVar asw_cfx_debug("asw_cfx_debug", "0", FCVAR_NONE, "Shows debug messages for client effects.");

ConVar asw_cfx_lce_multi("asw_cfx_lce_multi", "1.0", FCVAR_CHEAT, "Controls local contrast enhancement intensity.");
ConVar asw_cfx_lce_max("asw_cfx_lce_max", "1.0", FCVAR_CHEAT, "Maximum intensity of local contrast enhancement.");
ConVar asw_cfx_lce_deadzone("asw_cfx_lce_deadzone", "0.2", FCVAR_CHEAT, "'Dead zone' where we don't worry about a client's cvar being off by this much.");
ConVar asw_cfx_lce_hurt("asw_cfx_lce_hurt", "35", FCVAR_CHEAT, "Threshold of marine health at which LCE effects start to show.");
ConVar asw_cfx_lce_forceupdate("asw_cfx_lce_forceupdate", "1.0", FCVAR_CHEAT, "After this amount of time, force an update of the client's cvar whether it needs it or not.");

// Some defines so we don't have to type out full convar names everywhere.

#define MARINE_HURT asw_cfx_lce_hurt.GetFloat()	// Any health below this is defined as "hurt".

#define LCE_MAX	asw_cfx_lce_max.GetFloat()						// Maximum intensity LCE can reach.
#define LCE_MULTI asw_cfx_lce_multi.GetFloat()					// Multiplier for LCE intensity.

#define LCE_ONOFF "mat_local_contrast_enable"					// Enables or disables LCE.
#define LCE_VIGNETTE "mat_local_contrast_scale_override"		// Controls LCE in a vignette'd area (see below). Sort of a stressful, "tunnel vision" effect.
#define LCE_VSTART "mat_local_contrast_vignette_start_override"	// Vignette start.
#define LCE_VEND "mat_local_contrast_vignette_end_override"		// Vignette end.

#define CFX_FILE "cfg/sourcemod/cfx.cfg"

CASW_Client_Effects g_C_ASW_Effects;
CASW_Client_Effects* ASW_Client_Effects() { return &g_C_ASW_Effects; }

CASW_Client_Effects::CASW_Client_Effects(void) : CAutoGameSystemPerFrame("ASW Client Effects")
{
	ResetPlayers();
}

CASW_Client_Effects::~CASW_Client_Effects(void)
{
	ResetPlayers();
}

void CASW_Client_Effects::ResetPlayers()
{
	for (int i = 0; i < ASW_PLAYERINFO_SIZE; i++)
	{
		ResetPlayer(i);
	}
}

void CASW_Client_Effects::ResetPlayer(int iPlayerIndex)
{
	Assert(iPlayerIndex >= 0);
	Assert(iPlayerIndex < ASW_PLAYERINFO_SIZE);

	m_PlayerInfoArray[iPlayerIndex].m_hPlayer = NULL;
	m_PlayerInfoArray[iPlayerIndex].m_hMarine = NULL;

	m_PlayerInfoArray[iPlayerIndex].m_bEnabled = true;
	m_PlayerInfoArray[iPlayerIndex].m_flStrength = 0.0f;
	m_PlayerInfoArray[iPlayerIndex].m_flStart = 0.0f;
	m_PlayerInfoArray[iPlayerIndex].m_flEnd = 0.0f;
}

void CASW_Client_Effects::LevelShutdownPreEntity()
{
	// Clear out all our players.
	ResetPlayers();
}

bool CASW_Client_Effects::PlayerAdd(CASW_Player *pPlayer)
{
	if (!pPlayer)
		return false;

	for (int i = 0; i < ASW_PLAYERINFO_SIZE; i++)
	{
		if (m_PlayerInfoArray[i].m_hPlayer == NULL)
		{
			m_PlayerInfoArray[i].m_hPlayer = pPlayer;


			if (pPlayer->GetMarine())
			{
				if (pPlayer->GetMarine()->GetHealth() >= 1)
					m_PlayerInfoArray[i].m_hMarine = pPlayer->GetMarine();
			}
			else if (pPlayer->GetSpectatingMarine())
				m_PlayerInfoArray[i].m_hMarine = pPlayer->GetSpectatingMarine();

			// Load the player's saved value from our database
			if (filesystem->FileExists(CFX_FILE))
			{
				HypKeyValues rootKV;
				rootKV.FileToKeyValues(CFX_FILE);

				HypKeyValues *clientFXKey = rootKV.FindKey("Client Effects Settings");
				Assert(clientFXKey);

				HypKeyValues *playersKey = clientFXKey->FindKey("Players");
				Assert(playersKey);

				CSteamID steamID;
				pPlayer->GetSteamID(&steamID);
				uint64 steamID_64 = steamID.ConvertToUint64();

				char buf[64];
				V_snprintf(buf, sizeof(buf), "%llu", steamID_64);

				HypKeyValues *playerKey = playersKey->FindKey(buf);
				if (playerKey)
				{
					// We have a saved value for this Steam ID.
					playerKey->GetBool("cfx enabled", m_PlayerInfoArray[i].m_bEnabled.m_Value, true);
				}
			}

			if (asw_cfx_debug.GetBool())
			{
				Msg("Added player '%s' to CFX array. Contents:\n", pPlayer->GetPlayerName());

				for (int i = 0; i < ASW_PLAYERINFO_SIZE; i++)
				{
					CASW_Player *pPlayer2 = m_PlayerInfoArray[i].m_hPlayer;
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
		if (m_PlayerInfoArray[i].m_hPlayer == pPlayer)
		{
			// Save the client's cfx enabled/disabled state.
			{
				HypKeyValues rootKV;
				if (filesystem->FileExists(CFX_FILE))
					rootKV.FileToKeyValues(CFX_FILE);

				HypKeyValues *clientFXKey = rootKV.FindKey("Client Effects Settings", true);
				Assert(clientFXKey);

				HypKeyValues *playersKey = clientFXKey->FindKey("Players", true);
				Assert(playersKey);

				CSteamID steamID;
				pPlayer->GetSteamID(&steamID);
				uint64 steamID_64 = steamID.ConvertToUint64();

				char buf[64];
				V_snprintf(buf, sizeof(buf), "%llu", steamID_64);

				HypKeyValues *playerKey = playersKey->FindKey(buf, true);
				Assert(playerKey);
				playerKey->SetValue("cfx enabled", m_PlayerInfoArray[i].m_bEnabled.m_Value);

				// Save the keyvalues
				clientFXKey->KeyValuesToFile(CFX_FILE);
			}

			ResetPlayer(i);

			if (asw_cfx_debug.GetBool())
				Msg("Removing player '%s' from CFX array.\n", pPlayer->GetPlayerName());

			for (int i = 0; i < ASW_PLAYERINFO_SIZE; i++)
			{
				CASW_Player *pPlayer2 = m_PlayerInfoArray[i].m_hPlayer;
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
	}
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
		if (m_PlayerInfoArray[i].m_hPlayer == pPlayer)
		{
			m_PlayerInfoArray[i].m_hMarine = pMarine_new;
		}
	}
}

void CASW_Client_Effects::FrameUpdatePostEntityThink()
{	
	if (!asw_cfx_enable.GetBool())	//Don't think if not enabled
		return;

	// Only think when we're in-game.
	if ( !ASWGameRules() || ASWGameRules()->GetGameState() != ASW_GS_INGAME )
		return;

	for (int i = 0; i < ASW_PLAYERINFO_SIZE; i++)
	{
		CASW_Marine *pMarine = m_PlayerInfoArray[i].m_hMarine;
		CASW_Player *pPlayer = m_PlayerInfoArray[i].m_hPlayer;

		if (pMarine && pPlayer && pPlayer->IsConnected())
		{
			edict_t *pPlayerEdict = pPlayer->edict();
			CFX_Value<float> flStrength_old = m_PlayerInfoArray[i].m_flStrength;
			CFX_Value<float> flStart_old = m_PlayerInfoArray[i].m_flStart;
			CFX_Value<float> flEnd_old = m_PlayerInfoArray[i].m_flEnd;
			CFX_Value<bool> bEnabled = m_PlayerInfoArray[i].m_bEnabled;

			float flStart_new = 0.5f;	// Static for now.
			float flEnd_new = 1.0f;

			float flStrength_max = LCE_MAX;
			float flStrength_new = 0.0f;

			flStrength_new += IsMarineHurt(pMarine);
			flStrength_new += GetMarineIntensity(pMarine);
			flStrength_new *= LCE_MULTI;

			// Make sure we can't get to insanely high intensity values.
			if (flStrength_new > flStrength_max)
				flStrength_new = flStrength_max;

			if (ShouldUpdateCvar(flStrength_old, flStrength_new, EFFECT_LCE))
			{
				if (!SendClientCommand(pPlayerEdict, LCE_VIGNETTE, flStrength_new) && asw_cfx_debug.GetBool())
					Msg("Unable to send command %s!\n", LCE_VIGNETTE);

				m_PlayerInfoArray[i].m_flStrength = flStrength_new;
			}

			if (ShouldUpdateCvar(flStart_old, flStart_new, EFFECT_LCE))
			{
				if (!SendClientCommand(pPlayerEdict, LCE_VSTART, flStart_new) && asw_cfx_debug.GetBool())
					Msg("Unable to send command %s!\n", LCE_VSTART);

				m_PlayerInfoArray[i].m_flStart = flStart_new;
			}

			if (ShouldUpdateCvar(flEnd_old, flEnd_new, EFFECT_LCE))
			{
				if (!SendClientCommand(pPlayerEdict, LCE_VEND, flEnd_new) && asw_cfx_debug.GetBool())
					Msg("Unable to send command %s!\n", LCE_VEND);

				m_PlayerInfoArray[i].m_flEnd = flEnd_new;
			}

			// Basically just send the value if it's been too long.
			if (ShouldUpdateCvar(bEnabled, bEnabled.m_Value, EFFECT_LCE))
			{
				if (!SendClientCommand(pPlayerEdict, LCE_ONOFF, bEnabled.m_Value) && asw_cfx_debug.GetBool())
					Msg("Unable to send command %s!\n", LCE_ONOFF);
			}
		}
	}
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

void CASW_Client_Effects::EnableForPlayer(CASW_Player *pPlayer, bool bEnabled)
{
	if (!pPlayer)
		return;

	for (int i = 0; i < ASW_PLAYERINFO_SIZE; i++)
	{
		if (m_PlayerInfoArray[i].m_hPlayer == pPlayer)
		{
			if (asw_cfx_debug.GetBool())
				Msg("Set cfx for player '%s' to %i.\n", pPlayer->GetPlayerName(), bEnabled);

			m_PlayerInfoArray[i].m_bEnabled = bEnabled;
		}
	}
}

bool CASW_Client_Effects::ToggleForPlayer(CASW_Player *pPlayer)
{
	if (!pPlayer)
		return false;

	for (int i = 0; i < ASW_PLAYERINFO_SIZE; i++)
	{
		if (m_PlayerInfoArray[i].m_hPlayer == pPlayer)
		{
			m_PlayerInfoArray[i].m_bEnabled = !m_PlayerInfoArray[i].m_bEnabled.m_Value;

			if (asw_cfx_debug.GetBool())
				Msg("Toggled cfx for player '%s' to '%i'.\n", pPlayer->GetPlayerName(), m_PlayerInfoArray[i].m_bEnabled);

			return m_PlayerInfoArray[i].m_bEnabled.m_Value;
		}
	}

	return false;
}

static void ToggleCommandCallback(const CCommand &args)
{
	CASW_Player *pCommandPlayer = dynamic_cast<CASW_Player*>(UTIL_GetCommandClient());
	if (!pCommandPlayer)
		return;

	// Toggle CFX for this player.
	bool bNewValue = ASW_Client_Effects()->ToggleForPlayer(pCommandPlayer);

	// Tell the player who sent this command that CFX has been toggled.
	CRecipientFilter filter;
	filter.AddRecipient(pCommandPlayer);

	char szEnabled[16];
	if (bNewValue)
		V_snprintf(szEnabled, sizeof(szEnabled), "enabled");
	else
		V_snprintf(szEnabled, sizeof(szEnabled), "disabled");

	char buf[64];
	V_snprintf(buf, sizeof(buf), "CFX has been %s for you.\n", szEnabled);

	UTIL_ClientPrintFilter(filter, ASW_HUD_PRINTTALKANDCONSOLE, buf);
}
ChatCommand cfx_toggle_command("/cfx", ToggleCommandCallback);