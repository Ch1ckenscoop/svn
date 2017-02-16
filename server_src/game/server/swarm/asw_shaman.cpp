#include "cbase.h"
#include "asw_shaman.h"
#include "npcevent.h"
#include "asw_gamerules.h"
#include "asw_shareddefs.h"
#include "asw_fx_shared.h"
#include "asw_grenade_cluster.h"
#include "world.h"
#include "particle_parse.h"
#include "asw_util_shared.h"
#include "ai_squad.h"
#include "asw_marine.h"
#include "asw_ai_behavior_fear.h"
#include "gib.h"
#include "te_effect_dispatch.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( asw_shaman, CASW_Shaman );

IMPLEMENT_SERVERCLASS_ST(CASW_Shaman, DT_ASW_Shaman)
	SendPropEHandle		( SENDINFO( m_hHealingTarget ) ),
END_SEND_TABLE()


BEGIN_DATADESC( CASW_Shaman )
DEFINE_EMBEDDEDBYREF( m_pExpresser ),
DEFINE_FIELD( m_fLastTouchHurtTime, FIELD_TIME ),	//softcopy:
END_DATADESC()
 
int AE_SHAMAN_SPRAY_START;
int AE_SHAMAN_SPRAY_END;

ConVar asw_shaman_health( "asw_shaman_health", "60", FCVAR_CHEAT );
//softcopy:
ConVar asw_shaman_color("asw_shaman_color", "255 255 255", FCVAR_NONE, "Sets the color of shaman.");
ConVar asw_shaman_color2("asw_shaman_color2", "255 255 255", FCVAR_NONE, "Sets the color of shamans.");
ConVar asw_shaman_color2_percent("asw_shaman_color2_percent", "0.0", FCVAR_NONE, "Sets the percentage of the shamans you want to give the color",true,0,true,1);
ConVar asw_shaman_color3("asw_shaman_color3", "255 255 255", FCVAR_NONE, "Sets the color of shamans.");
ConVar asw_shaman_color3_percent("asw_shaman_color3_percent", "0.0", FCVAR_NONE, "Sets the percentage of the shamans you want to give the color",true,0,true,1);
ConVar asw_shaman_scalemod("asw_shaman_scalemod", "0.0", FCVAR_NONE, "Sets the scale of normal shamans.",true,0,true,3);
ConVar asw_shaman_scalemod_percent("asw_shaman_scalemod_percent", "0.0", FCVAR_NONE, "Sets the percentage of the shamans you want to scale.",true,0,true,1);
ConVar asw_shaman_ignite("asw_shaman_ignite", "0", FCVAR_CHEAT, "Ignites marine by shaman on touch.");
ConVar asw_shaman_gib_chance("asw_shaman_gib_chance", "0.80", FCVAR_CHEAT, "Chance of shaman break into ragdoll pieces instead of ragdoll.");
ConVar asw_shaman_touch_onfire("asw_shaman_touch_onfire", "0", FCVAR_CHEAT, "Ignites marine if shaman body on fire touch.");

extern ConVar asw_debug_alien_damage;

//-----------------------------------------------------------------------------
// Purpose:	
// Input:	
// Output:	
//-----------------------------------------------------------------------------
CASW_Shaman::CASW_Shaman()
{
	m_pszAlienModelName = "models/aliens/shaman/shaman.mdl";
	m_fLastTouchHurtTime = 0;	//softcopy:
}


//-----------------------------------------------------------------------------
// Purpose:	
// Input:	
// Output:	
//-----------------------------------------------------------------------------
void CASW_Shaman::Spawn( void )
{
	SetHullType( HULL_MEDIUM );

	BaseClass::Spawn();

	//softcopy: set colors for shaman
	alienLabel = "shaman";
	ASWGameRules()->SetColorScale( this,  alienLabel );

	SetHullType( HULL_MEDIUM );
	SetHealthByDifficultyLevel();
	SetBloodColor( BLOOD_COLOR_GREEN );
	CapabilitiesAdd( bits_CAP_MOVE_GROUND | bits_CAP_AUTO_DOORS );

	AddFactionRelationship( FACTION_MARINES, D_FEAR, 10 );

	SetIdealState( NPC_STATE_ALERT );
	m_bNeverRagdoll = true;

	SetCollisionGroup( ASW_COLLISION_GROUP_PARASITE );

}

//-----------------------------------------------------------------------------
// Purpose:	
// Input:	
// Output:	
//-----------------------------------------------------------------------------
void CASW_Shaman::Precache( void )
{
	BaseClass::Precache();

	PrecacheModel(m_pszAlienModelName);
    //softcopy: sound doesn't exist & add more sound effect
	//PrecacheScriptSound( "Shaman.Pain" );     
	//PrecacheScriptSound( "Shaman.Die" );      
	PrecacheScriptSound( "ASW_Drone.DeathFireSizzle" );
	PrecacheScriptSound( "Ranger.GibSplatHeavy" );
	PrecacheScriptSound( "ASW_Parasite.Pain" );

}


