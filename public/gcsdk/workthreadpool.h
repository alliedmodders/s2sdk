#ifndef WORKTHREADPOOL_H
#define WORKTHREADPOOL_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/reliabletimer.h"
#include "tier0/threadtools.h"
#include "tier0/tslist.h"
#include "tier0/utlvector.h"

namespace GCSDK
{

class CWorkItem;
class CWorkThread;
class IWorkThreadFactory;

//-----------------------------------------------------------------------------
// Purpose: Threads that run work items queued by the job manager, which then dispatches the completed ones
//-----------------------------------------------------------------------------
class CWorkThreadPool
{
public:
	virtual ~CWorkThreadPool();
	virtual void OnWorkItemCompleted( CWorkItem *pWorkItem );

	char m_szThreadNamePfx[32];
	bool m_bThreadsInitialized;
	CTSQueue<CWorkItem *> *m_pTSQueueToProcess;
	CTSQueue<CWorkItem *> *m_pTSQueueCompleted;

	// AMNOTE: With m_bEnsureOutputOrdering, completed items wait here until those queued before them complete
	CAtomicMutex m_MutexOnItemCompletedOrdered;
	CUtlVector<CWorkItem *> m_vecCompletedAndWaiting;
	bool m_bEnsureOutputOrdering;
	uint64 m_ulLastUsedSequenceNumber;
	uint64 m_ulLastCompletedSequenceNumber;
	uint64 m_ulLastDispatchedSequenceNumber;

	bool m_bMayHaveJobTimeouts;
	CUtlVector<CWorkThread *> m_WorkThreads;
	CAtomicMutex m_WorkThreadMutex;
	CInterlockedUInt m_cThreadsRunning;
	volatile bool m_bExiting;
	CThreadEvent m_EventNewWorkItem;
	CInterlockedInt m_cActiveThreads;
	volatile bool m_bNeverSetOnAdd;
	bool m_bAutoCreateThreads;
	int m_cMaxThreads;
	IWorkThreadFactory *m_pWorkThreadConstructor;
	int m_cSuccesses;
	int m_cFailures;
	int m_cRetries;
	CLimitTimer m_LimitTimerCreateNewThreads;
};

} // namespace GCSDK

#endif // WORKTHREADPOOL_H
