#include "cbase.h"
#include "asw_boomer.h"
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
#include "gib.h"
#include "te_effect_dispatch.h"
#include "asw_ai_behavior.h"
#include "props_shared.h"
#include "asw_player.h"
#include "asw_achievements.h"
#include "asw_marine_resource.h"
#include "asw_barrel_explosive.h"	//softcopy:


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( asw_boomer, CASW_Boomer );

IMPLEMENT_SERVERCLASS_ST( CASW_Boomer, DT_ASW_Boomer )
	SendPropBool( SENDINFO( m_bInflated ) ),
END_SEND_TABLE()

BEGIN_DATADESC( CASW_Boomer )
	DEFINE_EMBEDDEDBYREF( m_pExpresser ),
	DEFINE_FIELD( m_bInflating, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_fLastTouchHurtTime, FIELD_TIME ),	//softcopy:
END_DATADESC()

ConVar asw_boomer_health( "asw_boomer_health", "800", FCVAR_CHEAT );
ConVar asw_boomer_inflate_speed( "asw_boomer_inflate_speed", "6.0f", FCVAR_CHEAT );
ConVar asw_boomer_inflate_debug( "asw_boomer_inflate_debug", "1.0f", FCVAR_CHEAT );

ConVar asw_boomer_color("asw_boomer_color", "255 255 255", FCVAR_NONE, "Sets the color of boomers.");
//softcopy:
ConVar asw_boomer_color2("asw_boomer_color2", "255 255 255", FCVAR_NONE, "Sets the color of boomers.");
ConVar asw_boomer_color2_percent("asw_boomer_color2_percent", "0.0", FCVAR_NONE, "Sets the percentage of the boomers you want to give the color",true,0,true,1);
ConVar asw_boomer_color3("asw_boomer_color3", "255 255 255", FCVAR_NONE, "Sets the color of boomers.");
ConVar asw_boomer_color3_percent("asw_boomer_color3_percent", "0.0", FCVAR_NONE, "Sets the percentage of the boomers you want to give the color",true,0,true,1);
ConVar asw_boomer_scalemod("asw_boomer_scalemod", "0.0", FCVAR_NONE, "Sets the scale of normal new model parasites.");
ConVar asw_boomer_scalemod_percent("asw_boomer_scalemod_percent", "0.0", FCVAR_NONE, "Sets the percentage of the normal boomers you want to scale.",true,0,true,1);
ConVar asw_boomer_explode_range( "asw_boomer_explode_range", "200", FCVAR_CHEAT, "Sets boomer explode range." );
ConVar asw_boomer_max_projectile( "asw_boomer_max_projectile", "8", FCVAR_CHEAT, "Sets boomer max projectile." );
ConVar asw_boomer_explode_damage( "asw_boomer_explode_damage", "55", FCVAR_CHEAT, "Sets boomer explode damage." );
ConVar asw_boomer_explode_radius( "asw_boomer_explode_radius", "240", FCVAR_CHEAT, "Sets boomer explode radius." );
ConVar asw_boomer_melee_range( "asw_boomer_melee_range", "140", FCVAR_CHEAT, "Sets boomer melee range." );
ConVar asw_boomer_melee_min_damage( "asw_boomer_melee_min_damage", "4", FCVAR_CHEAT, "Sets boomer melee min damage." );
ConVar asw_boomer_melee_max_damage( "asw_boomer_melee_max_damage", "6", FCVAR_CHEAT, "Sets boomer melee max damage." );
ConVar asw_boomer_melee_force( "asw_boomer_melee_force", "4", FCVAR_CHEAT, "Sets boomer melee force." );
ConVar asw_boomer_touch_damage( "asw_boomer_touch_damage", "5", FCVAR_CHEAT, "Sets damage caused by boomer on touch." );
ConVar asw_boomer_ignite("asw_boomer_ignite", "0", FCVAR_CHEAT, "Sets 1=melee, 2=touch, 3=All, ignite marine on boomer melee/touch.");
ConVar asw_boomer_explode("asw_boomer_explode", "0", FCVAR_CHEAT, "Sets 1=melee, 2=touch, 3=All, explode marine on boomer melee/touch.");
ConVar asw_boomer_touch_onfire("asw_boomer_touch_onfire", "0", FCVAR_CHEAT, "Ignite marine if boomer body on fire touch.");
extern ConVar asw_debug_alien_ignite;

