#pragma once
#include "asw_spawn_manager.h"

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

	//AlienInfo[HIGHEST_CLASS_ENTRY - 1];

	struct AlienInfo
	{
		const char *pAlienClassName;
		int flag;
		int max;
		int min;
	} AlienInfoArray [HIGHEST_CLASS_ENTRY - 1];

};
