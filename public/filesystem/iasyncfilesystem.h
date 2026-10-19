#ifndef IASYNCFILESYSTEM_H
#define IASYNCFILESYSTEM_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"
#include "tier0/buffer_string.h"
#include "appframework/iappsystem.h"

#include <functional>
#include <memory>

class IMemoryMappedFile;
class IAsyncReadRequest2;
class IAsyncWriteRequest2;

enum AsyncRequestPriority_t
{
	ASYNC_REQUEST_PRIORITY_INVALID = -1,
	ASYNC_REQUEST_PRIORITY_LOW = 0,
	ASYNC_REQUEST_PRIORITY_MEDIUM,
	ASYNC_REQUEST_PRIORITY_HIGH,
	ASYNC_REQUEST_PRIORITY_IMMEDIATE,

	ASYNC_REQUEST_PRIORITY_COUNT,
	ASYNC_REQUEST_PRIORITY_DEFAULT = ASYNC_REQUEST_PRIORITY_MEDIUM,
};

enum AsyncRequestStatus_t
{
	ASYNC_REQUEST_ERROR_BADREQUEST = -7,
	ASYNC_REQUEST_ERROR_READING,
	ASYNC_REQUEST_ERROR_NOMEMORY,
	ASYNC_REQUEST_ERROR_FILEOPEN,
	ASYNC_REQUEST_STATUS_ABORTED,
	ASYNC_REQUEST_STATUS_PENDING,
	ASYNC_REQUEST_STATUS_UNDEFINED,
	ASYNC_REQUEST_OK,
};

enum AsyncFSProfileEvent_t
{
	AFS_PROFILE_EVENT_INVALID = -1,
	AFS_PROFILE_EVENT_SUBMITTED,
	AFS_PROFILE_EVENT_SERVICE_BEGAN,
	AFS_PROFILE_EVENT_DATA_ARRIVED,

	AFS_PROFILE_EVENT_COUNT,
};

//-----------------------------------------------------------------------------
// Holds the results of the requests that were built with it, until ProcessResults()
// runs their callbacks on the calling thread
//-----------------------------------------------------------------------------
abstract_class IIOCompletionQueue
{
public:
	virtual ~IIOCompletionQueue() {}

	// Runs the callbacks of up to nLimit results, skips the requests that were already freed
	virtual void ProcessResults( int nLimit ) = 0;
};

//-----------------------------------------------------------------------------
// A read request, made by IAsyncFileSystem2::IReadRequestBuilder::Build()
//-----------------------------------------------------------------------------
abstract_class IAsyncReadRequest2
{
protected:
	IAsyncReadRequest2( const char *pFileName, const char *pPathID ) : m_fileName( pFileName ), m_szPathID( pPathID ) {}

public:
	virtual ~IAsyncReadRequest2() {}

	const char *GetFileName() const { return m_fileName.Get(); }
	const char *GetPathId() const { return m_szPathID; }

	// Valid once the request completed, the buffer belongs to the request unless KeepResultBuffer() was called
	virtual void *GetResultBuffer() const = 0;
	virtual size_t GetResultSize() const = 0;
	// Takes over the result buffer, free it with IAsyncFileSystem2::ReleaseBuffer() when done.
	// pOutMemoryMap is not written to
	virtual void KeepResultBuffer( IMemoryMappedFile **pOutMemoryMap ) = 0;

	virtual void Submit() = 0;
	virtual bool IsComplete() const = 0;
	virtual void Cancel( bool bRunCallback ) = 0;
	virtual bool WasCanceled() const = 0;

	virtual AsyncRequestPriority_t GetPriority() const = 0;
	virtual bool SetPriority( AsyncRequestPriority_t priority ) = 0;

	// Profiling isn't available, this zeroes 5 uint64s of pOutEventTimesArrayUS whatever nBufferSizeBytes is, and returns false
	virtual bool GetProfilingInfo( uint64 *pOutEventTimesArrayUS, int nBufferSizeBytes ) = 0;

protected:
	const CPathBufferString m_fileName;
	const char *const m_szPathID;
};

//-----------------------------------------------------------------------------
// A write request, made by IAsyncFileSystem2::IWriteRequestBuilder::Build()
//-----------------------------------------------------------------------------
abstract_class IAsyncWriteRequest2
{
protected:
	IAsyncWriteRequest2( const char *pFileName, const char *pPathID ) : m_fileName( pFileName ), m_szPathID( pPathID ) {}

public:
	virtual ~IAsyncWriteRequest2() {}

	const char *GetFileName() const { return m_fileName.Get(); }
	const char *GetPathId() const { return m_szPathID; }

	virtual void Submit() = 0;
	virtual bool IsComplete() const = 0;
	virtual AsyncRequestStatus_t GetStatus() const = 0;

	virtual AsyncRequestPriority_t GetPriority() const = 0;
	virtual bool SetPriority( AsyncRequestPriority_t priority ) = 0;

	// Profiling isn't available, this zeroes the first uint64 of pOutEventTimesArrayUS and returns false
	virtual bool GetProfilingInfo( uint64 *pOutEventTimesArrayUS, int nBufferSizeBytes ) = 0;

protected:
	const CPathBufferString m_fileName;
	const char *const m_szPathID;
};

