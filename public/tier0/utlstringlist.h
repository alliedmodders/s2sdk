#ifndef UTLSTRINGLIST_H
#define UTLSTRINGLIST_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/tier0_strtools.h"
#include "tier0/utlvector.h"

// easy string list class with dynamically allocated strings. For use with V_SplitString, etc.
// Frees the dynamic strings in destructor.
class CUtlStringList : public CUtlVectorAutoPurge< char * >
{
public:
	CUtlStringList() {}

	CUtlStringList( const char *pString, const char *pSeparator )
	{
		SplitString( pString, &pSeparator, 1 );
	}

	CUtlStringList( const char *pString, const char * const *pSeparators, int nSeparators )
	{
		SplitString( pString, pSeparators, nSeparators );
	}

	~CUtlStringList()
	{
		PurgeAndDeleteElements();
	}

	void CopyAndAddToTail( char const *pString )			// clone the string and add to the end
	{
		char *pNewStr = new char[1 + strlen( pString )];
		V_strcpy( pNewStr, pString );
		AddToTail( pNewStr );
	}

	static int __cdecl SortFunc( char *const *sz1, char *const *sz2 )
	{
		return strcmp( *sz1, *sz2 );
	}

	inline void PurgeAndDeleteElements()
	{
		for(int i = 0; i < m_Size; i++)
		{
			delete[] Element( i );
		}
		Purge();
	}

private:
	DLL_CLASS_IMPORT void SplitString( const char *pString, const char * const *pSeparators, int nSeparators );
};

#endif // UTLSTRINGLIST_H
