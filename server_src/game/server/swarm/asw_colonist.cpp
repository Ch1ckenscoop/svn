///////////////////////////////////////////////////////////////////////////
//     added part of statments from asbi asw_colonist.cpp to play bi-rescue
//     modified colonist immune status
//     modified colonist sound types
//     added zombie style for fun.
///////////////////////////////////////////////////////////////////////////

#include "cbase.h"
#include "asw_colonist.h"
#include "asw_marine.h"
#include "asw_alien.h"
#include "asw_gamerules.h"
#include "asw_parasite.h"
#include "asw_fx_shared.h"
#include "asw_player.h"
//softcopy:
#include "asw_burning.h"            
#include "effect_dispatch_data.h"
#include "te_effect_dispatch.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

const int MAX_PLAYER_SQUAD = 4;

ConVar	asw_colonist_health ( "asw_colonist_health", "90" );
ConVar	asw_colonist_tom_health ( "asw_colonist_tom_health", "30" );	// tutorial guy who gets eaten
//softcopy:
#define SWARM_COLONIST_MODEL_FEMALE  "models/humans/group01/female_01.mdl"	//female colonist

#define SWARM_COLONIST_MODEL         "models/swarm/Colonist/Male/MaleColonist.mdl"

LINK_ENTITY_TO_CLASS( asw_colonist, CASW_Colonist );

BEGIN_DATADESC( CASW_Colonist )
	DEFINE_FIELD( m_bInfested, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_fInfestedTime, FIELD_TIME ),
	DEFINE_FIELD( m_fLastASWThink, FIELD_TIME ),
	DEFINE_FIELD( m_hInfestationCurer, FIELD_EHANDLE ),
	DEFINE_FIELD( m_bSlowHeal, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_iSlowHealAmount, FIELD_INTEGER ),
	DEFINE_FIELD( m_fNextSlowHealTick, FIELD_TIME ),	
	DEFINE_KEYFIELD(	m_bNotifyNavFailBlocked,	FIELD_BOOLEAN, "notifynavfailblocked" ),
	DEFINE_OUTPUT(		m_OnNavFailBlocked,		"OnNavFailBlocked" ),
END_DATADESC()

extern ConVar asw_debug_marine_damage;

CASW_Colonist::CASW_Colonist()
{
	m_fInfestedTime = 0;
	m_iInfestCycle = 0;
	//softcopy:
	//Msg("CASW_Colonist created\n");        
	isFemale=RandomInt(0,1)==0;
    Msg("%s created\n", isFemale ? "CASW_Colonist female" : "CASW_Colonist male");
	selectedBy = -1;  
}

CASW_Colonist::~CASW_Colonist()
{
	//softcopy:
	//Msg("CASW_Colonist destroyed\n");
	Msg("%s destroyed\n", isFemale ? "CASW_Colonist female" : "CASW_Colonist male");    
}

void CASW_Colonist::Precache()
{
	PrecacheModel( SWARM_COLONIST_MODEL );
	
	//softcopy: doesn't exist, no use, skip it!
	//PrecacheScriptSound( "NPC_Citizen.FootstepLeft" );      
	//PrecacheScriptSound( "NPC_Citizen.FootstepRight" );
	//PrecacheScriptSound( "NPC_Citizen.Die" );
	//PrecacheScriptSound( "MaleMarine.Pain" );
	//add female model
	PrecacheModel( SWARM_COLONIST_MODEL_FEMALE );
	PrecacheScriptSound( "Crash.Dead0" );
	PrecacheScriptSound( "Crash.SmallPain0" );
	PrecacheScriptSound( "Faith.Dead0" );
	PrecacheScriptSound( "Faith.SmallPain0" );


	//PrecacheInstancedScene( "scenes/swarmscenes/tutorialscript3.vcd" );
	//PrecacheInstancedScene( "scenes/swarmscenes/FishermanAlert.vcd" );

	BaseClass::Precache();
}

