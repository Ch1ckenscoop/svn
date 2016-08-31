#ifndef _INCLUDED_ASW_DRONE_UBER_H
#define _INCLUDED_ASW_DRONE_UBER_H
#ifdef _WIN32
#pragma once
#endif

#include "asw_drone_advanced.h"

class CASW_Drone_Uber : public CASW_Drone_Advanced
{
public:
	DECLARE_CLASS( CASW_Drone_Uber, CASW_Drone_Advanced  );	
	DECLARE_DATADESC();

	CASW_Drone_Uber( void );
	virtual ~CASW_Drone_Uber( void );
	
	virtual Class_T		Classify( void ) { return (Class_T) CLASS_ASW_DRONE_UBER; }	//Ch1ckensCoop: Statistics reporting (don't report this as a drone, but as an uber drone instead.)
	virtual void Spawn();
	virtual void Precache();
	virtual bool ShouldMoveSlow() const;
	virtual void SetHealthByDifficultyLevel();
	virtual float GetIdealSpeed() const;
	virtual int OnTakeDamage_Alive( const CTakeDamageInfo &info );
	virtual bool ModifyAutoMovement( Vector &vecNewPos );
	//softcopy: 
	virtual void StartTouch( CBaseEntity *pOther );
	const char	*alienLabel, *damageTypes;
	virtual void MarineIgnite(CBaseEntity *pOther, const CTakeDamageInfo &info, const char *alienLabel, const char *damageTypes)
				{return	BaseClass::MarineIgnite(pOther, info, alienLabel, damageTypes);}
	virtual void SetColorScale(const char *alienLabel) {return BaseClass::SetColorScale(alienLabel);}

private:
	float GetDamage();	//Ch1ckensCoop: easy modification of alien damages.

};

#endif // _INCLUDED_ASW_DRONE_UBER_H
