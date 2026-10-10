#include "tier0/jobthread.h"
#include "tier0/utlvector.h"
#include <vector>

struct JobItem_t
{
	int m_nValue;
};

class CJobItemOwner
{
public:
	void Process( JobItem_t &item ) {}
	void Begin() {}
	void End() {}
};

static void ProcessJobItem( JobItem_t &item )
{
}

class CDependentJob : public CThreadedJobWithDependencies
{
};

void InstantiateJobs( JobItem_t *pItems, unsigned nItems, CJobItemOwner *pOwner, std::vector<JobItem_t> &items, CUtlVector<CThreadedJobWithDependencies *> &jobs )
{
	ParallelProcess( "ProcessJobItem", pItems, nItems, &ProcessJobItem );
	ParallelProcess( "CJobItemOwner::Process", g_pThreadPool, pItems, nItems, pOwner, &CJobItemOwner::Process, &CJobItemOwner::Begin, &CJobItemOwner::End );

	ParallelFor( 0, nItems, "ParallelFor", [pItems]( int i ) { pItems[i].m_nValue = i; }, -1, INT_MAX, JP_HIGH );
	ParallelForEach( items, "ParallelForEach", []( JobItem_t &item ) { item.m_nValue = 0; }, 16, INT_MAX, JP_HIGH );

	CSmartPtr<CThreadedJob, CRefCountAccessor> pLambdaJob = g_pThreadPool->QueueJobWithFlags( "Lambda", JP_HIGH, 0, [pItems]() { pItems[0].m_nValue = 0; } );
	CSmartPtr<CThreadedJob, CRefCountAccessor> pFunctionJob = g_pThreadPool->QueueJobWithFlags( "Function", JP_HIGH, 0, std::function<void()>( [] {} ) );

	CDependentJob *pJob = new CDependentJob;
	jobs.AddToTail( pJob );
	Start( jobs );
	pJob->Wait();
	pJob->ReleaseUpstreamDependency();
}
