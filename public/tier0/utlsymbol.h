//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: Defines a symbol table
//
// $Header: $
// $NoKeywords: $
//===========================================================================//

#ifndef UTLSYMBOL_H
#define UTLSYMBOL_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/threadtools.h"
#include "tier0/utlvector.h"
#include "tier0/utlbuffer.h"
#include "tier0/utllinkedlist.h"
#include "tier0/stringpool.h"
#include "tier0/utlhashtable.h"
#include "tier0/mempool.h"


//-----------------------------------------------------------------------------
// forward declarations
//-----------------------------------------------------------------------------
class CUtlSymbolTable;
class CUtlSymbolTableMT;


//-----------------------------------------------------------------------------
// This is a symbol, which is a easier way of dealing with strings.
//-----------------------------------------------------------------------------
typedef unsigned short UtlSymId_t;

#define UTL_INVAL_SYMBOL  ((UtlSymId_t)~0)

class CUtlSymbol
{
public:
	// constructor, destructor
	CUtlSymbol() : m_Id(UTL_INVAL_SYMBOL) {}
	CUtlSymbol( UtlSymId_t id ) : m_Id(id) {}
	CUtlSymbol( CUtlSymbol const& sym ) : m_Id(sym.m_Id) {}
	
	// operator=
	CUtlSymbol& operator=( CUtlSymbol const& src ) { m_Id = src.m_Id; return *this; }
	
	// operator==
	bool operator==( CUtlSymbol const& src ) const { return m_Id == src.m_Id; }

	// Is valid?
	bool IsValid() const { return m_Id != UTL_INVAL_SYMBOL; }
	
	// Gets at the symbol
	operator UtlSymId_t () const { return m_Id; }

protected:
	CUtlSymbol( const char* pStr );
	bool operator==( const char* pStr ) const;
	const char* String( ) const;

	UtlSymId_t   m_Id;
};


// This creates a simple class that includes the underlying CUtlSymbol
//  as a private member and then instances a private symbol table to
//  manage those symbols.  Avoids the possibility of the code polluting the
//  'global'/default symbol table, while letting the code look like 
//  it's just using = and .String() to look at CUtlSymbol type objects
//
// NOTE:  You can't pass these objects between .dlls in an interface (also true of CUtlSymbol of course)
//
#define DECLARE_PRIVATE_SYMBOLTYPE( typename )			\
	class typename										\
	{													\
	public:												\
		typename();										\
		typename( const char* pStr );					\
		typename& operator=( typename const& src );		\
		bool operator==( typename const& src ) const;	\
		const char* String( ) const;					\
	private:											\
		CUtlSymbol m_SymbolId;							\
	};	

// Put this in the .cpp file that uses the above typename
#define IMPLEMENT_PRIVATE_SYMBOLTYPE( typename )					\
	static CUtlSymbolTable g_##typename##SymbolTable;				\
	typename::typename()											\
	{																\
		m_SymbolId = UTL_INVAL_SYMBOL;								\
	}																\
	typename::typename( const char* pStr )							\
	{																\
		m_SymbolId = g_##typename##SymbolTable.AddString( pStr );	\
	}																\
	typename& typename::operator=( typename const& src )			\
	{																\
		m_SymbolId = src.m_SymbolId;								\
		return *this;												\
	}																\
	bool typename::operator==( typename const& src ) const			\
	{																\
		return ( m_SymbolId == src.m_SymbolId );					\
	}																\
	const char* typename::String( ) const							\
	{																\
		return g_##typename##SymbolTable.String( m_SymbolId );		\
	}

#endif // UTLSYMBOL_H
