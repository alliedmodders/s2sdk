#ifndef SPAWNGROUPMGRGAMESYSTEM_H
#define SPAWNGROUPMGRGAMESYSTEM_H

#ifdef _WIN32
#pragma once
#endif

#include "bitvec.h"
#include "entityhandle.h"
#include "igamesystem.h"
#include "engine2/isource2engine.h"
#include "scenesystem/iscenesystem.h"
#include "tier0/convar.h"
#include "tier0/threadtools.h"
#include "tier0/utlmap.h"
#include "tier0/utlstring.h"
#include "tier0/utlstringtoken.h"
#include "tier0/utlvector.h"

class IPVS;
class CMapSpawnGroup;

// A level transition link from a map spawn group to another, through a landmark
struct SpawnGroupConnectionInfo_t
{
	CMapSpawnGroup *m_pSpawnGroup;
	int m_nSpawnGroupCacheMarker;
	CUtlString m_SpawnGroupName;
	CEntityHandle m_hLandMark; // info_spawngroup_landmark
	CEntityHandle m_hSpawnGroupLoad; // info_spawngroup_load_unload
	vis_info_t m_LandmarkPVS;
	CUtlString m_LandmarkName;
	CUtlStringToken m_LandmarkStringToken;
	int m_nClusterForLandmark;
	CBitVec<16> m_unConnectionBits;
	bool m_bForwardLink;
};

class CMapSpawnGroup
{
public:
	const char *GetWorldName() const
	{
		return m_pSpawnGroup->GetWorldName();
	}

	const char *GetEntityLumpName() const
	{
		return m_pSpawnGroup->GetEntityLumpName();
	}

	const char *GetEntityFilterName() const
	{
		return m_pSpawnGroup->GetEntityFilterName();
	}

	SpawnGroupHandle_t GetSpawnGroupHandle() const
	{
		return m_pSpawnGroup->GetHandle();
	}

	const matrix3x4a_t &GetWorldOffset() const
	{
		return m_pSpawnGroup->GetWorldOffset();
	}

	const char *GetParentNameFixup() const
	{
		return m_pSpawnGroup->GetParentNameFixup();
	}

	const char *GetLocalNameFixup() const
	{
		return m_pSpawnGroup->GetLocalNameFixup();
	}

	SpawnGroupHandle_t GetOwnerSpawnGroup() const
	{
		return m_pSpawnGroup->GetOwnerSpawnGroup();
	}

	ILoadingSpawnGroup *GetLoadingSpawnGroup() const
	{
		return m_pSpawnGroup->GetLoadingSpawnGroup();
	}

	void SetLoadingSpawnGroup(ILoadingSpawnGroup *pLoading)
	{
		m_pSpawnGroup->SetLoadingSpawnGroup(pLoading);
	}

	int GetCreationTick() const
	{
		return m_pSpawnGroup->GetCreationTick();
	}

	bool DontSpawnEntities() const
	{
		return m_pSpawnGroup->DontSpawnEntities();
	}

	void GetSpawnGroupDesc(SpawnGroupDesc_t *pDesc) const
	{
		m_pSpawnGroup->GetSpawnGroupDesc(pDesc);
	}

public:
	ISpawnGroup *GetSpawnGroup() const
	{
		return m_pSpawnGroup;
	}

private:
	ISpawnGroup *m_pSpawnGroup;
	bool m_bSpawnGroupPrecacheDispatched;
	bool m_bSpawnGroupLoadDispatched;
	IPVS *m_pPVS;
	CUtlVector<SpawnGroupConnectionInfo_t> m_Connections;
};

// AMNOTE: Short stub representing game class hierarchy
class CLoadingSpawnGroup : public ILoadingSpawnGroup
{
};

class CSpawnGroupMgrGameSystem : public IGameSpawnGroupMgr, public CAutoGameSystem
{
public:
	DECLARE_GAME_SYSTEM();

	CConCommandMemberAccessor<CSpawnGroupMgrGameSystem> m_SpawnGroupActivateCmd;
	CConCommandMemberAccessor<CSpawnGroupMgrGameSystem> m_SpawnGroupLoadCmd;
	CConCommandMemberAccessor<CSpawnGroupMgrGameSystem> m_SpawnGroupUnloadCmd;

	// SpawnGroupSpawnEntities calls queued from GameInit until they are executed
	CUtlVector<SpawnGroupHandle_t> m_SpawnEntitiesCallQueue;
	bool m_bQueueSpawnEntitiesCalls;

	CUtlOrderedMap<SpawnGroupHandle_t, CMapSpawnGroup *, CDefLess<SpawnGroupHandle_t>, unsigned short> m_SpawnGroups;
	CThreadRWLock_FastRead m_hHandleMutex;

	// UnloadSpawnGroup requests, carried out at the next frame boundary
	CUtlVector<SpawnGroupHandle_t> m_QueuedUnloadRequests;

	CUtlString m_Name;
};

#endif // SPAWNGROUPMGRGAMESYSTEM_H
