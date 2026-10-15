//========== Copyright © 2005, Valve Corporation, All rights reserved. ========
//
// Purpose: A collection of utility classes to simplify thread handling, and
//			as much as possible contain portability problems. Here avoiding 
//			including windows.h.
//
//=============================================================================

#ifndef THREADTOOLS_H
#define THREADTOOLS_H

#include <limits.h>
#include <shared_mutex>

#include "tier0/platform.h"
#include "tier0/dbg.h"
#include "tier0/threadinterlocks.h"

#ifdef PLATFORM_WINDOWS_PC
#include <intrin.h>
#endif

#ifdef POSIX
#include <pthread.h>
#include <errno.h>
#define WAIT_OBJECT_0 0
#define WAIT_TIMEOUT 0x00000102
#define WAIT_FAILED -1
#define THREAD_PRIORITY_HIGHEST 2
#endif

#ifdef OSX
// Add some missing defines
#define PTHREAD_MUTEX_TIMED_NP         PTHREAD_MUTEX_NORMAL
#define PTHREAD_MUTEX_RECURSIVE_NP     PTHREAD_MUTEX_RECURSIVE
#define PTHREAD_MUTEX_ERRORCHECK_NP    PTHREAD_MUTEX_ERRORCHECK
#define PTHREAD_MUTEX_ADAPTIVE_NP      3
#endif


#if defined( _WIN32 )
#pragma once
#pragma warning(push)
#pragma warning(disable:4251)
#endif

// #define THREAD_PROFILER 1

#define THREAD_MUTEX_TRACING_SUPPORTED
#if defined(_WIN32) && defined(_DEBUG)
#define THREAD_MUTEX_TRACING_ENABLED
#endif

#ifdef _WIN32
typedef void *HANDLE;
#endif

#ifdef _X360
#define MAX_THREADS_SUPPORTED 16
#else
#define MAX_THREADS_SUPPORTED 32
#endif



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

const unsigned TT_INFINITE = 0xffffffff;

#ifdef _WIN32
	typedef uint32 ThreadId_t;
#else
	typedef uint64 ThreadId_t;
#endif

//-----------------------------------------------------------------------------
//
// Simple thread creation. Differs from VCR mode/CreateThread/_beginthreadex
// in that it accepts a standard C function rather than compiler specific one.
//
//-----------------------------------------------------------------------------
FORWARD_DECLARE_HANDLE( ThreadHandle_t );
typedef uintp (*ThreadFunc_t)( void *pParam );

PLATFORM_INTERFACE ThreadHandle_t CreateSimpleThread( ThreadFunc_t, void *pParam, unsigned stackSize = 0 );
PLATFORM_INTERFACE bool ReleaseThreadHandle( ThreadHandle_t );


//-----------------------------------------------------------------------------

PLATFORM_OVERLOAD void ThreadAtomicNotifyOne( const uint32 *addr );
PLATFORM_OVERLOAD void ThreadAtomicNotifyN( const uint32 *addr, uint32 n );
PLATFORM_OVERLOAD void ThreadAtomicNotifyAll( const uint32 *addr );
PLATFORM_OVERLOAD void ThreadAtomicWait( volatile uint32 *addr, uint32 value );

PLATFORM_INTERFACE void ThreadSleep(unsigned duration = 0);
PLATFORM_INTERFACE void ThreadNanoSleep( uint32 nNanoseconds );
PLATFORM_INTERFACE void ThreadYield();
// Busy waits for nCycles ticks of the CPU's timestamp counter
PLATFORM_INTERFACE void ThreadSpin( uint32 nCycles );
PLATFORM_INTERFACE ThreadId_t ThreadGetCurrentId();
PLATFORM_INTERFACE ThreadHandle_t ThreadGetCurrentHandle();
PLATFORM_INTERFACE int ThreadGetPriority( ThreadHandle_t hThread = NULL );
PLATFORM_INTERFACE bool ThreadSetPriority( ThreadHandle_t hThread, int priority );
inline		 bool ThreadSetPriority( int priority ) { return ThreadSetPriority( NULL, priority ); }
PLATFORM_INTERFACE bool ThreadInMainThread();
PLATFORM_INTERFACE void DeclareCurrentThreadIsMainThread();
PLATFORM_INTERFACE ThreadId_t ThreadGetMainThreadId();

// AMNOTE: The engine declares a secondary main thread while main thread work runs on another thread,
// like preparing the client updates after a server tick
PLATFORM_INTERFACE void DeclareCurrentThreadIsSecondaryMainThread();
PLATFORM_INTERFACE void ClearSecondaryMainThread();
PLATFORM_INTERFACE bool ThreadInSecondaryMainThread();
PLATFORM_INTERFACE bool ThreadInPrimaryOrSecondaryMainThread();

// NOTE: ThreadedLoadLibraryFunc_t needs to return the sleep time in milliseconds or TT_INFINITE
typedef int (*ThreadedLoadLibraryFunc_t)(); 
PLATFORM_INTERFACE void SetThreadedLoadLibraryFunc( ThreadedLoadLibraryFunc_t func );
PLATFORM_INTERFACE ThreadedLoadLibraryFunc_t GetThreadedLoadLibraryFunc();

#if defined( PLATFORM_WINDOWS_PC )
DLL_IMPORT unsigned long STDCALL GetCurrentThreadId();
#define ThreadGetCurrentId GetCurrentThreadId
#endif

inline void ThreadPause()
{
#if defined( PLATFORM_WINDOWS_PC )
	// Intrinsic for __asm pause; from <intrin.h>
	_mm_pause();
#elif defined( POSIX )
	__asm __volatile( "pause" );
#elif defined( _X360 )
#else
#error "implement me"
#endif
}

PLATFORM_INTERFACE bool ThreadJoin( ThreadHandle_t, unsigned timeout = TT_INFINITE );

PLATFORM_INTERFACE void ThreadSetDebugNameS2( ThreadHandle_t hThread, const char *pszName );
inline		 void ThreadSetDebugName( const char *pszName ) { ThreadSetDebugNameS2( NULL, pszName ); }
PLATFORM_INTERFACE bool ThreadGetDebugNameForThreadHandle( ThreadHandle_t hThread, char *pszNameOut, size_t nNameOutSize );
PLATFORM_INTERFACE bool ThreadGetDebugNameForThreadId( ThreadId_t nThreadId, char *pszNameOut, size_t nNameOutSize );