//-----------------------------------------------------------------------------
// Purpose:	
// Input:	
// Output:	
//-----------------------------------------------------------------------------
void CASW_Shaman::SetHealthByDifficultyLevel()
{
	//softcopy: Ch1ckensCoop: NOOOOO!!!! I want my control.
	//int iHealth = MAX( 25, ASWGameRules()->ModifyAlienHealthBySkillLevel( asw_shaman_health.GetInt() ) );
	int iHealth = ASWGameRules()->ModifyAlienHealthBySkillLevel( asw_shaman_health.GetInt() );	
	if ( asw_debug_alien_damage.GetBool() )
		Msg( "Setting shaman's initial health to %d\n", iHealth );
	SetHealth( iHealth );
	SetMaxHealth( iHealth );
}


#if 0
//-----------------------------------------------------------------------------
// Purpose: A scalar to apply to base (walk/run) speeds.
//-----------------------------------------------------------------------------
float CASW_Shaman::GetMovementSpeedModifier()
{
	// don't like the way this is done, but haven't thought of a better approach yet
	if ( IsRunningBehavior() && static_cast< CAI_ASW_Behavior * >(  GetPrimaryBehavior() )->Classify() == BEHAVIOR_CLASS_FEAR )
	{
		return ASW_CONCAT_SPEED_ADD( 0.55f );
	}

	return BaseClass::GetMovementSpeedModifier();
}
#endif

//-----------------------------------------------------------------------------
// Purpose:	
// Input:	
// Output:	
//-----------------------------------------------------------------------------
float CASW_Shaman::MaxYawSpeed( void )
{
	return 32.0f;// * GetMovementSpeedModifier();
}


//-----------------------------------------------------------------------------
// Purpose:	
// Input:	
// Output:	
//-----------------------------------------------------------------------------
void CASW_Shaman::PainSound( const CTakeDamageInfo &info )
{	
	// sounds for pain and death are defined in the npc_tier_tables excel sheet
	// they are called from the asw_alien base class (m_fNextPainSound is handled there)
	BaseClass::PainSound(info);
	//softcopy: sound more obvious.
	//m_fNextPainSound = gpGlobals->curtime + RandomFloat( 0.75f, 1.25f );
	EmitSound("ASW_Parasite.Pain");
	m_fNextPainSound = gpGlobals->curtime + RandomFloat( 1.0f, 1.5f );
}


//-----------------------------------------------------------------------------
// Purpose:	
// Input:	
// Output:	
//-----------------------------------------------------------------------------


void CASW_Shaman::DeathSound( const CTakeDamageInfo &info )
{
	//softcopy: add sound more notice obviously.
	// sounds for pain and death are defined in the npc_tier_tables excel sheet
	// they are called from the asw_alien base class
	//BaseClass::DeathSound(info);
	if ( m_nDeathStyle == kDIE_FANCY )
	    // if we are playing a fancy death animation, don't play death sounds from code
	    // all death sounds are played from anim events inside the fancy death animation    
	    return;
	EmitSound(m_bOnFire ? "ASW_Drone.DeathFireSizzle" : "Ranger.GibSplatHeavy");
}

//softcopy: event_killed for adding death animations.
void CASW_Shaman::Event_Killed( const CTakeDamageInfo &info )
{
	trace_t tr;
	UTIL_TraceLine( GetAbsOrigin() + Vector( 0, 0, 16 ), GetAbsOrigin() - Vector( 0, 0, 64 ), MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );
	UTIL_DecalTrace( &tr, "GreenBloodBig" );
	// add death animations
	CTakeDamageInfo newInfo(info);
	if (m_bElectroStunned )
			m_nDeathStyle = kDIE_INSTAGIB;
	else if (m_bOnFire)
			m_nDeathStyle = kDIE_FANCY;
	else if (newInfo.GetDamageType() & (DMG_BULLET | DMG_BUCKSHOT | DMG_ENERGYBEAM))
			m_nDeathStyle = kDIE_BREAKABLE;
	else if (newInfo.GetDamageType() & (DMG_BLAST | DMG_SONIC))
			m_nDeathStyle = RandomFloat() < asw_shaman_gib_chance.GetFloat() ? kDIE_BREAKABLE : kDIE_HURL;
	else 	m_nDeathStyle = kDIE_FANCY;
	
	BaseClass::Event_Killed(info);
}
//softcopy: //ignite marine on touch/on fire touch function
void CASW_Shaman::StartTouch( CBaseEntity *pOther )
{
	BaseClass::StartTouch( pOther );
	CASW_Marine *pMarine = CASW_Marine::AsMarine( pOther );
	if ( pMarine )
	{
		CTakeDamageInfo info( this, this, 4, DMG_GENERIC );
		if ( asw_shaman_ignite.GetBool() || (m_bOnFire && asw_shaman_touch_onfire.GetBool()) )
			ASWGameRules()->MarineIgnite(pMarine, info, alienLabel, /*damageTypes*/ "on touch");

		if (m_fLastTouchHurtTime + 0.35f /*0.6f*/ > gpGlobals->curtime)	// don't hurt him if he was hurt recently
			return;

		Vector vecForceDir = (pMarine->GetAbsOrigin() - GetAbsOrigin());	// hurt the marine
		CalculateMeleeDamageForce( &info, vecForceDir, pMarine->GetAbsOrigin() );
		pMarine->TakeDamage( info );

		m_fLastTouchHurtTime = gpGlobals->curtime;
	}
}