extern ConVar asw_alien_debug_death_style;

extern ConVar asw_debug_alien_damage;

int ACT_RUN_INFLATED;
int ACT_IDLE_INFLATED;
int ACT_MELEE_ATTACK1_INFLATED;
int ACT_DEATH_FIRE_INFLATED;   
int AE_BOOMER_INFLATED;

#define ASW_BOOMER_MELEE_RANGE asw_boomer_melee_range.GetFloat()	//softcopy:

CASW_Boomer::CASW_Boomer()
{
	m_pszAlienModelName = "models/aliens/boomer/boomer.mdl";
	m_bInflated = false;
	m_fLastTouchHurtTime = 0;	//softcopy:
}

//-----------------------------------------------------------------------------
// Purpose:	
// Input:	
// Output:	
//-----------------------------------------------------------------------------
void CASW_Boomer::Spawn( void )
{
	SetHullType( HULL_LARGE );

	BaseClass::Spawn();

	SetHullType( HULL_LARGE );
	SetCollisionGroup( ASW_COLLISION_GROUP_ALIEN );
	SetHealthByDifficultyLevel();
	CapabilitiesAdd( bits_CAP_MOVE_GROUND | bits_CAP_INNATE_MELEE_ATTACK1 | bits_CAP_INNATE_MELEE_ATTACK2 );
	
// 			"Health"	"435"
// 			"WalkSpeed"	"45"
// 			"RunSpeed"	"254"

	SetIdealState( NPC_STATE_ALERT );

	m_bNeverRagdoll = true;

	//softcopy: 
	//SetRenderColor(asw_boomer_color.GetColor().r(), asw_boomer_color.GetColor().g(), asw_boomer_color.GetColor().b());		//Ch1ckensCoop: Allow setting colors.
	alienLabel = "boomer";
	SetColorScale( alienLabel );

}

void CASW_Boomer::SetHealthByDifficultyLevel()
{
	int iHealth = MAX( 25, ASWGameRules()->ModifyAlienHealthBySkillLevel( asw_boomer_health.GetInt() ) );
	if ( asw_debug_alien_damage.GetBool() )
		Msg( "Setting boomer's initial health to %d\n", iHealth );
	SetHealth( iHealth );
	SetMaxHealth( iHealth );
}

