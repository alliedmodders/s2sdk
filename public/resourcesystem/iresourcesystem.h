#ifndef IRESOURCESYSTEM_H
#define IRESOURCESYSTEM_H

#ifdef COMPILER_MSVC
#pragma once
#endif

#include "appframework/IAppSystem.h"
#include "resourcefile/resourcetype.h"
#include "tier1/utlstring.h"
#include "tier1/utlvector.h"

class CBufferString;
class ICodeResourceManifestManager;
class IFileSystem;
class IResourceSystemLeakTracker;
class IResourceSystemProfiler;
class IResourceTypeManager;
class IResourceUpdater;
class IToolsResourceListener;
class IToolsResourcePreReloadListener;
class IVDataTypeManager;

DECLARE_POINTER_HANDLE( HResourceManifest );

enum ResourceManifestPriority_t
{
	RESOURCE_TYPE_MANIFEST_PRIORITY_LOAD_FIRST = -1,
	RESOURCE_TYPE_MANIFEST_PRIORITY_DEFAULT = 0,
	RESOURCE_TYPE_MANIFEST_PRIORITY_LOAD_LAST = 1,
};

enum ResourceSystemDebugMode_t
{
	RESOURCE_SYSTEM_DEBUG_MODE_DEFER_MANIFEST_FINALIZATION = 0,
	RESOURCE_SYSTEM_DEBUG_MODE_SKIP_EXTREF_DEPTH_ASSERTIONS,
	RESOURCE_SYSTEM_DEBUG_MODE_IGNORE_DUPLICATE_TYPE_MANAGERS,

	RESOURCE_SYSTEM_DEBUG_MODE_COUNT,
};

enum ResourceManifestType_t : int8
{
	RESOURCE_MANIFEST_TYPE_DEFAULT = 0,
	RESOURCE_MANIFEST_TYPE_BLOCKING_LOAD,
	RESOURCE_MANIFEST_TYPE_BLOCKING_RELOAD,
};

typedef void (*ResourceManifestLoadCompletionCallback_t)( HResourceManifest hManifest, void *pContext );

struct ResourceManifestCreationInfo_t
{
	int m_nCount;
	const char **m_ppResourceFiles;
	ResourceManifestType_t m_nType;
	ResourceManifestLoadBehavior_t m_nLoadBehavior;
	const char *m_pDebugName;
	ResourceManifestLoadPriority_t m_nPriority;
	ResourceManifestLoadCompletionCallback_t m_pfnCallback;
	void *m_pContext;
};

typedef void (*ResourceSystemForcedSynchronizationCallback_t)( void *pContext );

enum ProceduralResourceType_t
{
	PROCEDURAL_RESOURCE_ANONYMOUS = 0,
	PROCEDURAL_RESOURCE_NAMED,
};

enum ResourceSystemUpdateMode_t
{
	RESOURCE_SYSTEM_UPDATE_RETURN_QUICKLY = 0,
	RESOURCE_SYSTEM_UPDATE_USE_ALL_TIME,
	RESOURCE_SYSTEM_UPDATE_USE_ALL_TIME_EXACT,
	RESOURCE_SYSTEM_UPDATE_SYNCHRONOUSLY_LOADING,
};

enum ResourceSystemGetResourcesFlags_t
{
	RESOURCE_SYSTEM_INCLUDE_PENDING_RESOURCES = 0x1,
	RESOURCE_SYSTEM_INCLUDE_NAMED_RESOURCES = 0x2,
	RESOURCE_SYSTEM_INCLUDE_ANONYMOUS_RESOURCES = 0x4,
	RESOURCE_SYSTEM_INCLUDE_NAMED_AND_ANONYMOUS_RESOURCES = RESOURCE_SYSTEM_INCLUDE_NAMED_RESOURCES | RESOURCE_SYSTEM_INCLUDE_ANONYMOUS_RESOURCES,
	RESOURCE_SYSTEM_INCLUDE_ALL_RESOURCES = RESOURCE_SYSTEM_INCLUDE_PENDING_RESOURCES | RESOURCE_SYSTEM_INCLUDE_NAMED_AND_ANONYMOUS_RESOURCES,
};

