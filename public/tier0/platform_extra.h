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

// If pPath is a relative path, this function makes it into an absolute path
// using the current working directory as the base, or pStartingDir if it's non-NULL.
// Returns false if pPath tries to ".." past the root directory.
PLATFORM_INTERFACE bool V_MakeAbsolutePathBuffer( CBufferString &buf, const char *pPath, const char *pStartingDir = NULL );

// Creates a relative path given two full paths
// The first is the full path of the file to make a relative path for.
// The second is the full path of the directory to make the first file relative to
// Returns false if they can't be made relative (on separate drives, for example)
PLATFORM_INTERFACE bool V_MakeRelativePathBuffer( const char *pFullPath, const char *pDirectory, CBufferString &buf, bool );

#endif // PLATFORM_EXTRA_H
