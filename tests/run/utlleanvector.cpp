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

static int g_nLive = 0;

// Points into itself, so a bitwise copy leaves m_pSelf pointing at the old location
struct SelfRef_t
{
	SelfRef_t() : m_nValue( 0 ), m_pSelf( &m_nValue ) { ++g_nLive; }
	SelfRef_t( int nValue ) : m_nValue( nValue ), m_pSelf( &m_nValue ) { ++g_nLive; }
	SelfRef_t( const SelfRef_t &other ) : m_nValue( other.m_nValue ), m_pSelf( &m_nValue ) { ++g_nLive; }
	~SelfRef_t() { --g_nLive; }

	int m_nValue;
	int *m_pSelf;
};

template< class V >
static bool IsIntact( const V &vec )
{
	for ( int i = 0; i < vec.Count(); i++ )
	{
		if ( vec[i].m_pSelf != &vec[i].m_nValue || vec[i].m_nValue != i )
			return false;
	}

	return true;
}

// Growing moves non-trivially copyable elements with their constructors instead of bitwise
template< class V >
static void TestGrowRelocates()
{
	g_nLive = 0;

	{
		// Interleaved growth keeps realloc from extending either block in place
		V vec1, vec2;
		for ( int i = 0; i < 100; i++ )
		{
			vec1.AddToTail( SelfRef_t( i ) );
			vec2.AddToTail( SelfRef_t( i ) );
		}

		CHECK( vec1.Count() == 100 );
		CHECK( IsIntact( vec1 ) );
		CHECK( IsIntact( vec2 ) );
		CHECK( g_nLive == 200 );
	}

	CHECK( g_nLive == 0 );
}

static void TestExternalGrowRelocates()
{
	g_nLive = 0;

	{
		alignas( SelfRef_t ) unsigned char buffer[ 2 * sizeof( SelfRef_t ) ];
		SelfRef_t *pBuffer = reinterpret_cast<SelfRef_t *>( buffer );
		Construct( &pBuffer[0], 0 );
		Construct( &pBuffer[1], 1 );

		CUtlLeanVector<SelfRef_t> vec( pBuffer, 2, 2 );
		vec.AddToTail( SelfRef_t( 2 ) );

		CHECK( !vec.IsExternallyAllocated() );
		CHECK( vec.Count() == 3 );
		CHECK( IsIntact( vec ) );
		CHECK( g_nLive == 3 );
	}

	CHECK( g_nLive == 0 );
}

// Counts and indices use the vector's index type, whatever its width and signedness
template< class V, class I >
static void TestIndexType()
{
	V vec;
	for ( int i = 0; i < 40; i++ )
		CHECK( vec.AddToTail( i ) == ( I )i );

	I nCount = vec.Count();
	CHECK( nCount == 40 );
	CHECK( vec.NumAllocated() >= nCount );
	CHECK( vec.IsValidIndex( nCount - 1 ) && !vec.IsValidIndex( nCount ) );

	CHECK( vec.AddMultipleToTail( 2 ) == nCount );
	CHECK( vec.Count() == 42 );
	vec.RemoveMultipleFromTail( 2 );

	*vec.InsertBeforeGetPtr( 0 ) = -1;
	CHECK( vec.Count() == 41 && vec[ 0 ] == -1 && vec[ 1 ] == 0 && vec.Tail() == 39 );

	CHECK( vec.Find( 20 ) == 21 );
	CHECK( vec.Find( 100 ) == V::InvalidIndex() );
	CHECK( vec.FindAndRemove( -1 ) && !vec.FindAndRemove( 100 ) );
	CHECK( vec.Count() == 40 && vec[ 0 ] == 0 );

	vec.RemoveMultiple( 10, 10 );
	CHECK( vec.Count() == 30 && vec[ 10 ] == 20 );

	vec.FastRemove( 0 );
	CHECK( vec.Count() == 29 && vec[ 0 ] == 39 );

	vec.SetCount( 5 );
	vec.EnsureCount( 8 );
	CHECK( vec.Count() == 8 );
}

int main()
{
	TestExternalPurge();
	TestExternalPurgeKeepsContents();
	TestGrowRelocates<CUtlLeanVector<SelfRef_t>>();
	TestGrowRelocates<CUtlLeanVectorFixedGrowable<SelfRef_t, 2>>();
	TestExternalGrowRelocates();
	TestIndexType<CUtlLeanVector<int>, int>();
	TestIndexType<CUtlLeanVector<int, short>, short>();
	TestIndexType<CUtlLeanVector<int, unsigned short>, unsigned short>();
	TestIndexType<CUtlLeanVector<int, unsigned int>, unsigned int>();
	TestIndexType<CUtlLeanVector<int, int64>, int64>();
	TestIndexType<CUtlLeanVectorFixedGrowable<int, 4, unsigned int>, unsigned int>();
	TestIndexType<CUtlLeanVectorFixedGrowable<int, 4, int64>, int64>();

	if ( g_nFailures )
	{
		printf( "%d checks failed\n", g_nFailures );
		return 1;
	}

	printf( "All checks passed\n" );
	return 0;
}