//-----------------------------------------------------------------------------
// Purpose:	
// Input:	
// Output:	
//-----------------------------------------------------------------------------
void CASW_Boomer::Precache( void )
{
	PrecacheParticleSystem ( "boomer_explode" );
	PrecacheParticleSystem ( "joint_goo" );
	PrecacheModel( "models/aliens/boomer/boomerLegA.mdl");
	PrecacheModel( "models/aliens/boomer/boomerLegB.mdl");
	PrecacheModel( "models/aliens/boomer/boomerLegC.mdl");
	PrecacheScriptSound( "ASW_Boomer.Death_Explode" );
	PrecacheScriptSound( "ASW_Boomer.Death_Gib" );
	//softcopy:
	PrecacheScriptSound( "ASW_T75.Explode" );
	PrecacheParticleSystem( "explosion_barrel" );
	
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:	
// Input:	
// Output:	
//-----------------------------------------------------------------------------
float CASW_Boomer::MaxYawSpeed( void )
{
	if ( GetActivity() == ACT_STRAFE_LEFT || GetActivity() == ACT_STRAFE_RIGHT )
	{
		return 0.0f;
	}

	return 32.0f;// * GetMovementSpeedModifier();
}

//-----------------------------------------------------------------------------
// Purpose:	
// Input:	
// Output:	
//-----------------------------------------------------------------------------
void CASW_Boomer::HandleAnimEvent( animevent_t *pEvent )
{
	//softcopy: ignite/explode marine by boomer melee attack
	if ( GetActivity() == ACT_MELEE_ATTACK1 ) 
	{
		float damage =  MAX(3.0f, ASWGameRules()->ModifyAlienDamageBySkillLevel(asw_boomer_melee_min_damage.GetFloat()));
		MeleeAttack(ASW_BOOMER_MELEE_RANGE, damage);
	}

	int nEvent = pEvent->Event();
	if ( nEvent == AE_BOOMER_INFLATED )
	{
		m_bInflated = true;
		return;
	}

	BaseClass::HandleAnimEvent( pEvent );
}

void CASW_Boomer::DeathSound( const CTakeDamageInfo &info )
{
	// if we are playing a fancy death animation, don't play death sounds from code
	// all death sounds are played from anim events inside the fancy death animation
	if ( m_nDeathStyle == kDIE_FANCY )
		return;

	if ( m_nDeathStyle == kDIE_INSTAGIB )
		EmitSound( "ASW_Boomer.Death_Explode" );
	else
		EmitSound( "ASW_Boomer.Death_Gib" );

}

int CASW_Boomer::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	CTakeDamageInfo infoNew( info );

	if ( infoNew.GetAttacker() && info.GetAttacker()->Classify() == CLASS_ASW_MARINE )
	{
		EHANDLE hAttacker = infoNew.GetAttacker();
		if ( m_hMarineAttackers.Find( hAttacker ) == m_hMarineAttackers.InvalidIndex() )
		{
			m_hMarineAttackers.AddToTail( hAttacker );
		}

		if ( infoNew.GetAttacker()->WorldSpaceCenter().DistTo( WorldSpaceCenter() ) < 40 )
		{
			// Stuck inside! Kill it good!
			infoNew.ScaleDamage( 8 );
		}
	}
	return BaseClass::OnTakeDamage_Alive( infoNew );
}

//-----------------------------------------------------------------------------
// Purpose:	
// Input:	
// Output:	
//-----------------------------------------------------------------------------
void CASW_Boomer::Event_Killed( const CTakeDamageInfo &info )
{
	SendBehaviorEvent( info.GetAttacker(), BEHAVIOR_EVENT_EXPLODE, 1, false );

	BaseClass::Event_Killed( info );

	if ( m_bInflated )
	{
		m_nDeathStyle = kDIE_INSTAGIB;
	}
	else
	{
		if ( m_bOnFire )
		{
			m_nDeathStyle = kDIE_FANCY;
		}
		else
		{
			m_nDeathStyle = kDIE_BREAKABLE;
		}

		for ( int i = 0; i < m_hMarineAttackers.Count(); i++ )
		{
			CASW_Marine *pMarine = dynamic_cast<CASW_Marine*>( m_hMarineAttackers[i].Get() );
			if ( pMarine && pMarine->IsInhabited() && pMarine->GetCommander() )
			{
				pMarine->GetCommander()->AwardAchievement( ACHIEVEMENT_ASW_BOOMER_KILL_EARLY );
				if ( pMarine->GetMarineResource() )
				{
					pMarine->GetMarineResource()->m_bKilledBoomerEarly = true;
				}
			}
		}
	}

	if ( asw_alien_debug_death_style.GetBool() )
		Msg( "CASW_Boomer::Event_Killed: m_nDeathStyle = %d\n", m_nDeathStyle );
}

bool CASW_Boomer::CanDoFancyDeath()
{
	if ( m_bInflated )
		return false;
	else
		return BaseClass::CanDoFancyDeath();
}

