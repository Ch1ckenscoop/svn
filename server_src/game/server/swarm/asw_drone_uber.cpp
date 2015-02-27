// A tougher version of the standard Swarm drone.  It's green, bigger and has more health.
#include "cbase.h"
#include "asw_drone_uber.h"
#include "asw_gamerules.h"
#include "asw_marine.h"
#include "asw_weapon_assault_shotgun_shared.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//Ch1ckenscoop - drone uber color mod
ConVar asw_drone_uber_color("asw_drone_uber_color", "255 255 255", FCVAR_NONE, "Color255: Adjusts the Uber drones' color.");

//ConVar asw_drone_uber_color_r("asw_drone_uber_color_r", "255", FCVAR_NONE, "Adjusts the red componant of the Uber drones' color.", true, 0.0f, true, 255.0f);
//ConVar asw_drone_uber_color_g("asw_drone_uber_color_g", "255", FCVAR_NONE, "Adjusts the green componant of the Uber drones' color.", true, 0.0f, true, 255.0f);
//ConVar asw_drone_uber_color_b("asw_drone_uber_color_b", "255", FCVAR_NONE, "Adjusts the blue componant of the Uber drones' color.", true, 0.0f, true, 255.0f);
// Doesn't change anything - ConVar asw_drone_uber_color("asw_drone_uber_color", "1", FCVAR_NONE, "Enables/disables the Color rendermode for the Uber drones.");
ConVar asw_drone_uber_scale("asw_drone_uber_scale", "1.0", FCVAR_CHEAT, "Scales the Uber drone model.");
//softcopy: 
ConVar asw_drone_uber_color2("asw_drone_uber_color2", "255 255 255", FCVAR_NONE, "Sets the color of drone_ubers.");
ConVar asw_drone_uber_color2_percent("asw_drone_uber_color2_percent", "0.0", FCVAR_NONE, "Sets the percentage of the drone_ubers you want to give the color",true,0,true,1);
ConVar asw_drone_uber_color3("asw_drone_uber_color3", "255 255 255", FCVAR_NONE, "Sets the color of drone_ubers.");
ConVar asw_drone_uber_color3_percent("asw_drone_uber_color3_percent", "0.0", FCVAR_NONE, "Sets the percentage of the drone_ubers you want to give the color",true,0,true,1);
ConVar asw_drone_uber_scalemod("asw_drone_uber_scalemod", "1.0", FCVAR_NONE, "Sets the scale of the normal drone_ubers.");
ConVar asw_drone_uber_scalemod_percent("asw_drone_uber_scalemod_percent", "1.0", FCVAR_NONE, "Sets the percentage of the normal drone_ubers you want to scale.",true,0,true,1);

ConVar asw_drone_uber_health("asw_drone_uber_health", "500", FCVAR_CHEAT, "How much health the uber Swarm drones have");
ConVar asw_uber_speed_scale("asw_uber_speed_scale", "0.5f", FCVAR_CHEAT, "Speed scale of uber drone compared to normal");
ConVar asw_uber_auto_speed_scale("asw_uber_auto_speed_scale", "0.3f", FCVAR_CHEAT, "Speed scale of uber drones when attacking");
ConVar asw_uber_scary("asw_uber_scary", "1", FCVAR_NONE, "Set bodygroups on ubers to the scariest appendage.");

ConVar sk_asw_uber_damage("sk_asw_uber_damage", "15", FCVAR_CHEAT, "Damage inflicted by uber drone attacks.");

extern ConVar asw_alien_hurt_speed;
extern ConVar asw_alien_stunned_speed;

#define	SWARM_DRONE_UBER_MODEL	"models/swarm/drone/UberDrone.mdl"

CASW_Drone_Uber::CASW_Drone_Uber()	
{
	
}

CASW_Drone_Uber::~CASW_Drone_Uber()
{

}

LINK_ENTITY_TO_CLASS( asw_drone_uber, CASW_Drone_Uber );

BEGIN_DATADESC( CASW_Drone_Uber )

END_DATADESC()

