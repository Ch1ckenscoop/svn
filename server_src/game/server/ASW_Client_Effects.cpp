#include "cbase.h"
#include "ASW_Client_Effects.h"

#include "memdbgon.h"

//Some defines so we don't have to type out the full convar name everywhere

// LCE - Local Contrast Enhancement

#define LCE_MAX	1.0

#define LCE_DEADZONE 0.2										//"Dead zone" where we won't worry about a client's cvar being off by this much, so we don't spam them with clientcommands.

#define LCE_ONOFF "mat_local_contrast_enable"					//Enables or disables LCE.
//#define LCE_OVERALL "mat_local_contrast_edge_scale_override"	//Controls LCE for the entire screen.
#define LCE_VIGNETTE "mat_local_contrast_scale_override"		//Controls LCE in a vignette'd area (see below). Sort of a stressful, "tunnel vision" effect.
#define LCE_VSTART "mat_local_contrast_vignette_start_override"	//Vignette start.
#define LCE_VEND "mat_local_contrast_vignette_end_override"		//Vignette end.

CASW_Client_Effects g_C_ASW_Effects;
CASW_Client_Effects* ASW_Client_Effects() { return &g_C_ASW_Effects; }

CASW_Client_Effects::CASW_Client_Effects(void)
{
}

CASW_Client_Effects::~CASW_Client_Effects(void)
{
}

void CASW_Client_Effects::PlayerConnected(CASW_Player *pPlayer)
{

}

void CASW_Client_Effects::PlayerDisconnected(CASW_Player *pPlayer)
{

}

void CASW_Client_Effects::FrameUpdatePostEntityThink()
{

}

float CASW_Client_Effects::IsMarineHurt(CASW_Marine *pMarine)
{
	return 0.0f;
}