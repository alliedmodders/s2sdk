#include "tier0/threadtools.h"

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

static CThreadRWLock_FastRead *g_pLock = NULL;
static CInterlockedInt g_nState;

// Tries a read lock from another thread and reports whether it got it
static uintp TryReadThread( void * )
{
	bool bLocked = g_pLock->TryLockForRead( __FILE__, __LINE__ );
	if ( bLocked )
		g_pLock->UnlockRead( __FILE__, __LINE__ );
	g_nState = bLocked ? 1 : 2;
	return 0;
}

// Takes the write lock from another thread, which waits for the main thread's read lock to go
static uintp WriteThread( void * )
{
	g_pLock->LockForWrite( __FILE__, __LINE__ );
	g_nState = 3;
	g_pLock->UnlockWrite( __FILE__, __LINE__ );
	return 0;
}

static bool RunThread( ThreadFunc_t pfnThread )
{
	g_nState = 0;
	ThreadHandle_t hThread = CreateSimpleThread( pfnThread, NULL );
	bool bJoined = ThreadJoin( hThread );
	ReleaseThreadHandle( hThread );
	return bJoined;
}

int main()
{
	CThreadRWLock_FastRead lock;
	g_pLock = &lock;

	// Nested reads on one thread, then a write lock is free to take
	lock.LockForRead( __FILE__, __LINE__ );
	CHECK( lock.TryLockForRead( __FILE__, __LINE__ ) );
	lock.UnlockRead( __FILE__, __LINE__ );
	lock.UnlockRead( __FILE__, __LINE__ );

	CHECK( RunThread( TryReadThread ) && g_nState == 1 );

	// The write lock is recursive and keeps other threads' reads out until the last unlock
	lock.LockForWrite( __FILE__, __LINE__ );
	lock.LockForWrite( __FILE__, __LINE__ );
	CHECK( RunThread( TryReadThread ) && g_nState == 2 );
	lock.UnlockWrite( __FILE__, __LINE__ );
	CHECK( RunThread( TryReadThread ) && g_nState == 2 );
	lock.UnlockWrite( __FILE__, __LINE__ );
	CHECK( RunThread( TryReadThread ) && g_nState == 1 );

	// A writer waits for the readers
	lock.LockForRead( __FILE__, __LINE__ );
	g_nState = 0;
	ThreadHandle_t hWriter = CreateSimpleThread( WriteThread, NULL );
	ThreadSleep( 100 );
	CHECK( g_nState == 0 );
	lock.UnlockRead( __FILE__, __LINE__ );
	CHECK( ThreadJoin( hWriter ) );
	ReleaseThreadHandle( hWriter );
	CHECK( g_nState == 3 );

	// And the lock is free again
	CHECK( RunThread( TryReadThread ) && g_nState == 1 );

	if ( g_nFailures )
	{
		printf( "%d checks failed\n", g_nFailures );
		return 1;
	}

	printf( "All checks passed\n" );
	return 0;
}