//softcopy: ignited/explode marine by boomer on touch/on fire touch, 1=melee, 2=touch, 3=All
void CASW_Boomer::StartTouch( CBaseEntity *pOther )
{
	BaseClass::StartTouch( pOther );
	CASW_Marine *pMarine = CASW_Marine::AsMarine( pOther );
	if ( pMarine )
	{
		m_TouchExplosionDamage = asw_boomer_touch_damage.GetInt();
		CTakeDamageInfo info( this, this, m_TouchExplosionDamage, DMG_SLASH );
		damageTypes = "on touch";
		if (asw_boomer_ignite.GetInt() >= 2 || (m_bOnFire && asw_boomer_touch_onfire.GetBool()))
			MarineIgnite(pMarine, info, alienLabel, damageTypes);
		if ( m_fLastTouchHurtTime + 0.35f /*0.6f*/ > gpGlobals->curtime || m_TouchExplosionDamage <=0 )	//don't hurt him if he was hurt recently
			return;
		Vector vecForceDir = ( pMarine->GetAbsOrigin() - GetAbsOrigin() );	// hurt the marine
		CalculateMeleeDamageForce( &info, vecForceDir, pMarine->GetAbsOrigin() );
		pMarine->TakeDamage( info );
		if (asw_boomer_explode.GetInt() >= 2 )
			MarineExplode(pMarine, alienLabel, damageTypes);

		m_fLastTouchHurtTime = gpGlobals->curtime;
	}
}
//softcopy: ignite/explode marine by boomer melee attack, 1=melee, 2=touch, 3=All
void CASW_Boomer::MeleeAttack(float distance, float damage)
{
	CBaseEntity *pHurt = CheckTraceHullAttack( distance, -Vector(16,16,32), Vector(16,16,32), damage, DMG_SLASH, asw_boomer_melee_force.GetFloat() );
	if ( pHurt )
	{
		CASW_Marine *pMarine = CASW_Marine::AsMarine( pHurt );
		if ( pMarine )
		{
			CTakeDamageInfo info( this, this, damage, DMG_SLASH );
			damageTypes = "melee attack";
			if (asw_boomer_ignite.GetInt() == 1 || asw_boomer_ignite.GetInt() == 3)
				MarineIgnite(pMarine, info, alienLabel, damageTypes);

			if ((asw_boomer_explode.GetInt()==1 || asw_boomer_explode.GetInt()==3) && asw_boomer_touch_damage.GetInt() > 0)
				MarineExplode(pMarine, alienLabel, damageTypes);
		}
	}
}
//softcopy:
void CASW_Boomer::SetColorScale(const char *alienLabel)
{
	BaseClass::SetColorScale(alienLabel);
}
//softcopy:
void CASW_Boomer::MarineIgnite(CBaseEntity *pOther, const CTakeDamageInfo &info, const char *alienLabel, const char *damageTypes)
{
	BaseClass::MarineIgnite(pOther, info, alienLabel, damageTypes);
}
//softcopy:
void CASW_Boomer::MarineExplode(CBaseEntity *pMarine, const char *alienLabel, const char *damageTypes)
{
	BaseClass::MarineExplode(pMarine, alienLabel, damageTypes);
}

