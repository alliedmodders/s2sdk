#include "tier0/utlvector.h"
#include "tier0/utlstring.h"

#include <stdio.h>
#include <string.h>
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

template< class V >
static void Fill( V &vec, int nCount, int nFirst )
{
	vec.RemoveAll();
	for ( int i = 0; i < nCount; i++ )
		vec.AddToTail( nFirst + i );
}

template< class V >
static bool Holds( const V &vec, int nCount, int nFirst )
{
	if ( vec.Count() != nCount )
		return false;

	for ( int i = 0; i < nCount; i++ )
	{
		if ( vec[i] != nFirst + i )
			return false;
	}

	return true;
}

// Copy assignment copies the elements, it doesn't share them
static void TestGrowableCopy()
{
	CUtlVector<int> src, dst;
	Fill( src, 5, 0 );
	Fill( dst, 2, 100 );

	dst = src;
	CHECK( Holds( dst, 5, 0 ) );
	CHECK( dst.Base() != src.Base() );

	Fill( src, 5, 50 );
	CHECK( Holds( dst, 5, 0 ) );

	dst = src;
	CHECK( Holds( dst, 5, 50 ) );
}

// Resets itself before taking the other's value, so only an early return keeps it when a vector is assigned to itself
struct ResetsFirst_t
{
	ResetsFirst_t &operator=( const ResetsFirst_t &other ) { m_nValue = 0; m_nValue = other.m_nValue; return *this; }

	int m_nValue = 0;
};

template< class V >
static bool HoldsValues( const V &vec, int nCount )
{
	if ( vec.Count() != nCount )
		return false;

	for ( int i = 0; i < nCount; i++ )
	{
		if ( vec[i].m_nValue != i + 1 )
			return false;
	}

	return true;
}

template< class V >
static void TestSelfAssignReturnsEarly( int nCount )
{
	V vec;
	for ( int i = 0; i < nCount; i++ )
		vec[vec.AddToTail()].m_nValue = i + 1;

	V &self = vec;
	vec = self;
	CHECK( HoldsValues( vec, nCount ) );

	vec = std::move( self );
	CHECK( HoldsValues( vec, nCount ) );
}

static const char *const g_pszStrings[] = { "a", "b", "c", "d", "e", "f", "g", "h", "i", "j" };

template< class V >
static bool HoldsStrings( const V &vec, int nCount )
{
	if ( vec.Count() != nCount )
		return false;

	for ( int i = 0; i < nCount; i++ )
	{
		if ( strcmp( vec[i].Get(), g_pszStrings[i] ) != 0 )
			return false;
	}

	return true;
}

// Assigning a vector to itself keeps its elements, also ones that default construct to something else
template< class V >
static void TestSelfAssign( int nCount )
{
	V vec;
	for ( int i = 0; i < nCount; i++ )
		vec.AddToTail( CUtlString( g_pszStrings[i] ) );

	V &self = vec;
	vec = self;
	CHECK( HoldsStrings( vec, nCount ) );
}

int main()
{
	TestGrowableCopy();
	TestSelfAssignReturnsEarly<CUtlVector<ResetsFirst_t>>( 3 );
	TestSelfAssignReturnsEarly<CUtlVectorFixedGrowable<ResetsFirst_t, 4>>( 2 );
	TestSelfAssignReturnsEarly<CUtlVectorFixedGrowable<ResetsFirst_t, 4>>( 10 );

	TestSelfAssign<CUtlVector<CUtlString>>( 5 );
	TestSelfAssign<CUtlVectorFixedGrowable<CUtlString, 4>>( 2 );
	TestSelfAssign<CUtlVectorFixedGrowable<CUtlString, 4>>( 10 );
	TestSelfAssign<CUtlVectorFixed<CUtlString, 4>>( 3 );
	TestSelfAssign<CUtlVectorConservative<CUtlString>>( 3 );
	TestSelfAssign<CUtlVectorRawAllocator<CUtlString>>( 3 );

	if ( g_nFailures )
	{
		printf( "%d checks failed\n", g_nFailures );
		return 1;
	}

	printf( "All checks passed\n" );
	return 0;
}
