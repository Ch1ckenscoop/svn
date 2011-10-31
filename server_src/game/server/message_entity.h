#include "cbase.h"

#include "memdbgon.h"

class CMessageEntity : public CPointEntity
{
	DECLARE_CLASS( CMessageEntity, CPointEntity );

public:
	void	Spawn( void );
	void	Activate( void );
	void	Think( void );
	void	DrawOverlays(void);

	virtual void UpdateOnRemove();

	void	InputEnable( inputdata_t &inputdata );
	void	InputDisable( inputdata_t &inputdata );

	void	SetText(const char *t);		//Ch1ckensCoop: Set text from code.

	DECLARE_DATADESC();

	bool			m_bAlwaysDraw;

protected:
	int				m_radius;
	string_t		m_messageText;
	bool			m_drawText;
	bool			m_bDeveloperOnly;
	bool			m_bEnabled;
};