//===================== Copyright (c) Valve Corporation. All Rights Reserved. ======================
//
//
//
//==================================================================================================

#ifndef SENDPROP_PRIORITIES_H
#define SENDPROP_PRIORITIES_H
#ifdef _WIN32
#pragma once
#endif


enum SendpropPriority
{
	SENDPROP_DEFAULT_PRIORITY = 0,
	SENDPROP_CHANGES_OFTEN_PRIORITY = 1,

	// Nonlocal players exclude local player origin X and Y, not vice-versa,
	// so our props should come after the most frequent local player props
	// so we don't eat up their prop index bits.
	SENDPROP_NONLOCALPLAYER_ANGLES_PRIORITY = 245,
	SENDPROP_NONLOCALPLAYER_ORIGINZ_PRIORITY,
	SENDPROP_NONLOCALPLAYER_ORIGINXY_PRIORITY,
	
	SENDPROP_PLAYER_VELOCITY_Z_PRIORITY,
	SENDPROP_PLAYER_VELOCITY_XY_PRIORITY,
	
	SENDPROP_LOCALPLAYER_ANGLES_PRIORITY,
	SENDPROP_LOCALPLAYER_ORIGINZ_PRIORITY,
	SENDPROP_PLAYER_EYE_ANGLES_PRIORITY,
	SENDPROP_LOCALPLAYER_ORIGINXY_PRIORITY,
	
	SENDPROP_TICKBASE_PRIORITY,
	SENDPROP_SIMULATION_TIME_PRIORITY,

	SENDPROP_CELL_INFO_PRIORITY	= 32,		// SendProp priority for cell bits and x/y/z.
};

class __AssertPrioritiesConfigured
{
	__AssertPrioritiesConfigured()
	{
		COMPILE_TIME_ASSERT(SENDPROP_SIMULATION_TIME_PRIORITY == 255);
	}
};

#endif // SENDPROP_PRIORITIES_H
