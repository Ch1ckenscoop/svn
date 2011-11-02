#pragma once
#include "convar.h"

class CASW_Horde_Mode : public CLogicalEntity
{
public:
	DECLARE_CLASS( CASW_Horde_Mode, CLogicalEntity );

	CASW_Horde_Mode(void);
	~CASW_Horde_Mode(void);
	
	virtual void Spawn();
	virtual void Think();
	virtual void SetHordeMode(bool bEnabled);
	virtual int GetRandomValidAlien();
	virtual void InitAlienData();
	
	enum HordeModeAliens
	{
		DRONE_INDEX,
		BUZZER_INDEX,
		PARASITE_INDEX,
		SHIELDBUG_INDEX,
		GRUB_INDEX,
		JUMPER_INDEX,
		HARVESTER_INDEX,
		PARASITE_DEFANGED_INDEX,
		QUEEN_INDEX,
		BOOMER_INDEX,
		RANGER_INDEX,
		MORTAR_INDEX,
		SHAMEN_INDEX,
		UBER_INDEX,

		BETA_DRONE_INDEX,
		BETA_SHIELDBUG_INDEX,

		HIGHEST_INDEX,	//Ch1ckensCoop: If you're going to add anything, add it before this!
	};
	
	struct AlienInfo
	{
		const char *pAlienClassName;
		int flag;
		ConVarRef max;
		ConVarRef min;
		ConVarRef healthMax;
		ConVarRef healthMin;
		ConVarRef betaAlienConVar;
		bool betaAlienCvarReversed;
		//int defaultHealth;
	} AlienInfoArray [HIGHEST_INDEX - 1];
};
