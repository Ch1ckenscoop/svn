#pragma once
#include "convar.h"
#include "igamesystem.h"

class CASW_Horde_Mode : public CAutoGameSystemPerFrame
{
public:
	DECLARE_CLASS( CASW_Horde_Mode, CLogicalEntity );

	CASW_Horde_Mode();
	~CASW_Horde_Mode();
	
	enum HordeModeAlien
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

		ALIEN_INDEX_COUNT,	//Ch1ckensCoop: If you're going to add anything, add it before this!
	};
	
	struct AlienInfo
	{
		const char *m_szAlienClassName;
		int m_iFlag;
		ConVar *m_pMaxCvar;
		ConVar *m_pMinCvar;
		ConVar *m_pHealthMaxCvar;
		ConVar *m_pHealthMinCvar;
		ConVar *m_pBetaAlienCvar;
		bool m_bBetaAlienCvarReversed;
		bool m_bBeta;
		ConVar *m_pAlienHealthCvar;
	};

	const AlienInfo *GetAlienInfo(int index);
	
	virtual bool Init();
	virtual void LevelInitPostEntity();
	virtual void FrameUpdatePostEntityThink();

	virtual void HordeFinishedSpawning();

private:
	float m_flLastThinkTime;
	int m_iLastAlienClass;

	AlienInfo m_AlienInfoArray[ALIEN_INDEX_COUNT];

	virtual int GetRandomValidAlien();
	virtual void InitAlienData();
	virtual void UpdateHordeMode();
	virtual void RandomizeHealth(int alienNum);	//Randomize a specific alien type's health.
	virtual void RandomizeHealth();	//Randomize all alien healths.
	
};

CASW_Horde_Mode* ASWHordeMode();