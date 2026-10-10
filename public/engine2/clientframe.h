#ifndef CLIENTFRAME_H
#define CLIENTFRAME_H

#ifdef _WIN32
#pragma once
#endif

#include "bitvec.h"
#include "const.h"
#include "tier0/threadtools.h"
#include "tier1/mempool.h"
#include "tier1/utllinkedlist.h"

class CFrameSnapshot;

// What was sent to a client in one snapshot
class CClientFrame
{
public:
	virtual ~CClientFrame() = 0;

	// AMNOTE: Returns true
	virtual bool unk001() = 0;

public:
	// The snapshot's tick
	int m_nTickCount;
	int m_unk001;
	void *m_unk002;
	CFrameSnapshot *m_pSnapshot;

	// Filled through the client's CCheckTransmitInfo, see its pointers of the same names
	CBitVec<MAX_EDICTS> m_TransmitEntity;
	CBitVec<MAX_EDICTS> m_NonTransmitEntity;
	CBitVec<MAX_EDICTS> m_TransmitOutOfPVS;
	CBitVec<MAX_EDICTS> *m_pTransmitAlways;
};

// The frames kept for a client, to delta snapshots from
class CClientFrameManager
{
public:
	virtual ~CClientFrameManager();

public:
	int m_unk001;
	CThreadFastMutex m_Mutex;

	// Ordered from the oldest to the newest tick
	CUtlLinkedList<CClientFrame *, unsigned short> m_Frames;
	CUtlMemoryPool<CClientFrame> m_ClientFramePool;
	int m_nNewestTick;
	int m_nOldestTick;

	// AMNOTE: Frames older than m_nOldestTick are looked up in it when it's set
	void *m_unk101;
};

#endif // CLIENTFRAME_H
