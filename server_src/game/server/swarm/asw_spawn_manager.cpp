#include "cbase.h"
#include "asw_spawn_manager.h"
#include "asw_marine.h"
#include "asw_marine_resource.h"
#include "asw_game_resource.h"
#include "iasw_spawnable_npc.h"
#include "asw_player.h"
#include "ai_network.h"
#include "ai_waypoint.h"
#include "ai_node.h"
#include "asw_director.h"
#include "asw_util_shared.h"
#include "asw_path_utils.h"
#include "asw_trace_filter_doors.h"
#include "asw_objective_escape.h"
#include "triggers.h"
#include "datacache/imdlcache.h"
#include "ai_link.h"
#include "asw_alien.h"
#include "entitylist.h"
#include "asw_gamerules.h"
#include "ASW_Horde_Mode.h"	//softcopy:

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CASW_Spawn_Manager g_Spawn_Manager;
CASW_Spawn_Manager* ASWSpawnManager() { return &g_Spawn_Manager; }

#define CANDIDATE_ALIEN_HULL 11		// TODO: have this use the hull of the alien type we're spawning a horde of?
#define MARINE_NEAR_DISTANCE 740.0f

extern ConVar asw_director_debug;
extern ConVar asw_wanderer_max;
extern ConVar asw_debug_alien_spawn;	//softcopy:
ConVar asw_horde_min_distance("asw_horde_min_distance", "800", FCVAR_CHEAT, "Minimum distance away from the marines the horde can spawn" );
ConVar asw_horde_max_distance("asw_horde_max_distance", "1500", FCVAR_CHEAT, "Maximum distance away from the marines the horde can spawn" );
ConVar asw_max_alien_batch("asw_max_alien_batch", "10", FCVAR_CHEAT, "Max number of aliens spawned in a horde batch" );
ConVar asw_batch_interval("asw_batch_interval", "5", FCVAR_CHEAT, "Time between successive batches spawning in the same spot");
ConVar asw_candidate_interval("asw_candidate_interval", "1.0", FCVAR_CHEAT, "Interval between updating candidate spawning nodes");
ConVar asw_horde_class( "asw_horde_class", "asw_drone", FCVAR_CHEAT, "Alien class used when spawning hordes" );

ConVar asw_horde_enforce_range("asw_horde_enforce_range", "1", FCVAR_CHEAT, "ALWAYS check if marines are within the min and max distances.");
ConVar asw_horde_buffer("asw_horde_buffer", "25", FCVAR_CHEAT, "Buffer to prevent aliens from spawning partly inside walls.");

CASW_Spawn_Manager::CASW_Spawn_Manager()
{
	m_nAwakeAliens = 0;
	m_nAwakeDrones = 0;
	m_bFreshHorde = 0;
}

CASW_Spawn_Manager::~CASW_Spawn_Manager()
{

}

// ==================================
// == Master list of alien classes ==
// ==================================

// NOTE: If you add new entries to this list, update the asw_spawner choices in swarm.fgd.
//       Do not rearrange the order or you will be changing what spawns in all the maps.
//Ch1ckensCoop: Added "asw_drone_uber"
//Ch1ckensCoop: Set asw_queen hull size from HULL_TINY to HULL_LARGE
ASW_Alien_Class_Entry g_Aliens[]=
{
	ASW_Alien_Class_Entry( "asw_drone", HULL_MEDIUMBIG ),
	ASW_Alien_Class_Entry( "asw_buzzer", HULL_TINY_CENTERED ),
	ASW_Alien_Class_Entry( "asw_parasite", HULL_TINY ),
	ASW_Alien_Class_Entry( "asw_shieldbug", HULL_LARGE ),
	ASW_Alien_Class_Entry( "asw_grub", HULL_TINY ),
	ASW_Alien_Class_Entry( "asw_drone_jumper", HULL_MEDIUMBIG ),
	ASW_Alien_Class_Entry( "asw_harvester", HULL_HUMAN ),
	ASW_Alien_Class_Entry( "asw_parasite_defanged", HULL_TINY ),
	ASW_Alien_Class_Entry( "asw_queen", HULL_LARGE ),
	ASW_Alien_Class_Entry( "asw_boomer", HULL_HUMAN ),
	ASW_Alien_Class_Entry( "asw_ranger", HULL_HUMAN ),
	ASW_Alien_Class_Entry( "asw_mortarbug", HULL_LARGE ),
	ASW_Alien_Class_Entry( "asw_shaman", HULL_LARGE ),
	ASW_Alien_Class_Entry( "asw_drone_uber", HULL_MEDIUMBIG ),
	ASW_Alien_Class_Entry( "npc_zombie", HULL_HUMAN ),	//softcopy:

};
//Ch1ckensCoop: Expanded to include all alien types
// Array indices of drones.  Used by carnage mode.
//Ch1ckensCoop: Converted to enumeration.
/*
const int g_nDroneClassEntry = 0;
const int g_nBuzzerClassEntry = 1;
const int g_nParasiteClassEntry = 2;
const int g_nShieldbugClassEntry = 3;
const int g_nGrubClassEntry = 4;
const int g_nDroneJumperClassEntry = 5;
const int g_nHarvesterClassEntry = 6;
const int g_nSafeParasiteClassEntry = 7;
const int g_nQueenClassEntry = 8;
const int g_nBoomerClassEntry = 9;
const int g_nRangerClassEntry = 10;
const int g_nMortarClassEntry = 11;
const int g_nShamenClassEntry = 12;
const int g_nUberDroneClassEntry = 13;
*/

int CASW_Spawn_Manager::GetNumAlienClasses()
{
	return NELEMS( g_Aliens );
}

ASW_Alien_Class_Entry* CASW_Spawn_Manager::GetAlienClass( int i )
{
	Assert( i >= 0 && i < NELEMS( g_Aliens ) );
	return &g_Aliens[ i ];
}

void CASW_Spawn_Manager::LevelInitPreEntity()
{
	m_nAwakeAliens = 0;
	m_nAwakeDrones = 0;
	// init alien classes
	for ( int i = 0; i < GetNumAlienClasses(); i++ )
	{
		GetAlienClass( i )->m_iszAlienClass = AllocPooledString( GetAlienClass( i )->m_pszAlienClass );
	}
}

void CASW_Spawn_Manager::LevelInitPostEntity()
{
	m_vecHordePosition = vec3_origin;
	m_angHordeAngle = vec3_angle;
	m_batchInterval.Invalidate();
	m_CandidateUpdateTimer.Invalidate();
	m_iHordeToSpawn = 0;
	m_iAliensToSpawn = 0;

	m_northCandidateNodes.Purge();
	m_southCandidateNodes.Purge();

	FindEscapeTriggers();
}

void CASW_Spawn_Manager::OnAlienWokeUp( CASW_Alien *pAlien )
{
	m_nAwakeAliens++;
	if ( pAlien && pAlien->Classify() == CLASS_ASW_DRONE )
	{
		m_nAwakeDrones++;
	}
}

