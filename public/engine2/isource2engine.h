//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose:
//
// $NoKeywords: $
//
//===========================================================================//

#ifndef ISOURCE2ENGINE_H
#define ISOURCE2ENGINE_H

#ifdef _WIN32
#pragma once
#endif

#include "appframework/IAppSystem.h"
#include "engine2/iscreenshotcallback.h"

class CEntityLump;

//-----------------------------------------------------------------------------
// Purpose: Interface the engine exposes to the game DLL and client DLL
//-----------------------------------------------------------------------------
abstract_class ISource2Engine : public IAppSystem
{
public:
	// Is the game paused?
	virtual bool		IsPaused() = 0;

	// What is the game timescale multiplied with the host_timescale?
	virtual float		GetTimescale( void ) const = 0;

	virtual void		*FindOrCreateWorldSession( const char *pszWorldName, const char *, void * ) = 0;

	virtual CEntityLump	*GetEntityLumpForTemplate( const char *, bool, const char *, const char *, bool ) = 0;

	virtual uint32		GetStatsAppID() const = 0;

	virtual void		WriteJpegScreenshot( const char *pszFilename, int nQuality, int nWidth, int nHeight, bool bAsyncWrite ) = 0;
	virtual void		RequestScreenshot( IScreenshotCallback *pCallback, void *pContext ) = 0;
};

#endif // ISOURCE2ENGINE_H
