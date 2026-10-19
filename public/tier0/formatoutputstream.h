#ifndef FORMATOUTPUTSTREAM_H
#define FORMATOUTPUTSTREAM_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"
#include "tier0/buffer_string.h"
#include "tier0/utlbuffer.h"
#include "tier0/utlstring.h"

#include <limits.h>
#include <stdio.h>
#include <initializer_list>
#include <string_view>
#include <utility>

class IFormatOutputStream;

//-----------------------------------------------------------------------------
// A printf style element of a format string, like "%08.3f"
//-----------------------------------------------------------------------------
class CFormatStringElement
{
public:
	CFormatStringElement() : m_unk001( 0 ) {}
	// A nullptr element is "%?"
	DLL_CLASS_IMPORT CFormatStringElement( const char *pszElement );

	// Parses the element from the text after its '%', returning the text after the element
	DLL_CLASS_IMPORT const char *ParseElement( const char *pszFormat );
	// Makes the element's type one of pszTypes, else the first of them, with the pszSize length modifier
	DLL_CLASS_IMPORT void EnforceTypeAndSize( const char *pszTypes, const char *pszSize );

	operator const char *() const { return m_szElement; }

private:
	uint16 m_unk001;
	char m_szElement[12];
};

// A value along with the function that formats it
class CTypeErasedFormattingWrapper
{
public:
	typedef void (*FormatFunc_t)( IFormatOutputStream *pStream, CFormatStringElement element, const void *pValue );

	const void *m_pValue;
	FormatFunc_t m_pfnFormat;
};

//-----------------------------------------------------------------------------
// Where formatted text goes
//-----------------------------------------------------------------------------
class PLATFORM_CLASS IFormatOutputStream
{
public:
	enum EOutputFlags_t
	{
		UNICODE_OK = (1 << 0),
		HTML = (1 << 1),
		MULTI_LINE = (1 << 2)
	};

	IFormatOutputStream() : m_nFlags( 0 ), m_nCharsWritten( 0 ), m_unk101( 64 ), m_unk102( 1 ), m_unk103( INT_MAX ) {}
	virtual ~IFormatOutputStream() {}

	virtual void PutC( char c ) = 0;
	virtual void PutS( std::string_view str );
	virtual void PutRepeatedChar( char c, int nCount );

	virtual void FormatArray( CTypeErasedFormattingWrapper wrapper, CFormatStringElement element, const void **ppValues, int nCount, bool bTruncated );
	virtual void Format2DTable( CTypeErasedFormattingWrapper wrapper, CFormatStringElement element, const void **ppValues, int nColumns, int nRows );
	virtual void FormatEnumHelper( CFormatStringElement element, int nValue, std::initializer_list< std::pair< int, const char * > > names );
	virtual void FormatMaskHelper( int nValue, std::initializer_list< std::pair< int, const char * > > names );

	// Null terminates the output, where the stream needs it
	virtual void Terminate();
	virtual void BeginItem();
	virtual void EndItem();
	virtual int RemainingCapacity() const;
	virtual bool IsFull() const;
	// Puts the current thread's indentation as spaces
	virtual void OutputIndentation();
	virtual void SetTargetLine( int nLine );
	virtual void SetAnchor();

	bool UnicodeOK() const { return ( m_nFlags & UNICODE_OK ) != 0; }
	bool IsHTML() const { return ( m_nFlags & HTML ) != 0; }
	bool IsMultiLine() const { return ( m_nFlags & MULTI_LINE ) != 0; }
	bool IsNonFull() const { return !IsFull(); }

	void NewLineAndIndent() { PutC( '\n' ); OutputIndentation(); }

	int GetCharsWritten() const { return m_nCharsWritten; }

protected:
	// EOutputFlags_t
	int m_nFlags;
	int m_nCharsWritten;
	int m_unk101;
	int m_unk102;
	int m_unk103;
};

// Appends to a CBufferString
class PLATFORM_CLASS CBufferStringFormatOutputStream : public IFormatOutputStream
{
public:
	CBufferStringFormatOutputStream() : m_pString( nullptr ) {}
	CBufferStringFormatOutputStream( CBufferString &str ) : m_pString( &str ) {}

