#ifndef IRESOURCESYSTEM_H
#define IRESOURCESYSTEM_H

#ifdef COMPILER_MSVC
#pragma once
#endif

#include "appframework/iappsystem.h"
#include "resourcefile/resourcetype.h"
#include "resourcesystem/resourcesystemtypes.h"
#include "filesystem/iasyncfilesystem.h"
#include "tier0/utlstring.h"
#include "tier0/utlvector.h"
#include <initializer_list>
#include <memory>

class CBufferString;
class IAsyncResourceDataRequest;
class ICodeResourceManifestManager;
class IFileSystem;
class IRD_RegisterResourceDataUtils;
class IResourceDeallocatorUtils;
class IResourceSystemLeakTracker;
class IResourceSystemProfiler;
class IResourceUpdater;
class IToolsResourceListener;
class IToolsResourcePreReloadListener;
class IVDataTypeManager;
struct ResourceFileHeader_t;

enum ResourceManifestLoadBehavior_t : int8
{
	RESOURCE_MANIFEST_LOAD_DEFAULT = -1,
	RESOURCE_MANIFEST_LOAD_STREAMING_DATA = 0,
	RESOURCE_MANIFEST_INITIALLY_USE_FALLBACKS,
};

enum ResourceManifestLoadPriority_t : int8
{
	RESOURCE_MANIFEST_LOAD_PRIORITY_DEFAULT = -1,
	RESOURCE_MANIFEST_LOAD_PRIORITY_LOW = 0,
	RESOURCE_MANIFEST_LOAD_PRIORITY_MEDIUM,
	RESOURCE_MANIFEST_LOAD_PRIORITY_HIGH,
	RESOURCE_MANIFEST_LOAD_PRIORITY_IMMEDIATE,

	RESOURCE_MANIFEST_LOAD_PRIORITY_COUNT,
};

DECLARE_POINTER_HANDLE( HResourceManifest );

enum ResourceLoadType_t
{
	RESOURCE_LOAD_INVALID = -1,
	RESOURCE_LOAD_SUCCESS = 0,
	RESOURCE_LOAD_FAIL,
	RESOURCE_LOAD_RELOAD,
	RESOURCE_LOAD_PRE_RELOAD,
};

enum ResourceStreamingType_t : uint8
{
	RESOURCE_STREAMING_TYPE_NO_DATA = 0,
	RESOURCE_STREAMING_TYPE_HAS_DATA,
	RESOURCE_STREAMING_TYPE_HAS_DATA_ALWAYS_LOAD,
	RESOURCE_STREAMING_TYPE_STREAM_DATA_FROM_ALLOCATE,
};

abstract_class IAsyncProcessedDataResult
{
public:
	virtual ~IAsyncProcessedDataResult() {}
};

abstract_class IResourceSystemUtils
{
public:
	virtual void MarkLoadingWorkPending( ResourceHandle_t hResource, bool bHasWork ) = 0;

	// nQueuePosition 1 queues the read ahead of the others
	virtual std::shared_ptr<IAsyncResourceDataRequest> CreateAsyncResourceDataRequest( ResourceHandle_t hResource, int64 nFileOffset, size_t nLoadSizeBytes, int nQueuePosition, const char *pFileNameOverride ) = 0;
	virtual void SubmitAsyncResourceDataRequest( IAsyncResourceDataRequest *pRequest ) = 0;

	virtual bool IsBlockingOnManifestLoad() = 0;

	// Replaces the resource's references with every resource reachable from them, following the references of
	// resources whose type is in pTypes
	virtual void FlattenResourceReferences( ResourceHandle_t hResource, int nTypeCount, const ResourceType_t *pTypes ) = 0;
};

