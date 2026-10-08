#ifndef SPAWNGROUP_MANAGER_H
#define SPAWNGROUP_MANAGER_H
#ifdef _WIN32
#pragma once
#endif

#include "igamesystem.h"
#include "entity2/entitysystem.h"
#include "entity2/entityidentity.h"
#include "mathlib/mathlib.h"
#include "resourcefile/resourcetype.h"
#include "tier1/utlstring.h"
#include "tier1/utlscratchmemory.h"
#include "tier1/utlvector.h"
#include "engine2/igameresourceservice.h"
#include "worldrenderer/iworldrenderermgr.h"
#include "engine2/isource2engine.h"

class matrix3x4a_t;
class CCompressedResourceManifest;
class IPVS;
class IGameSpawnGroupMgr;
class CKV3Arena;
class CEntityKeyValues;
class ILoadingSpawnGroup;
class CGameResourceManifest;
class ISpawnGroupPrerequisiteRegistry;
class IWorld;
class IWorldReference;

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
	// CUtlVector<SpawnGroupConnectionInfo_t> m_Connections;
};

// AMNOTE: Short stubs representing game class hierarchy
class CBaseSpawnGroup : public ISpawnGroup, public IComputeWorldOriginCallback, public IGameResourceManifestLoadCompletionCallback
{
};

class CLoadingSpawnGroup : public ILoadingSpawnGroup
{
};

class CSpawnGroupMgrGameSystem : public IGameSpawnGroupMgr, public IGameSystem
{
};

#endif // SPAWNGROUP_MANAGER_H
