#include "tier0/bitvec.h"

#include <new>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

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

// nSize bytes that end where a no access page starts, so reading past them crashes
class CGuardedBlock
{
public:
	explicit CGuardedBlock( size_t nSize )
	{
#ifdef _WIN32
		SYSTEM_INFO info;
		GetSystemInfo( &info );
		m_nPageSize = info.dwPageSize;
		m_pPages = (uint8 *)VirtualAlloc( NULL, 2 * m_nPageSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE );
		DWORD nOldProtect;
		bool bGuarded = m_pPages && VirtualProtect( m_pPages + m_nPageSize, m_nPageSize, PAGE_NOACCESS, &nOldProtect );
#else
		m_nPageSize = (size_t)sysconf( _SC_PAGESIZE );
		void *pPages = mmap( NULL, 2 * m_nPageSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0 );
		m_pPages = pPages != MAP_FAILED ? (uint8 *)pPages : NULL;
		bool bGuarded = m_pPages && mprotect( m_pPages + m_nPageSize, m_nPageSize, PROT_NONE ) == 0;
#endif
		// Without the guard page a read past the block would go unnoticed
		if ( !bGuarded )
		{
			printf( "Couldn't set up a guard page\n" );
			exit( 1 );
		}

		m_pBlock = m_pPages + m_nPageSize - nSize;
	}

	~CGuardedBlock()
	{
#ifdef _WIN32
		VirtualFree( m_pPages, 0, MEM_RELEASE );
#else
		munmap( m_pPages, 2 * m_nPageSize );
#endif
	}

	void *Get() const { return m_pBlock; }

private:
	CGuardedBlock( const CGuardedBlock & );
	CGuardedBlock &operator=( const CGuardedBlock & );

	uint8 *m_pPages;
	uint8 *m_pBlock;
	size_t m_nPageSize;
};

template <int NUM_BITS>
static void TestFixed()
{
	CGuardedBlock block( sizeof( CBitVec<NUM_BITS> ) );
	CBitVec<NUM_BITS> *pBits = new ( block.Get() ) CBitVec<NUM_BITS>;
	const CBitVec<NUM_BITS> &bits = *pBits;

	CHECK( bits.FindNextSetBit( 0 ) == -1 );
	CHECK( bits.FindNextSetBit( NUM_BITS - 1 ) == -1 );
	CHECK( bits.FindNextSetBit( NUM_BITS ) == -1 );

	pBits->Set( 0 );
	pBits->Set( NUM_BITS - 1 );
	CHECK( bits.FindNextSetBit( 0 ) == 0 );
	CHECK( bits.FindNextSetBit( 1 ) == NUM_BITS - 1 );
	CHECK( bits.FindNextSetBit( NUM_BITS - 1 ) == NUM_BITS - 1 );
	CHECK( bits.FindNextSetBit( NUM_BITS ) == -1 );

	pBits->Clear( NUM_BITS - 1 );
	CHECK( bits.FindNextSetBit( 1 ) == -1 );

	// A word between the first and the last
	if ( NUM_BITS > 64 )
	{
		pBits->Set( 40 );
		CHECK( bits.FindNextSetBit( 1 ) == 40 );
		CHECK( bits.FindNextSetBit( 33 ) == 40 );
		CHECK( bits.FindNextSetBit( 41 ) == -1 );
	}

	pBits->~CBitVec<NUM_BITS>();
}

// Bits past the end in the last word don't count
static void TestFixedEndMask()
{
	CBitVec<40> bits;
	bits.SetDWord( 1, ~0u );
	CHECK( bits.FindNextSetBit( 0 ) == 32 );
	CHECK( bits.FindNextSetBit( 39 ) == 39 );
	CHECK( bits.FindNextSetBit( 40 ) == -1 );

	bits.SetDWord( 1, 0xffffff00 );
	CHECK( bits.FindNextSetBit( 0 ) == -1 );
}

static void TestVar( int numBits )
{
	const int numInts = CalcNumIntsForBits( numBits );
	CGuardedBlock block( numInts * sizeof( uint32 ) );
	uint32 *pInts = (uint32 *)block.Get();
	for ( int i = 0; i < numInts; i++ )
		pInts[i] = 0;

	CVarBitVec bits;
	bits.Attach( pInts, numBits );

	CHECK( bits.FindNextSetBit( 0 ) == -1 );
	CHECK( bits.FindNextSetBit( numBits - 1 ) == -1 );

	bits.Set( 1 );
	bits.Set( numBits - 1 );
	CHECK( bits.FindNextSetBit( 0 ) == 1 );
	CHECK( bits.FindNextSetBit( 2 ) == numBits - 1 );
	CHECK( bits.FindNextSetBit( numBits ) == -1 );

	// A word between the first and the last
	if ( numBits > 64 )
	{
		bits.Set( 40 );
		CHECK( bits.FindNextSetBit( 2 ) == 40 );
		CHECK( bits.FindNextSetBit( 41 ) == numBits - 1 );
	}

	// The block isn't the vector's to free
	uint32 *pDetached = NULL;
	int numDetached = 0;
	CHECK( bits.Detach( &pDetached, &numDetached ) );
	CHECK( pDetached == pInts && numDetached == numBits );
}

// Up to 32 bits are stored in the vector itself
static void TestVarSingleWord()
{
	CVarBitVec bits( 20 );
	CHECK( bits.FindNextSetBit( 0 ) == -1 );

	bits.Set( 5 );
	bits.Set( 19 );
	CHECK( bits.FindNextSetBit( 0 ) == 5 );
	CHECK( bits.FindNextSetBit( 6 ) == 19 );
	CHECK( bits.FindNextSetBit( 20 ) == -1 );

	CVarBitVec full( 32 );
	full.Set( 31 );
	CHECK( full.FindNextSetBit( 0 ) == 31 );
	CHECK( full.FindNextSetBit( 32 ) == -1 );
}

int main()
{
	TestFixed<32>();
	TestFixed<64>();
	TestFixed<96>();
	TestFixed<128>();
	TestFixed<40>();
	TestFixedEndMask();
	TestVar( 64 );
	TestVar( 96 );
	TestVar( 40 );
	TestVarSingleWord();

	if ( g_nFailures )
	{
		printf( "%d checks failed\n", g_nFailures );
		return 1;
	}

	printf( "All checks passed\n" );
	return 0;
}
