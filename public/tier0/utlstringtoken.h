//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//
//===========================================================================//

#ifndef UTLSTRINGTOKEN_H
#define UTLSTRINGTOKEN_H

#ifdef _WIN32
#pragma once
#endif

#include <limits.h>
#include "tier0/generichash.h"
#include "tier0/iutlstringtokensystem.h"

#include <type_traits>

#define STRINGTOKEN_MURMURHASH_SEED 0x31415926

class CUtlString;
class IFormatOutputStream;
class CFormatStringElement;

class CUtlStringToken
{
public:
	FORCEINLINE CUtlStringToken( uint32 nHashCode = 0 ) : m_nHashCode( nHashCode ) {}

	template <size_t N>
	constexpr FORCEINLINE CUtlStringToken( const char (&str)[N] ) : m_nHashCode( str[0] ? MurmurHash2LowerCase( str, STRINGTOKEN_MURMURHASH_SEED ) : 0 ) { }

	// AMNOTE: Template is required to enforce compiler to pick the correct overload when inlining
	// as otherwise non templated overload would always win the pick thus no const folding would happen
	template <typename T, std::enable_if_t<std::is_same_v<T, const char *>, int> = 0>
	FORCEINLINE CUtlStringToken( T str ) : m_nHashCode( 0 )
	{
		if(str && *str)
			m_nHashCode = MurmurHash2LowerCase( str, STRINGTOKEN_MURMURHASH_SEED );
	};

	FORCEINLINE bool operator==( CUtlStringToken const &other ) const { return ( other.m_nHashCode == m_nHashCode ); }
	FORCEINLINE bool operator!=( CUtlStringToken const &other ) const { return !operator==( other ); }
	FORCEINLINE bool operator<( CUtlStringToken const &other ) const { return ( m_nHashCode < other.m_nHashCode ); }

	FORCEINLINE bool IsValid() const { return m_nHashCode != 0; }
	FORCEINLINE uint32 GetHashCode() const { return m_nHashCode; }
	FORCEINLINE void SetHashCode( uint32 hash ) { m_nHashCode = hash; }

	DLL_CLASS_IMPORT void FormatTo( IFormatOutputStream* pOutputStream, CFormatStringElement pElement ) const;
	DLL_CLASS_IMPORT static bool TrackTokenCreation( const char *s1, const char *s2 );

private:
	uint32 m_nHashCode;
};

class CUtlStringAndToken : public CUtlStringToken
{
public:
	FORCEINLINE CUtlStringAndToken() : m_pString( "" ) {}

	template <size_t N>
	constexpr FORCEINLINE CUtlStringAndToken( const char (&str)[N] ) : CUtlStringToken( str ), m_pString( str ) {}

	template <typename T, std::enable_if_t<std::is_same_v<T, const char *>, int> = 0>
	FORCEINLINE CUtlStringAndToken( T str ) : CUtlStringToken( str ), m_pString( str ? str : "" ) {}

	FORCEINLINE const CUtlStringToken &GetToken() const { return *this; }
	FORCEINLINE const char *GetString() const { return m_pString; }

	// An empty or nullptr string equals the empty one
	DLL_CLASS_IMPORT bool IsEqual_CaseSensitive( const char *pString ) const;
	DLL_CLASS_IMPORT bool IsEqual_FastCaseInsensitive( const char *pString ) const;

private:
	// The token always matches the string
	using CUtlStringToken::SetHashCode;

protected:
	const char *m_pString;
};

#endif // UTLSTRINGTOKEN_H
