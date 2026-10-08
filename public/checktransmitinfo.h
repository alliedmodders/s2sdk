#ifndef CHECKTRANSMITINFO_H
#define CHECKTRANSMITINFO_H
#ifdef _WIN32
#pragma once
#endif

#include "bitvec.h"
#include "const.h"
#include "playerslot.h"
#include "tier1/utlvector.h"
#include "entity2/entityidentity.h"
#include "scenesystem/iscenesystem.h"

// Entities can span this many clusters before we revert to a slower area checking algorithm
#define	MAX_FAST_ENT_CLUSTERS	4
#define	MAX_ENT_CLUSTERS	64
#define MAX_WORLD_AREAS		8

class CCheckTransmitInfo
{
public:
	CBitVec<MAX_EDICTS>	*m_pTransmitEntity;	// entity n is already marked for transmission
	CBitVec<MAX_EDICTS>	*m_pNonTransmitEntity; // entity n exists but isn't transmitted; filled from m_pTransmitEntity after the checks, and the client is sent its changes
	CBitVec<MAX_EDICTS>	*m_pTransmitOutOfPVS; // entity n is outside the PVS but still gets out-of-PVS updates (sv_outofpvsentityupdates)
	CBitVec<MAX_EDICTS>	*m_pTransmitAlways; // entity n is always send even if not in PVS (HLTV and Replay only)
	CUtlVector<SpawnGroupHandle_t> m_LoadedSpawnGroups; // sorted; entities from spawn groups the client hasn't loaded aren't checked
	vis_info_t m_VisInfo; // filled by ISource2GameClients::ClientSetupVisibility
	CPlayerSlot m_nPlayerSlot;
	bool m_bFullUpdate; // the client gets a full update instead of a delta
};

//-----------------------------------------------------------------------------
// Stores information necessary to perform PVS testing.
//-----------------------------------------------------------------------------
struct PVSInfo_t
{
	// headnode for the entity's bounding box
	short		m_nHeadNode;			

	// number of clusters or -1 if too many
	short		m_nClusterCount;		

	// cluster indices
	unsigned short *m_pClusters;	

	// For dynamic "area portals"
	short		m_nAreaNum;
	short		m_nAreaNum2;

	// current position
	float		m_vCenter[3];

private:
	unsigned short m_pClustersInline[MAX_FAST_ENT_CLUSTERS];
};


#endif // CHECKTRANSMITINFO_H
