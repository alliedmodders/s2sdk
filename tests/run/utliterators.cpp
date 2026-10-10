#include "tier1/utlvector.h"
#include "tier1/utlleanvector.h"
#include "tier1/utllinkedlist.h"
#include "tier1/utlrbtree.h"
#include "tier1/utlmap.h"
#include "tier1/utldict.h"
#include "tier1/utlstring.h"
#include "tier0/utlblockvector.h"

#include <algorithm>
#include <iterator>
#include <numeric>
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

static const int s_Values[] = { 5, 3, 9, 1, 7, 2, 8 };
static const int s_Sorted[] = { 1, 2, 3, 5, 7, 8, 9 };
static const int s_nValues = sizeof( s_Values ) / sizeof( s_Values[0] );

// Walks the container forwards, as const too, and checks it yields expected in order
template < typename Container >
static bool YieldsInOrder( Container &c, const int *expected, int count )
{
	int i = 0;
	for ( int &v : c )
	{
		if ( i >= count || v != expected[i] )
			return false;
		++i;
	}
	if ( i != count )
		return false;

	i = 0;
	const Container &cc = c;
	for ( const int &v : cc )
	{
		if ( i >= count || v != expected[i] )
			return false;
		++i;
	}
	return i == count && std::distance( c.begin(), c.end() ) == count;
}

// Walks a bidirectional container backwards from end()
template < typename Container >
static bool YieldsReversed( const Container &c, const int *expected, int count )
{
	int i = count;
	for ( auto it = c.end(); it != c.begin(); )
	{
		--it;
		if ( --i < 0 || *it != expected[i] )
			return false;
	}
	return i == 0;
}

// Doubles every element through the non-const iterator
template < typename Container >
static void DoubleAll( Container &c )
{
	for ( int &v : c )
		v *= 2;
}

template < typename Vector >
static void TestVector( Vector &vec, int count )
{
	CHECK( vec.begin() == vec.end() );

	int *expected = new int[count];
	for ( int i = 0; i < count; i++ )
	{
		expected[i] = i * 3 + 1;
		vec.AddToTail( expected[i] );
	}
	CHECK( YieldsInOrder( vec, expected, count ) );
	CHECK( YieldsReversed( vec, expected, count ) );
	CHECK( std::accumulate( vec.begin(), vec.end(), 0 ) == std::accumulate( expected, expected + count, 0 ) );
	CHECK( std::find( vec.begin(), vec.end(), expected[count / 2] ) - vec.begin() == count / 2 );

	DoubleAll( vec );
	for ( int i = 0; i < count; i++ )
		expected[i] *= 2;
	CHECK( YieldsInOrder( vec, expected, count ) );

	vec.RemoveAll();
	CHECK( vec.begin() == vec.end() );
	delete[] expected;
}

static void TestVectors()
{
	{
		CUtlVector<int> vec;
		TestVector( vec, 100 );
	}

	{
		// Grows past its inline storage
		CUtlVectorFixedGrowable<int, 4> vec;
		TestVector( vec, 100 );
	}

	{
		CUtlVectorFixed<int, 16> vec;
		TestVector( vec, 16 );
	}

	{
		CUtlVectorConservative<int> vec;
		TestVector( vec, 100 );
	}

	{
		CUtlVectorRawAllocator<int> vec;
		TestVector( vec, 100 );
	}

	{
		// Spans several blocks
		CUtlBlockVector<int> vec;
		TestVector( vec, 1000 );
	}

	{
		CUtlLeanVector<int> vec;
		TestVector( vec, 100 );
	}

	{
		CUtlVector<int> vec;
		for ( int v : s_Values )
			vec.AddToTail( v );
		vec.Sort();
		CHECK( YieldsInOrder( vec, s_Sorted, s_nValues ) );
	}
}