//-----------------------------------------------------------------------------
// File IO on the async IO threads. Usage:
//
//	auto pRequest = g_pAsyncFileSystem->NewReadRequest( "cfg/server.cfg", "GAME" )
//		->SetTextMode( true )
//		->SetPriority( ASYNC_REQUEST_PRIORITY_HIGH )
//		->SetCallback( []( IAsyncReadRequest2 *pRequest, AsyncRequestStatus_t status ) { ... } )
//		->Build();
//	pRequest->Submit();
//
// Callbacks run on an IO thread, unless a completion queue was set.
//-----------------------------------------------------------------------------
abstract_class IAsyncFileSystem2 : public IAppSystem
{
public:
	typedef std::function<void( IAsyncReadRequest2 *pRequest, AsyncRequestStatus_t status )> ReadCallback_t;
	typedef std::function<void( IAsyncWriteRequest2 *pRequest, AsyncRequestStatus_t status )> WriteCallback_t;

	class IReadRequestBuilder
	{
	public:
		typedef IReadRequestBuilder This;

		virtual ~IReadRequestBuilder() {}

		// 0 reads to the end of the file
		This *SetReadBytes( size_t nReadBytes ) { m_nReadBytes = nReadBytes; return this; }
		size_t GetReadBytes() const { return m_nReadBytes; }
		// 0 or less reads from the start of the file
		This *SetReadOffset( int64 nReadOffset ) { m_nReadOffset = nReadOffset; return this; }
		int64 GetReadOffset() const { return m_nReadOffset; }

		// AMNOTE: The filesystem ignores these
		This *SetAllowMemoryMap( bool bAllowMemoryMap ) { m_bAllowMemoryMap = bAllowMemoryMap; return this; }
		bool GetAllowMemoryMap() const { return m_bAllowMemoryMap; }
		This *SetPreferMemoryMap( bool bPreferMemoryMap ) { m_bPreferMemoryMap = bPreferMemoryMap; return this; }
		bool GetPreferMemoryMap() const { return m_bPreferMemoryMap; }

		This *SetPriority( AsyncRequestPriority_t priority ) { m_priority = priority; return this; }
		AsyncRequestPriority_t GetPriority() const { return m_priority; }
		// Queues the request ahead of the queued requests of its priority
		This *SetMoveToFrontOfQueue( bool bFrontOfQueue ) { m_bFrontOfQueue = bFrontOfQueue; return this; }
		bool GetMoveToFrontOfQueue() const { return m_bFrontOfQueue; }

		// Opens the file with "r" rather than "rb"
		This *SetTextMode( bool bTextMode ) { m_bTextMode = bTextMode; return this; }
		bool GetTextMode() const { return m_bTextMode; }

		// AMNOTE: The filesystem ignores it, GetProfilingInfo() always fails
		This *SetProfilingForRequest( bool bProfile ) { m_bProfile = bProfile; return this; }
		bool GetProfilingForRequest() const { return m_bProfile; }

		// Reads up to nResultBufferSize bytes into pResultBuffer instead of a buffer allocated with AllocateBuffer()
		This *SetResultBuffer( void *pResultBuffer, size_t nResultBufferSize ) { m_pResultBuffer = pResultBuffer; m_nResultBufferSize = nResultBufferSize; return this; }
		void *GetResultBuffer() const { return m_pResultBuffer; }
		size_t GetResultBufferSize() const { return m_nResultBufferSize; }

		// Runs the callback from pCompletionQueue->ProcessResults() instead of the IO thread
		This *SetCompletionQueue( IIOCompletionQueue *pCompletionQueue ) { m_pCompletionQueue = pCompletionQueue; return this; }
		IIOCompletionQueue *GetCompletionQueue() const { return m_pCompletionQueue; }

		// Called once the request is done, holding the lock of the async filesystem when it runs on the IO thread
		This *SetCallback( ReadCallback_t callback ) { m_callback = std::move( callback ); return this; }

		virtual std::shared_ptr<IAsyncReadRequest2> Build() = 0;

	protected:
		IReadRequestBuilder() : m_bProfile( false ), m_bAllowMemoryMap( false ), m_bPreferMemoryMap( false ), m_bTextMode( false ), m_bFrontOfQueue( false ),
			m_nReadOffset( -1 ), m_nReadBytes( 0 ), m_pResultBuffer( NULL ), m_nResultBufferSize( 0 ), m_priority( ASYNC_REQUEST_PRIORITY_DEFAULT ), m_pCompletionQueue( NULL ) {}

		bool m_bProfile;
		bool m_bAllowMemoryMap;
		bool m_bPreferMemoryMap;
		bool m_bTextMode;
		bool m_bFrontOfQueue;
		int64 m_nReadOffset;
		size_t m_nReadBytes;
		void *m_pResultBuffer;
		size_t m_nResultBufferSize;
		AsyncRequestPriority_t m_priority;
		IIOCompletionQueue *m_pCompletionQueue;
		ReadCallback_t m_callback;
	};

	class IWriteRequestBuilder
	{
	public:
		typedef IWriteRequestBuilder This;

		virtual ~IWriteRequestBuilder() {}

		This *SetPriority( AsyncRequestPriority_t priority ) { m_priority = priority; return this; }
		AsyncRequestPriority_t GetPriority() const { return m_priority; }

		// AMNOTE: The filesystem ignores it, GetProfilingInfo() always fails
		This *SetProfilingForRequest( bool bProfile ) { m_bProfile = bProfile; return this; }
		bool GetProfilingForRequest() const { return m_bProfile; }

		// Opens the file with "w" rather than "wb", or "a+" rather than "ab+" when appending
		This *SetTextMode( bool bTextMode ) { m_bTextMode = bTextMode; return this; }
		bool GetTextMode() const { return m_bTextMode; }
		// Opens the file with "ab+" rather than "wb", so the data is written after what the file already has
		This *SetAppend( bool bAppend ) { m_bAppend = bAppend; return this; }
		bool GetAppend() const { return m_bAppend; }

		// Called with the written buffer once the request is done with it
		This *SetFreeBufferCallback( std::function<void( const void *pBuffer )> freeBuffer ) { m_freeBuffer = std::move( freeBuffer ); return this; }
		// Called once the request is done, on the IO thread holding the lock of the async filesystem
		This *SetCallback( WriteCallback_t callback ) { m_callback = std::move( callback ); return this; }

		virtual std::shared_ptr<IAsyncWriteRequest2> Build() = 0;

	protected:
		IWriteRequestBuilder() : m_bProfile( false ), m_bTextMode( false ), m_bAppend( false ), m_priority( ASYNC_REQUEST_PRIORITY_DEFAULT ) {}

		bool m_bProfile;
		bool m_bTextMode;
		bool m_bAppend;
		AsyncRequestPriority_t m_priority;
		WriteCallback_t m_callback;
		std::function<void( const void *pBuffer )> m_freeBuffer;
	};

	// Stops the IO threads after the requests they are on, and restarts them
	virtual void SuspendAsyncIO() = 0;
	virtual void ResumeAsyncIO() = 0;

	// Blocks until every request submitted so far is done
	virtual void FinishAsyncIO() = 0;
	// Like FinishAsyncIO, but only waits for the requests in progress and the ones for the files under pPath,
	// then calls fnWithQueueLocked while holding the request queue lock
	virtual void FinishAsyncIOForPath( const char *pPath, const std::function<void()> &fnWithQueueLocked ) = 0;
	virtual void FinishAsyncWrites() = 0;
	// Drops the queued requests, then waits for the ones in progress
	virtual void AbortAllAsyncIO() = 0;

	virtual void BlockUntilComplete( std::shared_ptr<IAsyncReadRequest2> pRequest ) = 0;
	virtual void BlockUntilComplete( std::shared_ptr<IAsyncWriteRequest2> pRequest ) = 0;

	// pPathID can be NULL, the request keeps its own copy of the file name
	virtual std::unique_ptr<IReadRequestBuilder> NewReadRequest( const char *pFileName, const char *pPathID ) = 0;
	// pBuffer has to stay valid until the request is done, see IWriteRequestBuilder::SetFreeBufferCallback()
	virtual std::unique_ptr<IWriteRequestBuilder> NewWriteRequest( const char *pFileName, const char *pPathID, const void *pBuffer, size_t nWriteBytes ) = 0;

	virtual std::unique_ptr<IIOCompletionQueue> CreateCompletionQueue() = 0;

	// For result buffers that outlive their request
	virtual void *AllocateBuffer( size_t nBufferSize, const char *pDebugName = NULL ) = 0;
	virtual void ReleaseBuffer( void *pBuffer ) = 0;
};

#endif // IASYNCFILESYSTEM_H
