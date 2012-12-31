//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef CLIENT_H
#define CLIENT_H

#ifdef _WIN32
#pragma once
#endif


class CCommand;
class CUserCmd;
class CBasePlayer;

// Ch1ckenscoop: Allow easy creation of chat "commands".
#define MAX_CHATCOMMAND_LENGTH 64
class ChatCommand
{
public:
	ChatCommand(const char *szName, FnCommandCallback_t commandCallback);
};

class ChatCommand_t
{
public:
	char m_szName[MAX_CHATCOMMAND_LENGTH];
	FnCommandCallback_t m_CommandCallback;
};


void ClientActive( edict_t *pEdict, bool bLoadGame );
void ClientFullyConnect( edict_t *pEdict );
void ClientPutInServer( edict_t *pEdict, const char *playername );
void ClientCommand( CBasePlayer *pSender, const CCommand &args );
void ClientPrecache( void );
// Game specific precaches
void ClientGamePrecache( void );
const char *GetGameDescription( void );
void Host_Say( edict_t *pEdict, bool teamonly );



#endif		// CLIENT_H
