#pragma once
#include "convar.h"
#include "igamesystem.h"

class CASW_Horde_Mode : public CAutoGameSystemPerFrame
{
public:
	DECLARE_CLASS( CASW_Horde_Mode, CLogicalEntity );

	CASW_Horde_Mode();
	~CASW_Horde_Mode();
	
	enum HordeModeAliens
	{
		DRONE_INDEX,
		BUZZER_INDEX,
		PARASITE_INDEX,
		SHIELDBUG_INDEX,
		JUMPER_INDEX,
		HARVESTER_INDEX,
		PARASITE_DEFANGED_INDEX,
		QUEEN_INDEX,
		BOOMER_INDEX,
		RANGER_INDEX,
		MORTAR_INDEX,
		SHAMAN_INDEX,
		UBER_INDEX,

		BETA_DRONE_INDEX,
		BETA_SHIELDBUG_INDEX,
		BETA_HARVESTER_INDEX,

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
		bool isBeta;
		ConVarRef alienHealthCvar;
		
		AlienInfo() {}

	} AlienInfoArray[HIGHEST_INDEX];
	
	virtual bool Init();
	virtual void LevelInitPostEntity();
	virtual void FrameUpdatePostEntityThink();

	virtual void HordeFinishedSpawning();
	virtual int GetRandomValidAlien();
	virtual void InitAlienData();
	virtual void UpdateHordeMode();
	virtual void RandomizeHealth(int alienNum);	//Randomize a specific alien type's health.
	virtual void RandomizeHealth();	//Randomize all alien healths.

	float fl_LastThinkTime;
	int i_LastAlienClass;
	
};

CASW_Horde_Mode* ASWHordeMode();