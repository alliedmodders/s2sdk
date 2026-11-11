#include "tier0/utlpriorityqueue.h"

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

// A sleeping job's entry: the earliest wakeup has the greatest priority
struct Sleeper_t
{
	uint64 m_nWakeupTime;
	int m_nID;
};

class CSleeperLess
{
public:
	bool operator()( const Sleeper_t &lhs, const Sleeper_t &rhs ) const { return lhs.m_nWakeupTime > rhs.m_nWakeupTime; }
};

static bool IntGreater( const int &lhs, const int &rhs )
{
	return lhs > rhs;
}

template < class Q, class L >
static bool IsHeap( const Q &queue, const L &less )
{
	for ( int i = 1; i < queue.Count(); i++ )
	{
		if ( less( queue.Element( ( ( i + 1 ) / 2 ) - 1 ), queue.Element( i ) ) )
			return false;
	}

	return true;
}

static unsigned int g_nSeed = 12345;

static int NextRandom( int nRange )
{
	g_nSeed = g_nSeed * 1103515245u + 12345u;
	return (int)( ( g_nSeed >> 16 ) % (unsigned int)nRange );
}

// Values come out of the head in priority order, duplicates included, with the default less (greatest first)
static void TestDefaultOrder()
{
	CUtlPriorityQueue<int> queue;
	CDefUtlPriorityQueueLessFunc<int> less;
	for ( int i = 0; i < 200; i++ )
	{
		queue.Insert( NextRandom( 50 ) );
		CHECK( IsHeap( queue, less ) );
	}
	CHECK( queue.Count() == 200 );

	int nLast = queue.ElementAtHead();
	int nPopped = 0;
	while ( queue.Count() > 0 )
	{
		int nHead = queue.ElementAtHead();
		CHECK( nHead <= nLast );
		nLast = nHead;
		queue.RemoveAtHead();
		CHECK( IsHeap( queue, less ) );
		nPopped++;
	}
	CHECK( nPopped == 200 );
}

// A function pointer type as the less func, set through the constructor and SetLessFunc (least first)
static void TestFunctionPointer()
{
	CUtlPriorityQueue<int, bool (*)( const int &, const int & )> queue( 0, 0, IntGreater );
	for ( int i = 0; i < 64; i++ )
		queue.Insert( 1000 - i * 7 % 61 );

	int nLast = -1;
	while ( queue.Count() > 0 )
	{
		CHECK( queue.ElementAtHead() >= nLast );
		nLast = queue.ElementAtHead();
		queue.RemoveAtHead();
	}

	CUtlPriorityQueue<int, bool (*)( const int &, const int & )> late;
	late.SetLessFunc( IntGreater );
	late.Insert( 3 );
	late.Insert( 1 );
	late.Insert( 2 );
	CHECK( late.ElementAtHead() == 1 );
}

// Removing any element keeps the heap and the other elements; a functor less like the game's job queue
static void TestRemoveAt()
{
	CUtlPriorityQueue<Sleeper_t, CSleeperLess> queue;
	CSleeperLess less;
	int nPresent[128] = {};
	for ( int i = 0; i < 128; i++ )
	{
		Sleeper_t sleeper = { (uint64)NextRandom( 40 ), i };
		queue.Insert( sleeper );
		nPresent[i] = 1;
	}

	while ( queue.Count() > 0 )
	{
		int index = NextRandom( queue.Count() );
		nPresent[queue.Element( index ).m_nID]--;
		queue.RemoveAt( index );
		CHECK( IsHeap( queue, less ) );

		int nLeft = 0;
		for ( int i = 0; i < queue.Count(); i++ )
			nLeft += nPresent[queue.Element( i ).m_nID];
		CHECK( nLeft == queue.Count() );
	}

	for ( int i = 0; i < 128; i++ )
		CHECK( nPresent[i] == 0 );
}

// Equal priorities: a new element moves up past equal ones, as the engine's queues do
static void TestEqualPriorities()
{
	CUtlPriorityQueue<Sleeper_t, CSleeperLess> queue;
	Sleeper_t first = { 10, 1 };
	Sleeper_t second = { 10, 2 };
	queue.Insert( first );
	queue.Insert( second );
	CHECK( queue.ElementAtHead().m_nID == 2 );

	Sleeper_t later = { 20, 3 };
	queue.Insert( later );
	CHECK( queue.ElementAtHead().m_nID == 2 );
	CHECK( queue.Count() == 3 );

	// The element RemoveAt moves into the hole also moves up past a parent of equal priority
	CUtlPriorityQueue<Sleeper_t, CSleeperLess> removal;
	const Sleeper_t sleepers[] = { { 1, 1 }, { 2, 2 }, { 2, 3 }, { 3, 4 }, { 2, 5 } };
	for ( const Sleeper_t &sleeper : sleepers )
		removal.Insert( sleeper );
	CHECK( removal.Element( 1 ).m_nID == 5 && removal.Element( 4 ).m_nID == 2 );

	removal.RemoveAt( 3 );
	CHECK( removal.Count() == 4 );
	CHECK( removal.Element( 1 ).m_nID == 2 && removal.Element( 3 ).m_nID == 5 );
}

// Memory: RemoveAll keeps the allocation, Purge frees it, and external memory is used in place
static void TestMemory()
{
	CUtlPriorityQueue<int> queue( 0, 16 );
	for ( int i = 0; i < 10; i++ )
		queue.Insert( i );
	queue.RemoveAll();
	CHECK( queue.Count() == 0 );
	queue.Insert( 5 );
	CHECK( queue.ElementAtHead() == 5 );
	queue.Purge();
	CHECK( queue.Count() == 0 );

	int buffer[8];
	CUtlPriorityQueue<int> external( buffer, 8 );
	external.Insert( 2 );
	external.Insert( 7 );
	CHECK( external.ElementAtHead() == 7 );
	CHECK( &external.Element( 0 ) == &buffer[0] );
}

int main()
{
	TestDefaultOrder();
	TestFunctionPointer();
	TestRemoveAt();
	TestEqualPriorities();
	TestMemory();

	if ( g_nFailures )
	{
		printf( "%d checks failed\n", g_nFailures );
		return 1;
	}

	printf( "All checks passed\n" );
	return 0;
}
