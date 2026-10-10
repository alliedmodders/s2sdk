#include "tier0/utlleanvector.h"

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

static int g_nDestructed = 0;

struct Counted_t
{
	~Counted_t() { ++g_nDestructed; }

	int m_nValue;
};

// Purge on an external buffer destructs the elements and detaches the buffer without freeing it
static void TestExternalPurge()
{
	Counted_t buffer[4] = {};

	{
		CUtlLeanVector<Counted_t> vec( buffer, 4, 3 );
		CHECK( vec.IsExternallyAllocated() );
		CHECK( vec.Count() == 3 );
		CHECK( vec.Base() == buffer );

		g_nDestructed = 0;
		vec.Purge();
		CHECK( g_nDestructed == 3 );
		CHECK( vec.Count() == 0 );
		CHECK( vec.NumAllocated() == 0 );
		CHECK( !vec.IsExternallyAllocated() );
		CHECK( vec.Base() == nullptr );

		vec.AddToTail( Counted_t{ 5 } );
		CHECK( vec.Count() == 1 );
		CHECK( !vec.IsExternallyAllocated() );
		CHECK( vec.Base() != buffer );
		CHECK( vec[0].m_nValue == 5 );
	}

	{
		CUtlLeanVector<int> vec;
		vec.AddToTail( 1 );
		vec.AddToTail( 2 );
		vec.Purge();
		CHECK( vec.Count() == 0 );
		CHECK( vec.NumAllocated() == 0 );
		CHECK( vec.Base() == nullptr );
	}
}

// Purge leaves the contents of an external buffer intact for its owner
static void TestExternalPurgeKeepsContents()
{
	char buffer[16];
	memset( buffer, 'A', sizeof( buffer ) );

	{
		CUtlLeanVector<char> vec( buffer, sizeof( buffer ), sizeof( buffer ) );
		CHECK( vec.IsExternallyAllocated() );
	}

	for ( char c : buffer )
		CHECK( c == 'A' );
}

int main()
{
	TestExternalPurge();
	TestExternalPurgeKeepsContents();

	if ( g_nFailures )
	{
		printf( "%d checks failed\n", g_nFailures );
		return 1;
	}

	printf( "All checks passed\n" );
	return 0;
}
