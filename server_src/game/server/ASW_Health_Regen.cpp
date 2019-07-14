#include "cbase.h"
#include "convar.h"
#include "entitylist.h"
#include "asw_weapon.h"
#include "asw_marine.h"
#include "ASW_Health_Regen.h"
#include "asw_marine_profile.h"
#include "asw_colonist.h"	//softcopy:

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//Ch1ckensCoop: Replacement for spamming medkits.

ConVar asw_marine_health_regen_speed("asw_marine_health_regen_speed", "0.5", FCVAR_CHEAT, "Adjusts the number of seconds between healing marines this amount.", true, 0.01f, false, 0.0f);
ConVar asw_marine_health_regen_amount("asw_marine_health_regen_amount", "1", FCVAR_CHEAT, "Adjusts the amount that is healed every <asw_marine_health_regen_speed> seconds.");
ConVar asw_marine_health_regen_amount_low("asw_marine_health_regen_amount_low", "10", FCVAR_CHEAT, "Adjusts the amount that is healed after crossing below the threshold.");
ConVar asw_marine_health_regen_threshold("asw_marine_health_regen_threshold", "0.2", FCVAR_CHEAT, "Adjusts the threshold below which <asw_marine_health_regen_amount_low> takes effect.", true, 0.0f, true, 1.0f);
ConVar asw_marine_health_regen_infestation_boost("asw_marine_health_regen_infestation_boost", "5", FCVAR_CHEAT, "Boost the healing by this many points if the marine is infested and below the threshold.");
ConVar asw_marine_health_regen_medkitcheck("asw_marine_health_regen_medkitcheck", "1", FCVAR_CHEAT, "Don't heal a marine if he's carrying a medkit.", true, 0.0f, true, 1.0f);	//softcopy: typo cvar
ConVar asw_marine_health_regen_medicboost("asw_marine_health_regen_medicboost", "1", FCVAR_CHEAT, "Medics get this much more healing every think compared to normal marines.");
//softcopy:
ConVar asw_colonist_health_regen_speed("asw_colonist_health_regen_speed", "0.5", FCVAR_CHEAT, "Adjusts the number of seconds between healing colonists this amount.", true, 0.01f, false, 0.0f);
ConVar asw_colonist_health_regen_amount("asw_colonist_health_regen_amount", "3", FCVAR_CHEAT, "Adjusts the amount that is healed every <asw_colonist_health_regen_speed> seconds.");
ConVar asw_colonist_health_regen_amount_low("asw_colonist_health_regen_amount_low", "10", FCVAR_CHEAT, "Adjusts the amount that is healed after crossing below the threshold.");
ConVar asw_colonist_health_regen_threshold("asw_colonist_health_regen_threshold", "0.2", FCVAR_CHEAT, "Adjusts the threshold below which <asw_colonist_health_regen_amount_low> takes effect.", true, 0.0f, true, 1.0f);
ConVar asw_colonist_health_regen_infestation_boost("asw_colonist_health_regen_infestation_boost", "5", FCVAR_CHEAT, "Boost the healing by this many points if the colonist is infested and below the threshold.");


LINK_ENTITY_TO_CLASS( asw_health_regen, CASW_Health_Regen );

CASW_Health_Regen::CASW_Health_Regen(void)
{
}

CASW_Health_Regen::~CASW_Health_Regen(void)
{
}

void CASW_Health_Regen::Spawn()
{
	BaseClass::Spawn();
	SetNextThink( gpGlobals->curtime ); // Think now
}