PLATFORM_INTERFACE void ThreadSetAffinity( ThreadHandle_t hThread, int nAffinityMask );

// Fills pSortIndices with indices into CPUInformation::m_pProcessorInfo, highest m_EfficiencyClass first,
// returning how many it wrote (0 without processor info)
PLATFORM_INTERFACE uint16 GetPerfSortedLogicalProcessors( uint16 *pSortIndices, uint16 nSortIndices );
// On hybrid CPUs, limits the current thread to the logical processors of the given efficiency type
PLATFORM_INTERFACE void ThreadSetPerfSetAffinity( CPUInformation::ProcessorInfo::EfficiencyType eType );


//-----------------------------------------------------------------------------
// Runs code once across threads, with a marker initialized to 0:
//	if ( ThreadInitOnceCall_ShouldRun( &marker ) ) { ...; ThreadInitOnceCall_Finish( &marker ); }
// ShouldRun returns true to the first caller only, and makes the others wait until it finishes.
//-----------------------------------------------------------------------------
typedef volatile int32 ThreadInitOnce_t;

PLATFORM_INTERFACE bool ThreadInitOnceCall_ShouldRun( ThreadInitOnce_t *pMarker );
PLATFORM_INTERFACE void ThreadInitOnceCall_Finish( ThreadInitOnce_t *pMarker );

//-----------------------------------------------------------------------------
// Processes
//-----------------------------------------------------------------------------
// AMNOTE: A HANDLE on Windows; on Linux a pointer to tier0's process record
FORWARD_DECLARE_HANDLE( ProcessHandle_t );

enum ThreadProcessPriority_t
{
	THREAD_PROCESS_PRIORITY_IDLE,
	THREAD_PROCESS_PRIORITY_LOW,
	THREAD_PROCESS_PRIORITY_NORMAL,
	THREAD_PROCESS_PRIORITY_HIGH,
	THREAD_PROCESS_PRIORITY_REALTIME,
};

// A pseudo handle that doesn't need to be closed
PLATFORM_INTERFACE ProcessHandle_t ThreadGetCurrentProcessHandle();
PLATFORM_INTERFACE uint32 ThreadGetCurrentProcessId();
PLATFORM_INTERFACE uint32 ThreadGetProcessIdFromHandle( ProcessHandle_t hProcess );

// Translated to the matching PROCESS_* access rights on Windows.
// On Linux only READ_MEMORY and WRITE_MEMORY have an effect, they ptrace attach to the process.
enum ThreadProcessAccessFlags_t
{
	THREAD_PROCESS_ACCESS_NONE = 0,
	THREAD_PROCESS_ACCESS_SYNCHRONIZE = (1 << 0),
	THREAD_PROCESS_ACCESS_READ_MEMORY = (1 << 1),
	THREAD_PROCESS_ACCESS_WRITE_MEMORY = (1 << 2),
	THREAD_PROCESS_ACCESS_MEMORY_OPERATION = (1 << 3),
	THREAD_PROCESS_ACCESS_QUERY_INFORMATION = (1 << 4),
	THREAD_PROCESS_ACCESS_SET_INFORMATION = (1 << 5),
	THREAD_PROCESS_ACCESS_SUSPEND_RESUME = (1 << 6),
	THREAD_PROCESS_ACCESS_CREATE_THREAD = (1 << 7),
	THREAD_PROCESS_ACCESS_TERMINATE = (1 << 8),
	THREAD_PROCESS_ACCESS_DUPLICATE_HANDLE = (1 << 9),
};
DEFINE_ENUM_BITWISE_OPERATORS( ThreadProcessAccessFlags_t )

// Returns 0 on success, otherwise the OS error code.
PLATFORM_INTERFACE int ThreadOpenProcess( uint32 nProcessId, ThreadProcessAccessFlags_t nAccessFlags, ProcessHandle_t *pProcessOut );
PLATFORM_INTERFACE bool ThreadCloseProcess( ProcessHandle_t hProcess );

PLATFORM_INTERFACE bool ThreadIsProcessIdActive( uint32 nProcessId );
// Return false and the exit code once the process has exited
PLATFORM_INTERFACE bool ThreadIsProcessActive( ProcessHandle_t hProcess, int32 *pExitCode );
PLATFORM_INTERFACE bool ThreadWaitForProcessExit( ProcessHandle_t hProcess, uint32 nMillis, int32 *pExitCode );
// Returns false while the process is running
PLATFORM_INTERFACE bool ThreadGetProcessExitCode( ProcessHandle_t hProcess, int32 *pExitCode );
// On Linux the process is killed, without nExitCode
PLATFORM_INTERFACE bool ThreadTerminateProcessCode( ProcessHandle_t hProcess, int32 nExitCode );

PLATFORM_INTERFACE ThreadProcessPriority_t ThreadGetProcessPriority( ProcessHandle_t hProcess );
PLATFORM_INTERFACE bool ThreadSetProcessPriority( ProcessHandle_t hProcess, ThreadProcessPriority_t ePriority );

// Returns the number of bytes read
PLATFORM_INTERFACE int ThreadReadProcessMemory( ProcessHandle_t hProcess, const void *pAddress, void *pBuffer, int nSize );

// Return the id of the started process, or 0 on failure
PLATFORM_INTERFACE uint32 ThreadShellExecute( const char *lpApplicationName, const char *pchCommandLine, const char *pchCurrentDirectory );
PLATFORM_INTERFACE uint32 ThreadShellExecuteEx( const char *pchVerb, const char *pchFile, const char *pchParameters, const char *pchCurrentDirectory );