void CASW_Colonist::Spawn()
{
	Precache();
	//softcopy:
	//SetModel( SWARM_COLONIST_MODEL );
	SetModel( isFemale ? SWARM_COLONIST_MODEL_FEMALE:SWARM_COLONIST_MODEL );

 	SetRenderMode(kRenderNormal);
	SetRenderColor(180,180,180);
	
	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();
	
	ChangeFaction( FACTION_MARINES );
	
	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );
	SetBloodColor( BLOOD_COLOR_RED );
	m_flFieldOfView		= 0.02;
	m_NPCState		= NPC_STATE_NONE;
	
	CapabilitiesClear();
	CapabilitiesAdd( bits_CAP_SQUAD );
	if ( !HasSpawnFlags( SF_NPC_START_EFFICIENT ) )
	{
		CapabilitiesAdd( bits_CAP_ANIMATEDFACE | bits_CAP_TURN_HEAD );
		CapabilitiesAdd( bits_CAP_USE_WEAPONS | bits_CAP_AIM_GUN | bits_CAP_MOVE_SHOOT );
		CapabilitiesAdd( bits_CAP_DUCK | bits_CAP_DOORS_GROUP );
		CapabilitiesAdd( bits_CAP_USE_SHOT_REGULATOR );
	}

	CapabilitiesAdd( bits_CAP_NO_HIT_PLAYER | bits_CAP_NO_HIT_SQUADMATES | bits_CAP_FRIENDLY_DMG_IMMUNE );
	CapabilitiesAdd( bits_CAP_MOVE_GROUND );
	//softcopy: if on map "bi-rescure", colonist will die on any type of damages
	if ( !stricmp(gpGlobals->mapname.ToCStr(), "bi_rescue") )
		CapabilitiesRemove( bits_CAP_FRIENDLY_DMG_IMMUNE );
		
	SetMoveType( MOVETYPE_STEP );

	m_HackedGunPos = Vector( 0, 0, ASW_MARINE_GUN_OFFSET_Z );

	BaseClass::Spawn();

	m_iHealth = asw_colonist_health.GetFloat();

	const char* szName = STRING(GetEntityName());
	if (!Q_strcmp(szName, "Tom"))
	{
		m_iHealth = asw_colonist_tom_health.GetFloat();
	}

	AddEFlags( EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL | EFL_NO_PHYSCANNON_INTERACTION );

	NPCInit();
	//softcopy:
	//Msg("Colonist health after NPCInit %d\n", m_iHealth);
	Msg("%s health after NPCInit %d\n", "Colonist", m_iHealth);
}

Activity CASW_Colonist::NPC_TranslateActivity( Activity activity )
{
	if ( activity == ACT_MELEE_ATTACK1 )
	{
		return ACT_MELEE_ATTACK_SWING;
	}
	
	// !!!HACK - Citizens don't have the required animations for shotguns, 
	// so trick them into using the rifle counterparts for now (sjb)
	if ( activity == ACT_RUN_AIM_SHOTGUN )
		return ACT_RUN_AIM_RIFLE;
	if ( activity == ACT_WALK_AIM_SHOTGUN )
		return ACT_WALK_AIM_RIFLE;
	if ( activity == ACT_IDLE_ANGRY_SHOTGUN )
		return ACT_IDLE_ANGRY_SMG1;
	if ( activity == ACT_RANGE_ATTACK_SHOTGUN_LOW )
		return ACT_RANGE_ATTACK_SMG1_LOW;
		
	return BaseClass::NPC_TranslateActivity( activity );
}

//softcopy:
void CASW_Colonist::ASW_Ignite( float flFlameLifetime, float flSize, CBaseEntity *pAttacker, CBaseEntity *pDamagingWeapon ) 
{
	if ( ASWBurning() ) 
		ASWBurning()->BurnEntity(this, pAttacker, flFlameLifetime, 0.4f, 10.0f * 0.4f, pDamagingWeapon );	// 10 dps, applied every 0.4 seconds
	Ignite( flFlameLifetime, false, 1, true );
}

