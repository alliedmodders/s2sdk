#ifndef IMANIFESTREGISTRY_H
#define IMANIFESTREGISTRY_H

#ifdef COMPILER_MSVC
#pragma once
#endif

#include "tier0/platform.h"
#include "resourcesystem/resourcemanifesthelpers.h"

abstract_class IResourceManifestRegistry
{
public:
	// The resource system keeps a list of the registered counters
	virtual void RegisterResourceSystemUser( uint32 *pFrameCounter ) = 0;
	virtual void UnregisterResourceSystemUser( uint32 *pFrameCounter ) = 0;

	virtual void RegisterNamedManifest( ResourceManifestDesc_t *pDesc ) = 0;
	virtual void UnregisterNamedManifest( ResourceManifestDesc_t *pDesc ) = 0;
};

#endif // IMANIFESTREGISTRY_H
