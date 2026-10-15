#include "tier0/buffer_string.h"

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

static const char k_szStackPointer[] = "longer than eight";
static const char k_szLong[] = "a string longer than the eight chars of CBufferString";

// The stack capacity of CBufferStringN is its size
static void TestFixedCapacity()
{
	CBufferStringN<100> str;
	CHECK( str.IsStackAllocated() );
	CHECK( str.AllocatedNum() == 100 );

	CBufferStringN<30> small( "twelve chars" );
	CHECK( small.IsStackAllocated() );
	CHECK( small.AllocatedNum() == 30 );
	CHECK( strcmp( small.Get(), "twelve chars" ) == 0 );
}

// A copied CBufferStringN keeps its own stack buffer
static void TestFixedCopy()
{
	CBufferStringN<64> a( k_szLong );
	CHECK( a.IsStackAllocated() );

	CBufferStringN<64> b( a );
	CHECK( b.IsStackAllocated() );
	CHECK( b.AllocatedNum() == 64 );
	CHECK( strcmp( b.Get(), k_szLong ) == 0 );

	CBufferString base( k_szLong );
	CBufferStringN<64> c( base );
	CHECK( c.IsStackAllocated() );
	CHECK( strcmp( c.Get(), k_szLong ) == 0 );
}

// Assigning a CBufferStringN copies only the string, not stale bytes of the source's stack buffer
static void TestFixedAssign()
{
	CBufferStringN<16> a( "0123456789abcde" );
	CHECK( a.IsStackAllocated() );

	// Moves to the heap, leaving "89abcde" behind in the stack buffer
	a.Insert( a.Length(), "fghij" );
	CHECK( !a.IsStackAllocated() );
	a.TruncateAt( 12 );
	CHECK( strcmp( a.Get(), "0123456789ab" ) == 0 );

	CBufferStringN<16> b;
	b = a;
	CHECK( b.IsStackAllocated() );
	CHECK( b.Length() == 12 );
	CHECK( strcmp( b.Get(), "0123456789ab" ) == 0 );
}

// Moving takes over the source's heap buffer and empties the source
static void TestMove()
{
	CBufferString a( k_szLong );
	CHECK( !a.IsStackAllocated() );
	const char *pBuffer = a.Get();

	CBufferString b( std::move( a ) );
	CHECK( b.Get() == pBuffer );
	CHECK( strcmp( b.Get(), k_szLong ) == 0 );
	CHECK( a.Length() == 0 );

	CBufferString c;
	c = std::move( b );
	CHECK( c.Get() == pBuffer );
	CHECK( b.Length() == 0 );
}

// A moved CBufferStringN keeps its own stack buffer
static void TestFixedMove()
{
	CBufferStringN<64> a( "short" );
	CBufferStringN<64> b( std::move( a ) );
	CHECK( b.IsStackAllocated() );
	CHECK( b.AllocatedNum() == 64 );
	CHECK( strcmp( b.Get(), "short" ) == 0 );
	CHECK( a.Length() == 0 );
	CHECK( a.IsStackAllocated() );
	CHECK( a.AllocatedNum() == 64 );

	CBufferStringN<64> c;
	c = std::move( b );
	CHECK( c.IsStackAllocated() );
	CHECK( strcmp( c.Get(), "short" ) == 0 );
	CHECK( b.Length() == 0 );
}

// Assigning a C string replaces the contents
static void TestAssignString()
{
	CBufferString a( "old" );
	a = "new contents";
	CHECK( strcmp( a.Get(), "new contents" ) == 0 );

	CBufferStringN<32> b( "old" );
	b = "new contents";
	CHECK( b.IsStackAllocated() );
	CHECK( strcmp( b.Get(), "new contents" ) == 0 );
}

struct StoredString_t
{
	void Set( const char *pString )
	{
		m_String.Clear();
		m_String.Insert( 0, pString );
		m_pString = m_String.Get();
	}

	CBufferStringN<32> m_String;
	const char *m_pString;
};

// A kept pointer into a stack buffer reads the whole string, not just its first 8 chars
static NOINLINE void TestStackPointer( const char *pString )
{
	StoredString_t stored;
	stored.Set( pString );
	CHECK( strcmp( stored.m_pString, "longer than eight" ) == 0 );
	CHECK( strcmp( stored.m_String.Get(), "longer than eight" ) == 0 );
}

int main()
{
	TestFixedCapacity();
	TestFixedCopy();
	TestFixedAssign();
	TestMove();
	TestFixedMove();
	TestAssignString();
	TestStackPointer( k_szStackPointer );

	if ( g_nFailures )
	{
		printf( "%d checks failed\n", g_nFailures );
		return 1;
	}

	printf( "All checks passed\n" );
	return 0;
}
