#pragma once

#include "asw_player.h"
#include "asw_marine.h"

#define ASW_PLAYERINFO_SIZE ASW_MAX_MARINE_RESOURCES

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
		void operator=(const T &rhs)
		{
			m_Value = rhs;
			
			if (gpGlobals)	// This gets called before a lot of systems are initialized.
				m_flLastUpdate = gpGlobals->curtime;
			else
				m_flLastUpdate = -1;
		}

		T m_Value;
		float m_flLastUpdate;
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

	template<class T> bool ShouldUpdateCvar(CFX_Value<T> PreviousValue, T NewValue, EffectType_t EffectType)
	{		
		extern ConVar asw_cfx_lce_deadzone;
		extern ConVar asw_cfx_lce_forceupdate;

		// We don't support anything other than LCE at the moment.
		Assert(EffectType == EFFECT_LCE);

		if (EffectType == EFFECT_LCE)
		{
			float delta = fabs((float)PreviousValue.m_Value - NewValue);
			float deadZone = asw_cfx_lce_deadzone.GetFloat();
			float forceUpdateTime = asw_cfx_lce_forceupdate.GetFloat();

			delta = fabs(delta);	//Get the absolute value.

			if (delta > deadZone)
				return true;

			//If we haven't updated in a while, update anyway.
			if (PreviousValue.m_flLastUpdate + forceUpdateTime < gpGlobals->curtime)
				return true;
		}
		return false;
	}

	float IsMarineHurt(CASW_Marine *pMarine);
	float GetMarineIntensity(CASW_Marine *pMarine);

	void ResetPlayers();
	void ResetPlayer(int playerIndex);

	bool SendClientCommand(edict_t *pPlayerEdict, const char *Command, float Value);
	bool SendClientCommand(edict_t *pPlayerEdict, const char *Command, bool Value);
};

CASW_Client_Effects* ASW_Client_Effects();