//-----------------------------------------------------------------------------
// Access to VTune thread profiling
//-----------------------------------------------------------------------------
#if defined(_WIN32) && defined(THREAD_PROFILER)
PLATFORM_INTERFACE void ThreadNotifySyncPrepare(void *p);
PLATFORM_INTERFACE void ThreadNotifySyncCancel(void *p);
PLATFORM_INTERFACE void ThreadNotifySyncAcquired(void *p);
PLATFORM_INTERFACE void ThreadNotifySyncReleasing(void *p);
#else
#define ThreadNotifySyncPrepare(p)		((void)0)
#define ThreadNotifySyncCancel(p)		((void)0)
#define ThreadNotifySyncAcquired(p)		((void)0)
#define ThreadNotifySyncReleasing(p)	((void)0)
#endif

//-----------------------------------------------------------------------------
// Encapsulation of a thread local datum (needed because THREAD_LOCAL doesn't
// work in a DLL loaded with LoadLibrary()
//-----------------------------------------------------------------------------

#ifndef NO_THREAD_LOCAL

#ifdef _LINUX
// linux totally supports compiler thread locals, even across dll's.
#define PLAT_COMPILER_SUPPORTED_THREADLOCALS 1
#define CTHREADLOCALINTEGER( typ ) __thread int
#define CTHREADLOCALINT __thread int
#define CTHREADLOCALPTR( typ ) __thread typ *
#define CTHREADLOCAL( typ ) __thread typ
#define GETLOCAL( x ) ( x )
#endif

#if defined( _WIN32 ) || defined( OSX )
#ifndef __AFXTLS_H__ // not compatible with some Windows headers

#define CTHREADLOCALINT GenericThreadLocals::CThreadLocalInt<int>
#define CTHREADLOCALINTEGER( typ ) GenericThreadLocals::CThreadLocalInt<typ>
#define CTHREADLOCALPTR( typ ) GenericThreadLocals::CThreadLocalPtr<typ>
#define CTHREADLOCAL( typ ) GenericThreadLocals::CThreadLocal<typ>
#define GETLOCAL( x ) ( x.Get() )


namespace GenericThreadLocals
{
	// a (not so efficient) implementation of thread locals for compilers without full support (i.e. visual c).
	// don't use this explicity - instead, use the CTHREADxxx macros above.

	class PLATFORM_CLASS CThreadLocalBase
	{
public:
		CThreadLocalBase();
		~CThreadLocalBase();

		void * Get() const;
		void   Set(void *);

private:
#if defined(POSIX)
		pthread_key_t m_index;
#else
		uint32 m_index;
#endif
	};

	//---------------------------------------------------------

	template <class T>
	class CThreadLocal : public CThreadLocalBase
	{
	public:
		CThreadLocal()
		{
#ifdef PLATFORM_64BITS
			COMPILE_TIME_ASSERT( sizeof(T) <= sizeof(void *) );
#else
			COMPILE_TIME_ASSERT( sizeof(T) == sizeof(void *) );
#endif
		}

		void operator=( T i ) { Set( i ); }

		T Get() const
		{
#ifdef PLATFORM_64BITS
			void *pData = CThreadLocalBase::Get();
			return *reinterpret_cast<T*>( &pData );
#else
	#ifdef COMPILER_MSVC
		#pragma warning ( disable : 4311 )
	#endif
			return reinterpret_cast<T>( CThreadLocalBase::Get() );
	#ifdef COMPILER_MSVC
		#pragma warning ( default : 4311 )
	#endif
#endif
		}

		void Set(T val)
		{
#ifdef PLATFORM_64BITS
			void* pData = 0;
			*reinterpret_cast<T*>( &pData ) = val;
			CThreadLocalBase::Set( pData );
#else
	#ifdef COMPILER_MSVC
		#pragma warning ( disable : 4312 )
	#endif
			CThreadLocalBase::Set( reinterpret_cast<void *>(val) );
	#ifdef COMPILER_MSVC
		#pragma warning ( default : 4312 )
	#endif
#endif
		}
	};


	//---------------------------------------------------------

	template <class T = int32>
	class CThreadLocalInt : public CThreadLocal<T>
	{
	public:
		operator const T() const { return this->Get(); }
		int	operator=( T i ) { Set( i ); return i; }

		T operator++()					{ T i = this->Get(); this->Set( ++i ); return i; }
		T operator++(int)				{ T i = this->Get(); this->Set( i + 1 ); return i; }

		T operator--()					{ T i = this->Get(); this->Set( --i ); return i; }
		T operator--(int)				{ T i = this->Get(); this->Set( i - 1 ); return i; }

		inline CThreadLocalInt(  ) { }
		inline CThreadLocalInt( const T &initialvalue )
		{
			this->Set( initialvalue );
		}
	};


	//---------------------------------------------------------

	template <class T>
	class CThreadLocalPtr : private CThreadLocalBase
	{
	public:
		CThreadLocalPtr() {}

		operator const void *() const          					{ return (const T *)Get(); }
		operator void *()                      					{ return (T *)Get(); }

		operator const T *() const							    { return (const T *)Get(); }
		operator const T *()          							{ return (const T *)Get(); }
		operator T *()											{ return (T *)Get(); }

		T *			operator=( T *p )							{ Set( p ); return p; }

		bool        operator !() const							{ return (!Get()); }
		bool        operator!=( int i ) const					{ AssertMsg( i == 0, "Only NULL allowed on integer compare" ); return (Get() != NULL); }
		bool        operator==( int i ) const					{ AssertMsg( i == 0, "Only NULL allowed on integer compare" ); return (Get() == NULL); }
		bool		operator==( const void *p ) const			{ return (Get() == p); }
		bool		operator!=( const void *p ) const			{ return (Get() != p); }
		bool		operator==( const T *p ) const				{ return operator==((const void*)p); }
		bool		operator!=( const T *p ) const				{ return operator!=((const void*)p); }

		T *  		operator->()								{ return (T *)Get(); }
		T &  		operator *()								{ return *((T *)Get()); }

		const T *   operator->() const							{ return (const T *)Get(); }
		const T &   operator *() const							{ return *((const T *)Get()); }

		const T &	operator[]( int i ) const					{ return *((const T *)Get() + i); }
		T &			operator[]( int i )							{ return *((T *)Get() + i); }

