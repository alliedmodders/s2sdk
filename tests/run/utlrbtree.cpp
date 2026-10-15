#include "tier0/utlrbtree.h"
#include "tier0/utlmap.h"

#include <stdio.h>

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

// Ordered by m_nKey only, so a dupe can carry a different m_nValue
struct KeyValue_t
{
	KeyValue_t() : m_nKey( 0 ), m_nValue( 0 ) {}
	KeyValue_t( int nKey, int nValue ) : m_nKey( nKey ), m_nValue( nValue ) {}

	bool operator<( const KeyValue_t &other ) const { return m_nKey < other.m_nKey; }

	int m_nKey;
	int m_nValue;
};

static void TestTreeDupes()
{
	CUtlRBTree< KeyValue_t, unsigned int > tree;
	unsigned int i = tree.Insert( KeyValue_t( 1, 10 ) );
	unsigned int j = tree.Insert( KeyValue_t( 2, 20 ) );

	unsigned int k = tree.Insert( KeyValue_t( 1, 11 ), k_eInsertAllowDupes );
	CHECK( k != i );
	CHECK( tree.Count() == 3 );
	CHECK( tree[i].m_nValue == 10 && tree[k].m_nValue == 11 );

	unsigned int l = tree.Insert( KeyValue_t( 2, 21 ), k_eInsertUpdateDupes );
	CHECK( l == j );
	CHECK( tree.Count() == 3 );
	CHECK( tree[j].m_nValue == 21 );

	// Asserts, then replaces like k_eInsertUpdateDupes
	unsigned int m = tree.Insert( KeyValue_t( 2, 22 ) );
	CHECK( m == j );
	CHECK( tree.Count() == 3 );
	CHECK( tree[j].m_nValue == 22 );
	CHECK( tree.IsValid() );
}

static void TestMapDupes()
{
	CUtlOrderedMap< int, int, CDefLess< int >, unsigned int > map;
	unsigned int i = map.Insert( 1, 10 );

	unsigned int j = map.InsertWithDupes( 1, 11 );
	CHECK( j != i );
	CHECK( map.Count() == 2 );

	unsigned int k = map.Insert( 2, 20 );
	unsigned int l = map.InsertOrReplace( 2, 21 );
	CHECK( l == k );
	CHECK( map.Count() == 3 );
	CHECK( map[k] == 21 );

	// Asserts, then replaces like InsertOrReplace
	unsigned int m = map.Insert( 2, 22 );
	CHECK( m == k );
	CHECK( map.Count() == 3 );
	CHECK( map[k] == 22 );
}

int main()
{
	TestTreeDupes();
	TestMapDupes();

	if ( g_nFailures )
	{
		printf( "%d checks failed\n", g_nFailures );
		return 1;
	}

	printf( "All checks passed\n" );
	return 0;
}