int CASW_Colonist::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	if( (info.GetDamageType() & DMG_BURN) && (info.GetDamageType() & DMG_DIRECT) )
	{
#define CITIZEN_SCORCH_RATE		6
#define CITIZEN_SCORCH_FLOOR	75

		Scorch( CITIZEN_SCORCH_RATE, CITIZEN_SCORCH_FLOOR );
	}
	//softcopy:
	if ( info.GetDamageType() & DMG_BURN && !IsOnFire() )
		ASW_Ignite(7, 0, info.GetAttacker(), info.GetWeapon());

	CTakeDamageInfo newInfo = info;

	return BaseClass::OnTakeDamage_Alive( newInfo );
}

bool CASW_Colonist::IsPlayerAlly( CBasePlayer *pPlayer )
{ 
	return true;
}

void CASW_Colonist::DeathSound( const CTakeDamageInfo &info )
{
	// Sentences don't play on dead NPCs
	SentenceStop();
	
	//softcopy: female or male sound
	//EmitSound( "NPC_Citizen.Die" );
	EmitSound(isFemale ? "Faith.Dead0" : "Crash.Dead0");
}

bool CASW_Colonist::IsHeavyDamage( const CTakeDamageInfo &info )
{
	return (( info.GetDamage() >  5 ) || (info.GetDamageType() & DMG_INFEST));
}

void CASW_Colonist::TaskFail( AI_TaskFailureCode_t code )
{
	if( code == FAIL_NO_ROUTE_BLOCKED && m_bNotifyNavFailBlocked )
	{
		m_OnNavFailBlocked.FireOutput( this, this );
	}

	BaseClass::TaskFail( code );
}

void CASW_Colonist::BecomeInfested(CASW_Alien* pAlien)
{
	m_fInfestedTime = 20;
	// todo: scream about being infested!
	m_bInfested = true;
	if (m_fNextSlowHealTick < gpGlobals->curtime)
		m_fNextSlowHealTick = gpGlobals->curtime + 0.33f;
	// do some damage to us immediately
	float DamagePerTick = ASWGameRules()->TotalInfestDamage() / 20.0f;
	CTakeDamageInfo info(NULL, NULL, Vector(0,0,0), GetAbsOrigin(), DamagePerTick,
		DMG_INFEST);
	TakeDamage(info);
	
	//softcopy: female or male sound
	//EmitSound("MaleMarine.Pain");
	EmitSound(isFemale ? "Faith.SmallPain0" : "Crash.SmallPain0");
}

void CASW_Colonist::CureInfestation(CASW_Marine *pHealer, float fCureFraction)
{
	if (m_fInfestedTime > 0)
	{
		m_fInfestedTime = m_fInfestedTime * fCureFraction;
		if (pHealer)
			m_hInfestationCurer = pHealer;
	}
}

// if we died from infestation, then gib
bool CASW_Colonist::ShouldGib( const CTakeDamageInfo &info )
{
	if (info.GetDamageType() & DMG_INFEST)
		return true;

	return BaseClass::ShouldGib(info);
}

