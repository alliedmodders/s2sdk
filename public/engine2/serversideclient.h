#ifndef SERVERSIDECLIENT_H
#define SERVERSIDECLIENT_H

#ifdef _WIN32
#pragma once
#endif

#include "bitvec.h"
#include "const.h"
#include "engine2/clientframe.h"
#include "engine2/isource2server.h"
#include "engine2/serversideclientbase.h"
#include "tier1/jobthread.h"
#include "tier1/utlvector.h"

class CServerSideClient : public CServerSideClientBase
{
public:
	virtual ~CServerSideClient() = 0;

public:
	// Bit n is set when this client hears client n
	CPlayerBitVec m_VoiceStreams;
	// Bit n is set when this client hears client n by proximity
	CPlayerBitVec m_VoiceProximity;

	// Filled for each snapshot and passed to the game's transmit checks
	CCheckTransmitInfo m_CheckTransmitInfo;

	CClientFrameManager m_FrameManager;
	CClientFrame *m_pCurrentFrame;

	// AMNOTE: Move messages since the last packet
	int m_unk011;

	// AMNOTE: Server time of ActivatePlayer
	float m_unk012;

	// Whether the client hears its own voice
	bool m_bVoiceLoopback;
	bool m_unk111;
	bool m_unk112;
	int m_nHltvReplayDelay;
	void *m_unk211;
	int m_unk212;
	int m_unk213;
	int m_unk214;
	int m_unk215;
	int m_unk216;
	int m_unk217;
	double m_flLastHltvReplayRequestTime;

	// AMNOTE: Owns allocations of 4 bytes, Clear frees them
	CUtlVector<void *> m_unk311;

	// AMNOTE: HLTV replay statistics, formatted by GetHltvReplayStats
	uint32 m_unk312[14];

	// The job sending the client's snapshot, Await waits for it
	CThreadedJobWithDependencies *m_pSendJob;
};

#endif // SERVERSIDECLIENT_H