	private:
		// Disallowed operations
		CThreadLocalPtr( T *pFrom );
		CThreadLocalPtr( const CThreadLocalPtr<T> &from );
		T **operator &();
		T * const *operator &() const;
		void operator=( const CThreadLocalPtr<T> &from );
		bool operator==( const CThreadLocalPtr<T> &p ) const;
		bool operator!=( const CThreadLocalPtr<T> &p ) const;
	};
}

#ifdef _OSX
PLATFORM_INTERFACE GenericThreadLocals::CThreadLocalInt<int> g_nThreadID;
#else // _OSX
#ifndef TIER0_DLL_EXPORT
__declspec( dllimport ) GenericThreadLocals::CThreadLocalInt<int> g_nThreadID;
#endif
#endif // _OSX

#endif /// afx32
#endif //__win32

#endif // NO_THREAD_LOCAL

//-----------------------------------------------------------------------------
//
// Platform independent for critical sections management
//
//-----------------------------------------------------------------------------

class CAtomicMutex
{
public:
	enum class State
	{
		NONE = 0,
		Acquiring = (1 << 0),
		Acquired = (1 << 1),

		MASK = (1 << 2) - 1
	};

	CAtomicMutex( uint16 spin_iters = 200 ) :
		m_LockCount( 0 ),
		m_AcquireSpinIterations( spin_iters ),
		m_CurrentOwnerID( 0 )
	{}

	//------------------------------------------------------
	// Mutex acquisition/release. Const intentionally defeated.
	//------------------------------------------------------
	void Lock( const char *pFileName = NULL, int nLine = -1 );
	void Lock( const char *pFileName = NULL, int nLine = -1 ) const { (const_cast<CAtomicMutex *>(this))->Lock( pFileName, nLine ); }
	void Unlock( const char *pFileName = NULL, int nLine = -1 );
	void Unlock( const char *pFileName = NULL, int nLine = -1 ) const { (const_cast<CAtomicMutex *>(this))->Unlock( pFileName, nLine ); }

	bool AssertOwnedByCurrentThread();
	void SetTrace( bool ) {}

private:
	DLL_CLASS_IMPORT void AcquireLock( ThreadId_t threadid );

private:
	// Uses CAtomicMutex::State enum
	CInterlockedUInt m_State;
	uint16		m_LockCount;
	uint16		m_AcquireSpinIterations;
	ThreadId_t	m_CurrentOwnerID;
};

typedef CAtomicMutex CThreadSpinMutex;
typedef CThreadSpinMutex CThreadFastMutex;

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

class CThreadNullMutex
{
public:
	CThreadNullMutex( const char* pDebugName = NULL ) {}

	static void Lock( const char *pFileName = NULL, int nLine = -1 )	{}
	static void Unlock( const char *pFileName = NULL, int nLine = -1 )	{}

	static bool TryLock( const char *pFileName = NULL, int nLine = -1 )	{ return true; }
	static bool AssertOwnedByCurrentThread()							{ return true; }
	static void SetTrace( bool b )	{}

	static ThreadId_t GetOwnerId()	{ return 0;	}
	static int	GetDepth()			{ return 0; }
};

//-----------------------------------------------------------------------------
//
// A mutex decorator class used to control the use of a mutex, to make it
// less expensive when not multithreading
//
//-----------------------------------------------------------------------------

template <class BaseClass, bool *pCondition>
class CThreadConditionalMutex : public BaseClass
{
public:
	CThreadConditionalMutex( const char* pDebugName = NULL ) : BaseClass( pDebugName ) {}

	void Lock( const char *pFileName = NULL, int nLine = -1 )			{ if ( *pCondition ) BaseClass::Lock( pFileName, nLine ); }
	void Lock( const char *pFileName = NULL, int nLine = -1 ) const		{ if ( *pCondition ) BaseClass::Lock( pFileName, nLine ); }
	void Unlock( const char *pFileName = NULL, int nLine = -1 )			{ if ( *pCondition ) BaseClass::Unlock( pFileName, nLine ); }
	void Unlock( const char *pFileName = NULL, int nLine = -1 ) const	{ if ( *pCondition ) BaseClass::Unlock( pFileName, nLine ); }

	bool TryLock( const char *pFileName = NULL, int nLine = -1 )		{ if ( *pCondition ) return BaseClass::TryLock( pFileName, nLine ); else return true; }
	bool TryLock( const char *pFileName = NULL, int nLine = -1 ) const 	{ if ( *pCondition ) return BaseClass::TryLock( pFileName, nLine ); else return true; }
	bool AssertOwnedByCurrentThread()									{ if ( *pCondition ) return BaseClass::AssertOwnedByCurrentThread(); else return true; }
	void SetTrace( bool b )												{ if ( *pCondition ) BaseClass::SetTrace( b ); }
};

//-----------------------------------------------------------------------------
// Mutex decorator that blows up if another thread enters
//-----------------------------------------------------------------------------

template <class BaseClass>
class CThreadTerminalMutex : public BaseClass
{
public:
	CThreadTerminalMutex( const char* pDebugName = NULL ) : BaseClass( pDebugName ) {}

	bool TryLock( const char *pFileName = NULL, int nLine = -1 )		{ if ( !BaseClass::TryLock( pFileName, nLine ) ) { DebuggerBreak(); return false; } return true; }
	bool TryLock( const char *pFileName = NULL, int nLine = -1 ) const 	{ if ( !BaseClass::TryLock( pFileName, nLine ) ) { DebuggerBreak(); return false; } return true; }
	void Lock( const char *pFileName = NULL, int nLine = -1 )			{ if ( !TryLock( pFileName, nLine ) ) BaseClass::Lock( pFileName, nLine ); }
	void Lock( const char *pFileName = NULL, int nLine = -1 ) const		{ if ( !TryLock( pFileName, nLine ) ) BaseClass::Lock( pFileName, nLine ); }

};

//-----------------------------------------------------------------------------
//
// Class to Lock a critical section, and unlock it automatically
// when the lock goes out of scope
//
//-----------------------------------------------------------------------------

template <class MUTEX_TYPE = CAtomicMutex>
class CAutoLockT
{
public:
	FORCEINLINE CAutoLockT( MUTEX_TYPE &lock, const char *pFileName = nullptr, int nLine = 0 )
		: m_lock(lock)
	{
		m_lock.Lock( pFileName, nLine );
	}