enum AsyncRequestPriority_t
{
	ASYNC_REQUEST_PRIORITY_INVALID = -1,
	ASYNC_REQUEST_PRIORITY_LOW = 0,
	ASYNC_REQUEST_PRIORITY_MEDIUM,
	ASYNC_REQUEST_PRIORITY_HIGH,
	ASYNC_REQUEST_PRIORITY_IMMEDIATE,

	ASYNC_REQUEST_PRIORITY_COUNT,
	ASYNC_REQUEST_PRIORITY_DEFAULT = ASYNC_REQUEST_PRIORITY_MEDIUM,
};

abstract_class IResourceSystem : public IAppSystem
{
public:
	virtual void InstallTypeManager( ResourceType_t nType, IResourceTypeManager *pTypeManager, const char *pszTypeName, const char *pszManifestName ) = 0;
	virtual void InstallNullTypeManager( ResourceType_t nType, const char *pszTypeName ) = 0;
	virtual void Update( int nTimeBudgetNs, ResourceSystemUpdateMode_t eMode ) = 0;
	virtual void UpdateSimple() = 0;
	virtual bool HasPendingWork() = 0;
	virtual void RemoveNullTypeManager( ResourceType_t nType ) = 0;
	virtual void RemoveResourceTypeManager( IResourceTypeManager *pTypeManager ) = 0;
	virtual IResourceTypeManager *GetTypeManager( ResourceType_t nType ) = 0;

	virtual HResourceManifest LoadResourceManifestFile( const char *pszFileName, ResourceManifestLoadBehavior_t eBehavior, const char *pszDebugName, ResourceManifestLoadPriority_t ePriority ) = 0;
	virtual HResourceManifest LoadResourceManifestGroup( const char *pszGroupName, ResourceManifestLoadBehavior_t eBehavior, const char *pszDebugName, ResourceManifestLoadPriority_t ePriority ) = 0;
	virtual HResourceManifest LoadResourceManifestNamed( const char *pszManifestName, ResourceManifestLoadBehavior_t eBehavior, const char *pszDebugName, ResourceManifestLoadPriority_t ePriority ) = 0;
	virtual HResourceManifest CreateResourceManifest( const ResourceManifestCreationInfo_t &info ) = 0;
	virtual void SetManifestCompletionCallback( HResourceManifest hManifest, ResourceManifestLoadCompletionCallback_t pfnCallback, void *pContext ) = 0;
	virtual bool IsManifestLoaded( HResourceManifest hManifest ) = 0;
	virtual void DestroyResourceManifest( HResourceManifest hManifest ) = 0;
	virtual const char *GetResourceManifestDebugName( HResourceManifest hManifest ) = 0;
	virtual void ForceSynchronizationAndBlockUntilManifestLoaded( HResourceManifest hManifest ) = 0;

	virtual ResourceHandle_t FindOrCreateProceduralResource( const CResourceName &name, void *pData, ProceduralResourceType_t eType ) = 0;
	virtual ResourceHandle_t FindResourceById( ResourceId_t id, ResourceType_t nType ) = 0;
	virtual void GetAllNamedResourcesByType( ResourceType_t nType, CUtlVector<ResourceHandle_t> &resources, ResourceSystemGetResourcesFlags_t nFlags ) = 0;
	virtual void GetAllResources( CUtlVector<ResourceHandle_t> &resources, ResourceSystemGetResourcesFlags_t nFlags ) = 0;
	virtual void GetAllResourcesByType( ResourceType_t nType, CUtlVector<ResourceHandle_t> &resources, ResourceSystemGetResourcesFlags_t nFlags ) = 0;
	virtual ResourceId_t ResourceHandleToResourceId( ResourceHandle_t hResource ) const = 0;
	virtual void GetResourceName( ResourceHandle_t hResource, CBufferString *pName, bool ) = 0;
	virtual void GetResourceName( ResourceHandle_t hResource, CResourceName *pName ) = 0;
	virtual ResourceType_t GetResourceType( ResourceHandle_t hResource ) = 0;
	virtual ResourceHandle_t GetErrorResource( ResourceType_t nType ) = 0;
	virtual const char *GetResourceTypeName( ResourceType_t nType ) = 0;
	virtual void MarkErrorResourcesReloaded() = 0;

	virtual ResourceHandle_t BlockingLoadResourceByName( const CResourceName &name, const char * ) = 0;
	virtual ResourceHandle_t BlockingLoadResourceByNameIntoJustInTimeManifest( const CResourceName &name, const char * ) = 0;
	virtual void FreeJustInTimeManifests() = 0;
	virtual int GetJustInTimeManifestCount() = 0;
	virtual void GetJustInTimeManifestResources( CUtlVector<ResourceHandle_t> &resources ) = 0;
	virtual bool IsInFrameUpdate() = 0;

	virtual void InstallResourceUpdater( IResourceUpdater *pUpdater ) = 0;
	virtual void UninstallResourceUpdater( IResourceUpdater *pUpdater ) = 0;
	virtual void GetActualFileName( ResourceHandle_t hResource, CBufferString *pFileName, bool ) = 0;
	virtual ResourceStatus_t GetResourceStatus( const CResourceName &name ) = 0;
	virtual ResourceStatus_t GetResourceStatus( ResourceId_t id ) = 0;
	virtual ResourceStatus_t GetResourceStatus( ResourceHandle_t hResource ) = 0;
	virtual void RegisterForcedSynchronizationCallback( ResourceSystemForcedSynchronizationCallback_t pfnCallback, void *pContext ) = 0;
	virtual void UnregisterForcedSynchronizationCallback( ResourceSystemForcedSynchronizationCallback_t pfnCallback, void *pContext ) = 0;
	virtual void GetResourcesNamesInManifest( HResourceManifest hManifest, CUtlVector<CUtlString> &resourceNames ) const = 0;

	// AMNOTE: Appends the handles of the manifest's resources; a manifest made of a single .vrman file lists that file's resources instead
	virtual void unk001( HResourceManifest hManifest, CUtlVector<ResourceHandle_t> &resources ) = 0;
	// AMNOTE: Ignored outside tools mode
	virtual void RegisterToolsResourceListener( IToolsResourceListener *pListener ) = 0;
	virtual void UnregisterToolsResourceListener( IToolsResourceListener *pListener ) = 0;
	// AMNOTE: Register/unregister a listener whose first virtual is called with ( const char *pszResourceName, int nEvent, 0 ) on resource changes
	virtual void unk101( void *pListener ) = 0;
	virtual void unk102( void *pListener ) = 0;

	virtual ICodeResourceManifestManager *GetCodeResourceManifestManager() = 0;
	virtual IResourceSystemProfiler *GetProfiler() = 0;
	virtual IResourceSystemLeakTracker *GetLeakTracker() = 0;
	virtual void SetResourceTypeManifestPriority( ResourceType_t nType, ResourceManifestPriority_t ePriority ) = 0;
	virtual void BlockingFinishAllCurrentlyLoadingManifests() = 0;
	virtual bool HasCompletedIORequests() = 0;
	virtual void EnableDebugMode( ResourceSystemDebugMode_t eMode, bool bEnable ) = 0;
	virtual AsyncRequestPriority_t GetAsyncFileRequestPriority( ResourceManifestLoadPriority_t ePriority ) const = 0;
	virtual IVDataTypeManager *GetGenericDataTypeManager() = 0;
	virtual bool IsShuttingDown() = 0;
	virtual bool IsDebugModeEnabled( ResourceSystemDebugMode_t eMode ) = 0;

	// AMNOTE: Ignored outside tools mode
	virtual void RegisterToolsResourcePreReloadListener( IToolsResourcePreReloadListener *pListener ) = 0;
	virtual void UnregisterToolsResourcePreReloadListener( IToolsResourcePreReloadListener *pListener ) = 0;

	virtual void InstallTestFilesystem( IFileSystem *pTestFS ) = 0;
	virtual void UninstallTestFilesystem( IFileSystem *pTestFS ) = 0;

	// AMNOTE: Lock and unlock a recursive mutex, and check whether the calling thread owns it
	virtual void unk201() = 0;
	virtual void unk202() = 0;
	virtual bool unk203() = 0;
	// AMNOTE: Queues a { void *pContext; void (*pfn)( void *pContext ); } pair that the next Update calls once
	virtual void unk204( const void *pCallback ) = 0;

	virtual ResourceHandle_t FindOrRegisterResourceByName( const CResourceName &name, bool ) = 0;
};

#endif // IRESOURCESYSTEM_H
