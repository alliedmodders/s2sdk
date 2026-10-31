#ifndef ENTITYCOMPONENT_H
#define ENTITYCOMPONENT_H

#if _WIN32
#pragma once
#endif

#include "tier0/platform.h"
#include "tier0/smartptr.h"
#include "tier0/utlsymbollarge.h"
#include "tier0/utlstring.h"
#include "datamap.h"
#include "schemasystem/minschema.h"

class CEntityIdentity;
class CEntityInstance;
class CEntityComponentHelper;
class CKV3TransferLoadContext;
class CKV3TransferSaveContext;
class CNetworkSerializerClassInfo;
struct ComponentUnserializerClassInfo_t;

enum ActivateType_t
{
	ACTIVATE_TYPE_INITIAL_CREATION = 0x0,
	ACTIVATE_TYPE_DATAUPDATE_CREATION = 0x1,
	ACTIVATE_TYPE_ONRESTORE = 0x2,
};

enum DataUpdateType_t
{
	DATA_UPDATE_CREATED = 0x0,
	DATA_UPDATE_DATATABLE_CHANGED = 0x1,
};

enum EntityDormancyType_t
{
	ENTITY_NOT_DORMANT = 0x0,
	ENTITY_DORMANT = 0x1,
	ENTITY_SUSPENDED = 0x2,
};

struct ComponentUnserializerKeyNamesChunk_t
{
	CUtlStringToken m_keyNames[4];
};

struct ComponentUnserializerFieldInfo_t
{
	CSchemaEnumInfo* m_pSchemaEnum;
	const char* m_pKeyName;
	
	uint16 m_nOffset;
	uint16 m_nArraySize;
	
	SpawnKeyType_t m_Type;
	
	bool m_bUnserializeAsMatrix : 1;
	bool m_bArrayElement : 1;
	bool m_bRemovedKeyField : 1;
};

struct ComponentUnserializerPtrToClassInfo_t
{
	uint m_nOffset;
	ComponentUnserializerClassInfo_t* m_pClassInfo;
};

struct ComponentUnserializerClassInfo_t
{
	ComponentUnserializerClassInfo_t* m_pBaseClassInfo;

	ComponentUnserializerKeyNamesChunk_t* m_pKeyNamesChunks;
	ComponentUnserializerFieldInfo_t* m_pFieldInfos;
	ComponentUnserializerPtrToClassInfo_t* m_pClassInfoPtrs;

	uint16 m_nFieldInfoCount;
	uint16 m_nKeyNamesChunkCount;
	uint16 m_nClassInfoPtrCount;
};

enum EntComponentFlags_t
{
	ECF_NONE = 0,
	ECF_CONTAINED = (1 << 0), // Stored inside the entity
	ECF_REFERENCED = (1 << 1), // Allocated separately
	ECF_UNK2 = (1 << 2), // Unused
	ECF_UNK3 = (1 << 3), // Unused
	ECF_SCRIPTING = (1 << 4), // Has a script description
};

struct EntComponentInfo_t
{
	const char* m_pName;
	const char* m_pCPPClassname;
	const char* m_pNetworkDataReferencedDescription;
	const char* m_pNetworkDataReferencedPtrPropDescription;
	int m_nRuntimeIndex;
	uint32 m_nFlags; // EntComponentFlags_t
	ComponentUnserializerClassInfo_t m_componentUnserializerClassInfo;
	void* m_pScriptDesc;
	CEntityComponentHelper* m_pBaseClassComponentHelper;
};

struct EntOutput_t
{
	typedef void* (*GetOutputOwnerFn_t)( CEntityInstance* pEntity );

	const char* m_pName;
	uint32 m_nFlags;

	// Returns the object that holds the output, the entity itself when null
	GetOutputOwnerFn_t m_pfnGetOutputOwner;

	// Offset of the CEntityIOOutput in its owner, 0x7FFFFFFF when there is none
	uint32 m_nOutputOffset;
};

enum EntityHelperFlags_t
{
	EHF_None = 0,
	EHF_Allocate = (1 << 0),
	EHF_Free = (1 << 1),

	// AMNOTE: Unused flags, mostly remnants which recorded CEnttiyComponent callbacks the component's class override
	// <============================
	EHF_ConnectComponents = (1 << 2),
	EHF_Spawn = (1 << 3),
	EHF_BatchSpawn = (1 << 4),
	EHF_BatchSpawnDependencyIsland = (1 << 5),
	EHF_Activate = (1 << 6),
	EHF_BatchActivate = (1 << 7),
	EHF_PostDataUpdate = (1 << 8),
	EHF_BatchPostDataUpdate = (1 << 9),
	EHF_UpdateOnRemove = (1 << 10),
	EHF_BatchUpdateOnRemove = (1 << 11),
	EHF_OnSetDormant = (1 << 12),
	EHF_BatchOnSetDormant = (1 << 13),
	EHF_SaveEntity = (1 << 14),
	EHF_RestoreEntity = (1 << 15),
	EHF_UNK16 = (1 << 16),
	// ============================>

	EHF_Contained = (1 << 17), // The component info has ECF_CONTAINED
};

class CEntityComponentHelper
{
public:
	virtual SchemaMetaInfoHandle_t<CSchemaClassInfo> Schema_DynamicBinding() = 0;
	virtual void Finalize() = 0;
	virtual SchemaMetaInfoHandle_t<CSchemaClassInfo> GetSchemaBinding() = 0;
	virtual datamap_t* GetDataDescMap() = 0;
	virtual bool Allocate( CEntityIdentity* pEntity, void* pComponent ) = 0;
	virtual void Free( CEntityIdentity* pEntity, void* pComponent ) = 0;

public:
	uint32 m_flags; // EntityHelperFlags_t
	EntComponentInfo_t* m_pInfo;
	int m_nPriority;
	CEntityComponentHelper* m_pNext;
};

class CEntityComponent
{
public:
	virtual SchemaMetaInfoHandle_t<CSchemaClassInfo> Schema_DynamicBinding() = 0;
	virtual datamap_t* GetDataDescMap() = 0;
	virtual CNetworkSerializerClassInfo* GetNetworkSerializerInfo() = 0;
	virtual void KV3TransferSave( CKV3TransferSaveContext* pContext ) const = 0;
	virtual void KV3TransferLoad( CKV3TransferLoadContext* pContext ) = 0;
	virtual CEntityComponentHelper* GetComponentHelper() = 0;
};

// AMNOTE: The engine's default constructor takes CEntitySpawnHelper::tm_pCurEntityPtr, a thread local the game doesn't export
class CEntityOwnerPtr : public CSmartPtr<CEntityInstance, CNullRefCountAccessor>
{
public:
	operator CEntityInstance*() { return operator->(); }
	operator const CEntityInstance*() const { return operator->(); }

	void Set( CEntityInstance* pEntity ) { CSmartPtr<CEntityInstance, CNullRefCountAccessor>::operator=( pEntity ); }
};

#endif // ENTITYCOMPONENT_H