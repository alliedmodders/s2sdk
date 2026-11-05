#ifndef FIELDPATH_H
#define FIELDPATH_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"
#include "tier0/threadinterlocks.h"
#include "tier0/utlcommon.h"

#define DEFAULT_MAX_PATH_DEPTH 10

// A field path packed into 32 bits. Up to three components of a non-negative value are stored plus one: 9 bits from
// bit 22, then 11 bits from bit 11 and from bit 0, ending at the first zero; 0 is the empty path. -1 is the path
// { 0x7FFF }, and other negative values index the longer paths the engine's field path packer keeps
typedef uint32 PackedFieldPath_t;

class CFieldPath
{
public:
	int GetDepth() const { return m_nCount; }
	const int16 *GetPathData() const { return m_bUsingExternalPaths ? m_pPaths : m_FixedPaths; }
	int16 operator[]( int i ) const { return GetPathData()[ i ]; }

	bool operator==( const CFieldPath &other ) const
	{
		if ( m_nCount != other.m_nCount )
			return false;

		const int16 *pPaths = GetPathData();
		const int16 *pOtherPaths = other.GetPathData();
		for ( int i = 0; i < m_nCount; i++ )
		{
			if ( pPaths[ i ] != pOtherPaths[ i ] )
				return false;
		}

		return true;
	}
	bool operator!=( const CFieldPath &other ) const { return !( *this == other ); }

	union
	{
		// Used when m_bUsingExternalPaths is set
		int16 *m_pPaths;
		int16 m_FixedPaths[12];
	};

	int16 m_nCount;
	bool m_bUsingExternalPaths;
};

// FNV-1a over the components
template <> struct DefaultHashFunctor<CFieldPath>
{
	unsigned int operator()( const CFieldPath &path ) const
	{
		uint32 h = 2166136261u;
		const int16 *pPaths = path.GetPathData();
		for ( int i = 0; i < path.GetDepth(); i++ )
			h = ( h ^ (uint32)(int)pPaths[ i ] ) * 16777619;

		return ( h ^ ( h << 17 ) ) + ( h >> 21 );
	}
};

// AMNOTE: A reference counted list of an entity's serialized fields: baseclass, then its elements in the same allocation
template <typename RefBase, typename T>
struct CSerializedFieldStorage : public RefBase
{
	const T *GetFirstElementPtr() const { return reinterpret_cast<const T *>( this + 1 ); }
	const T *GetNextElementPtr( const T *pElement ) const { return pElement + 1; }
	// Only for bases with a Count()
	const T *GetElementPtrLimit() const { return GetFirstElementPtr() + this->Count(); }

	const T &Element( int i ) const { return GetFirstElementPtr()[ i ]; }
};

// AMNOTE: The header of the packed paths of an entity's fields
// (CSerializedFieldStorage<PackedFieldPathList_t, PackedFieldPath_t>)
struct PackedFieldPathList_t
{
	int Count() const { return (int)m_nCount; }

	CInterlockedInt m_nRefs;
	uint32 m_nCount;
};

#endif /* FIELDPATH_H */
