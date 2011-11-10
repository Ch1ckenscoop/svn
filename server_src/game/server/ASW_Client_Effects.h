#pragma once

#include "asw_player.h"
#include "asw_marine_resource.h"

class CASW_Client_Effects : CAutoGameSystemPerFrame
{
public:
	CASW_Client_Effects(void);
	~CASW_Client_Effects(void);

	struct PlayerInfo
	{
		CASW_Player *pPlayer;
		CASW_Marine_Resource *pMR;

		//Settings for local contrast enhancement
		bool lce_isEnabled;
		float lce_screenScale;
		float lce_vStrength;
		float lce_vStart;
		float lce_vEnd;
		
		bool playerWantsDisabled;
	};

	float IsMarineHurt(CASW_Marine *pMarine);
	
	void PlayerConnected(CASW_Player *pPlayer);
	void PlayerDisconnected(CASW_Player *pPlayer);
	void MarineSwitched(CASW_Player *pPlayer, CASW_Marine_Resource *pMR_New);

	void FrameUpdatePostEntityThink();
};