template < typename List >
static void TestList( List &list )
{
	CHECK( list.begin() == list.end() );
	CHECK( list.Count() == 0 );

	// 2, 1, 3, 4 with a removed element in the middle
	list.AddToTail( 1 );
	auto removed = list.AddToTail( 100 );
	list.AddToTail( 3 );
	list.AddToHead( 2 );
	list.AddToTail( 4 );
	CHECK( list.Count() == 5 );
	list.Remove( removed );
	CHECK( list.Count() == 4 );
	CHECK( !list.IsValidIndex( removed ) );

	const int expected[] = { 2, 1, 3, 4 };
	CHECK( YieldsInOrder( list, expected, 4 ) );
	CHECK( YieldsReversed( list, expected, 4 ) );

	DoubleAll( list );
	const int doubled[] = { 4, 2, 6, 8 };
	CHECK( YieldsInOrder( list, doubled, 4 ) );

	CHECK( list.AddToTail( 5 ) == removed );
	CHECK( list.Count() == 5 );

	list.RemoveAll();
	CHECK( list.begin() == list.end() );
	CHECK( list.Count() == 0 );

	list.AddToTail( 1 );
	CHECK( list.Count() == 1 );
	list.Purge();
	CHECK( list.Count() == 0 );
}

static void TestLinkedLists()
{
	{
		CUtlLinkedList<int> list;
		TestList( list );
	}

	{
		CUtlLinkedList<int, unsigned short, true> list;
		TestList( list );
	}

	{
		CUtlFixedLinkedList<int> list;
		TestList( list );
	}

	{
		CUtlBlockLinkedList<int> list;
		TestList( list );
	}
}

static void TestRBTree()
{
	CUtlRBTree<int> tree;
	CHECK( tree.begin() == tree.end() );

	for ( int v : s_Values )
		tree.Insert( v );

	CHECK( YieldsInOrder( tree, s_Sorted, s_nValues ) );
	CHECK( YieldsReversed( tree, s_Sorted, s_nValues ) );

	tree.Remove( 5 );
	const int expected[] = { 1, 2, 3, 7, 8, 9 };
	CHECK( YieldsInOrder( tree, expected, 6 ) );
}

static void TestOrderedMap()
{
	CUtlOrderedMap<int, CUtlString> map;
	CHECK( map.begin() == map.end() );

	for ( int v : s_Values )
	{
		char buf[16];
		snprintf( buf, sizeof( buf ), "v%d", v );
		map.Insert( v, buf );
	}

	int i = 0;
	for ( auto &node : map )
	{
		char buf[16];
		snprintf( buf, sizeof( buf ), "v%d", s_Sorted[i] );
		CHECK( i < s_nValues && node.key == s_Sorted[i] && node.elem == buf );
		node.elem = "changed";
		++i;
	}
	CHECK( i == s_nValues );

	i = 0;
	const CUtlOrderedMap<int, CUtlString> &cmap = map;
	for ( const auto &node : cmap )
	{
		CHECK( node.key == s_Sorted[i] && node.elem == "changed" );
		++i;
	}
	CHECK( i == s_nValues );

	i = s_nValues;
	for ( auto it = map.end(); it != map.begin(); )
	{
		--it;
		CHECK( it->key == s_Sorted[--i] );
	}
	CHECK( i == 0 );
}

static void TestDict()
{
	CUtlDict<int> dict;
	CHECK( dict.begin() == dict.end() );

	dict.Insert( "charlie", 3 );
	dict.Insert( "alpha", 1 );
	dict.Insert( "delta", 4 );
	dict.Insert( "bravo", 2 );

	static const char *const names[] = { "alpha", "bravo", "charlie", "delta" };
	int i = 0;
	for ( auto &node : dict )
	{
		CHECK( i < 4 && strcmp( node.key, names[i] ) == 0 && node.elem == i + 1 );
		node.elem *= 10;
		++i;
	}
	CHECK( i == 4 );

	i = 0;
	const CUtlDict<int> &cdict = dict;
	for ( const auto &node : cdict )
	{
		CHECK( strcmp( node.key, names[i] ) == 0 && node.elem == ( i + 1 ) * 10 );
		++i;
	}
	CHECK( i == 4 );
}

int main()
{
	TestVectors();
	TestLinkedLists();
	TestRBTree();
	TestOrderedMap();
	TestDict();

	if ( g_nFailures )
	{
		printf( "%d checks failed\n", g_nFailures );
		return 1;
	}

	printf( "All checks passed\n" );
	return 0;
}