	FORCEINLINE CAutoLockT( const MUTEX_TYPE &lock, const char *pFileName = nullptr, int nLine = 0 )
		: m_lock(const_cast<MUTEX_TYPE &>(lock))
	{
		m_lock.Lock( pFileName, nLine );
	}

	FORCEINLINE ~CAutoLockT()
	{
		m_lock.Unlock();
	}


private:
	MUTEX_TYPE &m_lock;

	// Disallow copying
	CAutoLockT( const CAutoLockT<MUTEX_TYPE> & );
	CAutoLockT &operator=( const CAutoLockT<MUTEX_TYPE> & );
};

typedef CAutoLockT<CAtomicMutex> CAutoLock;

//---------------------------------------------------------

#define AUTO_LOCK_( type, mutex ) \
	CAutoLockT< type > UNIQUE_ID( static_cast<const type &>( mutex ), __FILE__, __LINE__ )

template<typename T> T strip_cv_quals_for_mutex(T&);
template<typename T> T strip_cv_quals_for_mutex(const T&);
template<typename T> T strip_cv_quals_for_mutex(volatile T&);
template<typename T> T strip_cv_quals_for_mutex(const volatile T&);

#define AUTO_LOCK( mutex ) \
    AUTO_LOCK_( decltype(::strip_cv_quals_for_mutex(mutex)), mutex )

#define AUTO_LOCK_FM( mutex ) \
	AUTO_LOCK_( CThreadFastMutex, mutex )

