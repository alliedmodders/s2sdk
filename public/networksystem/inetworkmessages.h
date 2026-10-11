#ifndef NETWORKMESSAGES_H
#define NETWORKMESSAGES_H

#ifdef _WIN32
#pragma once
#endif

#include <interfaces/interfaces.h>

#include <networksystem/inetworkserializer.h>
#include <networksystem/networksystemtypes.h>
#include <tier0/bitbuf.h>
#include <tier0/utlstring.h>
#include <tier0/utlsymbol.h>
#include <tier0/utldelegate.h>
#include <tier0/logging.h>
#include "color.h"

class CSchemaClassBindingBase;
class INetworkSerializerBindingBuildFilter;
struct NetworkableDataType_t;
struct NetworkFieldResult_t;
struct NetworkSerializerInfo_t;
struct NetworkUnserializerInfo_t;
struct NetworkableData_t;
struct NetworkFieldInfo_t;
struct FieldMetaInfo_t;
class CSchemaType;

template<typename PROTO_TYPE>
class CNetMessagePB;

class CNetMessage
{
public:
	// Disabled as per CNetMessagePB note
	CNetMessage() = delete;
	CNetMessage( const CNetMessage & ) = delete;

	virtual ~CNetMessage() {}

	// Returns the underlying proto object
	virtual void *AsProto() const = 0;
	virtual void *AsProto2() const = 0;

	virtual INetworkMessageInternal *GetNetMessage() const = 0;
	virtual CNetMessage *CopyConstruct() const = 0;
	virtual NetworkMessageId GetMessageId() const = 0;
	virtual const char *GetName() const = 0;

	// Helper function to cast up the abstract message to a concrete CNetMessagePB<T> type.
	// Doesn't do any validity checks itself!
	template<typename T>
	const CNetMessagePB<T> *ToPB() const
	{
		return static_cast<const CNetMessagePB<T> *>(this);
	}

	template<typename T>
	CNetMessagePB<T> *ToPB()
	{
		return static_cast<CNetMessagePB<T> *>(this);
	}

	float GetMargin() const
	{
		return m_flMargin;
	}

private:
	double m_flRecvTime;
	int m_nTick;
	NetChannelBufType_t m_nBufType;
	int m_nBits;
	int m_nInSequenceNr;
	float m_flMargin;
	int64 m_unk101;
};

// AMNOTE: This is a stub class over real CNetMessagePB!
// This is mainly to access the game constructed objects, and not for direct initialization of them
// since this misses the CNetMessage implementation which requires supplying other proto related info like
// proto binding object, proto msg id/group, etc.
// So to allocate the message yourself use INetworkMessageInternal::AllocateMessage() or INetworkMessages::AllocateNetMessageAbstract()
// functions instead of direct initialization (they both are equivalent)!
// Example usage:
// CNetMessagePB<ProtoClass> *msg = INetworkMessageInternal::AllocateMessage()->ToPB<ProtoClass>();
// msg->field1( 2 );
// msg->field2( 3 );
// IGameEventSystem::PostEventAbstract( ..., msg, ... );
template<typename PROTO_TYPE>
class CNetMessagePB : public CNetMessage, public PROTO_TYPE
{
public:
	// Prevent manual construction of this object as per the comment above
	CNetMessagePB() = delete;
	CNetMessagePB( const CNetMessagePB & ) = delete;
};

abstract_class IProtobufBinding
{
public:
	virtual const char *GetName() = 0;
	virtual int GetSize() = 0;

	virtual const char *ToString(const void *pData, CUtlString &sResult) = 0;

	virtual const char *GetGroup() = 0;
	virtual Color GetGroupColor() = 0;
	virtual NetChannelBufType_t GetBufType() = 0;

	virtual CNetMessage *AllocateMessage() = 0;

	virtual bool OkToRedispatch() = 0;
	virtual bool unk001() = 0;
};

