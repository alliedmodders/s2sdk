//====== Copyright © 1996-2004, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef UTLBINARYBLOCK_H
#define UTLBINARYBLOCK_H
#ifdef _WIN32
#pragma once
#endif

#include "tier1/utlleanvector.h"
#include "tier1/strtools.h"
#include "limits.h"

//-----------------------------------------------------------------------------
// Base class, containing simple memory management
//-----------------------------------------------------------------------------
class CUtlBinaryBlock
{
public:
	CUtlBinaryBlock( int growSize = 0, int initSize = 0 ) : m_Memory( growSize, initSize ) {}

	// NOTE: nInitialLength indicates how much of the buffer starts full
	CUtlBinaryBlock( void* pMemory, int nSizeInBytes, int nInitialLength ) : m_Memory( (uint8 *)pMemory, nSizeInBytes, nInitialLength ) {}
	CUtlBinaryBlock( const CUtlBinaryBlock& src ) { *this = src; }

	void		Get( void *pValue, int nMaxLen ) const;
	void		Set( const void *pValue, int nLen );
	const void	*Get( ) const;
	void		*Get( );

	unsigned char& operator[]( int i );
	const unsigned char& operator[]( int i ) const;

	int			Length() const;
	void		SetLength( int nLength );	// Undefined memory will result
	bool		IsEmpty() const;
	void		Clear();
	void		Purge();

	CUtlBinaryBlock &operator=( const CUtlBinaryBlock &src );
	CUtlBinaryBlock &operator=( CUtlBinaryBlock &&src );

	// Test for equality
	bool operator==( const CUtlBinaryBlock &src ) const;

private:
	CUtlLeanVector<uint8, int> m_Memory;
};


//-----------------------------------------------------------------------------
// class inlines
//-----------------------------------------------------------------------------
inline const void *CUtlBinaryBlock::Get( ) const
{
	return m_Memory.Base();
}

inline void *CUtlBinaryBlock::Get( )
{
	return m_Memory.Base();
}

inline int CUtlBinaryBlock::Length() const
{
	return m_Memory.Count();
}

inline unsigned char& CUtlBinaryBlock::operator[]( int i )
{
	return m_Memory[i];
}

inline const unsigned char& CUtlBinaryBlock::operator[]( int i ) const
{
	return m_Memory[i];
}

inline bool CUtlBinaryBlock::IsEmpty() const
{
	return Length() == 0;
}

inline void CUtlBinaryBlock::Clear()
{
	SetLength( 0 );
}

inline void CUtlBinaryBlock::Purge()
{
	SetLength( 0 );
	m_Memory.Purge();
}

#endif // UTLBINARYBLOCK_H

