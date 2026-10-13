#ifndef FILENAMESYMBOLTABLE_H
#define FILENAMESYMBOLTABLE_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/utlsymboltable.h"

//-----------------------------------------------------------------------------
// CUtlFilenameSymbolTable:
// description:
//    This class defines a symbol table of individual filenames, stored more
//	  efficiently than a standard symbol table.  Internally filenames are broken
//	  up into file and path entries, and a file handle class allows convenient 
//	  access to these.
//-----------------------------------------------------------------------------

// The handle is a CUtlSymbol for the dirname and the same for the filename, the accessor
//  copies them into a static char buffer for return.
typedef uint32 FileNameHandle_t;

// Symbol table for more efficiently storing filenames by breaking paths and filenames apart.
// Refactored from BaseFileSystem.h
class CUtlFilenameSymbolTable
{
	// Internal representation of a FileHandle_t
	// If we get more than 64K filenames, we'll have to revisit...
	// Right now CUtlSymbol is a short, so this packs into an int/void * pointer size...
	struct FileNameHandleInternal_t
	{
		FileNameHandleInternal_t()
		{
			path = 0;
			file = 0;
		}

		// Part before the final '/' character
		unsigned short path;
		// Part after the final '/', including extension
		unsigned short file;
	};

public:
	FileNameHandle_t	FindOrAddFileName( const char *pFileName );
	FileNameHandle_t	FindFileName( const char *pFileName ) const;
	int					PathIndex(const FileNameHandle_t &handle) { return (( const FileNameHandleInternal_t * )&handle)->path; }
	bool				String( const FileNameHandle_t& handle, char *buf, int buflen );
	bool				String( const FileNameHandle_t& handle, CBufferString *pOut );
	void				RemoveAll();
	void				SpewStrings();
	bool				SaveToBuffer( CUtlBuffer &buffer );
	bool				RestoreFromBuffer( CUtlBuffer &buffer );

private:
	CCountedStringPool_CI	m_StringPool;
	mutable CThreadSpinRWLock m_lock;
};

#endif // FILENAMESYMBOLTABLE_H