abstract_class IResourceTypeManager
{
public:
	virtual bool Init( IResourceSystemUtils *pUtils ) = 0;
	virtual void Shutdown() = 0;
	// AMNOTE: Called after Shutdown when the type manager is removed, once the type's pending loads have finished
	virtual void unk001() = 0;
	virtual ResourceHandle_t GetErrorResource() = 0;
	virtual bool NeedsFrameUpdate() const = 0;
	virtual void FrameUpdate( int nFinishedFrameCount ) = 0;
	virtual bool RequiresFinalizeResourceCall() const = 0;
	virtual void FinalizeResource( ResourceHandle_t hResource, ResourceLoadType_t eLoadType ) = 0;
	// Cached when the type manager is installed; true makes the resource system call AsyncPreFinalizeResource from a job on a
	// worker thread before FinalizeResource
	virtual bool WantsAsyncPreFinalize() const = 0;
	virtual void AsyncPreFinalizeResource( ResourceHandle_t hResource, ResourceLoadType_t eLoadType ) = 0;
	virtual ResourceStreamingType_t GetStreamingType() const = 0;
	virtual int GetNonStreamingDataLoadSize( ResourceHandle_t hResource, bool bIsReloading ) const = 0;
	virtual void LoadStreamingData( ResourceHandle_t hResource, IResourceSystemUtils *pUtils ) = 0;
	virtual void AllocateResource( ResourceHandle_t hResource, ResourceId_t id, const ResourceFileHeader_t *pHeader, IRD_RegisterResourceDataUtils *pUtils ) = 0;
	virtual void DeallocateResource( void *pData, IResourceDeallocatorUtils *pUtils ) = 0;
	virtual void NotifyResourceWithWorkPendingLoadCancelled( ResourceHandle_t hResource, void *pData ) = 0;
	virtual bool UsesData() const = 0;
	virtual bool IsStreamingDataAlreadyLoaded( ResourceHandle_t hResource ) = 0;
	virtual bool WantsAsyncProcessData() const = 0;
	virtual std::unique_ptr<IAsyncProcessedDataResult> AsyncProcessData( ResourceHandle_t hResource, const ResourceFileHeader_t *pHeader ) = 0;
	// Cached when the type manager is installed; true makes the resource system call PostFinalizeResources at the end of
	// each update, only for the vdata type. The vdata type manager calls its listeners there for the vdata finalized
	// since the last call
	virtual bool WantsPostFinalize() const = 0;
	virtual void PostFinalizeResources() = 0;
	// Called on every type manager before and after each pass that deallocates the resources nothing references anymore
	virtual void BeginDeallocateResources() = 0;
	virtual void EndDeallocateResources() = 0;
};

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

// The resource names are referenced, not copied: CreateResourceManifest reads them right away, so build the info in the
// same expression as the call, or keep the names (and the list or array holding them) alive until it returns
struct ResourceManifestCreationInfo_t
{
	ResourceManifestCreationInfo_t() :
		m_nCount( 0 ), m_ppResourceFiles( nullptr ), m_nType( RESOURCE_MANIFEST_TYPE_DEFAULT ), m_nLoadBehavior( RESOURCE_MANIFEST_LOAD_DEFAULT ),
		m_pDebugName( nullptr ), m_nPriority( RESOURCE_MANIFEST_LOAD_PRIORITY_DEFAULT ), m_pfnCallback( nullptr ), m_pContext( nullptr )
	{ }

	ResourceManifestCreationInfo_t( int nCount, const char *const *ppResourceFiles, ResourceManifestType_t nType, ResourceManifestLoadBehavior_t nLoadBehavior,
		const char *pDebugName, ResourceManifestLoadPriority_t nPriority ) :
		m_nCount( nCount ), m_ppResourceFiles( ppResourceFiles ), m_nType( nType ), m_nLoadBehavior( nLoadBehavior ),
		m_pDebugName( pDebugName ), m_nPriority( nPriority ), m_pfnCallback( nullptr ), m_pContext( nullptr )
	{ }

	ResourceManifestCreationInfo_t( int nCount, const char *const *ppResourceFiles, ResourceManifestLoadBehavior_t nLoadBehavior = RESOURCE_MANIFEST_LOAD_DEFAULT,
		const char *pDebugName = nullptr, ResourceManifestLoadPriority_t nPriority = RESOURCE_MANIFEST_LOAD_PRIORITY_DEFAULT ) :
		ResourceManifestCreationInfo_t( nCount, ppResourceFiles, RESOURCE_MANIFEST_TYPE_DEFAULT, nLoadBehavior, pDebugName, nPriority )
	{ }