// if we gibbed from infestation damage, spawn some parasites
bool  CASW_Colonist::Event_Gibbed( const CTakeDamageInfo &info )
{
	if (info.GetDamageType() & DMG_INFEST)
	{
		if (asw_debug_marine_damage.GetBool())
			Msg("colonist infest gibbed at loc %f, %f, %f\n", GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z);
		if (asw_debug_marine_damage.GetBool())
			NDebugOverlay::EntityBounds(this, 255,0,0, 255, 15.0f);
		int iNumParasites = 3 + random->RandomInt(0,2);
		Vector vecSpawnPos[5];
		QAngle angParasiteFacing[5];
		float fJumpDistance[5];
		// for some reason if we calculate these inside the loop, the random numbers all come out the same.  Worrying.
		angParasiteFacing[0] = GetAbsAngles(); angParasiteFacing[0].y = random->RandomInt(0,360);
		angParasiteFacing[1] = GetAbsAngles(); angParasiteFacing[1].y = random->RandomInt(0,360);
		angParasiteFacing[2] = GetAbsAngles(); angParasiteFacing[2].y = random->RandomInt(0,360);
		angParasiteFacing[3] = GetAbsAngles(); angParasiteFacing[3].y = random->RandomInt(0,360);
		angParasiteFacing[4] = GetAbsAngles(); angParasiteFacing[4].y = random->RandomInt(0,360);
		fJumpDistance[0] = random->RandomInt(30,70);
		fJumpDistance[1] = random->RandomInt(30,70);
		fJumpDistance[2] = random->RandomInt(30,70);
		fJumpDistance[3] = random->RandomInt(30,70);
		fJumpDistance[4] = random->RandomInt(30,70);
		for (int i=0;i<iNumParasites;i++)
		{
			bool bBlocked = true;			
			int k = 0;
			vecSpawnPos[i] = vec3_origin;
			while (bBlocked && k<10)
			{
				vecSpawnPos[i] = GetAbsOrigin();
				vecSpawnPos[i].z += random->RandomInt(25,45);
				if (k > 0)
				{
					vecSpawnPos[i].x += random->RandomInt(-15, 15);
					vecSpawnPos[i].y += random->RandomInt(-15, 15);
				}
				
								
				// check if there's room at this position
				trace_t tr;
				UTIL_TraceHull( vecSpawnPos[i], vecSpawnPos[i]+Vector(0,0,1), 
					NAI_Hull::Mins(HULL_TINY),NAI_Hull::Maxs(HULL_TINY),
					MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr );	
				if (asw_debug_marine_damage.GetBool())
					NDebugOverlay::Box(vecSpawnPos[i], NAI_Hull::Mins(HULL_TINY),NAI_Hull::Maxs(HULL_TINY), 255,255,0,255,15.0f);
				if ( tr.fraction == 1.0 )
				{
					bBlocked = false;
				}

				k++;				
			}

			if (bBlocked)
				continue;	// couldn't find room for parasites

			if (asw_debug_marine_damage.GetBool())
				Msg("Found an unblocked pos for this entity, trying to spawn it there %f, %f, %f\n", vecSpawnPos[i].x, 
					vecSpawnPos[i].y, vecSpawnPos[i].z);

			CASW_Parasite* pParasite = dynamic_cast<CASW_Parasite*>(CreateNoSpawn("asw_parasite",
				vecSpawnPos[i], angParasiteFacing[i], this));

			if (pParasite)
			{
				pParasite->Spawn();
				pParasite->AddSolidFlags( FSOLID_NOT_SOLID );
				pParasite->SetSleepState(AISS_WAITING_FOR_INPUT);
				pParasite->SetJumpFromEgg(true, fJumpDistance[i]);
				pParasite->Wake();
			}
		}
	}
	Vector vecDir = RandomVector(-1, 1);
	UTIL_ASW_BloodDrips( GetAbsOrigin()+Vector(0,0,60)+vecDir*20, vecDir, BloodColor(), 5 );
	vecDir = RandomVector(-1, 1);
	UTIL_ASW_BloodDrips( GetAbsOrigin()+Vector(0,0,40)+vecDir*20, vecDir, BloodColor(), 5 );
	vecDir = RandomVector(-1, 1);
	UTIL_ASW_BloodDrips( GetAbsOrigin()+Vector(0,0,20)+vecDir*20, vecDir, BloodColor(), 5 );
	return BaseClass::Event_Gibbed(info);	
}

