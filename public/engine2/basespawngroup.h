#ifndef BASESPAWNGROUP_H
#define BASESPAWNGROUP_H

#ifdef _WIN32
#pragma once
#endif

#include "engine2/isource2engine.h"
#include "worldrenderer/iworldrenderermgr.h"
#include "engine2/igameresourceservice.h"

// AMNOTE: Short stub representing the engine class hierarchy
class CBaseSpawnGroup : public ISpawnGroup, public IComputeWorldOriginCallback, public IGameResourceManifestLoadCompletionCallback
{
};

#endif // BASESPAWNGROUP_H
