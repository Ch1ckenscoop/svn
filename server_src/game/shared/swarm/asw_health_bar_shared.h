//============ Copyright (c) Valve Corporation, All rights reserved. ============

//softcopy: it's splitted from asw_health_bar_shared.cpp

#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "Sprite.h"

#ifdef CLIENT_DLL
	#include "asw_hud_3dmarinenames.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifdef CLIENT_DLL
	#define CASWHealthBar C_ASWHealthBar
#endif


class CASWHealthBar : public CSprite
#ifdef CLIENT_DLL
	public IHealthTracked
#endif
{
#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif
	DECLARE_CLASS( CASWHealthBar, CSprite );
	DECLARE_NETWORKCLASS();

public:

#ifndef CLIENT_DLL
	virtual bool KeyValue( const char *szKeyName, const char *szValue );
	virtual void Spawn( void );

	void TrackHealthThink( void );

	void InputEnable( inputdata_t &inputdata );
	void InputDisable( inputdata_t &inputdata );
#else
	virtual int DrawModel( int flags, const RenderableInstance_t &instance );

	// IHealthTracked
	IMPLEMENT_AUTO_LIST_GET();
	virtual void PaintHealthBar( class CASWHud3DMarineNames *pSurface );
#endif

	virtual float GetHealthFraction() { return m_fHealthFraction; }	//softcopy:

private:

	CNetworkVar( float, m_fHealthFraction );
	CNetworkVar( bool, m_bDisabled );
	CNetworkVar( bool, m_bHideAtFullHealth );

};