void CASW_Spawn_Manager::OnAlienSleeping( CASW_Alien *pAlien )
{
	m_nAwakeAliens--;
	if ( pAlien && pAlien->Classify() == CLASS_ASW_DRONE )
	{
		m_nAwakeDrones--;
	}
}

// finds all trigger_multiples linked to asw_objective_escape entities
void CASW_Spawn_Manager::FindEscapeTriggers()
{
	m_EscapeTriggers.Purge();

	// go through all escape objectives
	CBaseEntity* pEntity = NULL;
	while ( (pEntity = gEntList.FindEntityByClassname( pEntity, "asw_objective_escape" )) != NULL )
	{
		CASW_Objective_Escape* pObjective = dynamic_cast<CASW_Objective_Escape*>(pEntity);
		if ( !pObjective )
			continue;

		const char *pszEscapeTargetName = STRING( pObjective->GetEntityName() );

		CBaseEntity* pOtherEntity = NULL;
		while ( (pOtherEntity = gEntList.FindEntityByClassname( pOtherEntity, "trigger_multiple" )) != NULL )
		{
			CTriggerMultiple *pTrigger = dynamic_cast<CTriggerMultiple*>( pOtherEntity );
			if ( !pTrigger )
				continue;

			bool bAdded = false;
			CBaseEntityOutput *pOutput = pTrigger->FindNamedOutput( "OnTrigger" );
			if ( pOutput )
			{
				CEventAction *pAction = pOutput->GetFirstAction();
				while ( pAction )
				{
					if ( !Q_stricmp( STRING( pAction->m_iTarget ), pszEscapeTargetName ) )
					{
						bAdded = true;
						m_EscapeTriggers.AddToTail( pTrigger );
						break;
					}
					pAction = pAction->m_pNext;
				}
			}

			if ( !bAdded )
			{
				pOutput = pTrigger->FindNamedOutput( "OnStartTouch" );
				if ( pOutput )
				{
					CEventAction *pAction = pOutput->GetFirstAction();
					while ( pAction )
					{
						if ( !Q_stricmp( STRING( pAction->m_iTarget ), pszEscapeTargetName ) )
						{
							bAdded = true;
							m_EscapeTriggers.AddToTail( pTrigger );
							break;
						}
						pAction = pAction->m_pNext;
					}
				}
			}

		}
	}
	Msg("Spawn manager found %d escape triggers\n", m_EscapeTriggers.Count() );
}


void CASW_Spawn_Manager::Update()
{
	if ( m_iHordeToSpawn > 0 )
	{		
		if ( m_vecHordePosition != vec3_origin && ( !m_batchInterval.HasStarted() || m_batchInterval.IsElapsed() ) )
		{
			int iToSpawn = MIN( m_iHordeToSpawn, asw_max_alien_batch.GetInt() );
			int iSpawned = 0;
			if (asw_horde_enforce_range.GetBool())		//Ch1ckensCoop: Make sure aliens are off screen. ALWAYS.
			{
				float flDistance = 0.0f;
				UTIL_ASW_NearestMarine( m_vecHordePosition, flDistance );

				if ( flDistance < asw_horde_max_distance.GetFloat() && flDistance > asw_horde_min_distance.GetFloat() )
				{
					iSpawned = SpawnAlienBatch( asw_horde_class.GetString(), iToSpawn, m_vecHordePosition, m_angHordeAngle );
					//softcopy: spawns more HL2 aliens if horde has spawned nothing 
					if (iSpawned <=0)
						iSpawned = SpawnHL2AlienBatch(asw_horde_class.GetString(), iToSpawn );
				}
				else
				{
					if (asw_director_debug.GetBool())
					{
						Warning("Marine too far/close from spawn point - Attempted to spawn horde at distance %0.2f!\n", flDistance);
					}
				}
			}
			else
			{
				iSpawned = SpawnAlienBatch( asw_horde_class.GetString(), iToSpawn, m_vecHordePosition, m_angHordeAngle, asw_horde_min_distance.GetFloat() );
				//softcopy:	spawns more HL2 aliens if horde has spawned nothing 
				if (iSpawned <=0)
					iSpawned = SpawnHL2AlienBatch(asw_horde_class.GetString(), iToSpawn , asw_horde_min_distance.GetFloat());
			}

			m_iHordeToSpawn -= iSpawned;
			if ( m_iHordeToSpawn <= 0 )
			{
				ASWDirector()->OnHordeFinishedSpawning();
				m_vecHordePosition = vec3_origin;
			}
			else if ( iSpawned == 0 )			// if we failed to spawn any aliens, then try to find a new horde location
			{
				if ( asw_director_debug.GetBool() )
				{
					Msg( "Horde failed to spawn any aliens, trying new horde position.\n" );
				}
				if ( !FindHordePosition() )		// if we failed to find a new location, just abort this horde
				{
					m_iHordeToSpawn = 0;
					ASWDirector()->OnHordeFinishedSpawning();
					m_vecHordePosition = vec3_origin;
				}
			}
			m_batchInterval.Start( asw_batch_interval.GetFloat() );
		}
		else if ( m_vecHordePosition == vec3_origin )
		{
			if (asw_director_debug.GetBool())
				Msg( "Warning: Had horde to spawn but no position, clearing.\n" );
			m_iHordeToSpawn = 0;
			ASWDirector()->OnHordeFinishedSpawning();
		}
	}

	if ( asw_director_debug.GetBool() )
	{
		engine->Con_NPrintf( 14, "Spawn Manager: Batch interval: %f pos = %f %f %f", m_batchInterval.HasStarted() ? m_batchInterval.GetRemainingTime() : -1, VectorExpand( m_vecHordePosition ) );		
		engine->Con_NPrintf( 17, "Wanderers to spawn: %i", m_iAliensToSpawn);
	}

	if ( m_iAliensToSpawn > 0 )
	{
		if ( SpawnAlientAtRandomNode() )
			m_iAliensToSpawn--;
	}
}

void CASW_Spawn_Manager::AddAlien()
{
	// don't stock up more than X wanderers at once if we
	// can't find an available AI node to spawn them at.
	if ( m_iAliensToSpawn > asw_wanderer_max.GetInt() )
		return;

	m_iAliensToSpawn++;
}

