#pragma once

#include "asw_player.h"
#include "asw_marine.h"
//softcopy: cvar max player in lobby can be 6, sets Client_Effects to match with it.  
//#define ASW_PLAYERINFO_SIZE ASW_MAX_MARINE_RESOURCES
#define ASW_PLAYERINFO_SIZE ASW_MAX_MARINE_RESOURCES+1

class CASW_Client_Effects : public CAutoGameSystemPerFrame
{
public:
	CASW_Client_Effects(void);
	~CASW_Client_Effects(void);

	bool PlayerAdd(CASW_Player *pPlayer);
	void PlayerRemove(CASW_Player *pPlayer);
	void PlayerSwitched(CASW_Player *pPlayer, CASW_Marine *pMarine_new);
	void EnableForPlayer(CASW_Player *pPlayer, bool bEnabled);
	bool ToggleForPlayer(CASW_Player *pPlayer);

private:
	virtual void FrameUpdatePostEntityThink();
	virtual void LevelShutdownPreEntity();

	template <class T> class CFX_Value
	{
	public:
		void Reset(T defaultValue, const char *szCvarName)
		{
			m_Value = defaultValue;
			m_LastSentValue = defaultValue;

			m_flLastSent = 0.0f;

			m_szCvarName[0] = '\0';
			V_strncpy(m_szCvarName, szCvarName, sizeof(m_szCvarName));
		}

		T m_Value;				// The server's value.
		T m_LastSentValue;		// The client's value.

		float m_flLastSent;		// Last client update time.
		
		char m_szCvarName[256];	// The string cvar name.
	};

	struct PlayerInfo
	{
		CHandle<CASW_Player> m_hPlayer;
		CHandle<CASW_Marine> m_hMarine;

		//Settings for local contrast enhancement
		CFX_Value<bool> m_bEnabled;
		CFX_Value<float> m_flStrength;
		CFX_Value<float> m_flStart;
		CFX_Value<float> m_flEnd;

	} m_PlayerInfoArray[ASW_PLAYERINFO_SIZE];

	enum EffectType_t
	{
		EFFECT_LCE,
		EFFECT_BLOOM,

		EFFECT_COUNT,
	};

	void OnSpawnedHorde(int num);

	bool ShouldUpdateCvar(CFX_Value<bool> Cvar, EffectType_t EffectType) const;
	bool ShouldUpdateCvar(CFX_Value<int> Cvar, EffectType_t EffectType) const;
	bool ShouldUpdateCvar(CFX_Value<float> Cvar, EffectType_t EffectType) const;

	float IsMarineHurt(CASW_Marine *pMarine) const;
	float GetMarineIntensity(CASW_Marine *pMarine) const;

	void ResetPlayers();
	void ResetPlayer(int playerIndex);

	bool SendClientCommand(edict_t *pPlayerEdict, CFX_Value<bool> &Cvar);
	bool SendClientCommand(edict_t *pPlayerEdict, CFX_Value<float> &Cvar);
};

CASW_Client_Effects* ASW_Client_Effects();