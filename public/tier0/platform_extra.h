#ifndef PLATFORM_EXTRA_H
#define PLATFORM_EXTRA_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"

class CBufferString;

// Get the process' executable filename.
PLATFORM_INTERFACE void				Plat_GetModuleFilename( char *pOut, int nMaxBytes );
PLATFORM_INTERFACE int				Plat_GetExecutablePathUTF8( CBufferString *buf );
PLATFORM_INTERFACE void				Plat_SetModuleFilename( const char *pExeName );

PLATFORM_INTERFACE void				Plat_TranslateSymbolicLinks( const char *pInDir, CBufferString *pOut );

PLATFORM_INTERFACE bool				Plat_GetCurrentDirectory( CBufferString *pOut, bool bAppendSlash );
PLATFORM_INTERFACE bool				Plat_SetCurrentDirectory( const char *pDir );

// The description of the file's type, like Windows Explorer shows it
PLATFORM_INTERFACE bool				Plat_GetFileType( const char *pPath, CBufferString *pOut );

// If pPath is a relative path, this function makes it into an absolute path
// using the current working directory as the base, or pStartingDir if it's non-NULL.
// Returns false if pPath tries to ".." past the root directory.
PLATFORM_INTERFACE bool V_MakeAbsolutePathBuffer( CBufferString &buf, const char *pPath, const char *pStartingDir = NULL );

// Like V_MakeAbsolutePathBuffer, but only joins the base directory and pPath without fixing up the result. Returns pOut's string.
PLATFORM_INTERFACE const char *V_MakeAbsolutePathBufferUnfixed( CBufferString *pOut, const char *pPath, const char *pStartingDir = NULL );

// Makes the path after the "vpk:" prefixes of pInPathName absolute, keeping the prefixes in front.
// Returns false and copies pInPathName unchanged if it has no such prefix.
PLATFORM_INTERFACE bool V_MakeAbsoluteVPKPathNameBuffer( CBufferString *pOut, const char *pInPathName, const char *pStartingDir = NULL );

// Creates a relative path given two full paths
// The first is the full path of the file to make a relative path for.
// The second is the full path of the directory to make the first file relative to
// Returns false if they can't be made relative (on separate drives, for example)
PLATFORM_INTERFACE bool V_MakeRelativePathBuffer( const char *pFullPath, const char *pDirectory, CBufferString &buf, bool );

#endif // PLATFORM_EXTRA_H
