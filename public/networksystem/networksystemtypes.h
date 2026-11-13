#ifndef NETWORKSYSTEMTYPES_H
#define NETWORKSYSTEMTYPES_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/basetypes.h"
#include "tier0/utlhashtable.h"
#include "tier0/utlleanvector.h"
#include "tier0/utlvector.h"

enum NetworkFieldChangeCallbackPerformType_t
{
	NETWORK_FIELD_CHANGE_CALLBACK_PERFORM_ON_CHANGE,
	NETWORK_FIELD_CHANGE_CALLBACK_PERFORM_ON_CHANGE_OR_CREATE
};

struct OffsetIgnore_t
{
	typedef uint16 OffsetIgnoreValueType_t;

	CUtlHashtable<uint16, empty_t> m_OffsetHashtable;
	OffsetIgnoreValueType_t m_unMaxOffset;
};

struct ChangeAccessorFieldPathIndex_t
{
	ChangeAccessorFieldPathIndex_t() { m_Value = -1; }
	ChangeAccessorFieldPathIndex_t( int32 value ) { m_Value = value; }

	int32 m_Value;
};

struct ChangeAccessorFieldPathIndexInfo_t
{
	struct IgnoreCache_t
	{
		uint16 m_Offsets[16];
		ChangeAccessorFieldPathIndex_t m_FieldPaths[16];
		int m_nCount;
		int m_nFirstElement;
	};

	typedef CUtlLeanVectorFixedGrowable<uint32> PackedFieldPathVec_t;

	PackedFieldPathVec_t m_ChangeAccessorFieldPathIdList;
	const OffsetIgnore_t *m_pBaseOffsetToIgnore;
	CUtlVectorFixedGrowable<const OffsetIgnore_t *, 4> m_OffsetsToIgnoreForPaths;
	IgnoreCache_t m_ShouldIgnore;
	IgnoreCache_t m_ShouldNotIgnore;
};

// AMNOTE: A changed field, or changed elements of a networked vector
struct VarChangeInfo_t
{
	uint32 m_nFieldOffset;
	ChangeAccessorFieldPathIndex_t m_nRootPathIndex;

	// All bits are set when only the whole field changed, and then these hold no element list.
	// Otherwise the changed elements are in VarChangeList_t::m_ChangedElements[m_nElementListIndex]
	uint16 m_nElementCount : 7;
	uint16 m_bElementsSorted : 1;
	uint16 m_nElementListIndex : 7;
	uint16 m_bFieldChanged : 1; // The whole field changed too
};

enum NetworkSerializationMode_t
{
	NET_SERIALIZATION_MODE_0 = 0x0,
	NET_SERIALIZATION_MODE_1 = 0x1,
	NET_SERIALIZATION_MODE_COUNT = 0x2,
	NET_SERIALIZATION_MODE_DEFAULT = 0x0,
	NET_SERIALIZATION_MODE_SERVER = 0x0,
	NET_SERIALIZATION_MODE_CLIENT = 0x1,
};

enum NetworkOverrideType_t
{
	MNetworkOverride_Invalid = -1,
	MNetworkSerializer = 0,
	MNetworkEncoder,
	MNetworkChangeCallback,
	MNetworkChangeTag,
	MNetworkBitCount,
	MNetworkUserGroup,
	MNetworkPriority,
	MNetworkOutOfPVSUpdates,
	MNetworkRemoveAll,
};

struct NetworkOverride_t
{
	const char *m_ParentClass;
	const char *m_FieldName;
	const char *m_Value;
	NetworkOverrideType_t m_Type;
};

enum NetChannelBufType_t : int8
{
	BUF_DEFAULT = -1,
	BUF_UNRELIABLE = 0,
	BUF_RELIABLE,
	BUF_VOICE,
};

// Where received data comes from, for a net channel and for INetworkMessages' unserialize methods
// AMNOTE: Only NET_RECV_LIMIT_MODE_CLIENT changes anything. It applies the server's limits on what clients send:
// net_limit_sv_recvbuffer_kb and net_limit_sv_recvbuffer_msg, net_limit_sv_message_process_time_ms_* and
// net_message_size_enforce. The other modes get the same defaults as NET_RECV_LIMIT_MODE_DEFAULT
enum NetRecvLimitMode_t
{
	NET_RECV_LIMIT_MODE_DEFAULT = 0,
	NET_RECV_LIMIT_MODE_CLIENT, // From a remote client, on the server
	NET_RECV_LIMIT_MODE_SERVER, // From the server, on the client
	NET_RECV_LIMIT_MODE_P2P,
	NET_RECV_LIMIT_MODE_DEMO,
};

DECLARE_POINTER_HANDLE( FlattenedSerializerHandle_t );

#endif // NETWORKSYSTEMTYPES_H