void CASW_Colonist::ASWThinkEffects()
{
	//float fDeltaTime = gpGlobals->curtime - m_fLastASWThink;
	// general timer for healing/infestation
	if ((m_bSlowHeal || IsInfested()) && GetHealth() > 0)
	{
		while (gpGlobals->curtime >= m_fNextSlowHealTick)
		{
			m_fNextSlowHealTick += 0.33f;
			// check slow heal isn't over out cap
			if (m_bSlowHeal)
			{
				if (m_iSlowHealAmount + GetHealth() > GetMaxHealth())
					m_iSlowHealAmount = GetMaxHealth() - GetHealth();
				int amount = MIN(4, m_iSlowHealAmount);
				// change the health
				SetHealth(GetHealth() + amount);			
				m_iSlowHealAmount -= amount;
				if (m_iSlowHealAmount <= 0)
				{
					m_bSlowHeal = false;
				}
			}
			if (IsInfested())
			{
				m_iInfestCycle++;
				if (m_iInfestCycle >= 3)	// only do the infest damage once per second
				{
					float DamagePerTick = ASWGameRules()->TotalInfestDamage() / 20.0f;
					CTakeDamageInfo info(NULL, NULL, Vector(0,0,0), GetAbsOrigin(), DamagePerTick,
						DMG_INFEST);
					TakeDamage(info);
					SetSchedule(SCHED_BIG_FLINCH);

					//EmitSound("MaleMarine.Pain");		//softcopy:
					m_iInfestCycle = 0;

					m_fInfestedTime-=1.0f;
					if (m_fInfestedTime <= 0)
					{
						m_fInfestedTime = 0;
						m_bInfested = false;
						//if (m_hInfestationCurer.Get() && m_hInfestationCurer->GetMarineResource())
						//{
							//m_hInfestationCurer->GetMarineResource()->m_iCuredInfestation++;
							m_hInfestationCurer = NULL;
						//}
					}
				}				
			}
		}
	}
	m_fLastASWThink = gpGlobals->curtime;
}

void CASW_Colonist::NPCThink()
{
	ASWThinkEffects();
	BaseClass::NPCThink();
}

// healing
void CASW_Colonist::AddSlowHeal(int iHealAmount, CASW_Marine *pMedic)
{
	if (iHealAmount > 0)
	{
		if (!m_bSlowHeal)
		{
			m_bSlowHeal = true;
			m_fNextSlowHealTick = gpGlobals->curtime + 0.33f;
		}
		m_iSlowHealAmount += iHealAmount;

		// note: no healing stats given to medic for colonists

		// healing puts out fires
		if (IsOnFire())
		{
			Extinguish();
		}
	}
}

//softcopy:
void CASW_Colonist::Extinguish()
{
	if (ASWBurning()) 
		ASWBurning()->Extinguish(this);

	BaseClass::Extinguish();
}


int CASW_Colonist::SelectFlinchSchedule_ASW()
{
	if ( IsCurSchedule( SCHED_BIG_FLINCH ) )
	{
		return SCHED_NONE;
	}

	if (!HasCondition(COND_HEAVY_DAMAGE))	// only flinch on heavy damage condition (which is set if a particular marine's weapon + skills cause a flinch)
	{		
		return SCHED_NONE;
	}

	return SCHED_BIG_FLINCH;
}


int CASW_Colonist::SelectSchedule( void )
{
	int nSched = SelectFlinchSchedule_ASW();
	if ( nSched != SCHED_NONE )
		return nSched;
	 //softcopy:
	if (selectedBy != -1) 
		return SCHED_SA_FOLLOW_MOVE;

	return BaseClass::SelectSchedule();
}

Activity CASW_Colonist::GetFlinchActivity( bool bHeavyDamage, bool bGesture )
{
   //softcopy:
	EmitSound(isFemale ? "Faith.SmallPain0" : "Crash.SmallPain0");

	if (isFemale)
		return (Activity) ACT_COWER;
	else


	return (Activity) ACT_BIG_FLINCH;
}

void CASW_Colonist::MeleeBleed(CTakeDamageInfo* info)
{

	Vector vecDir = vec3_origin;
	if (info->GetAttacker())
	{
		vecDir = info->GetAttacker()->GetAbsOrigin() - GetAbsOrigin();
		VectorNormalize(vecDir);
	}
	else
	{
		vecDir = RandomVector(-1, 1);
	}
		
	UTIL_ASW_BloodDrips( GetAbsOrigin()+Vector(0,0,60)+vecDir*3, vecDir, BloodColor(), 5 );
	SetSchedule(SCHED_BIG_FLINCH);
	//softcopy:
	//EmitSound("MaleMarine.Pain");
	EmitSound(isFemale ? "Faith.SmallPain0" : "Crash.SmallPain0");
}