struct NetMessageInfo_t
{
	int m_nCategories;
	IProtobufBinding *m_pBinding;
	CUtlString m_szGroup;
	NetworkMessageId m_MessageId;
	NetworkGroupId m_GroupId;

	// AMNOTE: From IProtobufBinding::GetBufType()
	NetChannelBufType_t m_nBufType : 6;
	// AMNOTE: Set by FindOrCreateNetMessage's bCannotBeNetworked
	bool m_bCannotBeNetworked : 1;
	uint8 m_unk001 : 1;

	// AMNOTE: AddCategoryMask ORs nMask into it too when bRequireHandler is true. ConfirmAllMessageHandlersInstalled
	// reports channels in these categories that have no handler registered for the message
	int m_nRequiredHandlerCategories;
	bool m_bOkayToRedispatch;
};

enum NetworkFieldChangedDelegateType_t
{
	NETWORK_FIELD_CHANGED_NO_VALUE,
	NETWORK_FIELD_CHANGED_WITH_VALUE,
	NETWORK_FIELD_CHANGED_ARRAY_WITH_VALUE,
	NETWORK_FIELD_CHANGED_NO_VALUE_NO_CONTEXT,
	NETWORK_FIELD_CHANGED_TYPE_NONE,
	NETWORK_FIELD_CHANGED_COUNT_TYPE
};

typedef NetworkFieldResult_t ( *NetworkFieldSerializeCB )(NetworkSerializerInfo_t const&, int, NetworkableData_t *);
typedef NetworkFieldResult_t ( *NetworkFieldUnserializeCB )(NetworkUnserializerInfo_t const&, int, NetworkableData_t *);

typedef NetworkFieldResult_t ( *NetworkFieldSerializeBufferCB )(NetworkSerializerInfo_t const&, bf_write &, int *, void const **, bool);
typedef NetworkFieldResult_t ( *NetworkFieldUnserializeBufferCB )(NetworkUnserializerInfo_t const&, bf_read &, int *, void **, bool, CSchemaClassBindingBase const*);

typedef uint ( *NetworkFieldInfoCB )(NetworkFieldInfo_t const&);
typedef bool ( *NetworkFieldMetaInfoCB )(FieldMetaInfo_t const&, CSchemaClassBindingBase const*, NetworkFieldInfo_t const&, void *);
typedef bool ( *NetworkableDataCB )(NetworkableData_t const*, CUtlString &);
typedef char const* ( *NetworkUnkCB001 )(void);

#define NETWORKMESSAGES_INTERFACE_VERSION "NetworkMessagesVersion001"

abstract_class INetworkMessages
{
public:
	virtual void RegisterNetworkCategory(NetworkCategoryId nCategoryId, char const *szDebugName) = 0;

	virtual void AssociateNetMessageWithChannelCategoryAbstract(INetworkMessageInternal *pNetMessage, NetworkCategoryId nCategoryId, bool) = 0;

	// Passing nMessasgeId as -1 would auto-assign the id based on the message name hash.
	// AMNOTE: bCannotBeNetworked marks the message as never sent over the network, Serialize rejects it,
	// and its id is auto-assigned too
	virtual INetworkMessageInternal *FindOrCreateNetMessage(int nMessageId, IProtobufBinding const *pProtoBinding, uint nMessageSize, INetworkSerializerBindingBuildFilter *pUnused, bool bCreateIfNotFound = true, bool bCannotBeNetworked = false) = 0;

	virtual bool Serialize(bf_write &pBuf, const CNetMessage *pData) = 0;

	virtual bool UnserializeMessageInternal( bf_read &pBuf, CNetMessage *pData ) = 0;
	virtual bool SerializeMessageInternal( bf_write &pBuf, const CNetMessage *pData ) = 0;

	// Returns nullptr if failed to unserialize, reason is written to err_reason
	virtual CNetMessage *UnserializeFromStream(bf_read &pBuf, CUtlString &err_reason) = 0;
	virtual bool SerializeAbstract(bf_write &pBuf, INetworkMessageInternal *pNetMessage, const CNetMessage *pData) = 0;

	virtual CNetMessage *AllocateAndCopyConstructNetMessageAbstract(INetworkMessageInternal *pNetMessage, const CNetMessage *pFrom) = 0;

	virtual void DeallocateNetMessageAbstract(INetworkMessageInternal *pNetMessage, CNetMessage *pData) = 0;

	virtual void *RegisterNetworkFieldSerializer(char const *, NetworkSerializationMode_t, NetworkableDataType_t, int, NetworkFieldSerializeCB, NetworkFieldUnserializeCB, NetworkFieldInfoCB, 
		NetworkFieldMetaInfoCB, NetworkableDataCB, NetworkUnkCB001, NetworkFieldSerializeCB, NetworkFieldUnserializeCB) = 0;
	virtual void *RegisterNetworkArrayFieldSerializer(char const *, NetworkSerializationMode_t, NetworkFieldSerializeBufferCB, NetworkFieldUnserializeBufferCB, NetworkFieldInfoCB,
		NetworkFieldMetaInfoCB, NetworkFieldSerializeBufferCB, NetworkFieldUnserializeBufferCB) = 0;

	virtual NetMessageInfo_t *GetNetMessageInfo(INetworkMessageInternal *pNetMessage) = 0;

	virtual INetworkMessageInternal* FindNetworkMessage(char const *szName) = 0;
	virtual INetworkMessageInternal* FindNetworkMessagePartial(char const *szPartialName) = 0;

	virtual NetworkGroupId FindNetworkGroup(char const *szGroup, bool bCreateIfNotFound = false) = 0;
	virtual int GetNetworkGroupCount() = 0;
	virtual const char *GetNetworkGroupName(NetworkGroupId nGroupId) = 0;
	virtual Color GetNetworkGroupColor(NetworkGroupId nGroupId) = 0;

	virtual void AssociateNetMessageGroupIdWithChannelCategory(NetworkCategoryId nCategoryId, char const *szGroup) = 0;

	virtual void SetNetworkSerializationContextData(char const *szContext, NetworkSerializationMode_t, void *) = 0;
	virtual uint16 GetNetworkSerializationContextData(char const *szContext) = 0;

	// AMNOTE: Returns the data SetNetworkSerializationContextData set
	virtual void *unk101(uint16 hContext, NetworkSerializationMode_t mode) = 0;
	virtual void unk102(const void *) = 0;

	// Doesn't support duplicated callbacks per field
	virtual void RegisterNetworkFieldChangeCallbackInternal(char const *szFieldName, uint64, NetworkFieldChangedDelegateType_t fieldType, CUtlAbstractDelegate pCallback, NetworkFieldChangeCallbackPerformType_t cbPerformType, int unkflag ) = 0;

	virtual void AllowAdditionalMessageRegistration(bool bAllow) = 0;
	virtual bool IsAdditionalMessageRegistrationAllowed() = 0;

	virtual int GetFieldChangeCallbackOrderCount() = 0;
	virtual void GetFieldChangeCallbackPriorities(int nCount, int *nPriorities) = 0;

	virtual void RegisterFieldChangeCallbackPriority(int nPriority) = 0;

	virtual INetworkMessageInternal *FindNetworkMessageById(int nMessageId) = 0;

	virtual void SetIsForServer(bool bIsForServer) = 0;
	virtual bool GetIsForServer() = 0;

	virtual void RegisterSchemaTypeOverride(uint, char const *) = 0;

	virtual int ComputeOrderForPriority(int nPriority) = 0;

	virtual LoggingChannelID_t GetLoggingChannel() = 0;

	virtual void unk201(const void *) = 0;
	virtual void *unk202() = 0;

	virtual ~INetworkMessages() = 0;
};

DECLARE_TIER2_INTERFACE(INetworkMessages, g_pNetworkMessages);

#endif /* NETWORKMESSAGES_H */