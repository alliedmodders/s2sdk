//========= Copyright ? 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#pragma once

#include "platform.h"

#if defined( _WIN32 ) && !defined( _X360 )
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#if !defined COMPILER_MSVC && !defined HMODULE
#define HMODULE void *
#endif

//-----------------------------------------------------------------------------
// Interface creation function
//-----------------------------------------------------------------------------
typedef void* (*CreateInterfaceFn)(const char *pName, int *pReturnCode);

//-----------------------------------------------------------------------------
// Load & Unload should be called in exactly one place for each module
// The factory for that module should be passed on to dependent components for
// proper versioning.
//-----------------------------------------------------------------------------
// pLastError receives the OS error code. bQuiet doesn't log dlopen failures on Linux, and does nothing on Windows.
PLATFORM_INTERFACE HMODULE			Plat_LoadModule( const char *pModuleName, int *pLastError = NULL, bool bQuiet = false );
// Returns the OS error code, or 0 on success. On Linux nFlags are dlopen flags, with -1 for the default; Windows ignores nFlags and bQuiet.
PLATFORM_INTERFACE int				Plat_LoadModuleRaw( const char *pModuleName, HMODULE *pModule, int nFlags, bool bQuiet );
PLATFORM_INTERFACE void				Plat_UnloadModule( HMODULE module );
PLATFORM_INTERFACE void *			Plat_GetModuleProcAddress( HMODULE module, const char *pName );

// Determines if current process is running with any debug modules
PLATFORM_INTERFACE bool				Plat_RunningWithDebugModules();

PLATFORM_INTERFACE HMODULE Plat_FindModuleByAddress( void *pAddress );
PLATFORM_INTERFACE CreateInterfaceFn Plat_GetModuleInterfaceFactory( HMODULE module, int *pReturnCode = NULL );

// This is a helper function to load a module, get its factory, and get a specific interface.
// You are expected to free all of these things.
// Returns false and cleans up if any of the steps fail.
PLATFORM_INTERFACE bool Plat_LoadInterface(
	const char *pModuleName,
	const char *pInterfaceVersionName,
	HMODULE *pOutModule,
	void **pOutInterface );