bool CASW_Spawn_Manager::SpawnAlientAtRandomNode()
{
	UpdateCandidateNodes();

	// decide if the alien is going to come from behind or in front
	bool bNorth = RandomFloat() < 0.7f;
	if ( m_northCandidateNodes.Count() <= 0 )
	{
		bNorth = false;
	}
	else if ( m_southCandidateNodes.Count() <= 0 )
	{
		bNorth = true;
	}

	CUtlVector<int> &candidateNodes = bNorth ? m_northCandidateNodes : m_southCandidateNodes;

	if ( candidateNodes.Count() <= 0 )
		return false;

	const char *szAlienClass = "asw_drone";
	Vector vecMins, vecMaxs;
	GetAlienBounds( szAlienClass, vecMins, vecMaxs );

	int iMaxTries = 1;
	for ( int i=0 ; i<iMaxTries ; i++ )
	{
		int iChosen = RandomInt( 0, candidateNodes.Count() - 1);
		CAI_Node *pNode = GetNetwork()->GetNode( candidateNodes[iChosen] );
		if ( !pNode )
			continue;

		float flDistance = 0;
		CASW_Marine *pMarine = dynamic_cast<CASW_Marine*>(UTIL_ASW_NearestMarine( pNode->GetPosition( CANDIDATE_ALIEN_HULL ), flDistance ));
		if ( !pMarine )
			return false;

		// check if there's a route from this node to the marine(s)
		AI_Waypoint_t *pRoute = ASWPathUtils()->BuildRoute( pNode->GetPosition( CANDIDATE_ALIEN_HULL ), pMarine->GetAbsOrigin(), NULL, 100 );
		if ( !pRoute )
		{
			if ( asw_director_debug.GetBool() )
			{
				NDebugOverlay::Cross3D( pNode->GetOrigin(), 10.0f, 255, 128, 0, true, 20.0f );
			}
			continue;
		}

		if ( bNorth && UTIL_ASW_DoorBlockingRoute( pRoute, true ) )
		{
			DeleteRoute( pRoute );
			continue;
		}

		Vector vecSpawnPos = pNode->GetPosition( CANDIDATE_ALIEN_HULL ) + Vector( 0, 0, 32 );
		if ( ValidSpawnPoint( vecSpawnPos, vecMins, vecMaxs, true ) )
		{
			if ( SpawnAlienAt( szAlienClass, vecSpawnPos, vec3_angle ) )
			{
				if ( asw_director_debug.GetBool() )
				{
					NDebugOverlay::Cross3D( vecSpawnPos, 25.0f, 128, 128, 255, true, 20.0f );
					float flDist;
					CASW_Marine *pMarine = UTIL_ASW_NearestMarine( vecSpawnPos, flDist );
					if ( pMarine )
					{
						NDebugOverlay::Line( pMarine->GetAbsOrigin(), vecSpawnPos, 128, 128, 255, true, 60.0f );
					}
				}
				DeleteRoute( pRoute );
				return true;
			}
		}
		else
		{
			if ( asw_director_debug.GetBool() )
			{
				NDebugOverlay::Cross3D( vecSpawnPos, 25.0f, 255, 0, 0, true, 20.0f );
			}
		}
		DeleteRoute( pRoute );
	}
	return false;
}

bool CASW_Spawn_Manager::AddHorde( int iHordeSize )
{
	if ( m_vecHordePosition == vec3_origin )
	{
		if ( !FindHordePosition() )
		{
			if (asw_director_debug.GetBool())
				Msg("Error: Failed to find horde position\n");
			return false;
		}
		else
		{

			if ( asw_director_debug.GetBool() )
			{
				NDebugOverlay::Cross3D( m_vecHordePosition, 32.0f, 255, 128, 0, true, 30.0f );
				float flDist;
				CASW_Marine *pMarine = UTIL_ASW_NearestMarine( m_vecHordePosition, flDist );
				if ( pMarine )
				{
					NDebugOverlay::Line( pMarine->GetAbsOrigin(), m_vecHordePosition, 255, 128, 0, true, 60.0f );
				}
			}
		}
	}

	m_iHordeToSpawn = iHordeSize;
	m_bFreshHorde = true;

	return true;
}

CAI_Network* CASW_Spawn_Manager::GetNetwork()
{
	return g_pBigAINet;
}

void CASW_Spawn_Manager::UpdateCandidateNodes()
{
	// don't update too frequently
	if ( m_CandidateUpdateTimer.HasStarted() && !m_CandidateUpdateTimer.IsElapsed() )
		return;

	m_CandidateUpdateTimer.Start( asw_candidate_interval.GetFloat() );

	if ( !GetNetwork() || !GetNetwork()->NumNodes() )
	{
		m_vecHordePosition = vec3_origin;
		//softcopy:
		//Msg("Error: Can't spawn hordes as this map has no node network\n");
		ASWGameRules()->MsgInterval("Error: Can't spawn hordes as this map has no node network", 4);

		return;
	}

	CASW_Game_Resource *pGameResource = ASWGameResource();
	if ( !pGameResource )
		return;

	Vector vecSouthMarine = vec3_origin;
	Vector vecNorthMarine = vec3_origin;
	for ( int i=0;i<pGameResource->GetMaxMarineResources();i++ )
	{
		CASW_Marine_Resource *pMR = pGameResource->GetMarineResource(i);
		if ( !pMR )
			continue;

		CASW_Marine *pMarine = pMR->GetMarineEntity();
		if ( !pMarine || pMarine->GetHealth() <= 0 )
			continue;

		if ( vecSouthMarine == vec3_origin || vecSouthMarine.y > pMarine->GetAbsOrigin().y )
		{
			vecSouthMarine = pMarine->GetAbsOrigin();
		}
		if ( vecNorthMarine == vec3_origin || vecNorthMarine.y < pMarine->GetAbsOrigin().y )
		{
			vecNorthMarine = pMarine->GetAbsOrigin();
		}
	}
	if ( vecSouthMarine == vec3_origin || vecNorthMarine == vec3_origin )		// no live marines
		return;

	int iNumNodes = GetNetwork()->NumNodes();
	m_northCandidateNodes.Purge();
	m_southCandidateNodes.Purge();
	for ( int i=0 ; i<iNumNodes; i++ )
	{
		CAI_Node *pNode = GetNetwork()->GetNode( i );
		if ( !pNode || pNode->GetType() != NODE_GROUND )
			continue;

		Vector vecPos = pNode->GetPosition( CANDIDATE_ALIEN_HULL );

		// find the nearest marine to this node
		float flDistance = 0;
		CASW_Marine *pMarine = dynamic_cast<CASW_Marine*>(UTIL_ASW_NearestMarine( vecPos, flDistance ));
		if ( !pMarine )
			return;

		if ( flDistance > asw_horde_max_distance.GetFloat() || flDistance < asw_horde_min_distance.GetFloat() )
			continue;

		// check node isn't in an exit trigger
		bool bInsideEscapeArea = false;
		for ( int d=0; d<m_EscapeTriggers.Count(); d++ )
		{
			if ( m_EscapeTriggers[d]->CollisionProp()->IsPointInBounds( vecPos ) )
			{
				bInsideEscapeArea = true;
				break;
			}
		}
		if ( bInsideEscapeArea )
			continue;

		if ( vecPos.y >= vecSouthMarine.y )
		{
			if ( asw_director_debug.GetInt() == 3 )
			{
				NDebugOverlay::Box( vecPos, -Vector( 5, 5, 5 ), Vector( 5, 5, 5 ), 32, 32, 128, 10, 60.0f );
			}
			m_northCandidateNodes.AddToTail( i );
		}
		if ( vecPos.y <= vecNorthMarine.y )
		{
			m_southCandidateNodes.AddToTail( i );
			if ( asw_director_debug.GetInt() == 3 )
			{
				NDebugOverlay::Box( vecPos, -Vector( 5, 5, 5 ), Vector( 5, 5, 5 ), 128, 32, 32, 10, 60.0f );
			}
		}
	}
}

