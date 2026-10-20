#include "tier0/utlrbtree.h"
#include "tier0/utlmap.h"
#include "tier0/utlvector.h"

#include <stdio.h>
#include <utility>

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

// Movable, but not copyable through its CUtlVector
struct NonCopyable_t
{
	NonCopyable_t() : m_nKey( 0 ) {}
	NonCopyable_t( int nKey, int nValues = 0 ) : m_nKey( nKey )
	{
		for ( int i = 0; i < nValues; i++ )
			m_Values.AddToTail( nKey * 100 + i );
	}

	bool operator<( const NonCopyable_t &other ) const { return m_nKey < other.m_nKey; }

	bool HasValues( int nValues ) const
	{
		if ( m_Values.Count() != nValues )
			return false;

		for ( int i = 0; i < nValues; i++ )
		{
			if ( m_Values[i] != m_nKey * 100 + i )
				return false;
		}

		return true;
	}

	int m_nKey;
	CUtlVector< int > m_Values;
};

typedef CUtlRBTree< NonCopyable_t, unsigned int > NonCopyableTree_t;

static bool IsInorder( const NonCopyableTree_t &tree )
{
	int nPrev = -1;
	for ( unsigned int i = tree.FirstInorder(); i != tree.InvalidIndex(); i = tree.NextInorder( i ) )
	{
		if ( tree[i].m_nKey < nPrev )
			return false;
		nPrev = tree[i].m_nKey;
	}

	return tree.IsValid();
}

// Moved in elements keep their contents through the node storage growing many times
static void TestTreeMoveInsert()
{
	NonCopyableTree_t tree;

	for ( int i = 0; i < 100; i++ )
		tree.Insert( NonCopyable_t( ( i * 37 ) % 100, 3 ) );

	CHECK( tree.Count() == 100 );
	CHECK( IsInorder( tree ) );

	bool bIntact = true;
	for ( unsigned int i = tree.FirstInorder(); i != tree.InvalidIndex(); i = tree.NextInorder( i ) )
		bIntact = bIntact && tree[i].HasValues( 3 );
	CHECK( bIntact );

	// A freed node is reused by the next insert
	tree.RemoveAt( tree.Find( NonCopyable_t( 50 ) ) );
	unsigned int i = tree.Insert( NonCopyable_t( 50, 1 ) );
	CHECK( tree[i].HasValues( 1 ) );
	CHECK( tree.Count() == 100 );
	CHECK( IsInorder( tree ) );
}

static void TestTreeMoveDupes()
{
	NonCopyableTree_t tree;
	unsigned int i = tree.Insert( NonCopyable_t( 1, 1 ) );
	tree.Insert( NonCopyable_t( 2, 1 ) );

	unsigned int j = tree.Insert( NonCopyable_t( 1, 2 ), k_eInsertAllowDupes );
	CHECK( j != i );
	CHECK( tree.Count() == 3 );
	CHECK( tree[i].HasValues( 1 ) && tree[j].HasValues( 2 ) );
	CHECK( IsInorder( tree ) );

	// Replaces the first match from the root, and keeps its node
	unsigned int k = tree.Insert( NonCopyable_t( 2, 3 ), k_eInsertUpdateDupes );
	CHECK( tree.Count() == 3 );
	CHECK( tree[k].m_nKey == 2 && tree[k].HasValues( 3 ) );
	CHECK( IsInorder( tree ) );

	// Asserts, then replaces like k_eInsertUpdateDupes
	unsigned int l = tree.Insert( NonCopyable_t( 2, 2 ) );
	CHECK( l == k );
	CHECK( tree.Count() == 3 );
	CHECK( tree[l].HasValues( 2 ) );
}

static void TestMapNonCopyable()
{
	CUtlOrderedMap< int, CUtlVector< int >, CDefLess< int >, unsigned int > map;

	for ( int i = 0; i < 100; i++ )
	{
		CUtlVector< int > values;
		values.AddToTail( i );
		map.Insert( ( i * 37 ) % 100, std::move( values ) );
	}

	CHECK( map.Count() == 100 );

	bool bIntact = true;
	int nPrev = -1;
	FOR_EACH_MAP( map, it )
	{
		bIntact = bIntact && map.Key( it ) > nPrev && map[it].Count() == 1 && ( map[it][0] * 37 ) % 100 == map.Key( it );
		nPrev = map.Key( it );
	}
	CHECK( bIntact );

	unsigned int i = map.Insert( 200 );
	CHECK( map.Key( i ) == 200 && map[i].Count() == 0 );

	unsigned int j = map.InsertWithDupes( 200 );
	CHECK( j != i );
	CHECK( map.Count() == 102 );

	CUtlVector< int > values;
	values.AddToTail( 7 );
	unsigned int k = map.InsertOrReplace( 10, std::move( values ) );
	CHECK( map.Count() == 102 );
	CHECK( map.Key( k ) == 10 && map[k].Count() == 1 && map[k][0] == 7 );

	// Asserts, then replaces the element with a default one
	unsigned int l = map.Insert( 10 );
	CHECK( l == k );
	CHECK( map.Count() == 102 );
	CHECK( map[l].Count() == 0 );
}

// Resets itself before taking the other's value, so only the node's own check keeps it on self-assignment
struct ResetsFirst_t
{
	ResetsFirst_t( int nValue = 0 ) : m_nValue( nValue ) {}
	ResetsFirst_t( const ResetsFirst_t &other ) = default;
	ResetsFirst_t &operator=( const ResetsFirst_t &other ) { m_nValue = 0; m_nValue = other.m_nValue; return *this; }
	ResetsFirst_t &operator=( ResetsFirst_t &&other ) { m_nValue = 0; V_swap( m_nValue, other.m_nValue ); return *this; }

	int m_nValue;
};

// Assigning a node to itself keeps its element
static void TestMapNodeSelfAssign()
{
	CUtlOrderedMap< int, ResetsFirst_t, CDefLess< int >, unsigned int > map;
	unsigned int i = map.Insert( 5, ResetsFirst_t( 7 ) );

	auto &node = ( *map.AccessTree() )[i];
	auto *pSelf = &node;

	node = *pSelf;
	CHECK( map.Key( i ) == 5 && map[i].m_nValue == 7 );

	node = std::move( *pSelf );
	CHECK( map.Key( i ) == 5 && map[i].m_nValue == 7 );
}

int main()
{
	TestTreeDupes();
	TestMapDupes();
	TestTreeMoveInsert();
	TestTreeMoveDupes();
	TestMapNonCopyable();
	TestMapNodeSelfAssign();

	if ( g_nFailures )
	{
		printf( "%d checks failed\n", g_nFailures );
		return 1;
	}

	printf( "All checks passed\n" );
	return 0;
}
