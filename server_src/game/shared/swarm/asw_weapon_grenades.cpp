#include "cbase.h"
#include "asw_weapon_grenades.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#include "c_asw_player.h"
#include "c_asw_weapon.h"
#include "c_asw_marine.h"
#include "prediction.h"
#else
#include "asw_marine.h"
#include "asw_player.h"
#include "asw_weapon.h"
#include "npcevent.h"
#include "shot_manipulator.h"
#include "asw_grenade_cluster.h"
#include "asw_marine_speech.h"
#include "asw_gamerules.h"
#endif
#include "asw_marine_skills.h"
#include "asw_util_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define ASW_FLARES_FASTEST_REFIRE_TIME		0.1f

ConVar asw_grenade_throw_delay("asw_grenade_throw_delay", "0.15", FCVAR_REPLICATED | FCVAR_CHEAT, "Delay before grenade entity is spawned when throwing");
//softcopy:
ConVar asw_grenade_hand_throw_damage("asw_grenade_hand_throw_damage", "0", FCVAR_CHEAT, "Adjust the Damage of hand grenade");
ConVar asw_grenade_hand_throw_radius("asw_grenade_hand_throw_radius", "0", FCVAR_CHEAT, "Adjust the damage radius of hand grenade");

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Grenades, DT_ASW_Weapon_Grenades )

BEGIN_NETWORK_TABLE( CASW_Weapon_Grenades, DT_ASW_Weapon_Grenades )
#ifdef CLIENT_DLL
	// recvprops
#else
	// sendprops
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CASW_Weapon_Grenades )
	
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( asw_weapon_grenades, CASW_Weapon_Grenades );
PRECACHE_WEAPON_REGISTER(asw_weapon_grenades);

#ifndef CLIENT_DLL
extern ConVar asw_debug_marine_damage;
//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CASW_Weapon_Grenades )
	DEFINE_FIELD( m_flSoonestPrimaryAttack, FIELD_TIME ),	
END_DATADESC()

#endif /* not client */

CASW_Weapon_Grenades::CASW_Weapon_Grenades()
{
	m_fMinRange1	= 0;
	m_fMaxRange1	= 2048;

	m_fMinRange2	= 256;
	m_fMaxRange2	= 1024;

	m_flSoonestPrimaryAttack = gpGlobals->curtime;
}


CASW_Weapon_Grenades::~CASW_Weapon_Grenades()
{

}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CASW_Weapon_Grenades::GetPrimaryAttackActivity( void )
{
	return ACT_VM_PRIMARYATTACK;
}

bool CASW_Weapon_Grenades::OffhandActivate()
{
	if (!GetMarine() || GetMarine()->GetFlags() & FL_FROZEN)	// don't allow this if the marine is frozen
		return false;
	PrimaryAttack();

	return true;
}

#define FLARE_PROJECTILE_AIR_VELOCITY	400

void CASW_Weapon_Grenades::PrimaryAttack( void )
{	
	CASW_Player *pPlayer = GetCommander();

	if (!pPlayer)
		return;

	CASW_Marine *pMarine = GetMarine();
	bool bThisActive = (pMarine && pMarine->GetActiveWeapon() == this);

	// grenade weapon is lost when all grenades are gone
	if ( UsesClipsForAmmo1() && !m_iClip1 ) 
	{		
#ifndef CLIENT_DLL
		if (pMarine)
		{
			pMarine->Weapon_Detach(this);
			if (bThisActive)
				pMarine->SwitchToNextBestWeapon(NULL);
		}
		Kill();
#endif
		return;
	}

	if (pMarine && gpGlobals->curtime > m_flDelayedFire)		// firing from a marine
	{
#ifdef CLIENT_DLL
		if ( !prediction->InPrediction() || prediction->IsFirstTimePredicted() )
		{
			pMarine->DoAnimationEvent( PLAYERANIMEVENT_THROW_GRENADE );
		}
#else
		pMarine->DoAnimationEvent( PLAYERANIMEVENT_THROW_GRENADE );
#endif

		// start our delayed attack
		m_bShotDelayed = true;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flDelayedFire = gpGlobals->curtime + asw_grenade_throw_delay.GetFloat();
		if (!bThisActive && pMarine->GetActiveASWWeapon())
		{
			// if we're offhan activating, make sure our primary weapon can't fire until we're done
			pMarine->GetActiveASWWeapon()->m_flNextPrimaryAttack = m_flNextPrimaryAttack  + 0.4f;
			pMarine->GetActiveASWWeapon()->m_bIsFiring = false;
		}
	}
}