	void SetString( CBufferString &str ) { m_pString = &str; }

	virtual void PutC( char c );

private:
	CBufferString *m_pString;
};

// Writes to a char buffer, truncating to leave room for the null terminator that Terminate() writes
class PLATFORM_CLASS CCharBufferFormatOutputStream : public IFormatOutputStream
{
public:
	CCharBufferFormatOutputStream( char *pBuffer, int nBufferSize ) : m_pCurrent( pBuffer ), m_pEnd( pBuffer + nBufferSize ) {}

	virtual void PutC( char c );
	virtual void PutS( std::string_view str );
	virtual void Terminate();
	virtual int RemainingCapacity() const;
	virtual bool IsFull() const;

private:
	char *m_pCurrent;
	char *m_pEnd;
};

class PLATFORM_CLASS CStdIOFormatOutputStream : public IFormatOutputStream
{
public:
	CStdIOFormatOutputStream( FILE *pFile, bool bFlush ) : m_pFile( pFile ), m_bFlush( bFlush ) {}

	virtual void PutC( char c );
	virtual void PutS( std::string_view str );

	// Flushes the file if the stream was created with bFlush
#ifdef PLATFORM_WINDOWS
	void FlushIfDesired();
#else
	void FlushIfDesired() { if ( m_bFlush ) fflush( m_pFile ); }
#endif

private:
	FILE *m_pFile;
	bool m_bFlush;
};

// Puts into a CUtlBuffer, indenting new lines of text buffers
class PLATFORM_CLASS CUtlBufferFormatOutputStream : public IFormatOutputStream
{
public:
	CUtlBufferFormatOutputStream( CUtlBuffer &buf ) : m_pBuffer( &buf ) {}

	virtual void PutC( char c );

private:
	CUtlBuffer *m_pBuffer;
};

PLATFORM_OVERLOAD void FormatTo( IFormatOutputStream *pStream, CFormatStringElement element, bool bValue );
PLATFORM_OVERLOAD void FormatTo( IFormatOutputStream *pStream, CFormatStringElement element, char cValue );
PLATFORM_OVERLOAD void FormatTo( IFormatOutputStream *pStream, CFormatStringElement element, int nValue );
PLATFORM_OVERLOAD void FormatTo( IFormatOutputStream *pStream, CFormatStringElement element, uint32 nValue );
PLATFORM_OVERLOAD void FormatTo( IFormatOutputStream *pStream, CFormatStringElement element, long nValue );
PLATFORM_OVERLOAD void FormatTo( IFormatOutputStream *pStream, CFormatStringElement element, unsigned long nValue );
PLATFORM_OVERLOAD void FormatTo( IFormatOutputStream *pStream, CFormatStringElement element, int64 nValue );
PLATFORM_OVERLOAD void FormatTo( IFormatOutputStream *pStream, CFormatStringElement element, uint64 nValue );
PLATFORM_OVERLOAD void FormatTo( IFormatOutputStream *pStream, CFormatStringElement element, float flValue );
PLATFORM_OVERLOAD void FormatTo( IFormatOutputStream *pStream, CFormatStringElement element, double flValue );
PLATFORM_OVERLOAD void FormatTo( IFormatOutputStream *pStream, CFormatStringElement element, const void *pValue );
PLATFORM_OVERLOAD void FormatTo( IFormatOutputStream *pStream, CFormatStringElement element, const char *pszValue );
PLATFORM_OVERLOAD void FormatTo( IFormatOutputStream *pStream, CFormatStringElement element, const wchar_t *pwszValue );
PLATFORM_OVERLOAD void FormatTo( IFormatOutputStream *pStream, CFormatStringElement element, std::string_view value );
PLATFORM_OVERLOAD void FormatTo( IFormatOutputStream *pStream, CFormatStringElement element, const CUtlString &value );

// Formats pszFormat with the arguments, whose elements can be any printf element ("%?" picks the default format)
PLATFORM_OVERLOAD void FormatArgumentsToBuffer( IFormatOutputStream *pStream, const char *pszFormat, const CTypeErasedFormattingWrapper *pArgs, int nArgs );

#endif // FORMATOUTPUTSTREAM_H
