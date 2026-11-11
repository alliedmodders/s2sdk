#include "tier0/utlbuffer.h"

#include <stdio.h>
#include <string.h>

static int g_nFailures = 0;

#define CHECK( expr ) \
	do \
	{ \
		if ( !( expr ) ) \
		{ \
			printf( "%s:%d: CHECK( %s ) failed\n", __FILE__, __LINE__, #expr ); \
			++g_nFailures; \
		} \
	} while ( 0 )

static bool Contains( const CUtlBuffer &buf, const char *pExpected )
{
	int nLength = (int)strlen( pExpected );
	return buf.TellPut() == nLength && memcmp( buf.Base(), pExpected, nLength ) == 0;
}

// Binary values of every type read back as written, in either byte order; reading past the end gives 0
static void TestBinaryRoundTrip()
{
	for ( int nSwap = 0; nSwap < 2; nSwap++ )
	{
		CUtlBuffer buf;
		buf.ActivateByteSwapping( nSwap != 0 );
		buf.PutChar( -5 );
		buf.PutUnsignedChar( 250 );
		buf.PutShort( -12345 );
		buf.PutUnsignedShort( 54321 );
		buf.PutInt( -123456789 );
		buf.PutUnsignedInt( 4000000000u );
		buf.PutInt64( -1234567890123LL );
		buf.PutFloat( 3.5f );
		buf.PutDouble( -2.25 );
		buf.PutPtr( &buf );
		CHECK( buf.TellPut() == 34 + (int)sizeof( void * ) );

		CHECK( buf.GetChar() == -5 );
		CHECK( buf.GetUnsignedChar() == 250 );
		CHECK( buf.GetShort() == -12345 );
		CHECK( buf.GetUnsignedShort() == 54321 );
		CHECK( buf.GetInt() == -123456789 );
		CHECK( buf.GetUnsignedInt() == 4000000000u );
		CHECK( buf.GetInt64() == -1234567890123LL );
		CHECK( buf.GetFloat() == 3.5f );
		CHECK( buf.GetDouble() == -2.25 );
		CHECK( buf.GetPtr() == &buf );
		CHECK( buf.GetBytesRemaining() == 0 && buf.IsValid() );

		CHECK( buf.GetInt() == 0 );
		CHECK( !buf.IsValid() );
	}

	CUtlBuffer big;
	big.SetBigEndian( true );
	big.PutInt( 0x01020304 );
	big.PutShort( 0x0506 );
	CHECK( Contains( big, "\x01\x02\x03\x04\x05\x06" ) );
}

// Text values read back as written; values that don't parse or don't fit their type read as 0
static void TestTextRoundTrip()
{
	CUtlBuffer buf( 0, 0, CUtlBuffer::TEXT_BUFFER );
	buf.PutShort( -123 );
	buf.PutChar( ' ' );
	buf.PutUnsignedShort( 60000 );
	buf.PutChar( ' ' );
	buf.PutInt( -2000000000 );
	buf.PutChar( ' ' );
	buf.PutUnsignedInt( 4000000000u );
	buf.PutChar( ' ' );
	buf.PutInt64( -9000000000LL );
	buf.PutChar( ' ' );
	buf.PutFloat( 0.5f );
	buf.PutChar( ' ' );
	buf.PutDouble( 0.125 );
	buf.PutChar( ' ' );
	buf.PutUnsignedChar( 200 );
	buf.PutString( " 1f" );

	CHECK( buf.GetShort() == -123 );
	buf.EatWhiteSpace();
	CHECK( buf.GetUnsignedShort() == 60000 );
	buf.EatWhiteSpace();
	CHECK( buf.GetInt() == -2000000000 );
	buf.EatWhiteSpace();
	CHECK( buf.GetUnsignedInt() == 4000000000u );
	buf.EatWhiteSpace();
	CHECK( buf.GetInt64() == -9000000000LL );
	buf.EatWhiteSpace();
	CHECK( buf.GetFloat() == 0.5f );
	buf.EatWhiteSpace();
	CHECK( buf.GetDouble() == 0.125 );
	buf.EatWhiteSpace();
	CHECK( buf.GetUnsignedChar() == 200 );
	buf.EatWhiteSpace();
	CHECK( buf.GetIntHex() == 0x1f );
	CHECK( buf.GetBytesRemaining() == 0 );

	CUtlBuffer bad( 0, 0, CUtlBuffer::TEXT_BUFFER );
	bad.PutString( "70000 99999999999 -5 word" );
	CHECK( bad.GetShort() == 0 );
	CHECK( bad.TellGet() == 5 );
	bad.EatWhiteSpace();
	CHECK( bad.GetInt() == 0 );
	bad.EatWhiteSpace();
	CHECK( bad.GetUnsignedInt() == 0 );
	bad.EatWhiteSpace();
	int nGet = bad.TellGet();
	CHECK( bad.GetInt() == 0 );
	CHECK( bad.TellGet() == nGet );
}

// Tabs indent every line that has text, the first one included; blank lines and disabled tabs get none
static void TestTabs()
{
	CUtlBuffer buf( 0, 0, CUtlBuffer::TEXT_BUFFER );
	buf.PutString( "{\n" );
	buf.PushTab();
	buf.PutString( "a\n" );
	buf.PutChar( 'b' );
	buf.PutChar( '\n' );
	buf.PutChar( '\n' );
	buf.PushTab();
	buf.PutString( "c\nd\n" );
	buf.PopTab();
	buf.PopTab();
	buf.PopTab();
	buf.PutString( "}\n" );
	CHECK( Contains( buf, "{\n\ta\n\tb\n\n\t\tc\n\t\td\n}\n" ) );

	CUtlBuffer first( 0, 0, CUtlBuffer::TEXT_BUFFER );
	first.PushTab();
	first.PutChar( 'x' );
	CHECK( Contains( first, "\tx" ) );

	CUtlBuffer disabled( 0, 0, CUtlBuffer::TEXT_BUFFER );
	disabled.PushTab();
	disabled.EnableTabs( false );
	disabled.PutString( "a\nb" );
	disabled.EnableTabs( true );
	disabled.PutChar( '\n' );
	disabled.PutChar( 'c' );
	CHECK( Contains( disabled, "a\nb\n\tc" ) );
}

// Get and put positions, seeking, peeking, and the memory Clear, Purge and Detach leave
static void TestPositions()
{
	CUtlBuffer buf;
	buf.PutInt( 1 );
	buf.PutInt( 2 );
	CHECK( buf.TellPut() == 8 && buf.TellMaxPut() == 8 && buf.GetBytesRemaining() == 8 );
	CHECK( buf.GetInt() == 1 );
	CHECK( buf.TellGet() == 4 && buf.GetBytesRemaining() == 4 );
	CHECK( *(const int *)buf.PeekGet() == 2 && buf.TellGet() == 4 );

	buf.SeekPut( CUtlBuffer::SEEK_HEAD, 0 );
	buf.PutInt( 3 );
	CHECK( buf.TellPut() == 4 && buf.TellMaxPut() == 8 );
	*(int *)buf.PeekPut() = 4;
	buf.SeekGet( CUtlBuffer::SEEK_HEAD, 0 );
	CHECK( buf.GetInt() == 3 && buf.GetInt() == 4 );

	int nSize = buf.Size();
	buf.Clear();
	CHECK( buf.TellPut() == 0 && buf.TellGet() == 0 && buf.IsValid() && buf.Size() == nSize );
	buf.Purge();
	CHECK( buf.TellPut() == 0 && buf.Size() == 0 && buf.Base() == nullptr );

	CUtlBuffer direct;
	void *pRead = direct.AccessForDirectRead( 4 );
	*(int *)pRead = 77;
	CHECK( direct.TellMaxPut() == 4 && direct.GetInt() == 77 );

	CUtlBuffer detach;
	detach.PutString( "kept" );
	char *pMemory = (char *)detach.Detach();
	CHECK( pMemory != nullptr && strcmp( pMemory, "kept" ) == 0 );
	CHECK( detach.TellPut() == 0 && detach.TellGet() == 0 );
	g_pMemAlloc->Free( pMemory );
}

BEGIN_CHAR_CONVERSION( s_TestConversion, "'", '^' )
	{ '\n', "n" },
	{ '\'', "q" },
	{ '^', "^" },
	{ '\xFF', "ff" },
END_CHAR_CONVERSION( s_TestConversion, "'", '^' )

class CDelimitedTestBuffer : public CUtlBuffer
{
public:
	CDelimitedTestBuffer() : CUtlBuffer( 0, 0, CUtlBuffer::TEXT_BUFFER ) {}

	void PutEscaped( CUtlCharConversion *pConv, char c ) { PutDelimitedCharInternal( pConv, c ); }
};

// Delimited chars and strings escape what the conversion lists and read back as written
static void TestDelimited()
{
	CHECK( s_TestConversion.GetEscapeChar() == '^' && s_TestConversion.GetConversionLength( '\xFF' ) == 2 );
	CHECK( s_TestConversion.MaxConversionLength() == 2 && s_TestConversion.GetConversionLength( 'a' ) == 0 );

	CDelimitedTestBuffer chars;
	chars.PutEscaped( &s_TestConversion, 'a' );
	chars.PutEscaped( &s_TestConversion, '\n' );
	chars.PutEscaped( &s_TestConversion, '\'' );
	chars.PutEscaped( &s_TestConversion, '^' );
	chars.PutEscaped( &s_TestConversion, '\xFF' );
	CHECK( Contains( chars, "a^n^q^^^ff" ) );

	CUtlBuffer buf( 0, 0, CUtlBuffer::TEXT_BUFFER );
	buf.PutDelimitedChar( &s_TestConversion, 'x' );
	buf.PutDelimitedChar( &s_TestConversion, '\n' );
	buf.PutDelimitedString( &s_TestConversion, "it's ^\n" );
	CHECK( Contains( buf, "x^n'it^qs ^^^n'" ) );

	CHECK( buf.GetDelimitedChar( &s_TestConversion ) == 'x' );
	CHECK( buf.GetDelimitedChar( &s_TestConversion ) == '\n' );
	char str[16];
	buf.GetDelimitedString( &s_TestConversion, str, sizeof( str ) );
	CHECK( strcmp( str, "it's ^\n" ) == 0 );
	CHECK( buf.GetBytesRemaining() == 0 && buf.IsValid() );

	// Tabs go before the escape char, as before any other char
	CDelimitedTestBuffer tabs;
	tabs.PushTab();
	tabs.PutEscaped( &s_TestConversion, '\'' );
	tabs.PutChar( '\n' );
	tabs.PutEscaped( &s_TestConversion, 'b' );
	CHECK( Contains( tabs, "\t^q\n\tb" ) );

	CUtlBuffer cstr( 0, 0, CUtlBuffer::TEXT_BUFFER );
	cstr.PutDelimitedString( GetCStringCharConversion(), "a\"b\n" );
	CHECK( Contains( cstr, "\"a\\\"b\\n\"" ) );
	cstr.GetDelimitedString( GetCStringCharConversion(), str, sizeof( str ) );
	CHECK( strcmp( str, "a\"b\n" ) == 0 );
}

// The flags and the memory a buffer was made with
static void TestFlags()
{
	static const char data[] = "abcd";
	CUtlBuffer ext( data, 4, CUtlBuffer::READ_ONLY );
	CHECK( ext.IsReadOnly() && ext.IsExternallyAllocated() && !ext.IsText() && !ext.IsGrowable() );
	CHECK( ext.Base() == data && ext.Size() == 4 && ext.TellMaxPut() == 4 );
	CHECK( ext.GetFlags() == CUtlBuffer::READ_ONLY );

	CUtlBuffer text( 0, 0, CUtlBuffer::TEXT_BUFFER | CUtlBuffer::CONTAINS_CRLF );
	CHECK( text.IsText() && text.ContainsCRLF() && !text.IsExternallyAllocated() );
	text.SetBufferType( false, false );
	CHECK( !text.IsText() && !text.ContainsCRLF() );
}

int main()
{
	TestBinaryRoundTrip();
	TestTextRoundTrip();
	TestTabs();
	TestPositions();
	TestDelimited();
	TestFlags();

	if ( g_nFailures )
	{
		printf( "%d checks failed\n", g_nFailures );
		return 1;
	}

	printf( "All checks passed\n" );
	return 0;
}
