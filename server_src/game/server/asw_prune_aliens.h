#pragma once

class CASW_Prune_Aliens : public CLogicalEntity
{
public:
	DECLARE_CLASS( CASW_Prune_Aliens, CLogicalEntity );
	CASW_Prune_Aliens(void);
	~CASW_Prune_Aliens(void);
	virtual void Think();
	virtual void Spawn();
};
