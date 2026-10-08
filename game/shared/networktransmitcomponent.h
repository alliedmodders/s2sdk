#ifndef NETWORKTRANSMITCOMPONENT_H
#define NETWORKTRANSMITCOMPONENT_H

#if _WIN32
#pragma once
#endif

#include "tier0/platform.h"
#include "tier0/threadtools.h"
#include "mathlib/vector.h"
#include "entity2/entityidentity.h"
#include "game/entitynetworkable.h"
#include "schemasystem/schematypes.h"
#include "timedeventmgr.h"

class CEntityInstance;
class CEntityInstancePolymorphicMetadataHelper;
class CEventRegister;
class CKV3TransferLoadContext;
class CKV3TransferSaveContext;

class CNetworkTransmitComponent : public IEventRegisterCallback
{
public:
	virtual SchemaMetaInfoHandle_t<CSchemaClassInfo> Schema_DynamicBinding() = 0;
	virtual void KV3TransferSave( CKV3TransferSaveContext* pContext ) const = 0;
	virtual void KV3TransferLoad( CKV3TransferLoadContext* pContext ) = 0;
	virtual const char* GetClassname() = 0;
	virtual ~CNetworkTransmitComponent() = 0;

public:
	// AMNOTE: One 32-bit word of the PVS cluster bit vector
	struct PVSClusterWord_t
	{
		uint16 m_nCount; // The number of used words, only set in the first one
		uint16 m_nIndex; // Of the word in the bit vector
		uint32 m_nBits;
	};

public:
	// Allocated on first use
	ChangeAccessorFieldPathIndexInfo_t* m_pChangeChangeAccessorFieldPathIndexInfo;

	// While it is registered, state changes are only marked pending until it fires
	CEventRegister* m_pTimerEvent;

	// The PVS clusters the entity's bounds touch, recomputed when m_bPVSClustersDirty is set
	Vector m_vCenter; // Center of those bounds
	int m_nPVSClusterWordCount;
	// A single { 1, 0, 1 } when the bounds touch more than 32 words
	PVSClusterWord_t m_PVSClusterWords[32];

	CThreadRWLock_FastRead m_unk101;

	// AMNOTE: Bounds that the transmit check can cull the entity by distance to
	Vector m_unk102;
	Vector m_unk103;

	// FL_EDICT_CHANGED, FL_FULL_EDICT_CHANGED and the FL_EDICT_ALWAYS, FL_EDICT_DONTSEND or FL_EDICT_PVSCHECK transmit state
	uint32 m_nStateFlags;
	uint8 m_nTransmitStateOwnedCounter;
	bool m_bPendingStateChange;

	// Set together to recompute m_PhysicalSpawnGroupHandle and the PVS clusters when needed
	bool m_bPhysicalSpawnGroupDirty;
	bool m_bPVSClustersDirty;

	bool m_bNetworkUpdatesDisabled;

	// Changed offsets one shared change info holds, 48 when negative
	int m_nMaxChangedOffsets;

	CEntityInstance* m_pOuter;
	CEntityIndex m_EntityIndex; // Of m_pOuter, -1 until it is set

	CChangeInfoAccessor m_Accessor;

	// The spawn group whose bounds contain the entity, preferring the entity's own spawn group
	SpawnGroupHandle_t m_PhysicalSpawnGroupHandle;

	CAtomicMutex m_unk201;
	// AMNOTE: Held while the PVS clusters are recomputed
	CAtomicMutex m_unk202;

	CEntityInstancePolymorphicMetadataHelper* m_pPolymorphicMetadataHelper;
	bool m_bPolymorphicMetadataHelperDirty;
};

#endif // NETWORKTRANSMITCOMPONENT_H
