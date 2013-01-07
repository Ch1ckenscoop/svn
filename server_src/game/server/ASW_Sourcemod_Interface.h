#pragma once
#include "igamesystem.h"

// Allows easy access to various sourcmod configs and options.
class CASW_Sourcemod_Interface : public CAutoGameSystem
{
public:
	CASW_Sourcemod_Interface(void);
	~CASW_Sourcemod_Interface(void);

	// PostInit, because we need the filesystem.
	virtual void PostInit();

	enum AdminPower
	{
		ADMIN_POWER_RESERVATION = (1 << 0),		// a
		ADMIN_POWER_GENERIC = (1 << 1),			// b
		ADMIN_POWER_KICK = (1 << 2),			// c
		ADMIN_POWER_BAN = (1 << 3),				// d
		ADMIN_POWER_UNBAN = (1 << 4),			// e
		ADMIN_POWER_SLAY = (1 << 5),			// f
		ADMIN_POWER_CHANGEMAP = (1 << 6),		// g
		ADMIN_POWER_CVARS = (1 << 7),			// h
		ADMIN_POWER_CONFIG = (1 << 8),			// i
		ADMIN_POWER_CHAT = (1 << 9),			// j
		ADMIN_POWER_VOTE = (1 << 10),			// k
		ADMIN_POWER_PASSWORD = (1 << 11),		// l
		ADMIN_POWER_RCON = (1 << 12),			// m
		ADMIN_POWER_CHEATS = (1 << 13),			// n

		ADMIN_POWER_CUSTOM1 = (1 << 14),		// o
		ADMIN_POWER_CUSTOM2 = (1 << 15),		// p
		ADMIN_POWER_CUSTOM3 = (1 << 16),		// q
		ADMIN_POWER_CUSTOM4 = (1 << 17),		// r
		ADMIN_POWER_CUSTOM5 = (1 << 18),		// s
		ADMIN_POWER_CUSTOM6 = (1 << 19),		// t

		ADMIN_POWER_UNUSED1 = (1 << 20),		// u
		ADMIN_POWER_UNUSED2 = (1 << 21),		// v
		ADMIN_POWER_UNUSED3 = (1 << 22),		// w
		ADMIN_POWER_UNUSED4 = (1 << 23),		// x
		ADMIN_POWER_UNUSED5 = (1 << 24),		// y

		ADMIN_POWER_ROOT = (1 << 25),			// z
	};

	int GetAdminCount() const { return m_Admins.Count(); }
	int GetAdminIndex(CSteamID steamID) const;
	CSteamID GetAdminSteamID(int iAdminIndex) const;
	byte GetAdminImmunity(int iAdminIndex) const;
	bool AdminHasPowers(int iAdminIndex, AdminPower powers) const;

private:
	struct Admin_t
	{
		CSteamID	m_SteamID;
		byte		m_iImmunity;
		char		m_szFlags[32];
	};

	CUtlVector<Admin_t*>	m_Admins;

	// Loads admins from the simple configuration file
	// (addons/sourcemod/configs/admins_simple.ini)
	void LoadAdminsSimple();
	bool StringContainsChar(const char *szTestString, char testChar);
	void GetStringBeforeChar(char *szStringOut, int iMaxSize, const char *szStringIn, char breakChar);
	void GetStringAfterChar(char *szStringOut, int iMaxSize, const char *szStringIn, char breakChar);
};

extern CASW_Sourcemod_Interface *Sourcemod();