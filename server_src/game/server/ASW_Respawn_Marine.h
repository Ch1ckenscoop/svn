#pragma once

class CASW_Respawn_Marine
{
public:
	CASW_Respawn_Marine(void);
	~CASW_Respawn_Marine(void);

//softcopy:
	enum MarineShortName
	{
		CRASH_INDEX,
		VEGAS_INDEX,
		WILDCAT_INDEX,
		WOLFE_INDEX,
		SARGE_INDEX,
		JAEGER_INDEX,
		FAITH_INDEX,
		BASTILLE_INDEX,

		MARINE_INDEX_COUNT,	//softcopy: If you're going to add anything, add it before this!
	};
	struct MarineInfo
	{
		const char *m_szMarineClassName;
		const char *m_MarineName;
	};
	const MarineInfo *GetMarineInfo(int index);
private:
	MarineInfo m_MarineInfoArray[MARINE_INDEX_COUNT];
	virtual void InitMarineName();

};

CASW_Respawn_Marine* ASWRespawnMarine();	//softcopy: