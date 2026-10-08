#ifndef ENTITYNETWORKABLE_H
#define ENTITYNETWORKABLE_H

#if _WIN32
#pragma once
#endif

#include "tier0/platform.h"
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

#endif // ENTITYNETWORKABLE_H