bool CASW_Spawn_Manager::FindHordePosition()
{
	// need to find a suitable place from which to spawn a horde
	// this place should:
	//   - be far enough away from the marines so the whole horde can spawn before the marines get there
	//   - should have a clear path to the marines

	UpdateCandidateNodes();

	// decide if the horde is going to come from behind or in front
	bool bNorth = RandomFloat() < 0.7f;
	if ( m_northCandidateNodes.Count() <= 0 )
	{
		bNorth = false;
	}
	else if ( m_southCandidateNodes.Count() <= 0 )
	{
		bNorth = true;
	}

	CUtlVector<int> &candidateNodes = bNorth ? m_northCandidateNodes : m_southCandidateNodes;

	if ( candidateNodes.Count() <= 0 )
	{
		if ( asw_director_debug.GetBool() )
		{
			Msg( "  Failed to find horde pos as there are no candidate nodes\n" );
		}
		return false;
	}

	int iMaxTries = 3;
	for ( int i=0 ; i<iMaxTries ; i++ )
	{
		int iChosen = RandomInt( 0, candidateNodes.Count() - 1);
		CAI_Node *pNode = GetNetwork()->GetNode( candidateNodes[iChosen] );
		if ( !pNode )
			continue;

		float flDistance = 0;
		CASW_Marine *pMarine = dynamic_cast<CASW_Marine*>(UTIL_ASW_NearestMarine( pNode->GetPosition( CANDIDATE_ALIEN_HULL ), flDistance ));
		if ( !pMarine )
		{
			if ( asw_director_debug.GetBool() )
			{
				Msg( "  Failed to find horde pos as there is no nearest marine\n" );
			}
			return false;
		}

		// check if there's a route from this node to the marine(s)
		AI_Waypoint_t *pRoute = ASWPathUtils()->BuildRoute( pNode->GetPosition( CANDIDATE_ALIEN_HULL ), pMarine->GetAbsOrigin(), NULL, 100 );
		if ( !pRoute )
		{
			if ( asw_director_debug.GetInt() >= 2 )
			{
				Msg( "  Discarding horde node %d as there's no route.\n", iChosen );
			}
			continue;
		}

		if ( bNorth && UTIL_ASW_DoorBlockingRoute( pRoute, true ) )
		{
			if ( asw_director_debug.GetInt() >= 2 )
			{
				Msg( "  Discarding horde node %d as there's a door in the way.\n", iChosen );
			}
			DeleteRoute( pRoute );
			continue;
		}

		m_vecHordePosition = pNode->GetPosition( CANDIDATE_ALIEN_HULL ) + Vector( 0, 0, 32 );

		// spawn facing the nearest marine
		Vector vecDir = pMarine->GetAbsOrigin() - m_vecHordePosition;
		vecDir.z = 0;
		vecDir.NormalizeInPlace();
		VectorAngles( vecDir, m_angHordeAngle );

		if ( asw_director_debug.GetInt() >= 2 )
		{
			Msg( "  Accepting horde node %d.\n", iChosen );
		}
		DeleteRoute( pRoute );
		return true;
	}

	if ( asw_director_debug.GetBool() )
	{
		Msg( "  Failed to find horde pos as we tried 3 times to build routes to possible locations, but failed\n" );
	}

	return false;
}

bool CASW_Spawn_Manager::LineBlockedByGeometry( const Vector &vecSrc, const Vector &vecEnd )
{
	trace_t tr;
	UTIL_TraceLine( vecSrc,
		vecEnd, MASK_SOLID_BRUSHONLY, 
		NULL, COLLISION_GROUP_NONE, &tr );

	return ( tr.fraction != 1.0f );
}

bool CASW_Spawn_Manager::GetAlienBounds( const char *szAlienClass, Vector &vecMins, Vector &vecMaxs )
{
	int nCount = GetNumAlienClasses();
	for ( int i = 0 ; i < nCount; i++ )
	{
		if ( !Q_stricmp( szAlienClass, GetAlienClass( i )->m_pszAlienClass ) )
		{
			vecMins = NAI_Hull::Mins( GetAlienClass( i )->m_nHullType );
			vecMaxs = NAI_Hull::Maxs (GetAlienClass( i )->m_nHullType );
			return true;
		}
	}
	return false;
}

bool CASW_Spawn_Manager::GetAlienBounds( string_t iszAlienClass, Vector &vecMins, Vector &vecMaxs )
{
	int nCount = GetNumAlienClasses();
	for ( int i = 0 ; i < nCount; i++ )
	{
		if ( iszAlienClass == GetAlienClass( i )->m_iszAlienClass )
		{
			vecMins = NAI_Hull::Mins( GetAlienClass( i )->m_nHullType );
			vecMaxs = NAI_Hull::Maxs (GetAlienClass( i )->m_nHullType );
			return true;
		}
	}
	return false;
}