#ifndef CLIENT_DLL
float CASW_Weapon_Grenades::GetBoomDamage( CASW_Marine *pMarine )
{
	return MarineSkills()->GetSkillBasedValueByMarine(pMarine, ASW_MARINE_SKILL_GRENADES, ASW_MARINE_SUBSKILL_GRENADE_CLUSTER_DMG);
}

float CASW_Weapon_Grenades::GetBoomRadius( CASW_Marine *pMarine )
{
	return MarineSkills()->GetSkillBasedValueByMarine(pMarine, ASW_MARINE_SKILL_GRENADES, ASW_MARINE_SUBSKILL_GRENADE_RADIUS);
}

#endif

void CASW_Weapon_Grenades::DelayedAttack( void )
{
	m_bShotDelayed = false;
	
	CASW_Player *pPlayer = GetCommander();
	if ( !pPlayer )
		return;

	CASW_Marine *pMarine = GetMarine();
	if ( !pMarine || pMarine->GetWaterLevel() == 3 )
		return;

#ifndef CLIENT_DLL
	Vector vecSrc = pMarine->GetOffhandThrowSource();

	Vector vecDest = pPlayer->GetCrosshairTracePos();
	if ( !pMarine->IsInhabited() )
	{
		vecDest = pMarine->GetOffhandItemSpot();
	}
	Vector newVel = UTIL_LaunchVector( vecSrc, vecDest, GetThrowGravity() ) * 28.0f;

	float fGrenadeRadius = GetBoomRadius( pMarine );
	float fGrenadeDamage = GetBoomDamage( pMarine );
	
	//softcopy:
	fGrenadeRadius += asw_grenade_hand_throw_radius.GetFloat();
	fGrenadeDamage += asw_grenade_hand_throw_damage.GetFloat();

	int iClusters = MarineSkills()->GetSkillBasedValueByMarine(pMarine, ASW_MARINE_SKILL_GRENADES, ASW_MARINE_SUBSKILL_GRENADE_CLUSTERS);
	if (asw_debug_marine_damage.GetBool())
	{
		Msg("Grenade damage = %f radius = %f clusters = %d\n", fGrenadeDamage, fGrenadeRadius, iClusters);
	}
	pMarine->GetMarineSpeech()->Chatter(CHATTER_GRENADE);


	if (ASWGameRules())
		ASWGameRules()->m_fLastFireTime = gpGlobals->curtime;

	CASW_Grenade_Cluster *pGrenade = CASW_Grenade_Cluster::Cluster_Grenade_Create( 
		fGrenadeDamage,
		fGrenadeRadius,
		iClusters,
		vecSrc, pMarine->EyeAngles(), newVel, AngularImpulse(0,0,0), pMarine, this );

	if ( pGrenade )
	{
		pGrenade->SetGravity( GetThrowGravity() );
	}
#endif
	// decrement ammo
	m_iClip1 -= 1;

#ifndef CLIENT_DLL
	DestroyIfEmpty( true );
	pMarine->OnWeaponFired( this, 1 );
#endif

	m_flSoonestPrimaryAttack = gpGlobals->curtime + ASW_FLARES_FASTEST_REFIRE_TIME;
	if (m_iClip1 > 0)		// only force the fire wait time if we have ammo for another shot
		m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
	else
		m_flNextPrimaryAttack = gpGlobals->curtime;
}

void CASW_Weapon_Grenades::Precache()
{	
	BaseClass::Precache();	

#ifndef CLIENT_DLL
	//UTIL_PrecacheOther( "asw_flare_projectile" );
#endif
}

// flares don't reload
bool CASW_Weapon_Grenades::Reload()
{
	return false;
}

void CASW_Weapon_Grenades::ItemPostFrame( void )
{	
	BaseClass::ItemPostFrame();

	if ( m_bInReload )
		return;
	
	CBasePlayer *pOwner = GetCommander();

	if ( pOwner == NULL )
		return;

	//Allow a refire as fast as the player can click
	if ( ( ( pOwner->m_nButtons & IN_ATTACK ) == false ) && ( m_flSoonestPrimaryAttack < gpGlobals->curtime ) )
	{
		m_flNextPrimaryAttack = gpGlobals->curtime - 0.1f;
	}
}

int CASW_Weapon_Grenades::ASW_SelectWeaponActivity(int idealActivity)
{
	// we just use the normal 'no weapon' anims for this
	return idealActivity;
}