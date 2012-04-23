//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//=============================================================================

// No spaces in event names, max length 32
// All strings are case sensitive
// total game event byte length must be < 1024
//
// valid data key types are:
//   none   : value is not networked
//   string : a zero terminated string
//   bool   : unsigned int, 1 bit
//   byte   : unsigned int, 8 bit
//   short  : signed int, 16 bit
//   long   : signed int, 32 bit
//   float  : float, 32 bit

"cc_events"
{
	"marine_hurt"
	{
		"marine"		"short"	// Entindex of hurt marine
		"damagetaken"	"short"	// Damage taken
		"attacker"		"short"	// Classify() of attacker
		"marineattacker"	"short"	// Entindex of marine attacker, if any
		"weapon"		"short"	// Classify() of weapon
	}
	
	"marine_died"
	{
		"marine"		"short"	// Entindex of ded marine
		"killer"		"short"	// Classify() of attacker
		"killermarine"	"short"	// Entindex of marine attacker, if any
		"weapon"		"short"	// Classify() of weapon
	}
	
	"alien_hurt"
	{
		"alien"			"short"	// Classify() of alien
		"marine"		"short"	// Entindex of attacking marine, if any
		"damagetaken"	"short"	// Damage taken
		"weapon"		"short"	// Classify() of weapon
	}
	
	"safepara_attack"
	{
		"marine"		"short"	// Entindex of attacked marine
	}
	
	"marine_rangermiss"	// We "dodged" a shot by being hit while we were airborne.
	{
		"marine"		"short"	// Entindex of dodging marine
	}
	
	"marine_spawned"
	{
		"marine"		"short"	// Entindex of spawned marine
		"client"		"short"	// Client index of marine
	}
}