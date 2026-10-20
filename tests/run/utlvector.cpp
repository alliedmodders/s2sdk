#include "tier0/utlvector.h"
#include "tier0/utlstring.h"

#include <stdio.h>
#include <string.h>
#include <type_traits>
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

template< class V >
static bool IsInside( const V &vec, const void *p )
{
	return p >= (const void *)&vec && p < (const void *)( &vec + 1 );
}

// Wipes a vector's inline storage before freeing it, so a vector still pointing into it reads other values
template< class V >
static void Destroy( V *pVec )
{
	pVec->~V();
	memset( (void *)pVec, 0xAB, sizeof( V ) );
	::operator delete( pVec );
}

// CUtlVector moves, the other vectors on CUtlVectorBase don't
static_assert( std::is_move_constructible_v<CUtlVector<int>> );
static_assert( !std::is_move_constructible_v<CUtlVectorFixedGrowable<int, 4>> );
static_assert( !std::is_move_constructible_v<CUtlVectorConservative<int>> );
static_assert( !std::is_move_constructible_v<CUtlVectorRawAllocator<int>> );

// Moves swap the memory, so the destination owns the source's elements once the source is gone
static void TestGrowableMove()
{
	{
		auto *pSrc = new CUtlVector<int>;
		Fill( *pSrc, 20, 0 );
		const int *pMemory = pSrc->Base();

		CUtlVector<int> dst( std::move( *pSrc ) );
		CHECK( dst.Base() == pMemory );
		CHECK( Holds( dst, 20, 0 ) );
		CHECK( pSrc->Count() == 0 );
		CHECK( pSrc->Base() == nullptr );

		Destroy( pSrc );
		CHECK( Holds( dst, 20, 0 ) );
	}

	{
		auto *pSrc = new CUtlVector<int>;
		Fill( *pSrc, 20, 0 );

		const int *pMemory = pSrc->Base();

		CUtlVector<int> dst;
		Fill( dst, 3, 100 );
		dst = std::move( *pSrc );
		CHECK( dst.Base() == pMemory );
		CHECK( Holds( dst, 20, 0 ) );
		CHECK( Holds( *pSrc, 3, 100 ) );

		Destroy( pSrc );
		CHECK( Holds( dst, 20, 0 ) );
	}

	{
		CUtlVector<int> vec;
		Fill( vec, 5, 0 );

		CUtlVector<int> &self = vec;
		vec = std::move( self );
		CHECK( Holds( vec, 5, 0 ) );
	}
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

static int g_nConstructed = 0;
static int g_nDestructed = 0;

struct Counted_t
{
	Counted_t() { ++g_nConstructed; }
	Counted_t( const Counted_t & ) { ++g_nConstructed; }
	Counted_t &operator=( const Counted_t & ) = default;
	~Counted_t() { ++g_nDestructed; }
};

// SetCount, and so copy assignment and CopyArray, constructs or destroys only the elements it adds or removes
static void TestCountKeepsElements()
{
	CUtlVector<Counted_t> five, three, one;
	five.AddMultipleToTail( 5 );
	three.AddMultipleToTail( 3 );
	one.AddMultipleToTail( 1 );
	three.EnsureCapacity( 5 );

	g_nConstructed = g_nDestructed = 0;
	three = five;
	CHECK( three.Count() == 5 && g_nConstructed == 2 && g_nDestructed == 0 );

	g_nConstructed = g_nDestructed = 0;
	three = one;
	CHECK( three.Count() == 1 && g_nConstructed == 0 && g_nDestructed == 4 );

	g_nConstructed = g_nDestructed = 0;
	three.SetCount( 3 );
	CHECK( three.Count() == 3 && g_nConstructed == 2 && g_nDestructed == 0 );

	g_nConstructed = g_nDestructed = 0;
	three.SetCount( 2 );
	CHECK( three.Count() == 2 && g_nConstructed == 0 && g_nDestructed == 1 );

	Counted_t source[3];
	g_nConstructed = g_nDestructed = 0;
	three.CopyArray( source, 3 );
	CHECK( three.Count() == 3 && g_nConstructed == 1 && g_nDestructed == 0 );
}

// SetCount adds and removes at the tail, so the elements before it keep their values
static void TestCountAtTail()
{
	CUtlVector<int> vec;
	Fill( vec, 5, 0 );

	vec.SetCount( 3 );
	CHECK( Holds( vec, 3, 0 ) );

	vec.SetCount( 4 );
	CHECK( vec.Count() == 4 && vec[0] == 0 && vec[1] == 1 && vec[2] == 2 );
}

// CopyArray of a NULL array only resizes
static void TestCopyArrayNull()
{
	CUtlVector<int> vec;
	Fill( vec, 3, 0 );

	vec.CopyArray( nullptr, 5 );
	CHECK( vec.Count() == 5 && vec[0] == 0 && vec[1] == 1 && vec[2] == 2 );

	vec.CopyArray( nullptr, 0 );
	CHECK( vec.Count() == 0 );
}

// Without moves, assigning from an rvalue copies into the destination's own memory, inline or on the heap
template< class V >
static void TestAssignCopies( int nSrcCount, int nDstCount )
{
	auto *pSrc = new V;
	Fill( *pSrc, nSrcCount, 0 );

	V dst;
	Fill( dst, nDstCount, 100 );
	dst = std::move( *pSrc );
	CHECK( Holds( dst, nSrcCount, 0 ) );
	CHECK( !IsInside( *pSrc, dst.Base() ) && ( nSrcCount == 0 || dst.Base() != pSrc->Base() ) );
	CHECK( Holds( *pSrc, nSrcCount, 0 ) );

	Destroy( pSrc );
	CHECK( Holds( dst, nSrcCount, 0 ) );

	V &self = dst;
	dst = std::move( self );
	CHECK( Holds( dst, nSrcCount, 0 ) );
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

	vec = std::move( self );
	CHECK( HoldsStrings( vec, nCount ) );
}

int main()
{
	TestGrowableMove();
	TestGrowableCopy();
	TestCountKeepsElements();
	TestCountAtTail();
	TestCopyArrayNull();
	TestSelfAssignReturnsEarly<CUtlVector<ResetsFirst_t>>( 3 );
	TestSelfAssignReturnsEarly<CUtlVectorFixedGrowable<ResetsFirst_t, 4>>( 2 );
	TestSelfAssignReturnsEarly<CUtlVectorFixedGrowable<ResetsFirst_t, 4>>( 10 );

	// Inline to inline, inline to heap, heap to inline, heap to heap
	TestAssignCopies<CUtlVectorFixedGrowable<int, 4>>( 2, 3 );
	TestAssignCopies<CUtlVectorFixedGrowable<int, 4>>( 2, 10 );
	TestAssignCopies<CUtlVectorFixedGrowable<int, 4>>( 10, 3 );
	TestAssignCopies<CUtlVectorFixedGrowable<int, 4>>( 10, 20 );
	TestAssignCopies<CUtlVectorFixed<int, 4>>( 2, 3 );
	TestAssignCopies<CUtlVectorFixed<int, 4>>( 0, 3 );
	TestAssignCopies<CUtlVectorConservative<int>>( 2, 3 );
	TestAssignCopies<CUtlVectorRawAllocator<int>>( 2, 3 );

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
