#ifndef ENTITYNETWORKABLE_H
#define ENTITYNETWORKABLE_H

#if _WIN32
#pragma once
#endif

#include "tier0/platform.h"
#include "tier1/utlmap.h"
#include "tier1/utlvector.h"
#include "entity2/entityidentity.h"

class CEntityInstance;
class CEntityInstancePolymorphicMetadataHelper;
class CNetworkTransmitComponent;
class ServerClass;

class CChangeInfoAccessor
{
public:
	uint16 m_iChangeInfo;
	uint16 m_iChangeInfoSerialNumber;
};

struct Entity2Networkable_t
{
	ServerClass* m_pServerClass;
	CEntityIdentity* m_pEntity;
	CEntityInstance* m_pEntityInstance;
	CEntityInstance* m_pParentEntity;
	const char* m_pszDesignerName;
	const char* m_pszCPPClassname;
	CNetworkTransmitComponent* m_pTransmitComponent;
	CChangeInfoAccessor* m_pChangeAccessor;
	uint32* m_pnChangeFlags;
	CEntityInstancePolymorphicMetadataHelper* m_pMetadataHelper;
	SpawnGroupHandle_t m_SpawnGroupHandle;
	int m_nSerialNumber;
	int m_nEntryIndex;
};

abstract_class IEntity2Networkables
{
public:
	virtual ~IEntity2Networkables() = 0;

	// Copies the networkable of the entity into pNetworkable, false if it isn't networked
	virtual bool GetEntity2Networkable( CEntityIndex nEntityIndex, Entity2Networkable_t* pNetworkable ) = 0;
	virtual CUtlOrderedMap<int, Entity2Networkable_t, CDefLess<int>, uint16>* GetEntity2Networkables() = 0;

	// AMNOTE: Empty in CS2's server
	virtual void unk101() = 0;
	virtual void unk102() = 0;

	virtual CUtlVector<ServerClass*>* GetServerClasses() = 0;
};

#endif // ENTITYNETWORKABLE_H