#define LOCAL_THREAD_LOCK_( tag ) \
	; \
	static CThreadFastMutex autoMutex_##tag; \
	AUTO_LOCK( autoMutex_##tag )

#define LOCAL_THREAD_LOCK() \
	LOCAL_THREAD_LOCK_(_)

//-----------------------------------------------------------------------------
//
// Base class for event, semaphore and mutex objects.
//
//-----------------------------------------------------------------------------

#define TW_TIMEOUT	0xFFFF
#define TW_FAILED	0xFFFE

class PLATFORM_CLASS CThreadSyncObject
{
public:
	~CThreadSyncObject();

	//-----------------------------------------------------
	// Query if object is useful
	//-----------------------------------------------------
	bool operator!() const;

	//-----------------------------------------------------
	// Access handle
	//-----------------------------------------------------
#ifdef _WIN32
	operator HANDLE() { return GetHandle(); }
	const HANDLE GetHandle() const { return m_hSyncObject; }
#endif
	//-----------------------------------------------------
	// Wait for a signal from the object
	//-----------------------------------------------------
	bool Wait( uint32 dwTimeout = TT_INFINITE );

	//-----------------------------------------------------
	// Wait for a signal from any of the specified objects.
	//
	// Returns the index of the object that signaled the event
	// or THREADSYNC_TIMEOUT if the timeout was hit before the wait condition was met.
	//
	// Returns TW_FAILED if an incoming object is invalid.
	//
	// If bWaitAll=true, then it'll return 0 if all the objects were set.
	//-----------------------------------------------------
	static uint32 WaitForMultiple( int nObjects, CThreadSyncObject **ppObjects, bool bWaitAll, uint32 dwTimeout = TT_INFINITE );
	
	// This builds a list of pointers and calls straight through to the other WaitForMultiple.
	static uint32 WaitForMultiple( int nObjects, CThreadSyncObject *ppObjects, bool bWaitAll, uint32 dwTimeout = TT_INFINITE );

protected:
	CThreadSyncObject();
	void AssertUseable();

#ifdef PLATFORM_POSIX
	virtual bool WaitImpl( uint32 dwTimeout );
#endif

#ifdef _WIN32
	HANDLE m_hSyncObject;
	bool m_bCreatedHandle;
#elif defined(POSIX)
	char *m_pszSemName;
	int m_pSemaphore;
	bool m_bSemOwner;
	bool m_bSemAutoUndo;
	pthread_mutex_t	m_Mutex;
	pthread_cond_t	m_Condition;
	bool m_bInitalized;
	bool m_bManualReset;
	CInterlockedInt m_cSet;
	int m_nBroadcastCount;
#else
#error "Implement me"
#endif

private:
	CThreadSyncObject( const CThreadSyncObject & );
	CThreadSyncObject &operator=( const CThreadSyncObject & );
};


//-----------------------------------------------------------------------------
//
// Wrapper for unnamed event objects
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// CThreadSemaphore
//
//-----------------------------------------------------------------------------

class PLATFORM_CLASS CThreadSemaphore : public CThreadSyncObject
{
public:
	// bAllUsers lets other users open a named semaphore on Linux, and does nothing on Windows
	CThreadSemaphore(int32 initialValue, int32 maxValue, const char *pszName = NULL, bool bAllUsers = false);
	~CThreadSemaphore();

	//-----------------------------------------------------
	// Increases the count of the semaphore object by a specified
	// amount.  Wait() decreases the count by one on return.
	//-----------------------------------------------------
	bool Release(int32 releaseCount = 1, int32 * pPreviousCount = NULL );

#ifdef PLATFORM_POSIX
protected:
	bool WaitImpl( uint32 dwTimeout ) override;
#endif

private:
	CThreadSemaphore(const CThreadSemaphore &);
	CThreadSemaphore &operator=(const CThreadSemaphore &);
};

#if defined( _WIN32 )

//-----------------------------------------------------------------------------
//
// A mutex suitable for out-of-process, multi-processor usage
//
//-----------------------------------------------------------------------------

class PLATFORM_CLASS CThreadFullMutex : public CThreadSyncObject
{
public:
	CThreadFullMutex( bool bEstablishInitialOwnership = false, const char * pszName = NULL );

	//-----------------------------------------------------
	// Release ownership of the mutex
	//-----------------------------------------------------
	bool Release();

	// To match regular CAtomicMutex:
	void Lock()							{ Wait(); }
	void Lock( unsigned timeout )		{ Wait( timeout ); }
	void Unlock()						{ Release(); }
	bool AssertOwnedByCurrentThread()	{ return true; }
	void SetTrace( bool )				{}

private:
	CThreadFullMutex( const CThreadFullMutex & );
	CThreadFullMutex &operator=( const CThreadFullMutex & );
};
#endif

enum NamedEventResult_t
{
	TT_EventDoesntExist	= 0,
	TT_EventNotSignaled,
	TT_EventSignaled
};

class PLATFORM_CLASS CThreadEvent : public CThreadSyncObject
{
public:
	CThreadEvent( bool fManualReset = false );
#ifdef PLATFORM_WINDOWS
	CThreadEvent( const char *name, bool initialState = false, bool bManualReset = false );
	static NamedEventResult_t CheckNamedEvent( const char *name, uint32 dwTimeout = 0 );
#endif
#ifdef WIN32
	CThreadEvent( HANDLE hHandle );
#endif
	//-----------------------------------------------------
	// Set the state to signaled
	//-----------------------------------------------------
	bool Set();

	//-----------------------------------------------------
	// Set the state to nonsignaled
	//-----------------------------------------------------
	bool Reset();

	//-----------------------------------------------------
	// Check if the event is signaled
	//-----------------------------------------------------
	bool Check();

	// See CThreadSyncObject for definitions of these functions.
	static uint32 WaitForMultiple( int nObjects, CThreadEvent **ppObjects, bool bWaitAll, uint32 dwTimeout = TT_INFINITE );
	static uint32 WaitForMultiple( int nObjects, CThreadEvent *ppObjects, bool bWaitAll, uint32 dwTimeout = TT_INFINITE );

private:
	CThreadEvent( const CThreadEvent & );
	CThreadEvent &operator=( const CThreadEvent & );
};

// Hard-wired manual event for use in array declarations
class CThreadManualEvent : public CThreadEvent
{
public:
	CThreadManualEvent()
	 :	CThreadEvent( true )
	{
	}
};

class CThreadMultiWaitEvent
{
public:
	DLL_CLASS_IMPORT CThreadMultiWaitEvent( bool bManualReset = false );
	DLL_CLASS_IMPORT ~CThreadMultiWaitEvent();

	DLL_CLASS_IMPORT void Set();
	DLL_CLASS_IMPORT void Reset();

private:
#ifdef _WIN32
	HANDLE m_hEvent;
#else
	int m_nEventFd;
#endif
	int m_nType; // AMNOTE: 2 for auto-reset, 4 for manual-reset
};


//-----------------------------------------------------------------------------
//
// CThreadRWLock
//
//-----------------------------------------------------------------------------

class PLATFORM_CLASS CThreadRWLock
{
public:
	CThreadRWLock();

	void LockForRead();
	void UnlockRead();
	void LockForWrite();
	void UnlockWrite();

	void LockForRead() const { const_cast<CThreadRWLock *>(this)->LockForRead(); }
	void UnlockRead() const { const_cast<CThreadRWLock *>(this)->UnlockRead(); }
	void LockForWrite() const { const_cast<CThreadRWLock *>(this)->LockForWrite(); }
	void UnlockWrite() const { const_cast<CThreadRWLock *>(this)->UnlockWrite(); }

private:
	void WaitForRead();

	CThreadFastMutex m_mutex;
	CThreadEvent m_CanWrite;
	CThreadEvent m_CanRead;

	int m_nWriters;
	int m_nActiveReaders;
	int m_nPendingReaders;
};

//-----------------------------------------------------------------------------
//
// CThreadRWLock_FastRead
//
//-----------------------------------------------------------------------------

enum RWLockTranstionResult_t
{
	// Another writer had the lock in between
	RWLTR_STATE_INVALIDATED = 0,
	RWLTR_STATE_REMAINED_VALID = 1,
};

// AMNOTE: Incomplete, tier0 doesn't export LockForRead, UnlockRead and LockForWrite
class CThreadRWLock_FastRead
{
public:
	enum WriteLockTransition_t
	{
		// AMNOTE: Blocks reads with HaveWriteLock_BlockReadsNow( true )
		TRANSITION_UNK001,
		// AMNOTE: Blocks reads with HaveWriteLock_BlockReadsNow( false )
		TRANSITION_UNK002,
		// AMNOTE: Doesn't block reads
		TRANSITION_UNK003
	};

	DLL_CLASS_IMPORT CThreadRWLock_FastRead();

	DLL_CLASS_IMPORT void UnlockWrite( const char *pFileName = NULL, int nLine = -1 );
	DLL_CLASS_IMPORT RWLockTranstionResult_t UnlockRead_LockForWrite( const char *pFileName, int nLine, WriteLockTransition_t transition );
	DLL_CLASS_IMPORT RWLockTranstionResult_t UnlockWrite_LockForRead( const char *pFileName = NULL, int nLine = -1 );
	DLL_CLASS_IMPORT bool TryUnlockRead_LockForWrite( const char *pFileName, int nLine, bool bForce, WriteLockTransition_t transition );
	DLL_CLASS_IMPORT bool TryUnlockWrite_LockForRead( const char *pFileName = NULL, int nLine = -1 );
	DLL_CLASS_IMPORT void HaveWriteLock_BlockReadsNow( bool bBlock );
	DLL_CLASS_IMPORT void HaveWriteLock_UnblockReads();

protected:
	DLL_CLASS_IMPORT bool EncounteredComplexReadLockOperation( uint32 nInterlock, bool bWait, const char *pFileName, int nLine );
	DLL_CLASS_IMPORT void UnlockReadAccounting( const char *pFileName, int nLine, uint32 nInterlock );

private:
	CInterlockedUInt m_Interlock;
	CInterlockedInt m_ReadlockAccounting;
	CThreadSemaphore m_WriteLockQueue;
	CThreadEvent m_WriteLockUnblocked;
	CThreadEvent m_ReadLockUnblocked;

	ThreadId_t m_WritingThreadID;
	int m_nWritingThreadLockCount;
};

//-----------------------------------------------------------------------------
//
// CThreadSpinRWLock
//
//-----------------------------------------------------------------------------

class CThreadSpinRWLock
{
public:
	bool TryLockForWrite( const char *pFileName = NULL, int nLine = -1 ) { return m_mutex.try_lock(); }
	bool TryLockForRead( const char *pFileName = NULL, int nLine = -1 ) { return m_mutex.try_lock_shared(); }

	void LockForRead( const char *pFileName = NULL, int nLine = -1 ) { m_mutex.lock_shared(); }
	void UnlockRead( const char *pFileName = NULL, int nLine = -1 ) { m_mutex.unlock_shared(); }
	void LockForWrite( const char *pFileName = NULL, int nLine = -1 ) { m_mutex.lock(); }
	void UnlockWrite( const char *pFileName = NULL, int nLine = -1 ) { m_mutex.unlock(); }

	bool TryLockForWrite( const char *pFileName = NULL, int nLine = -1 ) const { return const_cast<CThreadSpinRWLock *>(this)->TryLockForWrite( pFileName, nLine ); }
	bool TryLockForRead( const char *pFileName = NULL, int nLine = -1 ) const { return const_cast<CThreadSpinRWLock *>(this)->TryLockForRead( pFileName, nLine ); }

	void LockForRead( const char *pFileName = NULL, int nLine = -1 ) const { const_cast<CThreadSpinRWLock *>(this)->LockForRead( pFileName, nLine ); }
	void UnlockRead( const char *pFileName = NULL, int nLine = -1 ) const { const_cast<CThreadSpinRWLock *>(this)->UnlockRead( pFileName, nLine ); }
	void LockForWrite( const char *pFileName = NULL, int nLine = -1 ) const { const_cast<CThreadSpinRWLock *>(this)->LockForWrite( pFileName, nLine ); }
	void UnlockWrite( const char *pFileName = NULL, int nLine = -1 ) const { const_cast<CThreadSpinRWLock *>(this)->UnlockWrite( pFileName, nLine ); }

private:
	std::shared_mutex m_mutex;
};

//-----------------------------------------------------------------------------
//
// A thread wrapper similar to a Java thread.
//
//-----------------------------------------------------------------------------

class PLATFORM_CLASS CThread
{
public:
	CThread();
	virtual ~CThread();

	//-----------------------------------------------------

	const char *GetName();
	void SetName( const char * );

	size_t CalcStackDepth( void *pStackVariable )		{ return ((byte *)m_pStackBase - (byte *)pStackVariable); }

	//-----------------------------------------------------
	// Functions for the other threads
	//-----------------------------------------------------

	// Start thread running  - error if already running
	virtual bool Start( unsigned nBytesStack = 0 );

	// Returns true if thread has been created and hasn't yet exited
	bool IsAlive();

	// This method causes the current thread to wait until this thread
	// is no longer alive.
	bool Join( unsigned timeout = TT_INFINITE );

	// Access the thread handle directly
	ThreadHandle_t GetThreadHandle();

#ifdef _WIN32
	uint GetThreadId();
#endif

	//-----------------------------------------------------

	int GetResult();

	//-----------------------------------------------------
	// Functions for both this, and maybe, and other threads
	//-----------------------------------------------------

	// Forcibly, abnormally, but relatively cleanly stop the thread
	void Stop( int exitCode = 0 );

	// Get the priority
	int GetPriority() const;

	// Set the priority
	bool SetPriority( int );

	// Suspend a thread
	unsigned Suspend();

	// Resume a suspended thread
	unsigned Resume();

	// Force hard-termination of thread.  Used for critical failures.
	bool Terminate( int exitCode = 0 );

	//-----------------------------------------------------
	// Global methods
	//-----------------------------------------------------

	// Get the Thread object that represents the current thread, if any.
	// Can return NULL if the current thread was not created using
	// CThread
	static CThread *GetCurrentCThread();

	// Offer a context switch. Under Win32, equivalent to Sleep(0)
#ifdef Yield
#undef Yield
#endif
	static void Yield();

	// This method causes the current thread to yield and not to be
	// scheduled for further execution until a certain amount of real
	// time has elapsed, more or less. Duration is in milliseconds
	static void Sleep( unsigned duration );

protected:

	// Optional pre-run call, with ability to fail-create. Note Init()
	// is forced synchronous with Start()
	virtual bool Init();

	// Thread will run this function on startup, must be supplied by
	// derived class, performs the intended action of the thread.
	virtual int Run() = 0;

	// Called when the thread exits
	virtual void OnExit();

#ifdef _WIN32
	// Allow for custom start waiting
	virtual bool WaitForCreateComplete( CThreadEvent *pEvent );
#endif

	CAtomicMutex m_Lock;
	CThreadEvent m_ExitEvent;	// Set right before the thread's function exits.

private:
	enum Flags
	{
		SUPPORT_STOP_PROTOCOL = 1 << 0
	};

	// Thread initially runs this. param is actually 'this'. function
	// just gets this and calls ThreadProc
	struct ThreadInit_t
	{
		CThread *     pThread;
#ifdef _WIN32
		CThreadEvent *pInitCompleteEvent;
#endif
		bool *        pfInitSuccess;
	};

#ifdef PLATFORM_WINDOWS
	static unsigned long __stdcall ThreadProc( void * pv );
#else
	static void* ThreadProc( void * pv );
#endif

	// make copy constructor and assignment operator inaccessible
	CThread( const CThread & );
	CThread &operator=( const CThread & );

#ifdef _WIN32
	HANDLE 	m_hThread;
#elif defined(_POSIX)
	pthread_t m_threadId;
	CInterlockedInt m_nSuspendCount;
#endif
	int		m_result;
	char	m_szName[32];
	void *	m_pStackBase;
	unsigned m_flags;
};

//-----------------------------------------------------------------------------
// Simple thread class encompasses the notion of a worker thread, handing
// synchronized communication.
//-----------------------------------------------------------------------------

// These are internal reserved error results from a call attempt
enum WTCallResult_t
{
	WTCR_FAIL			= -1,
	WTCR_TIMEOUT		= -2,
	WTCR_THREAD_GONE	= -3,
};

class PLATFORM_CLASS CWorkerThread : public CThread
{
public:
	CWorkerThread();

	//-----------------------------------------------------
	//
	// Inter-thread communication
	//
	// Calls in either direction take place on the same "channel."
	// Seperate functions are specified to make identities obvious
	//
	//-----------------------------------------------------

	// Master: Signal the thread, and block for a response
	int CallWorker( unsigned, unsigned timeout = TT_INFINITE, bool fBoostWorkerPriorityToMaster = true );

	// Worker: Signal the thread, and block for a response
	int CallMaster( unsigned, unsigned timeout = TT_INFINITE );

	// Wait for the next request
	bool WaitForCall( unsigned dwTimeout, unsigned *pResult = NULL );
	bool WaitForCall( unsigned *pResult = NULL );

	// Is there a request?
	bool PeekCall( unsigned *pParam = NULL );

	// Reply to the request
	void Reply( unsigned );

	// Wait for a reply in the case when CallWorker() with timeout != TT_INFINITE
	int WaitForReply( unsigned timeout = TT_INFINITE );

	// If you want to do WaitForMultipleObjects you'll need to include
	// this handle in your wait list or you won't be responsive
	CThreadEvent& GetCallHandle();	// (returns m_EventSend)

	// Find out what the request was
	unsigned GetCallParam() const;

	// Boost the worker thread to the master thread, if worker thread is lesser, return old priority
	int BoostPriority();

protected:
	int Call( unsigned, unsigned timeout, bool fBoost );

private:
	CWorkerThread( const CWorkerThread & );
	CWorkerThread &operator=( const CWorkerThread & );

	CThreadEvent	m_EventSend;
	CThreadEvent	m_EventComplete;

	unsigned        m_Param;
	int				m_ReturnVal;
};


// a unidirectional message queue. A queue of type T. Not especially high speed since each message
// is malloced/freed. Note that if your message class has destructors/constructors, they MUST be
// thread safe!
template<class T> class CMessageQueue
{
	CThreadEvent SignalEvent;								// signals presence of data
	CAtomicMutex QueueAccessMutex;

	// the parts protected by the mutex
	struct MsgNode
	{
		MsgNode *Next;
		T Data;
	};

	MsgNode *Head;
	MsgNode *Tail;

public:
	CMessageQueue( void )
	{
		Head = Tail = NULL;
	}

	// check for a message. not 100% reliable - someone could grab the message first
	bool MessageWaiting( void ) 
	{
		return ( Head != NULL );
	}

	void WaitMessage( T *pMsg )
	{
		for(;;)
		{
			while( ! MessageWaiting() )
				SignalEvent.Wait();
			QueueAccessMutex.Lock( __FILE__, __LINE__ );
			if (! Head )
			{
				// multiple readers could make this null
				QueueAccessMutex.Unlock( __FILE__, __LINE__ );
				continue;
			}
			*( pMsg ) = Head->Data;
			MsgNode *remove_this = Head;
			Head = Head->Next;
			if (! Head)										// if empty, fix tail ptr
				Tail = NULL;
			QueueAccessMutex.Unlock( __FILE__, __LINE__ );
			delete remove_this;
			break;
		}
	}

	void QueueMessage( T const &Msg)
	{
		MsgNode *new1=new MsgNode;
		new1->Data=Msg;
		new1->Next=NULL;
		QueueAccessMutex.Lock( __FILE__, __LINE__ );
		if ( Tail )
		{
			Tail->Next=new1;
			Tail = new1;
		}
		else
		{
			Head = new1;
			Tail = new1;
		}
		SignalEvent.Set();
		QueueAccessMutex.Unlock( __FILE__, __LINE__ );
	}
};


//-----------------------------------------------------------------------------
//
// CAtomicMutex. Inlining to reduce overhead and to allow client code
// to decide debug status (tracing)
//
//-----------------------------------------------------------------------------

inline void CAtomicMutex::Lock( const char *pFileName, int nLine )
{
	ThreadId_t thisThreadID = ThreadGetCurrentId();

	if(m_CurrentOwnerID == thisThreadID)
	{
		m_LockCount++;
	}
	else if(m_State || m_State.CompareExchange( (uint32)State::MASK, 0 ))
	{
		AcquireLock( thisThreadID );
	}
	else
	{
		m_LockCount = 1;
		m_CurrentOwnerID = thisThreadID;
	}
}

//---------------------------------------------------------

inline void CAtomicMutex::Unlock( const char *pFileName, int nLine )
{
	m_LockCount--;
	if(m_LockCount == 0)
	{
		m_CurrentOwnerID = 0;
		if(m_State.AtomicAdd(0u - (uint32)State::MASK) != (uint32)State::MASK)
			ThreadAtomicNotifyOne( (uint32 *)&m_State );
	}
}

//---------------------------------------------------------

inline bool CAtomicMutex::AssertOwnedByCurrentThread()
{
	return ThreadGetCurrentId() == m_CurrentOwnerID;
}

//-----------------------------------------------------------------------------
//
// CThreadRWLock inline functions
//
//-----------------------------------------------------------------------------

inline CThreadRWLock::CThreadRWLock()
:	m_CanRead( true ),
	m_nWriters( 0 ),
	m_nActiveReaders( 0 ),
	m_nPendingReaders( 0 )
{
}

inline void CThreadRWLock::LockForRead()
{
	m_mutex.Lock( __FILE__, __LINE__ );
	if ( m_nWriters)
	{
		WaitForRead();
	}
	m_nActiveReaders++;
	m_mutex.Unlock( __FILE__, __LINE__ );
}

inline void CThreadRWLock::UnlockRead()
{
	m_mutex.Lock( __FILE__, __LINE__ );
	m_nActiveReaders--;
	if ( m_nActiveReaders == 0 && m_nWriters != 0 )
	{
		m_CanWrite.Set();
	}
	m_mutex.Unlock( __FILE__, __LINE__ );
}

// read data from a memory address
template<class T> FORCEINLINE T ReadVolatileMemory( T const *pPtr )
{
	volatile const T * pVolatilePtr = ( volatile const T * ) pPtr;
	return *pVolatilePtr;
}

//-----------------------------------------------------------------------------

#ifdef _LINUX
DLL_GLOBAL_IMPORT __thread int g_nThreadID;
#endif

#if defined( _WIN32 )
#pragma warning(pop)
#endif

#endif // THREADTOOLS_H
