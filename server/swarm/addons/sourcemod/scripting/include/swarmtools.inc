#if defined _swarmtools_included
  #endinput
#endif
#define _swarmtools_included

// do not edit this line
static ammoOffset = -1;

enum SwarmAmmoType
{
	SwarmAmmo_AR2,
	SwarmAmmo_Rifle,
	SwarmAmmo_RifleGrenades,
	SwarmAmmo_Autogun,
	SwarmAmmo_Shotgun,
	SwarmAmmo_AssaultShotgun,
	SwarmAmmo_Flamer,
	SwarmAmmo_Pistol,
	SwarmAmmo_MiningLaser,
	SwarmAmmo_TeslaGun,
	SwarmAmmo_Railun,
	SwarmAmmo_ChainSaw,
	SwarmAmmo_Flares,
	SwarmAmmo_Medkit,
	SwarmAmmo_MedSatchel,
	SwarmAmmo_MedSatchelSelf,
	SwarmAmmo_Stim,
	SwarmAmmo_Welder,
	SwarmAmmo_FireExtinguisher,
	SwarmAmmo_VGrenades,
	SwarmAmmo_PDW,
	SwarmAmmo_HandGrenades,
	SwarmAmmo_GrenadeLauncher,
	SwarmAmmo_SniperRifle,
}

/**
 * Returns whether or not the game is active. If not, all the below calls will error.
 *
 * @return              True if game is active, false otherwise
 */
native bool:Swarm_IsGameActive();

/**
 * Gets the marine entity controlled by the given player
 *
 * @param client        Client index
 * @return              Marine entity index or -1 for none or not found
 * @error               Invalid client index
 */
native Swarm_GetMarine(client);

/**
 * Gets the client index controlling given marine
 *
 * @param marine        Marine entity index
 * @return              Client index of controller or -1 for none or not found
 * @error               Invalid marine index
 */
stock Swarm_GetClientOfMarine(marine)
{
	return GetEntPropEnt(marine, Prop_Send, "m_Commander");
}

/**
 * Gets the marine resource entity index of given client
 *
 * @param client        Client index
 * @return              Marine resource entity index of client or -1 for none or not found
 * @error               Invalid client index
 */
native Swarm_GetMarineResFromCommander(client);

/**
 * Gets the marine resource entity index of given marine
 *
 * @param marine        Marine entity index
 * @return              Marine resource entity index of entity or -1 for none or not found
 * @error               Invalid marine index
 */
native Swarm_GetMarineResOfMarine(marine);

/**
 * Returns the weapon in a marine's slot.
 *
 * @param marine        Marine entity index
 * @param slot          Slot index (0-based)
 * @return              Entity index on success, -1 if no weapon existed.
 * @error               Invalid entity or unable to find marine's weapons
 */
stock Swarm_GetMarineWeaponSlot(marine, slot)
{
	static weaponsOffset = -1;
	if (weaponsOffset == -1)
		weaponsOffset = FindSendPropInfo("CASW_Marine", "m_hMyWeapons");
	
	if (weaponsOffset == -1)
		ThrowError("Cannot find m_hMyWeapons offset on CASW_Marine");
	
	return GetEntDataEnt2(marine, weaponsOffset + (slot * 4));
}

/**
 * Returns the marine's current weapon.
 *
 * @param marine        Marine entity index
 * @return              Entity index on success, -1 if no weapon existed.
 * @error               Invalid entity
 */
stock Swarm_GetMarineActiveWeapon(marine)
{
	return GetEntPropEnt(marine, Prop_Send, "m_hActiveWeapon");
}

/**
 * Returns the marine's health.
 *
 * @param marine        Marine entity index
 * @return              Health value
 * @error               Invalid entity
 */
stock Swarm_GetMarineHealth(marine)
{
	return GetEntProp(marine, Prop_Send, "m_iHealth");
}

/**
 * Set the marine's health. (same as SetEntityHealth)
 *
 * @param marine        Marine entity index
 * @param amount        Health amount
 * @noreturn
 * @error               Invalid entity
 */
stock Swarm_SetMarineHealth(marine, health)
{
	return SetEntProp(marine, Prop_Send, "m_iHealth", health);
}

/**
 * Returns the marine's max health.
 *
 * @param marine        Marine entity index
 * @return              Max Health value
 * @error               Invalid entity
 */
stock Swarm_GetMarineMaxHealth(marine)
{
	return GetEntProp(marine, Prop_Send, "m_iMaxHealth");
}

/**
 * Returns the marine's ammo amount of the specified type.
 *
 * @param marine        Marine entity index
 * @return              Amount
 * @error               Invalid entity or unable to find marine ammo offset
 */