// spawn a group of aliens at the target point
int CASW_Spawn_Manager::SpawnAlienBatch( const char* szAlienClass, int iNumAliens, const Vector &vecPosition, const QAngle &angFacing, float flMarinesBeyondDist )
{
	//Ch1ckensCoop: Make sure we're not over 1800 edicts to prevent crashing and leave some room for normal spawners
	if (gEntList.NumberOfEdicts() > 1800)
	{
		if (asw_director_debug.GetBool())
			Msg("Current edicts is over 1800; prevented director from spawning aliens.\n");
		return 0;
	}

	int iSpawned = 0;
	bool bCheckGround = true;
	Vector vecMins = NAI_Hull::Mins(HULL_MEDIUMBIG);
	Vector vecMaxs = NAI_Hull::Maxs(HULL_MEDIUMBIG);
	GetAlienBounds( szAlienClass, vecMins, vecMaxs );

	float flAlienWidth = vecMaxs.x - vecMins.x;
	float flAlienDepth = vecMaxs.y - vecMins.y;

	// spawn one in the middle
	if ( ValidSpawnPoint( vecPosition, vecMins, vecMaxs, bCheckGround, flMarinesBeyondDist ) )
	{
		if ( SpawnAlienAt( szAlienClass, vecPosition, angFacing ) )
			iSpawned++;
	}

	// try to spawn a 5x5 grid of aliens, starting at the centre and expanding outwards
	Vector vecNewPos = vecPosition;
	for ( int i=1; i<=5 && iSpawned < iNumAliens; i++ )
	{
		QAngle angle = angFacing;
		angle[YAW] += RandomFloat( -20, 20 );
		// spawn aliens along top of box
		for ( int x=-i; x<=i && iSpawned < iNumAliens; x++ )
		{
			vecNewPos = vecPosition;
			vecNewPos.x += x * flAlienWidth;
			vecNewPos.y -= i * flAlienDepth;
			if ( !LineBlockedByGeometry( vecPosition, vecNewPos) && ValidSpawnPoint( vecNewPos, vecMins, vecMaxs, bCheckGround, flMarinesBeyondDist ) )
			{
				if ( SpawnAlienAt( szAlienClass, vecNewPos, angle ) )
					iSpawned++;
			}
		}

		// spawn aliens along bottom of box
		for ( int x=-i; x<=i && iSpawned < iNumAliens; x++ )
		{
			vecNewPos = vecPosition;
			vecNewPos.x += x * flAlienWidth;
			vecNewPos.y += i * flAlienDepth;
			if ( !LineBlockedByGeometry( vecPosition, vecNewPos) && ValidSpawnPoint( vecNewPos, vecMins, vecMaxs, bCheckGround, flMarinesBeyondDist ) )
			{
				if ( SpawnAlienAt( szAlienClass, vecNewPos, angle ) )
					iSpawned++;
			}
		}

		// spawn aliens along left of box
		for ( int y=-i + 1; y<i && iSpawned < iNumAliens; y++ )
		{
			vecNewPos = vecPosition;
			vecNewPos.x -= i * flAlienWidth;
			vecNewPos.y += y * flAlienDepth;
			if ( !LineBlockedByGeometry( vecPosition, vecNewPos) && ValidSpawnPoint( vecNewPos, vecMins, vecMaxs, bCheckGround, flMarinesBeyondDist ) )
			{
				if ( SpawnAlienAt( szAlienClass, vecNewPos, angle ) )
					iSpawned++;
			}
		}

		// spawn aliens along right of box
		for ( int y=-i + 1; y<i && iSpawned < iNumAliens; y++ )
		{
			vecNewPos = vecPosition;
			vecNewPos.x += i * flAlienWidth;
			vecNewPos.y += y * flAlienDepth;
			if ( !LineBlockedByGeometry( vecPosition, vecNewPos) && ValidSpawnPoint( vecNewPos, vecMins, vecMaxs, bCheckGround, flMarinesBeyondDist ) )
			{
				if ( SpawnAlienAt( szAlienClass, vecNewPos, angle ) )
					iSpawned++;
			}
		}
	}

	// Ch1ckensCoop: If this was our first alien spawned since
	// a new horde was added, play the "horde incoming" sound.
	if ( ASWGameRules() && m_bFreshHorde && iSpawned > 0 )
	{
		ASWGameRules()->BroadcastSound( "Spawner.Horde" );
		m_bFreshHorde = false;
	}

	return iSpawned;
}

CBaseEntity* CASW_Spawn_Manager::SpawnAlienAt(const char* szAlienClass, const Vector& vecPos, const QAngle &angle)
{
	//softcopy; HL2 is available on city17 only
	if (ASWGameRules() && IsHL2Alien(szAlienClass) && !ASWGameRules()->IsCity17Map())
		return NULL;

	CBaseEntity	*pEntity = NULL;	
	pEntity = CreateEntityByName( szAlienClass );
	CAI_BaseNPC	*pNPC = dynamic_cast<CAI_BaseNPC*>(pEntity);

	if ( pNPC )
	{
		pNPC->AddSpawnFlags( SF_NPC_FALL_TO_GROUND );		
	}

	// Strip pitch and roll from the spawner's angles. Pass only yaw to the spawned NPC.
	QAngle angles = angle;
	angles.x = 0.0;
	angles.z = 0.0;	
	pEntity->SetAbsOrigin( vecPos );	
	pEntity->SetAbsAngles( angles );
	UTIL_DropToFloor( pEntity, MASK_SOLID );
	IASW_Spawnable_NPC* pSpawnable = dynamic_cast<IASW_Spawnable_NPC*>(pEntity);
	ASSERT(pSpawnable);	
	if ( !pSpawnable )
	{
		Warning("NULL Spawnable Ent in CASW_Spawn_Manager::SpawnAlienAt! AlienClass = %s\n", szAlienClass);
		UTIL_Remove(pEntity);
		return NULL;
	}

	// have drones unburrow by default, so we don't worry so much about them spawning onscreen
	if ( !Q_strcmp( szAlienClass, "asw_drone" ) )
	{			
		pSpawnable->StartBurrowed();
		pSpawnable->SetUnburrowIdleActivity( NULL_STRING );
		pSpawnable->SetUnburrowActivity( NULL_STRING );
	}

	DispatchSpawn( pEntity );	
	pEntity->Activate();	

	// give our aliens the orders
	pSpawnable->SetAlienOrders(AOT_MoveToNearestMarine, vec3_origin, NULL);

	return pEntity;
}

bool CASW_Spawn_Manager::ValidSpawnPoint( const Vector &vecPosition, const Vector &vecMins, const Vector &vecMaxs, bool bCheckGround, float flMarineNearDistance )
{
	// check if we can fit there

	Vector vecBuffer = Vector(asw_horde_buffer.GetFloat(), asw_horde_buffer.GetFloat(), 0);

	trace_t tr;
	UTIL_TraceHull( vecPosition,
		vecPosition + Vector( 0, 0, 1 ),
		vecMins - vecBuffer,
		vecMaxs + vecBuffer,
		MASK_NPCSOLID,
		NULL,
		COLLISION_GROUP_NONE,
		&tr );

	if( tr.fraction != 1.0 )
		return false;

	// check there's ground underneath this point
	if ( bCheckGround )
	{
		UTIL_TraceHull( vecPosition + Vector( 0, 0, 1 ),
			vecPosition - Vector( 0, 0, 64 ),
			vecMins,
			vecMaxs,
			MASK_NPCSOLID,
			NULL,
			COLLISION_GROUP_NONE,
			&tr );

		if( tr.fraction == 1.0 )
			return false;
	}

	if ( flMarineNearDistance > 0 )
	{
		CASW_Game_Resource* pGameResource = ASWGameResource();
		float distance = 0.0f;
		for ( int i=0 ; i < pGameResource->GetMaxMarineResources() ; i++ )
		{
			CASW_Marine_Resource* pMR = pGameResource->GetMarineResource(i);
			if ( pMR && pMR->GetMarineEntity() && pMR->GetMarineEntity()->GetHealth() > 0 )
			{
				distance = pMR->GetMarineEntity()->GetAbsOrigin().DistTo( vecPosition );
				if ( distance < flMarineNearDistance )
				{
					if (asw_director_debug.GetBool())
						Warning("Marine too close to spawn point!\n");
					return false;
				}
			}
		}
	}

	return true;
}

void CASW_Spawn_Manager::DeleteRoute( AI_Waypoint_t *pWaypointList )
{
	while ( pWaypointList )
	{
		AI_Waypoint_t *pPrevWaypoint = pWaypointList;
		pWaypointList = pWaypointList->GetNext();
		delete pPrevWaypoint;
	}
}

