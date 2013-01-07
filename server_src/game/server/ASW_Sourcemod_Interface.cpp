#include "cbase.h"
#include "ASW_Sourcemod_Interface.h"
#include "filesystem.h"
#include "HypKeyValues.h"

CASW_Sourcemod_Interface g_SourcemodInterface;
CASW_Sourcemod_Interface *Sourcemod() { return &g_SourcemodInterface; }

class AdminPowerMapping
{
public:
	AdminPowerMapping(CASW_Sourcemod_Interface::AdminPower power, char szMapping)
	{
		m_Power = power;
		m_szMapping = szMapping;
	}
	
	CASW_Sourcemod_Interface::AdminPower m_Power;
	char m_szMapping;
};

static AdminPowerMapping s_AdminPowerMap[26] = 
{
	AdminPowerMapping(CASW_Sourcemod_Interface::ADMIN_POWER_RESERVATION, 'a'),
	AdminPowerMapping(CASW_Sourcemod_Interface::ADMIN_POWER_GENERIC, 'b'),
	AdminPowerMapping(CASW_Sourcemod_Interface::ADMIN_POWER_KICK, 'c'),
	AdminPowerMapping(CASW_Sourcemod_Interface::ADMIN_POWER_BAN, 'd'),
	AdminPowerMapping(CASW_Sourcemod_Interface::ADMIN_POWER_UNBAN, 'e'),
	AdminPowerMapping(CASW_Sourcemod_Interface::ADMIN_POWER_SLAY, 'f'),
	AdminPowerMapping(CASW_Sourcemod_Interface::ADMIN_POWER_CHANGEMAP, 'g'),
	AdminPowerMapping(CASW_Sourcemod_Interface::ADMIN_POWER_CVARS, 'h'),
	AdminPowerMapping(CASW_Sourcemod_Interface::ADMIN_POWER_CONFIG, 'i'),
	AdminPowerMapping(CASW_Sourcemod_Interface::ADMIN_POWER_CHAT, 'j'),
	AdminPowerMapping(CASW_Sourcemod_Interface::ADMIN_POWER_VOTE, 'k'),
	AdminPowerMapping(CASW_Sourcemod_Interface::ADMIN_POWER_PASSWORD, 'l'),
	AdminPowerMapping(CASW_Sourcemod_Interface::ADMIN_POWER_RCON, 'm'),
	AdminPowerMapping(CASW_Sourcemod_Interface::ADMIN_POWER_CHEATS, 'n'),

	AdminPowerMapping(CASW_Sourcemod_Interface::ADMIN_POWER_CUSTOM1, 'o'),
	AdminPowerMapping(CASW_Sourcemod_Interface::ADMIN_POWER_CUSTOM2, 'p'),
	AdminPowerMapping(CASW_Sourcemod_Interface::ADMIN_POWER_CUSTOM3, 'q'),
	AdminPowerMapping(CASW_Sourcemod_Interface::ADMIN_POWER_CUSTOM4, 'r'),
	AdminPowerMapping(CASW_Sourcemod_Interface::ADMIN_POWER_CUSTOM5, 's'),
	AdminPowerMapping(CASW_Sourcemod_Interface::ADMIN_POWER_CUSTOM6, 't'),
	
	AdminPowerMapping(CASW_Sourcemod_Interface::ADMIN_POWER_UNUSED1, 'u'),
	AdminPowerMapping(CASW_Sourcemod_Interface::ADMIN_POWER_UNUSED2, 'v'),
	AdminPowerMapping(CASW_Sourcemod_Interface::ADMIN_POWER_UNUSED3, 'w'),
	AdminPowerMapping(CASW_Sourcemod_Interface::ADMIN_POWER_UNUSED4, 'x'),
	AdminPowerMapping(CASW_Sourcemod_Interface::ADMIN_POWER_UNUSED5, 'y'),
	
	AdminPowerMapping(CASW_Sourcemod_Interface::ADMIN_POWER_ROOT, 'z'),
};

char GetPowerChar(CASW_Sourcemod_Interface::AdminPower power)
{
	for (int i = 0; i < 26; i++)
	{
		if (power == s_AdminPowerMap[i].m_Power)
			return s_AdminPowerMap[i].m_szMapping;
	}

	return '\0';
}

CASW_Sourcemod_Interface::AdminPower GetPowerValue(char szMapping)
{
	for (int i = 0; i < 26; i++)
	{
		if (szMapping == s_AdminPowerMap[i].m_szMapping)
			return s_AdminPowerMap[i].m_Power;
	}

	return (CASW_Sourcemod_Interface::AdminPower)(-1);
}

CASW_Sourcemod_Interface::CASW_Sourcemod_Interface(void)
{
}


CASW_Sourcemod_Interface::~CASW_Sourcemod_Interface(void)
{
}

void CASW_Sourcemod_Interface::PostInit()
{
	LoadAdminsSimple();
}

int CASW_Sourcemod_Interface::GetAdminIndex(CSteamID steamID) const
{
	for (int i = 0; i < GetAdminCount(); i++)
	{
		if (m_Admins[i]->m_SteamID == steamID)
			return i;
	}

	return -1;
}