	// A single resource; pszResourceFile is referenced, so it has to outlive the use of the info
	ResourceManifestCreationInfo_t( const char *const &pszResourceFile, ResourceManifestLoadBehavior_t nLoadBehavior = RESOURCE_MANIFEST_LOAD_DEFAULT,
		const char *pDebugName = nullptr, ResourceManifestLoadPriority_t nPriority = RESOURCE_MANIFEST_LOAD_PRIORITY_DEFAULT ) :
		ResourceManifestCreationInfo_t( 1, &pszResourceFile, nLoadBehavior, pDebugName, nPriority )
	{ }

	ResourceManifestCreationInfo_t( std::initializer_list<const char *> resourceFiles, ResourceManifestLoadBehavior_t nLoadBehavior = RESOURCE_MANIFEST_LOAD_DEFAULT,
		const char *pDebugName = nullptr, ResourceManifestLoadPriority_t nPriority = RESOURCE_MANIFEST_LOAD_PRIORITY_DEFAULT ) :
		ResourceManifestCreationInfo_t( (int)resourceFiles.size(), resourceFiles.begin(), nLoadBehavior, pDebugName, nPriority )
	{ }

	int m_nCount;
	const char *const *m_ppResourceFiles;
	ResourceManifestType_t m_nType;
	ResourceManifestLoadBehavior_t m_nLoadBehavior;
	const char *m_pDebugName;
	ResourceManifestLoadPriority_t m_nPriority;
	ResourceManifestLoadCompletionCallback_t m_pfnCallback;
	void *m_pContext;
};

typedef void (*ResourceSystemForcedSynchronizationCallback_t)( void *pContext );
typedef void (*ResourceSystemQueuedCallbackFn_t)( void *pContext );

struct ResourceSystemQueuedCallback_t
{
	void *m_pContext;
	ResourceSystemQueuedCallbackFn_t m_pfnCallback;
};

enum ResourceEvent_t
{
	// Its file data was read, nDataSize is its size in bytes
	RESOURCE_EVENT_DATA_LOADED = 0,
	// Its streaming data was requested
	RESOURCE_EVENT_STREAMING_DATA_REQUESTED,
	// Finished loading, also sent for a resource with no file
	RESOURCE_EVENT_LOADED,
	// Just before it is deallocated
	RESOURCE_EVENT_DEALLOCATING,
	// The manifest starts loading its resources
	RESOURCE_EVENT_MANIFEST_LOAD_STARTED,
	// The manifest has loaded, after its completion callback ran
	RESOURCE_EVENT_MANIFEST_LOADED,
	// The manifest's load priority was raised, as when something blocks until it has loaded
	RESOURCE_EVENT_MANIFEST_PRIORITY_RAISED,
};

abstract_class IResourceEventListener
{
public:
	// nDataSize is 0 for every event but RESOURCE_EVENT_DATA_LOADED
	virtual void OnResourceEvent( const char *pszName, ResourceEvent_t nEvent, int nDataSize ) = 0;
};

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

