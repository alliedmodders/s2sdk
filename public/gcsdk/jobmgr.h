#ifndef JOBMGR_H
#define JOBMGR_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/reliabletimer.h"
#include "tier0/utlhashmaplarge.h"
#include "tier0/utllinkedlist.h"
#include "tier0/utlpriorityqueue.h"
#include "tier0/utlvector.h"
#include "gcsdk/jobtime.h"
#include "gcsdk/workthreadpool.h"

typedef uint64 JobID_t;

namespace GCSDK
{

class CJob;

// Its frame functions are pointers to members, which MSVC must give the engine's single inheritance size
#ifdef COMPILER_MSVC
class __single_inheritance CJobMgr;
#endif

struct JobStats_t
{
	uint32 m_cJobsCurrent;
	uint32 m_cJobsTotal;
	uint32 m_cJobsFailed;
	uint64 m_cJobsTimedOut;
	double m_flSumJobTimeMicrosec;
	uint64 m_unMaxJobTimeMicrosec;
	uint32 m_cTimeslices;
};

// AMNOTE: Per job name, while CJobMgr::m_bProfiling is set
struct JobStatsBucket_t
{
	char m_rgchName[64];
	uint64 m_cCompletes;
	uint64 m_u64RunTimeMax;
	uint64 m_cTimeoutNetMsg;
	uint64 m_cLongInterYieldTime;
	uint64 m_cLocksAttempted;
	uint64 m_cLocksWaitedFor;
	uint64 m_cLocksFailed;
	uint64 m_cLocksLongHeld;
	uint64 m_cLocksLongWait;
	uint64 m_cWaitTimeout;
	uint64 m_u64JobDuration;
	uint64 m_cJobsPaused;
	uint64 m_cJobsFailed;
	uint64 m_u64RunTime;
	uint64 m_cPauseReasonNetworkMsg;
	uint64 m_cPauseReasonSleepForTime;
	uint64 m_cPauseReasonWaitingForLock;
	uint64 m_cPauseReasonYield;
	uint64 m_cPauseReasonSQL;
	uint64 m_cPauseReasonWorkItem;
};

//-----------------------------------------------------------------------------
// Purpose: Runs the GC client's jobs: routes messages to them, resumes them and times them out
//-----------------------------------------------------------------------------
class CJobMgr
{
public:
	struct JobYielding_t
	{
		JobID_t m_JobID;
		uint32 m_nIteration;
	};

	struct JobSleeping_t
	{
		// AMNOTE: The earliest wakeup has the greatest priority
		bool operator<( const JobSleeping_t &rhs ) const { return m_SWakeupTime.LTime() > rhs.m_SWakeupTime.LTime(); }

		JobID_t m_JobID;
		CJobTime m_SWakeupTime;
		CJobTime m_STimeTouched;
	};

	struct JobTimeout_t
	{
		JobID_t m_JobID;
		CJobTime m_STimePaused;
		CJobTime m_STimeTouched;
		uint32 m_cHeartbeatsBeforeTimeout;
	};

	// AMNOTE: The job manager's work for a frame, each given the frame's time limit
	typedef bool (CJobMgr::*FrameFunc_t)( CLimitTimer &limitTimer );

	CUtlHashMapLarge<JobID_t, CJob *> m_MapJob;
	CUtlLinkedList<JobYielding_t, int> m_ListJobsYieldingRegPri;
	uint32 m_nCurrentYieldIterationRegPri;
	CUtlPriorityQueue<JobSleeping_t> m_QueueJobSleeping;
	CUtlLinkedList<JobTimeout_t, int> m_ListJobTimeouts;
	CUtlHashMapLarge<JobID_t, int> m_MapJobTimeoutsIndexByJobID;
	bool m_bJobTimedOut;
	CUtlVector<FrameFunc_t> m_vecFrameFuncs;

	// AMNOTE: Nothing reads or writes it
	uint8 m_unk101[8];

	CWorkThreadPool m_WorkThreadPool;
	JobStats_t m_JobStats;
	JobID_t m_unNextJobID;
	uint32 m_unFrameFuncThreadID;
	bool m_bProfiling;
	bool m_bIsShuttingDown;
	int m_cErrorsToReport;

	// AMNOTE: Keyed by the job name's address
	CUtlHashMapLarge<int64, JobStatsBucket_t> m_mapStatsBucket;

	CUtlHashMapLarge<uint32, int> m_mapOrphanMessages;

	// AMNOTE: Message types the message routing looks up
	CUtlVector<uint32> m_unk201;

	// AMNOTE: Only constructed and destroyed, element type unknown
	CUtlVector<void *> m_unk202;
};

} // namespace GCSDK

#endif // JOBMGR_H
