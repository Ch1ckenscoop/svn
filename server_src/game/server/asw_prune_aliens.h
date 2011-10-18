#pragma once

class CASW_Prune_Aliens : public CLogicalEntity
{
public:
	DECLARE_CLASS( CASW_Prune_Aliens, CLogicalEntity );
	CASW_Prune_Aliens(void);
	~CASW_Prune_Aliens(void);
	virtual void Think();
	virtual void Spawn();

	enum
	{
		ASW_TAG_UNKNOWN,	//Pruning hasn't gotten to this alien yet
		ASW_TAG_REMOVE,		//Alien is outside the pruning radius and should be removed
		ASW_TAG_SAFE,		//Alien is inside the pruning radius
	};
};