AI_BEGIN_CUSTOM_NPC( asw_colonist, CASW_Colonist )
//softcopy:
	DECLARE_TASK( TASK_SA_GET_PATH_TO_FOLLOW_TARGET )
	DECLARE_TASK( TASK_SA_WAIT_FOR_FOLLOW_MOVEMENT )
	DECLARE_TASK( TASK_SA_FACE_FOLLOW_WAIT )

	DEFINE_SCHEDULE
	(
		SCHED_SA_FOLLOW_MOVE,

		"	Tasks"
		"		 TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_SA_FOLLOW_WAIT"
		"		 TASK_SA_GET_PATH_TO_FOLLOW_TARGET			0"
		"		 TASK_RUN_PATH								0"
		"		 TASK_SA_WAIT_FOR_FOLLOW_MOVEMENT			0"
		"		 TASK_STOP_MOVING							1"
		"	"
		"	Interrupts"
		"		COND_TASK_FAILED"
		"		COND_DAMAGE"
		"		COND_REPEATED_DAMAGE"
		"		COND_HEAVY_DAMAGE"
	)

	DEFINE_SCHEDULE	
	(
		SCHED_SA_FOLLOW_WAIT,
		  
		"	Tasks"
		"		TASK_STOP_MOVING						0"
		"		TASK_SA_FACE_FOLLOW_WAIT				0.3"
		""
		"	Interrupts"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_IDLE_INTERRUPT"
		"       COND_ASW_NEW_ORDERS"
		"		COND_GIVE_WAY"
	)

AI_END_CUSTOM_NPC()

static ConVar asw_npc_go_do_run( "asw_npc_go_do_run", "1", FCVAR_CHEAT, "Set whether should run on asw_npc_go" );

void CC_ASW_NPC_Go( void )
{
	CASW_Player *pPlayer = ToASW_Player(UTIL_GetCommandClient());;
	if ( !pPlayer || !pPlayer->GetMarine() )
		return;

	trace_t tr;
	Vector forward;
	Vector vecSrc = pPlayer->GetMarine()->EyePosition();
	QAngle angAiming = pPlayer->EyeAnglesWithCursorRoll();
	float dist = tan(DEG2RAD(90 - angAiming.z)) * 60.0f;
	AngleVectors( pPlayer->EyeAngles(), &forward );
	
	//AI_TraceLine( vecSrc,
		//vecSrc + forward * dist,  MASK_NPCSOLID,
		//pPlayer->GetMarine(), COLLISION_GROUP_NONE, &tr );
	CAI_BaseNPC::ForceSelectedGo(pPlayer, vecSrc + forward * dist, forward, asw_npc_go_do_run.GetBool());
}
static ConCommand asw_npc_go("asw_npc_go", CC_ASW_NPC_Go, "Selected NPC(s) will go to the location that the player is looking (shown with a purple box)\n\tArguments:	-none-", FCVAR_CHEAT);

//softcopy:
const Vector CASW_Colonist::GetFollowPos() 
{
	if (!GetTarget()) 
	{
		selectedBy = -1;
		SetSchedule(SCHED_IDLE_STAND);
		SetRenderColor(180,180,180);
		return GetAbsOrigin();
	}

	Vector marineForward;
	AngleVectors( GetTarget()->GetAbsAngles(), &marineForward );
	Vector offset = marineForward*100;

	trace_t tr;
	UTIL_TraceLine( GetTarget()->GetAbsOrigin(),
		GetTarget()->GetAbsOrigin() - offset, MASK_SOLID_BRUSHONLY, 
		NULL, COLLISION_GROUP_NONE, &tr );


	Vector followPos;
	if (tr.fraction < 1) 
	   followPos = GetTarget()->GetAbsOrigin() - marineForward*(MAX(0, 100*tr.fraction-10));
	else
	   followPos = GetTarget()->GetAbsOrigin() - offset;
	
	return followPos;
}
void CASW_Colonist::RunTask( const Task_t *pTask ) 
{
	switch (pTask->iTask) 
	{
	case TASK_SA_FACE_FOLLOW_WAIT:
		{
			//UpdateFacing();
			if ( IsWaitFinished())
				TaskComplete();
			break;
		}
	case TASK_SA_WAIT_FOR_FOLLOW_MOVEMENT:
		{
			//UpdateFacing();
			bool fTimeExpired = ( pTask->flTaskData != 0 && pTask->flTaskData < gpGlobals->curtime - GetTimeTaskStarted() );
			if (fTimeExpired || GetNavigator()->GetGoalType() == GOALTYPE_NONE)
			{
				TaskComplete();
				GetNavigator()->StopMoving();		// Stop moving
			}
			else if (!GetNavigator()->IsGoalActive())
			{
				SetIdealActivity( GetStoppedActivity() );
			}
			else
			{
				// Check validity of goal type
				ValidateNavGoal();

				const Vector &vecFollowPos = GetFollowPos();
				if ( ( GetNavigator()->GetGoalPos() - vecFollowPos ).LengthSqr() > Square( 150 ) )
				{
					if ( GetNavigator()->GetNavType() != NAV_JUMP )
					{
						if ( !GetNavigator()->UpdateGoalPos( vecFollowPos ) )
						{
							TaskFail(FAIL_NO_ROUTE);
						}
					}
				}

#define ASW_FOLLOW_DISTANCE 150
				float dist = ( GetAbsOrigin() - GetFollowPos() ).Length2DSqr();
				if (dist < ( ASW_FOLLOW_DISTANCE * ASW_FOLLOW_DISTANCE )) 
					TaskComplete();
			}
			break;
		}
		default:
			BaseClass::RunTask(pTask);
			break;
	}
}
void CASW_Colonist::StartTask( const Task_t *pTask )
{
	switch( pTask->iTask ) 
	{
	case TASK_SA_GET_PATH_TO_FOLLOW_TARGET:
		{
			AI_NavGoal_t goal( GetFollowPos(), ACT_RUN, 60 ); // AIN_HULL_TOLERANCE
			GetNavigator()->SetGoal( goal );
			TaskComplete();
			break;
		}
	case TASK_SA_WAIT_FOR_FOLLOW_MOVEMENT:
		{
			if (GetNavigator()->GetGoalType() == GOALTYPE_NONE)
			{
				TaskComplete();
				GetNavigator()->ClearGoal();		// Clear residual state
			}
			else if (!GetNavigator()->IsGoalActive())
				SetIdealActivity( GetStoppedActivity() );
			else
				// Check validity of goal type
				ValidateNavGoal();
			break;
		}
	case TASK_SA_FACE_FOLLOW_WAIT:
		{
			SetWait( pTask->flTaskData );
			break;
		}

	default:
		BaseClass::StartTask( pTask );
		break;
	}
}
void CASW_Colonist::ActivateUseIcon( CASW_Marine* pMarine, int nHoldType )
{
	if ( nHoldType == ASW_USE_HOLD_START )
		return;

	if (!isSelectedBy(pMarine)) 
	{
		SetPrimaryBehavior( NULL );
		SetTarget(pMarine);
		SetSchedule(SCHED_SA_FOLLOW_MOVE);
		if (m_hCine != NULL) 
			ExitScriptedSequence();

		selectedBy = pMarine->entindex();
		SetEffects(0);
		SetRenderColor(255,255,255);
	} 
	else 
	{
		selectedBy = -1;
		SetSchedule(SCHED_IDLE_STAND);
		SetRenderColor(180,180,180);
	}
}
bool CASW_Colonist::isSelectedBy(CASW_Marine* marine) 
{
	return selectedBy == marine->entindex();
}
bool CASW_Colonist::IsUsable(CBaseEntity *pUser) 
{
	return (pUser && pUser->GetAbsOrigin().DistTo(GetAbsOrigin()) < ASW_MARINE_USE_RADIUS);	// near enough?
}