/*bool CASW_Spawn_Manager::SpawnRandomShieldbug()	//softcopy: replaced by SpawnRandomAlienPack
{
	int iNumNodes = g_pBigAINet->NumNodes();
	if ( iNumNodes < 6 )
		return false;

	int nHull = HULL_WIDE_SHORT;
	CUtlVector<CASW_Open_Area*> aAreas;
	for ( int i = 0; i < 6; i++ )
	{
		CAI_Node *pNode = NULL;
		int nTries = 0;
		while ( nTries < 5 && ( !pNode || pNode->GetType() != NODE_GROUND ) )
		{
			pNode = g_pBigAINet->GetNode( RandomInt( 0, iNumNodes ) );
			nTries++;
		}

		if ( pNode )
		{
			CASW_Open_Area *pArea = FindNearbyOpenArea( pNode->GetOrigin(), HULL_MEDIUMBIG );
			if ( pArea && pArea->m_nTotalLinks > 30 )
			{
				// test if there's room to spawn a shieldbug at that spot
				if ( ValidSpawnPoint( pArea->m_pNode->GetPosition( nHull ), NAI_Hull::Mins( nHull ), NAI_Hull::Maxs( nHull ), true, false ) )
				{
					aAreas.AddToTail( pArea );
				}
				else
				{
					delete pArea;
				}
			}
		}
		// stop searching once we have 3 acceptable candidates
		if ( aAreas.Count() >= 3 )
			break;
	}

	// find area with the highest connectivity
	CASW_Open_Area *pBestArea = NULL;
	for ( int i = 0; i < aAreas.Count(); i++ )
	{
		CASW_Open_Area *pArea = aAreas[i];
		if ( !pBestArea || pArea->m_nTotalLinks > pBestArea->m_nTotalLinks )
		{
			pBestArea = pArea;
		}
	}

	if ( pBestArea )
	{
		CBaseEntity *pAlien = SpawnAlienAt( "asw_shieldbug", pBestArea->m_pNode->GetPosition( nHull ), RandomAngle( 0, 360 ) );
		IASW_Spawnable_NPC *pSpawnable = dynamic_cast<IASW_Spawnable_NPC*>( pAlien );
		if ( pSpawnable )
		{
			pSpawnable->SetAlienOrders(AOT_SpreadThenHibernate, vec3_origin, NULL);
		}
		aAreas.PurgeAndDeleteElements();
		return true;
	}

	aAreas.PurgeAndDeleteElements();
	return false;
}*/

Vector TraceToGround( const Vector &vecPos )
{
	trace_t tr;
	UTIL_TraceLine( vecPos + Vector( 0, 0, 100 ), vecPos, MASK_NPCSOLID, NULL, ASW_COLLISION_GROUP_PARASITE, &tr );
	return tr.endpos;
}

/*bool CASW_Spawn_Manager::SpawnRandomParasitePack( int nParasites )	//softcopy: replaced by SpawnRandomAlienPack
{
	int iNumNodes = g_pBigAINet->NumNodes();
	if ( iNumNodes < 6 )
		return false;

	int nHull = HULL_TINY;
	CUtlVector<CASW_Open_Area*> aAreas;
	for ( int i = 0; i < 6; i++ )
	{
		CAI_Node *pNode = NULL;
		int nTries = 0;
		while ( nTries < 5 && ( !pNode || pNode->GetType() != NODE_GROUND ) )
		{
			pNode = g_pBigAINet->GetNode( RandomInt( 0, iNumNodes ) );
			nTries++;
		}

		if ( pNode )
		{
			CASW_Open_Area *pArea = FindNearbyOpenArea( pNode->GetOrigin(), HULL_MEDIUMBIG );
			if ( pArea && pArea->m_nTotalLinks > 30 )
			{
				// test if there's room to spawn a shieldbug at that spot
				if ( ValidSpawnPoint( pArea->m_pNode->GetPosition( nHull ), NAI_Hull::Mins( nHull ), NAI_Hull::Maxs( nHull ), true, false ) )
				{
					aAreas.AddToTail( pArea );
				}
				else
				{
					delete pArea;
				}
			}
		}
		// stop searching once we have 3 acceptable candidates
		if ( aAreas.Count() >= 3 )
			break;
	}

	// find area with the highest connectivity
	CASW_Open_Area *pBestArea = NULL;
	for ( int i = 0; i < aAreas.Count(); i++ )
	{
		CASW_Open_Area *pArea = aAreas[i];
		if ( !pBestArea || pArea->m_nTotalLinks > pBestArea->m_nTotalLinks )
		{
			pBestArea = pArea;
		}
	}

	if ( pBestArea )
	{
		for ( int i = 0; i < nParasites; i++ )
		{
			CBaseEntity *pAlien = SpawnAlienAt( "asw_parasite", TraceToGround( pBestArea->m_pNode->GetPosition( nHull ) ), RandomAngle( 0, 360 ) );
			IASW_Spawnable_NPC *pSpawnable = dynamic_cast<IASW_Spawnable_NPC*>( pAlien );
			if ( pSpawnable )
			{
				pSpawnable->SetAlienOrders(AOT_SpreadThenHibernate, vec3_origin, NULL);
			}
			if ( asw_director_debug.GetBool() && pAlien )
			{
				Msg( "Spawned parasite at %f %f %f\n", pAlien->GetAbsOrigin() );
				NDebugOverlay::Cross3D( pAlien->GetAbsOrigin(), 8.0f, 255, 0, 0, true, 20.0f );
			}
		}
		aAreas.PurgeAndDeleteElements();
		return true;
	}

	aAreas.PurgeAndDeleteElements();
	return false;
}*/

