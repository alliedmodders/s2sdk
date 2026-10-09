//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Variable size bit string, storage managed by tier0
//
//=============================================================================//

#ifndef BITSTRING_H
#define BITSTRING_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"

class CVariableBitStringBase
{
public:
	bool IsFixedSize() const { return false; }
	int Size() const { return m_numBits; }

	// Clears the bits past the old size when growing
	DLL_CLASS_IMPORT void Resize( int numBits );

	int GetNumInts() const { return m_numInts; }
	int *GetInts() { return m_pInt; }
	const int *GetInts() const { return m_pInt; }

protected:
	CVariableBitStringBase() : m_numBits( 0 ), m_numInts( 0 ), m_iBitStringStorage( 0 ), m_pInt( nullptr ) {}
	~CVariableBitStringBase() { FreeInts(); }

	// A single int lives in m_iBitStringStorage and m_pInt points at it, so the object can't be copied bitwise
	CVariableBitStringBase( const CVariableBitStringBase & ) = delete;
	CVariableBitStringBase &operator=( const CVariableBitStringBase & ) = delete;

	DLL_CLASS_IMPORT void ValidateOperand( const CVariableBitStringBase &operand ) const;

private:
	int m_numBits;
	int m_numInts;
	int m_iBitStringStorage;
	int *m_pInt;

	DLL_CLASS_IMPORT void AllocInts( int numInts );
	DLL_CLASS_IMPORT void ReallocInts( int numInts );
	DLL_CLASS_IMPORT void FreeInts();
};

template <class BASE_OPS>
class CBitStringT : public BASE_OPS
{
public:
	CBitStringT() {}

	bool GetBit( int bitNum ) const
	{
		return ( (uint32)this->GetInts()[ bitNum >> 5 ] & ( 1u << ( bitNum & 31 ) ) ) != 0;
	}

	void SetBit( int bitNum )
	{
		int *pInt = &this->GetInts()[ bitNum >> 5 ];
		*pInt = (int)( (uint32)*pInt | ( 1u << ( bitNum & 31 ) ) );
	}

	void ClearBit( int bitNum )
	{
		int *pInt = &this->GetInts()[ bitNum >> 5 ];
		*pInt = (int)( (uint32)*pInt & ~( 1u << ( bitNum & 31 ) ) );
	}
};

#endif // BITSTRING_H