void CASW_Health_Regen::Think()
{
	BaseClass::Think();
	CBaseEntity* pEntity = NULL;
	float fHealthRegenSpeed = 0;	//softcopy:
	while ((pEntity = gEntList.FindEntityByClassname( pEntity, "asw_marine" )) != NULL)
	{
		CASW_Marine *pMarine = dynamic_cast<CASW_Marine*>(pEntity);
		if (pMarine)
		{
			float currentHealth = pMarine->GetHealth();
			float maxHealth = pMarine->GetMaxHealth();
			int normalHealing = asw_marine_health_regen_amount.GetInt();
			int lowHealing = asw_marine_health_regen_amount_low.GetInt();
			int infestedLowHealing = asw_marine_health_regen_infestation_boost.GetInt() + asw_marine_health_regen_amount_low.GetInt();
			float threshold = asw_marine_health_regen_threshold.GetFloat();
			int medicHealing = asw_marine_health_regen_medicboost.GetInt();
			fHealthRegenSpeed = asw_marine_health_regen_speed.GetFloat();	//softcopy:

			int medicBoost = 0;

			if ( pMarine->GetMarineProfile() && pMarine->GetMarineProfile()->CanUseFirstAid() )
			{
				medicBoost = medicHealing;
			}

			if (currentHealth < maxHealth)
			{
				//Check if this player is carrying a medkit, and don't heal them if they are.
				CASW_Weapon *pExtra = pMarine->GetASWWeapon(2);
				if (pExtra)
				{
					if (pExtra->Classify() == CLASS_ASW_MEDKIT && asw_marine_health_regen_medkitcheck.GetBool())
						continue;
				}

				//Check if this marine is dead.
				if (currentHealth < 1)
					continue;
	
				if ((currentHealth / maxHealth) < threshold)
				{
					if (pMarine->IsInfested())
						SetMarineHealth(pMarine, currentHealth + infestedLowHealing + medicBoost);
					else
						SetMarineHealth(pMarine, currentHealth + lowHealing + medicBoost);
				}
				else
					SetMarineHealth(pMarine, currentHealth + normalHealing + medicBoost);
			}
		}
	}

	//softcopy: Colonist regen health
	while ((pEntity = gEntList.FindEntityByClassname( pEntity, "asw_colonist" )) != NULL)
	{
		CASW_Colonist* pColonist = dynamic_cast<CASW_Colonist*>(pEntity);
		if (pColonist)
		{
			float currentHealth = pColonist->GetHealth();
			float maxHealth = pColonist->GetMaxHealth();
			int normalHealing = asw_colonist_health_regen_amount.GetInt();
			int lowHealing = asw_colonist_health_regen_amount_low.GetInt();
			int infestedLowHealing = asw_colonist_health_regen_infestation_boost.GetInt() + asw_colonist_health_regen_amount_low.GetInt();
			float threshold = asw_colonist_health_regen_threshold.GetFloat();
			fHealthRegenSpeed = asw_colonist_health_regen_speed.GetFloat();

			if (currentHealth < maxHealth)
			{
				//Check if this colonist is dead.
				if (currentHealth < 1)
					continue;
	
				if ((currentHealth / maxHealth) < threshold)
				{
					if (pColonist->IsInfested())
						SetColonistHealth(pColonist, currentHealth + infestedLowHealing);
					else
						SetColonistHealth(pColonist, currentHealth + lowHealing);
				}
				else
					SetColonistHealth(pColonist, currentHealth + normalHealing);
			}
		}
	}

	//softcopy:
	//SetNextThink( gpGlobals->curtime + asw_marine_health_regen_speed.GetFloat());
	SetNextThink( gpGlobals->curtime + fHealthRegenSpeed );

}

void CASW_Health_Regen::SetMarineHealth(CASW_Marine *pMarine, int iHealth)
{
	if (pMarine)
	{
		// Ch1ckensCoop: Fire event for statistics
		IGameEvent * pEvent = gameeventmanager->CreateEvent( "marine_regenerated" );
		if (pEvent)
		{
			pEvent->SetInt("marine", pMarine->entindex());
			pEvent->SetInt("amount", iHealth - pMarine->GetHealth());
		}

		pMarine->SetHealth(iHealth);
	}
}

//softcopy:
void CASW_Health_Regen::SetColonistHealth(CASW_Colonist *pColonist, int iHealth)
{
	if (pColonist)
	{
		pColonist->SetHealth(iHealth);
	}
}