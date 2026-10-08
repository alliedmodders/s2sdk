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
#include "engine2/basespawngroup.h"
#include "spawngroupmgrgamesystem.h"

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

#endif // SPAWNGROUP_MANAGER_H
