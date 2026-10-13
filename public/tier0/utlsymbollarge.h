//===== Copyright ? 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: Defines a large symbol table (intp sized handles, can store more than 64k strings)
//
// $Header: $
// $NoKeywords: $
//===========================================================================//

#ifndef UTLSYMBOLLARGE_H
#define UTLSYMBOLLARGE_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/threadtools.h"
#include "tier0/generichash.h"
#include "tier0/utlvector.h"
#include "tier0/utlhashtable.h"
#include "tier0/mempool.h"

//-----------------------------------------------------------------------------
// CUtlSymbolTableLarge:
// description:
//    This class defines a symbol table, which allows us to perform mappings
//    of strings to symbols and back. 
// 
//    This class stores the strings in a series of string pools. The returned CUtlSymbolLarge is just a pointer
//     to the string data, the hash precedes it in memory and is used to speed up searching, etc.
//-----------------------------------------------------------------------------

typedef unsigned int UtlSymLargeId_t;

#define UTL_INVAL_SYMBOL_LARGE  ((UtlSymLargeId_t)~0)

class CUtlSymbolLarge
{
public:
	// constructor, destructor
	CUtlSymbolLarge() 
	{
		m_pString = NULL;
	}

	CUtlSymbolLarge( const char* pString )
	{
		m_pString = pString;
	}

	// operator==
	bool operator==( CUtlSymbolLarge const& src ) const 
	{ 
		return m_pString == src.m_pString; 
	}

	// operator!=
	bool operator!=( CUtlSymbolLarge const& src ) const 
	{ 
		return m_pString != src.m_pString; 
	}
	
	// operator<
	bool operator<( CUtlSymbolLarge const& src ) const
	{
		return ( intp )m_pString < ( intp )src.m_pString;
	}

	inline const char* String() const 
	{ 
		if ( !m_pString )
			return "";
		return m_pString; 
	}

	inline bool IsValid() const
	{
		return m_pString != NULL;
	}

private:
	// Disallowed
	bool operator==( const char* pString ) const; // disallow since we don't know if the table this is from was case sensitive or not... maybe we don't care

	const char* m_pString;
};

inline uint32 CUtlSymbolLarge_Hash( bool CASEINSENSITIVE, const char *pString, int len )
{
	return ( CASEINSENSITIVE ? MurmurHash2LowerCase( pString, len, 0x31415926 ) : MurmurHash2( pString, len, 0x31415926 ) ); 
}

#endif // UTLSYMBOLLARGE_H
