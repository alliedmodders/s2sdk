#include "tier0/utlsymbollarge.h"

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

// Lookups go through UtlSymTableLargeAltKey and inserts through UtlSymTableLargeIdKey,
// the two alternate key types of the table's CUtlHashtable
template < typename Table >
static void TestTable( bool bCaseInsensitive )
{
	Table table;

	CHECK( table.FindRaw( "missing" ) == UTL_INVAL_SYMBOL_LARGE );
	CHECK( table.Find( "missing" ) == CUtlSymbolLarge() );

	bool bCreated = false;
	UtlSymLargeId_t first = table.AddStringRaw( "Alpha", &bCreated );
	CHECK( bCreated && first != UTL_INVAL_SYMBOL_LARGE );
	CHECK( strcmp( table.String( first ), "Alpha" ) == 0 );

	CHECK( table.AddStringRaw( "Alpha", &bCreated ) == first && !bCreated );
	CHECK( table.FindRaw( "Alpha" ) == first );
	CHECK( ( table.FindRaw( "ALPHA" ) == first ) == bCaseInsensitive );
	CHECK( ( table.AddStringRaw( "alpha", &bCreated ) == first ) == bCaseInsensitive );
	CHECK( bCreated == !bCaseInsensitive );

	// By length, the prefix is a different string from the full one
	UtlSymLargeId_t prefix = table.AddStringRaw( "Alphabet", 5, &bCreated );
	CHECK( prefix == first && !bCreated );
	UtlSymLargeId_t alphabet = table.AddStringRaw( "Alphabet", &bCreated );
	CHECK( alphabet != first && bCreated );
	CHECK( table.FindRaw( "Alphabet", 5 ) == first );
	CHECK( table.FindRaw( "Alphabet" ) == alphabet );

	CUtlSymbolLarge sym = table.AddString( "Beta" );
	CHECK( strcmp( sym.String(), "Beta" ) == 0 );
	CHECK( table.Find( "Beta" ) == sym );

	// Enough strings of odd lengths to rehash the table several times and leave the hashes unaligned
	const int nCount = 5000;
	static UtlSymLargeId_t ids[ nCount ];
	char buf[ 64 ];
	for ( int i = 0; i < nCount; i++ )
	{
		int len = snprintf( buf, sizeof( buf ), "symbol_%d%.*s", i, i % 7, "xxxxxxx" );
		ids[ i ] = table.AddStringRaw( buf, len, &bCreated );
		CHECK( bCreated );
	}
	for ( int i = 0; i < nCount; i++ )
	{
		int len = snprintf( buf, sizeof( buf ), "symbol_%d%.*s", i, i % 7, "xxxxxxx" );
		CHECK( table.FindRaw( buf ) == ids[ i ] );
		CHECK( strcmp( table.String( ids[ i ] ), buf ) == 0 );
		CHECK( table.HashValue( ids[ i ] ) == CUtlSymbolLarge_Hash( bCaseInsensitive, buf, len ) );
		CHECK( table.AddStringRaw( buf, &bCreated ) == ids[ i ] && !bCreated );
	}
	CHECK( table.FindRaw( "Alpha" ) == first );

	table.RemoveAll();
	CHECK( table.FindRaw( "Alpha" ) == UTL_INVAL_SYMBOL_LARGE );
	CHECK( table.FindRaw( "symbol_1x" ) == UTL_INVAL_SYMBOL_LARGE );
	first = table.AddStringRaw( "Alpha", &bCreated );
	CHECK( bCreated && table.FindRaw( "Alpha" ) == first );

	table.Purge();
	CHECK( table.FindRaw( "Alpha" ) == UTL_INVAL_SYMBOL_LARGE );
}

int main()
{
	TestTable<CUtlSymbolTableLarge>( false );
	TestTable<CUtlSymbolTableLarge_CI>( true );
	TestTable<CUtlSymbolTableLargeMT>( false );
	TestTable<CUtlSymbolTableLargeMT_CI>( true );

	if ( g_nFailures )
	{
		printf( "%d checks failed\n", g_nFailures );
		return 1;
	}

	printf( "All checks passed\n" );
	return 0;
}