/*
	Resources are compiled assets (the "_c" files) addressed by their source name, like "models/foo.vmdl". The resource
	system binds a ResourceHandle_t to a name as soon as it is known, and loads the data when a manifest listing it
	loads. A resource stays resident while any manifest references it and is deallocated once none does.

	A manifest (HResourceManifest) is a set of resources loaded and kept together:
	* CreateResourceManifest() (or LoadResourceManifestFile/Named/Group()) queues its resources and returns right away,
	  the data is streamed in during the resource system's frame update;
	* once loaded, ResourceManifestCreationInfo_t::m_pfnCallback runs and IsManifestLoaded() returns true,
	  ForceSynchronizationAndBlockUntilManifestLoaded() blocks until then instead;
	* DestroyResourceManifest() drops its references, the resources no other manifest references get deallocated.

	Manifests created through this interface only exist on the server. Clients load what the server sends them
	in two kinds of manifests, and only files they already have (the game, mounted addons, workshop items),
	nothing gets transferred:
	* the game session manifest, built when the map loads and sent to each client while it connects
	  (CSVCMsg_ServerInfo::game_session_manifest);
	* spawn group manifests, sent with every spawn group the server loads (CNETMsg_SpawnGroup_Load::spawngroupmanifest,
	  changed later with CNETMsg_SpawnGroup_ManifestUpdate).

	Resources can get loaded in these ways:

	* With the map, on the server and every client: a game system (see igamesystemfactory.h) adds
	the resources to the game session manifest while it is built. It lives as long as the map, nothing to free:

		class CMyGameSystem : public CAutoGameSystem
		{
		public:
			DECLARE_GAME_SYSTEM();

			GS_EVENT( BuildGameSessionManifest )
			{
				msg->m_pResourceManifest->AddResource( "models/example/thing.vmdl" );
			}
		};

	* After the map has loaded, on the server and every client: BlockingLoadResourceByNameIntoJustInTimeManifest()
	loads it on the server right away, blocking the frame, and records it in a just-in-time manifest:

		g_pResourceSystem->BlockingLoadResourceByNameIntoJustInTimeManifest( CResourceName( "models/example/thing.vmdl" ), "example" );

	The engine checks the just-in-time manifest count every tick and, when it changed, loads a
	"JustInTime Resource Spawn Group" holding all of them, which clients receive like any spawn group and load
	a moment later, so don't count on clients having it in the same tick. The engine owns these manifests:
	FreeJustInTimeManifests() releases them all, after which the engine unloads that spawn group.

	* With a manifest created here, which keeps the resources loaded until it is destroyed. It only loads them on the
	server (like a model used just for server side traces or physics), and isn't tied to the map, so it stays loaded
	across map changes:

		HResourceManifest hManifest = g_pResourceSystem->CreateResourceManifest( { "models/example/collision.vmdl" }, RESOURCE_MANIFEST_LOAD_DEFAULT, "example" );
		// ...
		g_pResourceSystem->DestroyResourceManifest( hManifest );

	* With a spawn group of its own, on the server and every client: the same as the just-in-time manifest, but
	loaded and unloaded under the caller's control. An HResourceManifest can't be handed to a spawn group, but its
	resources can be added to one by name: load a spawn group with IVEngineServer2::LoadSpawnGroup, and a game system
	adds them to that spawn group's manifest in its SpawnGroupPrecache event (m_pManifest, picked by
	m_SpawnGroupHandle). The spawn group's manifest is sent to clients like the map's, so they load and unload the
	resources with that spawn group. The engine reuses handles of unloaded spawn groups, so the handle is cleared in
	SpawnGroupUncache, which comes after an unload and after a map change unloads it. A spawn group unloaded before its
	precache ran sends neither event.

		class CExampleSpawnGroupSystem : public CAutoGameSystem
		{
		public:
			DECLARE_GAME_SYSTEM();

			void Load()
			{
				SpawnGroupDesc_t desc;
				desc.m_sWorldName = "maps/prefabs/example";
				desc.m_sDescriptiveName = "example";

				// 0 when it fails; the spawn group loads over the next frames, its precache runs after this returns
				m_hSpawnGroup = g_pEngineServer->LoadSpawnGroup( desc );
			}

			GS_EVENT( SpawnGroupPrecache )
			{
				if ( msg->m_SpawnGroupHandle == m_hSpawnGroup )
					msg->m_pManifest->AddResource( "models/example/collision.vmdl" );
			}

			GS_EVENT( SpawnGroupUncache )
			{
				if ( msg->m_SpawnGroupHandle == m_hSpawnGroup )
					m_hSpawnGroup = 0;
			}

			SpawnGroupHandle_t m_hSpawnGroup = 0;
		};
*/
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
	HResourceManifest CreateResourceManifest( int nCount, const char *const *ppResourceFiles, ResourceManifestType_t nType, ResourceManifestLoadBehavior_t nLoadBehavior, const char *pDebugName, ResourceManifestLoadPriority_t nPriority )
	{
		return CreateResourceManifest( ResourceManifestCreationInfo_t( nCount, ppResourceFiles, nType, nLoadBehavior, pDebugName, nPriority ) );
	}
	HResourceManifest CreateResourceManifest( int nCount, const char *const *ppResourceFiles, ResourceManifestLoadBehavior_t nLoadBehavior, const char *pDebugName, ResourceManifestLoadPriority_t nPriority )
	{
		return CreateResourceManifest( ResourceManifestCreationInfo_t( nCount, ppResourceFiles, nLoadBehavior, pDebugName, nPriority ) );
	}
	HResourceManifest CreateResourceManifest( std::initializer_list<const char *> resourceFiles, ResourceManifestLoadBehavior_t nLoadBehavior = RESOURCE_MANIFEST_LOAD_DEFAULT,
		const char *pDebugName = nullptr, ResourceManifestLoadPriority_t nPriority = RESOURCE_MANIFEST_LOAD_PRIORITY_DEFAULT )
	{
		return CreateResourceManifest( ResourceManifestCreationInfo_t( resourceFiles, nLoadBehavior, pDebugName, nPriority ) );
	}

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

	// bAppend keeps what pName holds instead of clearing it first
	virtual void GetResourceName( ResourceHandle_t hResource, CBufferString *pName, bool bAppend ) = 0;
	virtual void GetResourceName( ResourceHandle_t hResource, CResourceName *pName ) = 0;

	virtual ResourceType_t GetResourceType( ResourceHandle_t hResource ) = 0;
	virtual ResourceHandle_t GetErrorResource( ResourceType_t nType ) = 0;
	virtual const char *GetResourceTypeName( ResourceType_t nType ) = 0;

	virtual void MarkErrorResourcesReloaded() = 0;

	virtual ResourceHandle_t BlockingLoadResourceByName( const CResourceName &name, const char *pszDebugName ) = 0;
	virtual ResourceHandle_t BlockingLoadResourceByNameIntoJustInTimeManifest( const CResourceName &name, const char *pszDebugName ) = 0;

	virtual void FreeJustInTimeManifests() = 0;
	virtual int GetJustInTimeManifestCount() = 0;
	virtual void GetJustInTimeManifestResources( CUtlVector<ResourceHandle_t> &resources ) = 0;

	virtual bool IsInFrameUpdate() = 0;

	virtual void InstallResourceUpdater( IResourceUpdater *pUpdater ) = 0;
	virtual void UninstallResourceUpdater( IResourceUpdater *pUpdater ) = 0;

	virtual void GetActualFileName( ResourceHandle_t hResource, CBufferString *pFileName, bool bAppend ) = 0;

	virtual ResourceStatus_t GetResourceStatus( const CResourceName &name ) = 0;
	virtual ResourceStatus_t GetResourceStatus( ResourceId_t id ) = 0;
	virtual ResourceStatus_t GetResourceStatus( ResourceHandle_t hResource ) = 0;

	virtual void RegisterForcedSynchronizationCallback( ResourceSystemForcedSynchronizationCallback_t pfnCallback, void *pContext ) = 0;
	virtual void UnregisterForcedSynchronizationCallback( ResourceSystemForcedSynchronizationCallback_t pfnCallback, void *pContext ) = 0;

	virtual void GetResourcesNamesInManifest( HResourceManifest hManifest, CUtlVector<CUtlString> &resourceNames ) const = 0;
	// Appends the handles of the manifest's resources; a manifest made of a single .vrman file lists that file's resources instead
	virtual void GetResourcesInManifest( HResourceManifest hManifest, CUtlVector<ResourceHandle_t> &resources ) = 0;

	// AMNOTE: Ignored outside tools mode
	virtual void RegisterToolsResourceListener( IToolsResourceListener *pListener ) = 0;
	virtual void UnregisterToolsResourceListener( IToolsResourceListener *pListener ) = 0;

	virtual void RegisterResourceEventListener( IResourceEventListener *pListener ) = 0;
	virtual void UnregisterResourceEventListener( IResourceEventListener *pListener ) = 0;

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

	// A recursive mutex for callers, the resource system doesn't take it itself
	virtual void Lock() = 0;
	virtual void Unlock() = 0;
	virtual bool IsLockedByCurrentThread() = 0;

	virtual void QueueFrameUpdateCallback( const ResourceSystemQueuedCallback_t &callback ) = 0;

	// bWarnIfNotLoaded logs resources that are missing from every manifest, still pending or not loaded
	virtual ResourceHandle_t FindOrRegisterResourceByName( const CResourceName &name, bool bWarnIfNotLoaded ) = 0;
};

#endif // IRESOURCESYSTEM_H