stock Swarm_GetMarineAmmo(marine, SwarmAmmoType:ammotype)
{
	if (ammoOffset == -1)
		ammoOffset = FindSendPropInfo("CASW_Marine", "m_iAmmo");
	
	if (ammoOffset == -1)
		ThrowError("Cannot find m_iAmmo offset on CASW_Marine");
	
	return GetEntData(marine, ammoOffset + (ammotype * 4));
}

/**
 * Set the marine's ammo amount of the specified type.
 *
 * @param marine        Marine entity index
 * @param amount        Amount
 * @noreturn
 * @error               Invalid entity or unable to find marine ammo offset
 */
stock Swarm_SetMarineAmmo(marine, SwarmAmmoType:ammotype, amount)
{
	if (ammoOffset == -1)
		ammoOffset = FindSendPropInfo("CASW_Marine", "m_iAmmo");
	
	if (ammoOffset == -1)
		ThrowError("Cannot find m_iAmmo offset on CASW_Marine");
	
	return SetEntData(marine, ammoOffset + (ammotype * 4), amount);
}

/**
 * Returns if a marine is infested.
 *
 * @param marine        Marine entity index
 * @return              True if marine is infested, false otherwise
 * @error               Invalid entity
 */
stock bool:Swarm_IsMarineInfested(marine)
{
	return bool:GetEntProp(Swarm_GetMarineResOfMarine(marine), Prop_Send, "m_bInfested");
}

/**
 * Causes a marine to become infested.
 *
 * @param marine        Marine entity index
 * @param duration      Length of infestion
 * @noreturn
 * @error               Function not found or invalid marine
 */
native Swarm_StartMarineInfestation(marine, Float:duration=20.0);

/**
 * Cures a marine's infestation.
 *
 * @param marine        Marine entity index
 * @noreturn
 * @error               Invalid entity
 */
stock Swarm_CureMarineInfestation(marine)
{
	SetEntPropFloat(marine, Prop_Send, "m_fInfestedTime");
}

/**
 * Returns if a marine is on fire.
 *
 * @param marine        Marine entity index
 * @return              True if marine is on fire, false otherwise
 * @error               Invalid entity
 */
stock bool:Swarm_IsMarineOnFire(marine)
{
	return bool:GetEntProp(marine, Prop_Send, "m_bOnFire");
}

/**
 * Adds a weapon to the marine's inventory and equips it.
 *
 * @param marine        Marine entity index
 * @param weapon        Weapon entity index
 * @noreturn
 * @error               Function not found or invalid marine
 */
native Swarm_EquipMarineWeapon(marine, weapon);

/**
 * Forces a marine to drop the specified weapon.
 *
 * @param marine        Marine entity index
 * @param weapon        Weapon entity index
 * @noreturn
 * @error               Function not found or invalid marine
 */
native Swarm_DropMarineWeapon(marine, weapon);
 
 /**
 * Forces a marine to commit suicide.
 *
 * @param marine        Marine entity index
 * @noreturn
 * @error               Function not found or invalid marine
 */
native Swarm_ForceMarineSuicide(marine);
 
/**
 * Slaps a marine in a random direction.
 *
 * @param marine        Marine entity index
 * @param amount        Health to subtract
 * @param bsound        False to disable sound effects
 * @noreturn
 * @error               Function not found or invalid marine
 */
native Swarm_SlapMarine(marine, amount=5, bool:bSound=true);

/**
 * Returns the marines's eye angles.
 *
 * @param marine        Marine entity index
 * @param ang           Destination vector to store the marine's eye angles
 * @noreturn
 * @error               Function not found or invalid marine
 */
native Swarm_GetMarineEyeAngles(marine, Float:ang[3]);


public SharedPlugin:__pl_swarmtools = 
{
	name = "swarmtools",
	file = "swarmtools.smx",
	#if defined REQUIRE_PLUGIN
	required = 1,
	#else
	required = 0,
	#endif
};

public __pl_swarmtools_SetNTVOptional()
{
	MarkNativeAsOptional("Swarm_GetMarine");
	MarkNativeAsOptional("Swarm_GetMarineResFromCommander");
	MarkNativeAsOptional("Swarm_GetMarineResOfMarine");
	MarkNativeAsOptional("Swarm_StartMarineInfestation");
	MarkNativeAsOptional("Swarm_EquipMarineWeapon");
	MarkNativeAsOptional("Swarm_DropMarineWeapon");
	MarkNativeAsOptional("Swarm_ForceMarineSuicide");
	MarkNativeAsOptional("Swarm_SlapMarine");
	MarkNativeAsOptional("Swarm_GetMarineEyeAngles");
}