//-----------------------------------------------------------------------------
// Purpose:	
// Input:	
// Output:	
//-----------------------------------------------------------------------------
void CASW_Shaman::HandleAnimEvent( animevent_t *pEvent )
{
	int nEvent = pEvent->Event();

	if ( nEvent == AE_SHAMAN_SPRAY_START )
	{
		//m_HealOtherBehavior.HandleBehaviorEvent( this, BEHAVIOR_EVENT_START_HEAL, 0 );
		return;
	}
	if ( nEvent == AE_SHAMAN_SPRAY_END )
	{
		//m_HealOtherBehavior.HandleBehaviorEvent( this, BEHAVIOR_EVENT_FINISH_HEAL, 0 );
		return;
	}

	BaseClass::HandleAnimEvent( pEvent );
}


//-----------------------------------------------------------------------------
// Purpose:	
// Input:	
// Output:	
//-----------------------------------------------------------------------------
bool CASW_Shaman::CreateBehaviors()
{
	
	//AddBehavior( &m_CombatStunBehavior );
	//m_CombatStunBehavior.Init();
	

	//self.AddBehavior( "behavior_protect", BehaviorParms );


	m_HealOtherBehavior.KeyValue( "heal_distance", "300" );
	m_HealOtherBehavior.KeyValue( "approach_distance", "120" );
	m_HealOtherBehavior.KeyValue( "heal_amount", "0.04" );	// percentage per tick healed
	m_HealOtherBehavior.KeyValue( "consideration_distance", "800" );
	AddBehavior( &m_HealOtherBehavior );
	m_HealOtherBehavior.Init();


	m_ScuttleBehavior.KeyValue( "pack_range", "800" );
	m_ScuttleBehavior.KeyValue( "min_backoff", "150" );
	m_ScuttleBehavior.KeyValue( "max_backoff", "300" );
	m_ScuttleBehavior.KeyValue( "min_yaw", "10" );
	m_ScuttleBehavior.KeyValue( "max_yaw", "25" );
	m_ScuttleBehavior.KeyValue( "min_wait", "1.25" );
	m_ScuttleBehavior.KeyValue( "max_wait", "2.0" );
	AddBehavior( &m_ScuttleBehavior );
	m_ScuttleBehavior.Init();

	//AddBehavior( &m_FearBehavior );	
	//m_FearBehavior.Init();


	AddBehavior( &m_IdleBehavior );
	m_IdleBehavior.Init();

	return BaseClass::CreateBehaviors();
}

void CASW_Shaman::SetCurrentHealingTarget( CBaseEntity *pTarget )
{
	if ( pTarget != m_hHealingTarget.Get() )
	{
		m_hHealingTarget = pTarget;
	}
}

void CASW_Shaman::NPCThink( void )
{
	BaseClass::NPCThink();

	CBaseEntity *pHealTarget = NULL;
	if ( GetPrimaryBehavior() == &m_HealOtherBehavior )
	{
		pHealTarget = m_HealOtherBehavior.GetCurrentHealTarget();
		if ( pHealTarget )
		{
			pHealTarget->TakeHealth( m_HealOtherBehavior.m_flHealAmount * pHealTarget->GetMaxHealth(), DMG_GENERIC );
		}
	}
	SetCurrentHealingTarget( pHealTarget );
}

//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------
AI_BEGIN_CUSTOM_NPC( asw_shaman, CASW_Shaman )
	DECLARE_ANIMEVENT( AE_SHAMAN_SPRAY_START );
	DECLARE_ANIMEVENT( AE_SHAMAN_SPRAY_END );
AI_END_CUSTOM_NPC()

