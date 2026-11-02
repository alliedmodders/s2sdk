#ifndef BASESPAWNGROUP_H
#define BASESPAWNGROUP_H

#ifdef _WIN32
#pragma once
#endif

#include "engine2/isource2engine.h"
#include "worldrenderer/iworldrenderermgr.h"
#include "engine2/igameresourceservice.h"
#include "resourcefile/compressedresourcemanifest.h"
#include "tier0/utlstring.h"
#include "tier0/utlvector.h"

class ISource2WorldSession;

class CBaseSpawnGroup : public ISpawnGroup, public IComputeWorldOriginCallback, public IGameResourceManifestLoadCompletionCallback
{
public:
	virtual matrix3x4_t ComputeWorldOrigin(const char *pWorldName, SpawnGroupHandle_t hSpawnGroup, IWorld *pWorld) = 0;
	virtual void Release() = 0;
	virtual void OnGameResourceManifestLoaded(HGameResourceManifest hManifest, int nResourceCount, ResourceHandle_t *pResourceHandles) = 0;
	virtual void Init(IResourceManifestRegistry *pResourceManifest, IEntityPrecacheConfiguration *pConfig, ISpawnGroupPrerequisiteRegistry *pRegistry) = 0;
	virtual void Shutdown() = 0;
	virtual bool GetLoadStatus() = 0;
	virtual void ForceBlockingLoad() = 0;
	virtual bool ShouldBlockUntilLoaded() const = 0;
	virtual void ServiceBlockingLoads() = 0;
	virtual bool GetEntityPrerequisites(HGameResourceManifest hManifest) = 0;
	virtual bool EntityPrerequisitesSatisfied() = 0;

public:
	virtual bool LoadEntities() = 0;
	virtual void SaveRestoreMap(ILoadingSpawnGroup *pLoading, SpawnGroupHandle_t hSpawnGroup, const matrix3x4a_t &vecSpawnOffset) = 0;
	virtual void SaveRestoreMap2(ILoadingSpawnGroup *pLoading, SpawnGroupHandle_t hSpawnGroup, const matrix3x4a_t &vecSpawnOffset) = 0;
	virtual void SetParentSpawnGroupForChild(SpawnGroupHandle_t hSpawnGroup) = 0;

public:
	virtual void TransferOwnershipOfManifestsTo(ISpawnGroup *pTarget) = 0;

public:
	virtual IGameSpawnGroupMgr *GetSpawnGroupMgr() const = 0;

public:
	CGameResourceManifestPrerequisite *GetManifestPrerequisite() { return &m_ManifestPrerequisite; }
	const CGameResourceManifestPrerequisite *GetManifestPrerequisite() const { return &m_ManifestPrerequisite; }

public:
	SpawnGroupHandle_t m_hSpawnGroupHandle;
	SpawnGroupState_t m_SpawnGroupState;
	SpawnGroupHandle_t m_hOwner;
	SpawnGroupHandle_t m_hParentSpawnGroup;
	WorldGroupId_t m_hWorldGroupId;
	ISource2WorldSession *m_pWorldSession;
	IWorldReference *m_pWorldReference;

	// AMNOTE: Copied from the SpawnGroupDesc_t it was created with
	matrix3x4a_t m_vecWorldOffset;
	IComputeWorldOriginCallback *m_pWorldOffsetCallback;
	CUtlString m_sWorldName;
	CUtlString m_sWorldMountName;
	CUtlString m_sEntityLumpName;
	CUtlString m_sEntityFilterName;
	CUtlString m_sDescriptiveName;
	CUtlString m_sParentNameFixup;
	CUtlString m_sLocalNameFixup;
	CUtlString m_sSaveFileName;
	CUtlString m_sWorldGroupName;
	uint8 m_nManifestLoadPriority; // ResourceManifestLoadPriority_t
	int m_iPriorityLoader;

	// AMNOTE: TransferOwnershipOfManifestsTo passes it on to the owner spawn group when it is higher
	int m_nCreationSequence;
	ILoadingSpawnGroup *m_pLoadingSpawnGroup;

	// Server spawn groups: the resources of the entity manifest, rebuilt as they load, for clients to precache
	CCompressedResourceManifest m_CompressedManifest;

private:
	// AMNOTE: The spawn group's game resource manifests, released on destruction. Private so its virtuals are called through
	// GetManifestPrerequisite(): GCC and Clang can bind a call on the member itself to the SDK's stubs
	CGameResourceManifestPrerequisite m_ManifestPrerequisite;

public:
	CUtlVector<SpawnGroupHandle_t> m_ChildSpawnGroups;
	float m_flTimeoutInterval;
	// AMNOTE: Plat_FloatTime() of the construction and of each m_SpawnGroupState change
	double m_flLastStateChangeTime;

	bool m_bServerSpawnGroup : 1;
	bool m_bLoadEntitiesFromSave : 1;
	bool m_bWaitingForManifestResources : 1;
	// AMNOTE: Set on a child added to its parent before both are ready to spawn their entities, until the parent is
	bool m_bWaitingForParent : 1;
	bool m_bDontSpawnEntities : 1;
	bool m_bBlockUntilLoaded : 1;
	// AMNOTE: Set at construction when the world session has world node streaming disabled; its world then loads all of its
	// nodes up front
	bool m_bDisableNodeStreaming : 1;
	bool m_bSpawnedSynchronously : 1;
	bool m_bLoadStreamingData : 1;
	bool m_bCreateNewSceneWorld : 1;
	bool m_bManualCompletion : 1;
	bool m_bSetActivePostLoad : 1;
	bool m_bIncludeInSaveGame : 1;
	bool m_bLevelTransition : 1;
	bool m_bChildSpawnGroupsReady : 1;
	bool m_bSuppressMapEntities : 1;
	bool m_bDisableBaseWorldLayer : 1;
};

#endif // BASESPAWNGROUP_H