//-----------------------------------------------------------------------------
// Purpose:	
// Input:	
// Output:	
//-----------------------------------------------------------------------------
bool CASW_Boomer::CorpseGib( const CTakeDamageInfo &info )
{
	CEffectData	data;

	m_LagCompensation.UndoLaggedPosition();

	data.m_vOrigin = WorldSpaceCenter();
	data.m_vNormal = data.m_vOrigin - info.GetDamagePosition();
	VectorNormalize( data.m_vNormal );

	data.m_flScale = RemapVal( m_iHealth, 0, 3, 0.5f, 2 );
	data.m_nColor = m_nSkin;
	data.m_fFlags = IsOnFire() ? ASW_GIBFLAG_ON_FIRE : 0;

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:	
// Input:	
// Output:	
//-----------------------------------------------------------------------------
int CASW_Boomer::SelectDeadSchedule()
{
	if ( m_lifeState == LIFE_DEAD )
	{
		return SCHED_NONE;
	}

	CleanupOnDeath();
	return SCHED_DIE;
}


//-----------------------------------------------------------------------------
// Purpose:	
// Input:	
// Output:	
//-----------------------------------------------------------------------------
void CASW_Boomer::BuildScheduleTestBits()
{

	// Ignore damage if we were recently damaged or we're attacking.
	if ( GetActivity() == ACT_MELEE_ATTACK1 )
	{
		ClearCustomInterruptCondition( COND_LIGHT_DAMAGE );
		ClearCustomInterruptCondition( COND_HEAVY_DAMAGE );
	}

	BaseClass::BuildScheduleTestBits();
}


//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------
AI_BEGIN_CUSTOM_NPC( asw_boomer, CASW_Boomer )
	DECLARE_ACTIVITY( ACT_RUN_INFLATED )
	DECLARE_ACTIVITY( ACT_IDLE_INFLATED );
	DECLARE_ACTIVITY( ACT_MELEE_ATTACK1_INFLATED );
	DECLARE_ACTIVITY( ACT_DEATH_FIRE_INFLATED );
	DECLARE_ANIMEVENT( AE_BOOMER_INFLATED )
AI_END_CUSTOM_NPC()

bool CASW_Boomer::CreateBehaviors()
{
	m_ExplodeBehavior.KeyValue( "schedule_chance", "40" );
	m_ExplodeBehavior.KeyValue( "schedule_chance_rate", "1" );
	//softcopy: explode range, max projectiles cvar
	//m_ExplodeBehavior.KeyValue( "range", "200" );
	//m_ExplodeBehavior.KeyValue( "max_projectiles", "8" );
	m_ExplodeBehavior.KeyValue( "range", asw_boomer_explode_range.GetString() );
	m_ExplodeBehavior.KeyValue( "max_projectiles",asw_boomer_max_projectile.GetString() );
	
	m_ExplodeBehavior.KeyValue( "max_buildup_time", "3" );
	m_ExplodeBehavior.KeyValue( "min_velocity", "150" );
	m_ExplodeBehavior.KeyValue( "max_velocity", "450" );
	m_ExplodeBehavior.KeyValue( "attach_name", "sack_" );
	m_ExplodeBehavior.KeyValue( "attach_count", "12" );
	//softcopy: explode damage, radius cvar
	//m_ExplodeBehavior.KeyValue( "damage", "55" );
	//m_ExplodeBehavior.KeyValue( "radius", "240" );
	m_ExplodeBehavior.KeyValue( "damage", asw_boomer_explode_damage.GetString() );
	m_ExplodeBehavior.KeyValue( "radius", asw_boomer_explode_radius.GetString() );

	AddBehavior( &m_ExplodeBehavior );
	m_ExplodeBehavior.Init();

	//softcopy: melee cvar
	//m_MeleeBehavior.KeyValue( "range", "140" );
	//m_MeleeBehavior.KeyValue( "min_damage", "4" );
	//m_MeleeBehavior.KeyValue( "max_damage", "6" );
	//m_MeleeBehavior.KeyValue( "force", "4" );
	m_MeleeBehavior.KeyValue( "range", asw_boomer_melee_range.GetString() );
	m_MeleeBehavior.KeyValue( "min_damage", asw_boomer_melee_min_damage.GetString() );
	m_MeleeBehavior.KeyValue( "max_damage", asw_boomer_melee_max_damage.GetString()  );
	m_MeleeBehavior.KeyValue( "force", asw_boomer_melee_force.GetString() );
	
	AddBehavior( &m_MeleeBehavior );
	m_MeleeBehavior.Init();
	//softcopy: increase boomer chase distance
	//m_ChaseEnemyBehavior.KeyValue( "chase_distance", "600" );
	m_ChaseEnemyBehavior.KeyValue( "chase_distance", "1100" );
	AddBehavior( &m_ChaseEnemyBehavior );
	m_ChaseEnemyBehavior.Init();

	return BaseClass::CreateBehaviors();
}

Activity CASW_Boomer::NPC_TranslateActivity( Activity baseAct )
{
	Activity translated = BaseClass::NPC_TranslateActivity( baseAct );

	if ( m_bInflating )
	{
		if ( translated == ACT_RUN ) return (Activity) ACT_RUN_INFLATED;
		if ( translated == ACT_IDLE ) return (Activity) ACT_IDLE_INFLATED;
		if ( translated == ACT_MELEE_ATTACK1 ) return (Activity) ACT_MELEE_ATTACK1_INFLATED;
		if ( translated == ACT_DEATH_FIRE ) return (Activity) ACT_DEATH_FIRE_INFLATED;
	}

	return translated;
}