//softcopy: dynamic aliens spawn
bool CASW_Spawn_Manager::SpawnRandomAlienPack( const char *szAlienClass, int nAlienSpawn )
{
	int iNumNodes = g_pBigAINet->NumNodes();
	if ( iNumNodes < 6 && szAlienClass == NULL )
		return false;

	int nHull = 0;
	int nCount = GetNumAlienClasses();
	for ( int i = 0 ; i < nCount; i++ )	//get the Hull type
	{
		if ( !Q_stricmp( szAlienClass, GetAlienClass( i )->m_pszAlienClass ) )
			nHull = GetAlienClass( i )->m_nHullType;
	}

	CUtlVector<CASW_Open_Area*> aAreas;
	for ( int i = 0; i < 6; i++ )
	{
		CAI_Node *pNode = NULL;
		int nTries = 0;
		while ( nTries < 5 && ( !pNode || pNode->GetType() != NODE_GROUND ) )
		{
			pNode = g_pBigAINet->GetNode( RandomInt( 0, iNumNodes ) );
			nTries++;
		}

		if ( pNode )
		{
			CASW_Open_Area *pArea = FindNearbyOpenArea( pNode->GetOrigin(), HULL_MEDIUMBIG );
			if ( pArea && pArea->m_nTotalLinks > 30 )
			{
				// test if there's room to spawn a alien at that spot
				if ( ValidSpawnPoint( pArea->m_pNode->GetPosition( nHull ), NAI_Hull::Mins( nHull ), NAI_Hull::Maxs( nHull ), true, false ) )
				{
					aAreas.AddToTail( pArea );
				}
				else
				{
					delete pArea;
				}
			}
		}
		// stop searching once we have 3 acceptable candidates
		if ( aAreas.Count() >= 3 )
			break;
	}

	// find area with the highest connectivity
	CASW_Open_Area *pBestArea = NULL;
	for ( int i = 0; i < aAreas.Count(); i++ )
	{
		CASW_Open_Area *pArea = aAreas[i];
		if ( !pBestArea || pArea->m_nTotalLinks > pBestArea->m_nTotalLinks )
		{
			pBestArea = pArea;
		}
	}

	if ( pBestArea )
	{
		int iSpawned = 0;
		for ( int i = 0; i < nAlienSpawn; i++ )
		{
			CBaseEntity *pAlien = NULL;
			pAlien = SpawnAlienAt(szAlienClass, !Q_strcmp(szAlienClass, "asw_parasite") ?
			TraceToGround(pBestArea->m_pNode->GetPosition(nHull)) :	pBestArea->m_pNode->GetPosition(nHull), RandomAngle(0, 360));

			IASW_Spawnable_NPC *pSpawnable = dynamic_cast<IASW_Spawnable_NPC*>( pAlien );
			if ( pSpawnable )
			{
				iSpawned++;
				pSpawnable->SetAlienOrders(AOT_SpreadThenHibernate, vec3_origin, NULL);
			}
			if ( asw_director_debug.GetBool() && pAlien )
			{
				Msg( "Spawned %s at %f %f %f\n", szAlienClass, pAlien->GetAbsOrigin() );
				NDebugOverlay::Cross3D( pAlien->GetAbsOrigin(), 8.0f, 255, 0, 0, true, 20.0f );
			}
		}
		if (iSpawned >0 && asw_debug_alien_spawn.GetBool())
			Msg("%s: %i %s has spawned.\n", "SpawnRandomAlienPack", iSpawned, szAlienClass); 		

		aAreas.PurgeAndDeleteElements();
		return true;
	}

	aAreas.PurgeAndDeleteElements();
	return false;
}
int CASW_Spawn_Manager::SpawnHL2AlienBatch(const char* szAlienClass, int iNumHL2, float flMarinesBeyondDist )
{
	int iSpawned = 0;

	if (!ASWHordeMode())
		return 0;

	const CASW_Horde_Mode::AlienInfo *pHL2 = ASWHordeMode()->GetHL2AlienInfo(ASWHordeMode()->GetHL2Index());
	if (pHL2)
	{	
		const char *szHL2Alien = pHL2->m_szAlienClassName;
		if (!FStrEq(szHL2Alien, szAlienClass))	//spawn HL2 if they are not the same type
		{
			iSpawned = SpawnAlienBatch( szHL2Alien, iNumHL2, m_vecHordePosition, m_angHordeAngle, flMarinesBeyondDist );
			if (iSpawned > 0 && asw_debug_alien_spawn.GetBool())
				Msg("%s: %i %s has spawned.\n", "SpawnHL2AlienBatch", iSpawned, szHL2Alien);
		}
	}

	return iSpawned;
}
void CASW_Spawn_Manager::SpawnHL2AlienClass()	//spawns HL2 aliens when start mission
{
	if (!ASWHordeMode() || !ASWGameRules())
		return;

	for (int i=0; i < ASWHordeMode()->ALIEN_INDEX_COUNT; i++)
	{
		const CASW_Horde_Mode::AlienInfo *pHL2 = ASWHordeMode()->GetHL2AlienInfo(i);
		if (pHL2 && ASWGameRules()->IsCity17Map())
		{
			const char *szHL2Alien = pHL2->m_szAlienClassName;	
			if (SpawnClassPrecache(szHL2Alien))	//spawn after precached 
			{
				int nHL2Alien = RandomInt(2, 6);
				while ( nHL2Alien > 0 )	//random amount
				{
					int nHL2InThisPack = RandomInt( 1,3 );
					if (SpawnRandomAlienPack(szHL2Alien, nHL2InThisPack))
						nHL2Alien -= nHL2InThisPack;
					else
						break;
				}
			}
		}
	}
}
bool CASW_Spawn_Manager::SpawnClassPrecache(const char *szClassName)	//precache spawnable class
{
	CBaseEntity *pEntity = dynamic_cast<CBaseEntity *>(CreateEntityByName(szClassName));
	bool bIsPrecached = false;
	if (pEntity)
	{	
		MDLCACHE_CRITICAL_SECTION();
		bool allowPrecache = CBaseEntity::IsPrecacheAllowed();
		CBaseEntity::SetAllowPrecache( true );
		pEntity->Precache();
		CBaseEntity::SetAllowPrecache( allowPrecache );
		bIsPrecached = true;
	}

	if (asw_debug_alien_spawn.GetBool())
		Msg("%s: %s %s\n", "SpawnClassPrecache", szClassName, bIsPrecached ? "has precached" : "precache failed");

	return bIsPrecached;
}
bool CASW_Spawn_Manager::IsHL2Alien(const char* szAlienClass)	//HL2 aliens validation
{
	if (ASWHordeMode())
	{
		for (int i = 0; i < ASWHordeMode()->ALIEN_INDEX_COUNT; i++)
		{
			const CASW_Horde_Mode::AlienInfo *pHL2 = ASWHordeMode()->GetHL2AlienInfo(i);
			if (pHL2 && FStrEq(pHL2->m_szAlienClassName, szAlienClass))
				return true;
		}
	}
	
	return false;
}
//

