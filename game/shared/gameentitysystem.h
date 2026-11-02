#ifndef GAMEENTITYSYSTEM_H
#define GAMEENTITYSYSTEM_H

#ifdef _WIN32
#pragma once
#endif

#include "entity2/entitysystem.h"

#include <initializer_list>

class CGameEntitySystem;

// Null before ISource2Server::Init and after its Shutdown
extern CGameEntitySystem* GameEntitySystem();

enum SpawnGroupEntityFilterType_t
{
	SPAWN_GROUP_ENTITY_FILTER_FALLBACK = 0x0,
	SPAWN_GROUP_ENTITY_FILTER_MOD_SPECIFIC = 0x1,
};

class IEntityListener
{
public:
	virtual void OnEntityCreated(CEntityInstance* pEntity) {};
	virtual void OnEntitySpawned(CEntityInstance* pEntity) {};
	virtual void OnEntityDeleted(CEntityInstance* pEntity) {};
	virtual void OnEntityParentChanged(CEntityInstance* pEntity, CEntityInstance* pNewParent) {};
};

class CGameEntitySystem : public CEntitySystem
{
	struct SpawnGroupEntityFilterInfo_t
	{
		ISpawnGroupEntityFilter* m_pFilter;
		SpawnGroupEntityFilterType_t m_nType;
	};

public:
	virtual ~CGameEntitySystem() = 0;

	void AddListenerEntity(IEntityListener* pListener);
	void RemoveListenerEntity(IEntityListener* pListener);

	// Spawns multiple entities of a given ekv's, expects classname to be set on them!
	// ppEntities, if given, receives one entry per key values: its entity, or null
	// where none was created (no spawnable class). An entity spawned from a copy of its key values (the game copies
	// those whose values have [PR#] or &0000 name fixups) also gets null.
	// 
	// Returns how many were created.
	// 
	// Some entities have different logic during spawning:
	//   - point_prefab (unless prefab_load_dynamic is false), or any with isPointPrefab true: no entity is created;
	//   instead the map in targetMapName starts loading asynchronously as a new spawn group, placed at origin and
	//   angles, and its entities spawn once it has loaded.
	//   - skybox_reference: the entity is created, and the map in its targetMapName starts loading the same way.
	//
	// AMNOTE: Requires g_pNetworkServerService to be available during the call, otherwise nothing will be spawned and null is returned!
	// Releases all entity keyvalues that were passed, if you want to keep them alive call AddRef() before doing any spawn call!
	int SpawnEntities(std::initializer_list<CEntityKeyValues*> keyValues, CEntityInstance** ppEntities = nullptr, SpawnGroupHandle_t hSpawnGroup = SPAWN_GROUP_HANDLE_DEFAULT)
	{
		return SpawnEntities(keyValues.begin(), (int)keyValues.size(), ppEntities, hSpawnGroup);
	}

	// Spawns multiple entities of a given ekv's, expects classname to be set on them!
	// ppEntities, if given, receives one entry per key values: its entity, or null
	// where none was created (no spawnable class). An entity spawned from a copy of its key values (the game copies
	// those whose values have [PR#] or &0000 name fixups) also gets null.
	// 
	// Returns how many were created.
	//
	// AMNOTE: Requires g_pNetworkServerService to be available during the call, otherwise nothing will be spawned and null is returned!
	// Releases all entity keyvalues that were passed, if you want to keep them alive call AddRef() before doing any spawn call!
	int SpawnEntities(CEntityKeyValues* const* ppKeyValues, int nCount, CEntityInstance** ppEntities = nullptr, SpawnGroupHandle_t hSpawnGroup = SPAWN_GROUP_HANDLE_DEFAULT);

	// Spawns one entity of a given ekv, expects classname to be set on it!
	//
	// AMNOTE: Requires g_pNetworkServerService to be available during the call, otherwise nothing will be spawned and null is returned!
	// Releases entity keyvalue that was passed, if you want to keep it alive call AddRef() before doing any spawn call!
	CEntityInstance* SpawnEntity(CEntityKeyValues* pKeyValues, SpawnGroupHandle_t hSpawnGroup = SPAWN_GROUP_HANDLE_DEFAULT)
	{
		CEntityInstance* pEntity = nullptr;
		SpawnEntities({ pKeyValues }, &pEntity, hSpawnGroup);
		return pEntity;
	}

	// Spawns one entity of a given classname, allocates ekv itself if no supplied
	//
	// AMNOTE: Requires g_pNetworkServerService to be available during the call, otherwise nothing will be spawned and null is returned!
	// Releases entity keyvalue that was passed, if you want to keep it alive call AddRef() before doing any spawn call!
	CEntityInstance* SpawnEntityByClassname(const char* pszClassName, CEntityKeyValues* pKeyValues = nullptr, SpawnGroupHandle_t hSpawnGroup = SPAWN_GROUP_HANDLE_DEFAULT);

	// AMNOTE: Wrapper over non templated SpawnEntityByClassname with a recast to your wanted entity type.
	// Target type is unchecked.
	template <typename DESIRED_CLASS>
	inline DESIRED_CLASS *SpawnEntity(CEntityKeyValues *pKeyValues, SpawnGroupHandle_t hSpawnGroup = SPAWN_GROUP_HANDLE_DEFAULT)
	{
		return (DESIRED_CLASS *)SpawnEntity( pKeyValues, hSpawnGroup );
	}

	// AMNOTE: Wrapper over non templated SpawnEntityByClassname with a recast to your wanted entity type.
	// Target type is unchecked.
	template <typename DESIRED_CLASS>
	inline DESIRED_CLASS *SpawnEntityByClassname(const char* pszClassName, CEntityKeyValues* pKeyValues = nullptr, SpawnGroupHandle_t hSpawnGroup = SPAWN_GROUP_HANDLE_DEFAULT)
	{
		return (DESIRED_CLASS *)SpawnEntityByClassname( pszClassName, pKeyValues, hSpawnGroup );
	}

public:
	int m_iNetworkedEntCount;
	int m_iNonNetworkedSavedEntCount;
	CUtlDict<SpawnGroupEntityFilterInfo_t> m_spawnGroupEntityFilters;
	CUtlVector<IEntityListener*> m_entityListeners;
	IEntity2SaveRestore* m_pEntity2SaveRestore;
	IEntity2Networkables* m_pEntity2Networkables;
};

#endif // GAMEENTITYSYSTEM_H
