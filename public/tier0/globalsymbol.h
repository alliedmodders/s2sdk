#ifndef GLOBALSYMBOL_H
#define GLOBALSYMBOL_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"

// Case-insensitive, nHash is the CUtlStringToken hash of the string
PLATFORM_INTERFACE const char *_FindGlobalSymbolByHash( uint32 nHash );
// Case-insensitive
PLATFORM_INTERFACE const char *_FindGlobalSymbol( const char *pString );
// Case-insensitive
PLATFORM_INTERFACE const char *_MakeGlobalSymbol( const char *pString );
// Case-sensitive, in a table of its own
PLATFORM_INTERFACE const char *_MakeGlobalSymbolCaseSensitive( const char *pString );

// A string interned in tier0's case-insensitive global symbol table, so equal strings share a pointer.
class CGlobalSymbol
{
public:
	CGlobalSymbol() : m_pString( NULL ) {}

	// User-provided like the engine's, so a CGlobalSymbol argument is passed by address
	CGlobalSymbol( const CGlobalSymbol &other ) : m_pString( other.m_pString ) {}
	CGlobalSymbol &operator=( const CGlobalSymbol &other ) = default;

	bool operator==( const CGlobalSymbol &other ) const { return m_pString == other.m_pString; }
	bool operator!=( const CGlobalSymbol &other ) const { return m_pString != other.m_pString; }

	bool IsValid() const { return m_pString != NULL; }
	const char *String() const { return m_pString ? m_pString : ""; }

private:
	explicit CGlobalSymbol( const char *pString ) : m_pString( pString ) {}

	friend CGlobalSymbol FindGlobalSymbolByHash( uint32 nHash );
	friend CGlobalSymbol FindGlobalSymbol( const char *pString );
	friend CGlobalSymbol MakeGlobalSymbol( const char *pString );

	const char *m_pString;
};

inline CGlobalSymbol FindGlobalSymbolByHash( uint32 nHash ) { return CGlobalSymbol( _FindGlobalSymbolByHash( nHash ) ); }
inline CGlobalSymbol FindGlobalSymbol( const char *pString ) { return CGlobalSymbol( _FindGlobalSymbol( pString ) ); }
inline CGlobalSymbol MakeGlobalSymbol( const char *pString ) { return CGlobalSymbol( _MakeGlobalSymbol( pString ) ); }

// A string interned in tier0's case-sensitive global symbol table, not interchangeable with CGlobalSymbol.
class CGlobalSymbolCaseSensitive
{
public:
	CGlobalSymbolCaseSensitive() : m_pString( NULL ) {}

	// AMNOTE: Assumed to match CGlobalSymbol, no engine function passes this type by value
	CGlobalSymbolCaseSensitive( const CGlobalSymbolCaseSensitive &other ) : m_pString( other.m_pString ) {}
	CGlobalSymbolCaseSensitive &operator=( const CGlobalSymbolCaseSensitive &other ) = default;

	bool operator==( const CGlobalSymbolCaseSensitive &other ) const { return m_pString == other.m_pString; }
	bool operator!=( const CGlobalSymbolCaseSensitive &other ) const { return m_pString != other.m_pString; }

	bool IsValid() const { return m_pString != NULL; }
	const char *String() const { return m_pString ? m_pString : ""; }

private:
	explicit CGlobalSymbolCaseSensitive( const char *pString ) : m_pString( pString ) {}

	friend CGlobalSymbolCaseSensitive MakeGlobalSymbolCaseSensitive( const char *pString );

	const char *m_pString;
};

inline CGlobalSymbolCaseSensitive MakeGlobalSymbolCaseSensitive( const char *pString ) { return CGlobalSymbolCaseSensitive( _MakeGlobalSymbolCaseSensitive( pString ) ); }

#endif // GLOBALSYMBOL_H