// heuristic to find reasonably open space - searches for areas with high node connectivity
CASW_Open_Area* CASW_Spawn_Manager::FindNearbyOpenArea( const Vector &vecSearchOrigin, int nSearchHull )
{
	CBaseEntity *pStartEntity = gEntList.FindEntityByClassname( NULL, "info_player_start" );
	int iNumNodes = g_pBigAINet->NumNodes();
	CAI_Node *pHighestConnectivity = NULL;
	int nHighestLinks = 0;
	for ( int i=0 ; i<iNumNodes; i++ )
	{
		CAI_Node *pNode = g_pBigAINet->GetNode( i );
		if ( !pNode || pNode->GetType() != NODE_GROUND )
			continue;

		Vector vecPos = pNode->GetOrigin();
		float flDist = vecPos.DistTo( vecSearchOrigin );
		if ( flDist > 400.0f )
			continue;

		// discard if node is too near start location
		if ( pStartEntity && vecPos.DistTo( pStartEntity->GetAbsOrigin() ) < 1400.0f )  // NOTE: assumes all start points are clustered near one another
			continue;

		// discard if node is inside an escape area
		bool bInsideEscapeArea = false;
		for ( int d=0; d<m_EscapeTriggers.Count(); d++ )
		{
			if ( m_EscapeTriggers[d]->CollisionProp()->IsPointInBounds( vecPos ) )
			{
				bInsideEscapeArea = true;
				break;
			}
		}
		if ( bInsideEscapeArea )
			continue;

		// count links that drones could follow
		int nLinks = pNode->NumLinks();
		int nValidLinks = 0;
		for ( int k = 0; k < nLinks; k++ )
		{
			CAI_Link *pLink = pNode->GetLinkByIndex( k );
			if ( !pLink )
				continue;

			if ( !( pLink->m_iAcceptedMoveTypes[nSearchHull] & bits_CAP_MOVE_GROUND ) )
				continue;

			nValidLinks++;
		}
		if ( nValidLinks > nHighestLinks )
		{
			nHighestLinks = nValidLinks;
			pHighestConnectivity = pNode;
		}
		if ( asw_director_debug.GetBool() )
		{
			NDebugOverlay::Text( vecPos, UTIL_VarArgs( "%d", nValidLinks ), false, 10.0f );
		}
	}

	if ( !pHighestConnectivity )
		return NULL;

	// now, starting at the new node, find all nearby nodes with a minimum connectivity
	CASW_Open_Area *pArea = new CASW_Open_Area();
	pArea->m_vecOrigin = pHighestConnectivity->GetOrigin();
	pArea->m_pNode = pHighestConnectivity;
	int nMinLinks = nHighestLinks * 0.3f;
	nMinLinks = MAX( nMinLinks, 4 );

	pArea->m_aAreaNodes.AddToTail( pHighestConnectivity );
	if ( asw_director_debug.GetBool() )
	{
		Msg( "minLinks = %d\n", nMinLinks );
	}
	pArea->m_nTotalLinks = 0;
	for ( int i=0 ; i<iNumNodes; i++ )
	{
		CAI_Node *pNode = g_pBigAINet->GetNode( i );
		if ( !pNode || pNode->GetType() != NODE_GROUND )
			continue;

		Vector vecPos = pNode->GetOrigin();
		float flDist = vecPos.DistTo( pArea->m_vecOrigin );
		if ( flDist > 400.0f )
			continue;

		// discard if node is inside an escape area
		bool bInsideEscapeArea = false;
		for ( int d=0; d<m_EscapeTriggers.Count(); d++ )
		{
			if ( m_EscapeTriggers[d]->CollisionProp()->IsPointInBounds( vecPos ) )
			{
				bInsideEscapeArea = true;
				break;
			}
		}
		if ( bInsideEscapeArea )
			continue;

		// count links that drones could follow
		int nLinks = pNode->NumLinks();
		int nValidLinks = 0;
		for ( int k = 0; k < nLinks; k++ )
		{
			CAI_Link *pLink = pNode->GetLinkByIndex( k );
			if ( !pLink )
				continue;

			if ( !( pLink->m_iAcceptedMoveTypes[nSearchHull] & bits_CAP_MOVE_GROUND ) )
				continue;

			nValidLinks++;
		}
		if ( nValidLinks >= nMinLinks )
		{
			pArea->m_aAreaNodes.AddToTail( pNode );
			pArea->m_nTotalLinks += nValidLinks;
		}
	}
	// highlight and measure bounds
	Vector vecAreaMins = Vector( FLT_MAX, FLT_MAX, FLT_MAX );
	Vector vecAreaMaxs = Vector( -FLT_MAX, -FLT_MAX, -FLT_MAX );

	for ( int i = 0; i < pArea->m_aAreaNodes.Count(); i++ )
	{
		vecAreaMins = VectorMin( vecAreaMins, pArea->m_aAreaNodes[i]->GetOrigin() );
		vecAreaMaxs = VectorMax( vecAreaMaxs, pArea->m_aAreaNodes[i]->GetOrigin() );

		if ( asw_director_debug.GetBool() )
		{
			if ( i == 0 )
			{
				NDebugOverlay::Cross3D( pArea->m_aAreaNodes[i]->GetOrigin(), 20.0f, 255, 255, 64, true, 10.0f );
			}
			else
			{
				NDebugOverlay::Cross3D( pArea->m_aAreaNodes[i]->GetOrigin(), 10.0f, 255, 128, 0, true, 10.0f );
			}
		}
	}

	Vector vecArea = ( vecAreaMaxs - vecAreaMins );
	float flArea = vecArea.x * vecArea.y;

	if ( asw_director_debug.GetBool() )
	{
		Msg( "area mins = %f %f %f\n", VectorExpand( vecAreaMins ) );
		Msg( "area maxs = %f %f %f\n", VectorExpand( vecAreaMaxs ) );
		NDebugOverlay::Box( vec3_origin, vecAreaMins, vecAreaMaxs, 255, 128, 128, 10, 10.0f );	
		Msg( "Total links = %d Area = %f\n", pArea->m_nTotalLinks, flArea );
	}

	return pArea;
}

// creates a batch of aliens at the mouse cursor
void asw_alien_batch_f( const CCommand& args )
{
	MDLCACHE_CRITICAL_SECTION();

	bool allowPrecache = CBaseEntity::IsPrecacheAllowed();
	CBaseEntity::SetAllowPrecache( true );

	// find spawn point
	CASW_Player* pPlayer = ToASW_Player(UTIL_GetCommandClient());
	if (!pPlayer)
		return;
	CASW_Marine *pMarine = pPlayer->GetMarine();
	if (!pMarine)
		return;
	trace_t tr;
	Vector forward;

	AngleVectors( pMarine->EyeAngles(), &forward );
	UTIL_TraceLine(pMarine->EyePosition(),
		pMarine->EyePosition() + forward * 300.0f,MASK_SOLID, 
		pMarine, COLLISION_GROUP_NONE, &tr );
	if ( tr.fraction != 0.0 )
	{
		// trace to the floor from this spot
		Vector vecSrc = tr.endpos;
		tr.endpos.z += 12;
		UTIL_TraceLine( vecSrc + Vector(0, 0, 12),
			vecSrc - Vector( 0, 0, 512 ) ,MASK_SOLID, 
			pMarine, COLLISION_GROUP_NONE, &tr );

		ASWSpawnManager()->SpawnAlienBatch( "asw_parasite", 25, tr.endpos, vec3_angle );
	}

	CBaseEntity::SetAllowPrecache( allowPrecache );
}
static ConCommand asw_alien_batch("asw_alien_batch", asw_alien_batch_f, "Creates a batch of aliens at the cursor", FCVAR_GAMEDLL | FCVAR_CHEAT);


void asw_alien_horde_f( const CCommand& args )
{
	if ( args.ArgC() < 2 )
	{
		Msg("supply horde size!\n");
		return;
	}
	if ( !ASWSpawnManager()->AddHorde( atoi(args[1]) ) )
	{
		Msg("Failed to add horde\n");
	}
}
static ConCommand asw_alien_horde("asw_alien_horde", asw_alien_horde_f, "Creates a horde of aliens somewhere nearby", FCVAR_GAMEDLL | FCVAR_CHEAT);


CON_COMMAND_F( asw_spawn_shieldbug, "Spawns a shieldbug somewhere randomly in the map", FCVAR_CHEAT )
{
	//softcopy: replaced function
	//ASWSpawnManager()->SpawnRandomShieldbug();
	ASWSpawnManager()->SpawnRandomAlienPack("asw_shieldbug", 1);
}

CON_COMMAND_F( asw_spawn_parasite_pack, "Spawns a group of parasites somewhere randomly in the map", FCVAR_CHEAT )
{
	//softcopy: replaced function
	//ASWSpawnManager()->SpawnRandomParasitePack( RandomInt( 3, 5 ) );
	ASWSpawnManager()->SpawnRandomAlienPack("asw_parasite", RandomInt( 3, 5 ));
}