#include "tier0/utlvectorultraconservative.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

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
static int g_nCopyConstructed = 0;
static int g_nAssigned = 0;

struct Counted_t
{
	explicit Counted_t( int nValue = 0 ) : m_nValue( nValue ) {}
	Counted_t( const Counted_t &other ) : m_nValue( other.m_nValue ) { ++g_nCopyConstructed; }
	Counted_t &operator=( const Counted_t &other ) { m_nValue = other.m_nValue; ++g_nAssigned; return *this; }
	~Counted_t() { ++g_nDestructed; }

	bool operator==( const Counted_t &other ) const { return m_nValue == other.m_nValue; }

	int m_nValue;
};

// An empty vector holds null, as the engine's do, and every query works on it
static void TestEmptyIsNull()
{
	CUtlVectorUltraConservative<int> vec;
	CHECK( vec.m_pData == nullptr );
	CHECK( vec.Count() == 0 );
	CHECK( vec.Base() == nullptr );
	CHECK( !vec.IsValidIndex( 0 ) );
	CHECK( vec.Find( 1 ) == -1 );
	CHECK( !vec.FindAndRemove( 1 ) );

	vec.RemoveAll();
	CHECK( vec.m_pData == nullptr );

	CUtlVectorUltraConservative<int *> pointers;
	pointers.PurgeAndDeleteElements();
	CHECK( pointers.m_pData == nullptr );
}

// A block the engine built (an int count, then the elements) reads through the SDK's class and is released by it
static void TestEngineBlock()
{
	static int a = 1, b = 2;

	typedef CUtlVectorUltraConservative<int *> Vector_t;
	Vector_t vec;
	Vector_t::Data_t *pData = (Vector_t::Data_t *)malloc( sizeof( Vector_t::Data_t ) + 2 * sizeof( int * ) );
	int **ppElements = (int **)pData->m_Elements;
	pData->m_Size = 2;
	ppElements[0] = &a;
	ppElements[1] = &b;
	vec.m_pData = pData;

	CHECK( vec.Count() == 2 );
	CHECK( vec[0] == &a && vec[1] == &b );
	CHECK( vec.Find( &b ) == 1 );

	vec.RemoveAll();
	CHECK( vec.m_pData == nullptr );
	CHECK( vec.Count() == 0 );
}

// The block is the engine's: an int count, then the elements at their own alignment
static void TestLayout()
{
	CHECK( offsetof( CUtlVectorUltraConservative<int *>::Data_t, m_Elements ) == 8 );
	CHECK( sizeof( CUtlVectorUltraConservative<int *>::Data_t ) == 8 );
	CHECK( offsetof( CUtlVectorUltraConservative<int>::Data_t, m_Elements ) == 4 );
	CHECK( offsetof( CUtlVectorUltraConservative<Counted_t>::Data_t, m_Elements ) == 4 );
}

// AddToTail copy constructs the new element in the block's raw storage instead of assigning to it
static void TestAddCopyConstructs()
{
	CUtlVectorUltraConservative<Counted_t> vec;
	Counted_t value( 7 );

	g_nCopyConstructed = g_nAssigned = 0;
	vec.AddToTail( value );
	vec.AddToTail( value );
	CHECK( g_nCopyConstructed == 2 );
	CHECK( g_nAssigned == 0 );
	CHECK( vec.Count() == 2 && vec[1].m_nValue == 7 );
	CHECK( vec.Base() == &vec[0] );
}

static void TestAddRemove()
{
	CUtlVectorUltraConservative<Counted_t> vec;
	CHECK( vec.AddToTail( Counted_t( 10 ) ) == 0 );
	CHECK( vec.AddToTail( Counted_t( 20 ) ) == 1 );
	CHECK( vec.AddToTail( Counted_t( 30 ) ) == 2 );
	CHECK( vec.AddToTail( Counted_t( 40 ) ) == 3 );
	CHECK( vec.Count() == 4 );

	// Remove keeps the order, FastRemove moves the last element into the gap
	vec.Remove( 1 );
	CHECK( vec.Count() == 3 && vec[0].m_nValue == 10 && vec[1].m_nValue == 30 && vec[2].m_nValue == 40 );
	vec.FastRemove( 0 );
	CHECK( vec.Count() == 2 && vec[0].m_nValue == 40 && vec[1].m_nValue == 30 );

	CHECK( vec.FindAndRemove( Counted_t( 30 ) ) );
	CHECK( vec.Count() == 1 );
	CHECK( vec.FindAndFastRemove( Counted_t( 40 ) ) );
	CHECK( vec.Count() == 0 );
	CHECK( vec.m_pData == nullptr );

	// Usable again once emptied
	vec.AddToTail( Counted_t( 50 ) );
	CHECK( vec.Count() == 1 && vec[0].m_nValue == 50 );
}

// RemoveAll destructs every element and leaves the vector empty
static void TestRemoveAllDestructs()
{
	CUtlVectorUltraConservative<Counted_t> vec;
	for ( int i = 0; i < 5; i++ )
		vec.AddToTail( Counted_t( i ) );

	g_nDestructed = 0;
	vec.RemoveAll();
	CHECK( g_nDestructed == 5 );
	CHECK( vec.Count() == 0 );
	CHECK( vec.m_pData == nullptr );
}

// PurgeAndDeleteElements deletes what the pointers point to
static void TestPurgeAndDeleteElements()
{
	CUtlVectorUltraConservative<Counted_t *> vec;
	for ( int i = 0; i < 3; i++ )
		vec.AddToTail( new Counted_t( i ) );

	g_nDestructed = 0;
	vec.PurgeAndDeleteElements();
	CHECK( g_nDestructed == 3 );
	CHECK( vec.Count() == 0 );
	CHECK( vec.m_pData == nullptr );
}

int main()
{
	TestEmptyIsNull();
	TestEngineBlock();
	TestLayout();
	TestAddCopyConstructs();
	TestAddRemove();
	TestRemoveAllDestructs();
	TestPurgeAndDeleteElements();

	if ( g_nFailures )
	{
		printf( "%d checks failed\n", g_nFailures );
		return 1;
	}

	printf( "All checks passed\n" );
	return 0;
}