bool CASW_Sourcemod_Interface::AdminHasPowers(int iAdminIndex, AdminPower powers) const
{
	if (iAdminIndex < 0 || iAdminIndex >= GetAdminCount())
		return false;

	int adminPowers = powers;

	// Get the admin
	Admin_t *pAdmin = m_Admins[iAdminIndex];
	
	while (adminPowers != 0)
	{
		// Get the character for one of our powers.
		char currentPower = '\0';
		AdminPower currentPowerValue = (AdminPower)(-1);
		for (int i = 0; i < 26; i++)
		{
			if (adminPowers & s_AdminPowerMap[i].m_Power)
			{
				currentPower = s_AdminPowerMap[i].m_szMapping;
				currentPowerValue = s_AdminPowerMap[i].m_Power;
				break;
			}
		}
		
		Assert(currentPower != '\0');
		Assert(currentPowerValue != (AdminPower)(-1));

		// Check for the flag in our admin's power string
		int powerStrLen = V_strlen(pAdmin->m_szFlags);
		bool bFound = false;
		for (int i = 0; i < powerStrLen; i++)
		{
			if (pAdmin->m_szFlags[i] == GetPowerChar(ADMIN_POWER_ROOT))
				bFound = true;	// If we have root, we have everything.
			else if (pAdmin->m_szFlags[i] == currentPower)
				bFound = true;
		}

		// If we're missing just one of possibly many requested powers,
		// still return false;
		if (!bFound)
			return false;

		// Remove this flag from our powers.
		adminPowers &= ~currentPowerValue;
	}

	return true;
}

void CASW_Sourcemod_Interface::LoadAdminsSimple()
{
	CUtlBuffer buf;
	buf.SetBufferType(true, false);

	buf.SeekPut(CUtlBuffer::SeekType_t::SEEK_HEAD, 0);
	buf.PutString("\"Admins Simple Converted To KeyValues\" {\n");

	filesystem->ReadFile("addons/sourcemod/configs/admins_simple.ini", NULL, buf);

	buf.SeekPut(CUtlBuffer::SeekType_t::SEEK_TAIL, 0);
	buf.PutString("\n}");

	HypKeyValues rootKV;
	rootKV.StringToKeyValues(buf, "addons/sourcemod/admins_simple.ini");

	HypKeyValues *adminsKV = rootKV.GetKey(0);
	Assert(adminsKV);

	for (int i = 0; i < adminsKV->ValueCount(); i++)
	{
		HypKeyValue *pValue = adminsKV->GetValue(i);
		Assert(pValue);
		const char *szName = pValue->GetName();
		const char *szValue = pValue->GetString();

		Admin_t *pNewAdmin = new Admin_t();

		// Load SteamID
		{
			pNewAdmin->m_SteamID.SetFromString(szName, EUniverse::k_EUniversePublic);
		}

		// Load immunity and flags
		{
			bool bContainsColon = StringContainsChar(szValue, ':');

			if (bContainsColon)
			{
				char szBeforeColon[32];
				char szAfterColon[32];

				GetStringBeforeChar(szBeforeColon, sizeof(szBeforeColon), szValue, ':');
				GetStringAfterChar(szAfterColon, sizeof(szAfterColon), szValue, ':');

				V_strncpy(pNewAdmin->m_szFlags, szAfterColon, sizeof(pNewAdmin->m_szFlags));

				int iImmunity = atoi(szBeforeColon);
				pNewAdmin->m_iImmunity = iImmunity;
			}
			else
			{
				V_strncpy(pNewAdmin->m_szFlags, szValue, sizeof(pNewAdmin->m_szFlags));
			}
		}

		m_Admins.AddToTail(pNewAdmin);
	}
}

bool CASW_Sourcemod_Interface::StringContainsChar(const char *szTestString, char testChar)
{
	if (!szTestString)
		return false;

	int i = 0;
	char currentChar = szTestString[i];
	while (currentChar != '\0')
	{
		if (currentChar == testChar)
			return true;

		currentChar = szTestString[++i];
	}

	return false;
}

void CASW_Sourcemod_Interface::GetStringBeforeChar(char *szStringOut, int iMaxSize, const char *szStringIn, char breakChar)
{
	if (!szStringIn)
		return;

	int i = 0; 
	char currentChar = szStringIn[i];
	while (currentChar != '\0')
	{
		if (currentChar == breakChar)
		{
			V_strncpy(szStringOut, szStringIn, MIN( iMaxSize, (i+1) ));
			return;
		}

		currentChar = szStringIn[++i];
	}
}

void CASW_Sourcemod_Interface::GetStringAfterChar(char *szStringOut, int iMaxSize, const char *szStringIn, char breakChar)
{
	if (!szStringIn)
		return;

	int i = 0;
	char currentChar = szStringIn[i];
	while (currentChar != '\0')
	{
		if (currentChar == breakChar)
		{
			V_strncpy(szStringOut, &(szStringIn[i+1]), iMaxSize);
			return;
		}

		currentChar = szStringIn[++i];
	}
}