#pragma once
//Ch1ckensCoop: Replacement for spamming medkits.
#include "asw_colonist.h"	//softcopy:

class CASW_Health_Regen : public CLogicalEntity
{
public:
	DECLARE_CLASS( CASW_Health_Regen, CLogicalEntity );
	CASW_Health_Regen(void);
	~CASW_Health_Regen(void);
	virtual void Spawn();
	virtual void Think();

private:
	void SetMarineHealth(CASW_Marine *pMarine, int iHealth);
	void SetColonistHealth(CASW_Colonist *pColonist, int iHealth);	//softcopy:
};