void CASW_Drone_Uber::Spawn( void )
{	
	BaseClass::Spawn();

	//SetRenderColor(asw_drone_uber_color_r.GetInt(), asw_drone_uber_color_g.GetInt(), asw_drone_uber_color_b.GetInt());
	//SetRenderColor(asw_drone_uber_color.GetColor().r(), asw_drone_uber_color.GetColor().g(), asw_drone_uber_color.GetColor().b());	//softcopy:


	SetModel( SWARM_NEW_DRONE_MODEL );	
	Precache();	

	SetHullType(HULL_MEDIUMBIG);
	SetHullSizeNormal();
	
	//UTIL_SetSize(this, Vector(-40,-40,0), Vector(40,40,130)); //ASBI settings
	//UTIL_SetSize(this, Vector(-17,-17,0), Vector(17,17,69)); //Standard drone settings
	UTIL_SetSize(this, Vector(-17,-17,20), Vector(17,17,69));
	// make sure uber drones are green
	m_nSkin = 0;
	SetHitboxSet(0);
	//Ch1ckensCoop: Made uber drones bigger
	//softcopy: set uber colors & scale
	//SetModelScale(asw_drone_uber_scale.GetFloat(), 0.0f);
	SetColorScale( "drone_uber" );	

	if (asw_uber_scary.GetBool())	//Ch1ckensCoop: Make uber drones SCARY :S
	{
		SetBodygroup ( 0, 0 );	//beefier body

		SetBodygroup ( 1, 2 );	//longest claws
		SetBodygroup ( 2, 2 );
		SetBodygroup ( 3, 2 );
		SetBodygroup ( 4, 2 );

		SetBodygroup ( 5, 1 );	//bones from back
	}
}

void CASW_Drone_Uber::Precache( void )
{
	PrecacheModel( SWARM_NEW_DRONE_MODEL );

	BaseClass::Precache();
}

bool CASW_Drone_Uber::ShouldMoveSlow() const
{
	return false;
}

void CASW_Drone_Uber::SetHealthByDifficultyLevel()
{
	SetHealth(ASWGameRules()->ModifyAlienHealthBySkillLevel(asw_drone_uber_health.GetInt()));
	SetMaxHealth(GetHealth());
	//if (ASWGameRules()->GetSkillLevel() <= 1)	// on easy we use the bigger hitbox set
		SetHitboxSet(0);
	//else
		//SetHitboxSet(2);
}

float CASW_Drone_Uber::GetDamage()	//Ch1ckensCoop: Easy customizing of alien damages.
{
	return sk_asw_uber_damage.GetFloat();
}

//softcopy:
void CASW_Drone_Uber::SetColorScale(const char *alienLabel)	
{
	BaseClass::SetColorScale(alienLabel);
	
	if (asw_drone_uber_scalemod.GetFloat() < asw_drone_uber_scale.GetFloat())	//avoid duplicated uber scale entities
		SetModelScale(asw_drone_uber_scale.GetFloat());

}

float CASW_Drone_Uber::GetIdealSpeed() const
{
	return BaseClass::GetIdealSpeed() * asw_uber_speed_scale.GetFloat();
}

int CASW_Drone_Uber::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	int result = 0;

	CTakeDamageInfo newInfo(info);
	float damage = info.GetDamage();

	// reduce damage from shotguns and mining laser
	if (info.GetDamageType() & DMG_ENERGYBEAM)
	{
		damage *= 0.5f;
	}
	if (info.GetDamageType() & DMG_BUCKSHOT)
	{
		// hack to reduce vindicator damage (not reducing normal shotty as much as it's not too strong)
		if (info.GetAttacker() && info.GetAttacker()->Classify() == CLASS_ASW_MARINE)
		{
			CASW_Marine *pMarine = dynamic_cast<CASW_Marine*>(info.GetAttacker());
			if (pMarine)
			{
				CASW_Weapon_Assault_Shotgun *pVindicator = dynamic_cast<CASW_Weapon_Assault_Shotgun*>(pMarine->GetActiveASWWeapon());
				if (pVindicator)
					damage *= 0.45f;
				else
					damage *= 0.6f;
			}
		}		
	}

	newInfo.SetDamage(damage);
	result = BaseClass::OnTakeDamage_Alive(newInfo);

	return result;
}

bool CASW_Drone_Uber::ModifyAutoMovement( Vector &vecNewPos )
{
	// melee auto movement on the drones seems way too fast
	float fFactor = asw_uber_auto_speed_scale.GetFloat();
	if ( ShouldMoveSlow() )
	{
		if ( m_bElectroStunned.Get() )
		{
			fFactor *= asw_alien_stunned_speed.GetFloat() * 0.1f;
		}
		else
		{
			fFactor *= asw_alien_hurt_speed.GetFloat() * 0.1f;
		}
	}
	Vector vecRelPos = vecNewPos - GetAbsOrigin();
	vecRelPos *= fFactor;
	vecNewPos = GetAbsOrigin() + vecRelPos;
	return true;
}