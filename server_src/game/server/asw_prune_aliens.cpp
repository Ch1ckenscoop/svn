#include "cbase.h"
#include "convar.h"
#include "entitylist.h"
#include "vprof.h"
#include "asw_prune_aliens.h"
#include "util_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar asw_alien_prune_distance("asw_alien_prune_distance", "3000", FCVAR_CHEAT, "Distance from nearest marine at which aliens are removed.");
ConVar asw_alien_prune_debug("asw_alien_prune_debug", "1", FCVAR_NONE, "Show debug messages for alien pruning.");
ConVar asw_alien_prune("asw_alien_prune", "1", FCVAR_NONE, "Set to 1 to enable alien pruning.");

CASW_Prune_Aliens::CASW_Prune_Aliens(void)
{
}

CASW_Prune_Aliens::~CASW_Prune_Aliens(void)
{
}

LINK_ENTITY_TO_CLASS( asw_alien_pruner, CASW_Prune_Aliens );

void CASW_Prune_Aliens::Spawn()
{
	BaseClass::Spawn();
	SetNextThink( gpGlobals->curtime ); // Think now
}

void CASW_Prune_Aliens::Think()
{
	if (asw_alien_prune.GetBool())
	{
	//TODO: Move this into each alien type's NPCThink() function to prevent this single entity from doing all the work.
	BaseClass::Think();
	CBaseEntity* pEntity = NULL;
	CBaseEntity* pEntity2 = NULL;
	int AliensRemoved = 0;
	int LoopCount = 0;
	bool FoundMarine = false;
	while ((pEntity = gEntList.NextEnt( pEntity )) != NULL)
	{
		const char *className = pEntity->GetClassname();
		if (strcmp(className, "asw_drone") == 0 || strcmp(className, "asw_drone_jumper") == 0 || strcmp(className, "asw_drone_uber") == 0 || strcmp(className, "asw_parasite") == 0 || strcmp(className, "asw_parasite_defanged") == 0 || strcmp(className, "asw_harvester") == 0 || strcmp(className, "asw_ranger") == 0)
		{
			while((pEntity2 = gEntList.FindEntityInSphere(pEntity2, pEntity->GetAbsOrigin(), asw_alien_prune_distance.GetFloat())) != NULL)
			{
				if (strcmp(pEntity2->GetClassname(), "asw_marine") == 0)
				{
					FoundMarine = true;
					continue;
				}
				LoopCount++;
			}
			if (!FoundMarine)
			{
				UTIL_Remove(pEntity);
				AliensRemoved++;
			}
		}
	}
	if (asw_alien_prune_debug.GetBool() && AliensRemoved != 0)
		Msg("Removed %i aliens and looped %i times.\n", AliensRemoved, LoopCount);
	}
	SetNextThink( gpGlobals->curtime + 5); // Think in 5 seconds
}
