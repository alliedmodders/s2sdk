#ifndef NETMESSAGE_H
#define NETMESSAGE_H

#ifdef _WIN32
#pragma once
#endif

#include "networksystem/inetworkserializer.h"

class CCLCMsg_Move;
class CNETMsg_Tick;
class CSVCMsg_UserCommands;
class CMsgSource1LegacyGameEvent;

template<typename PB_OBJECT_TYPE>
class CNetMessagePBView;

// Maps a proto to the CNetMessagePB instantiation the engine uses for it.
// Protos without a specialization map to CNetMessagePBView, which has the same layout.
template<typename PB_OBJECT_TYPE>
struct CNetMessagePBTraits
{
	using Type = CNetMessagePBView<PB_OBJECT_TYPE>;
};

template<> struct CNetMessagePBTraits<CNETMsg_Tick> { using Type = CNetMessagePB<4, CNETMsg_Tick, SG_GAMEENGINE, BUF_UNRELIABLE, false>; };
template<> struct CNetMessagePBTraits<CCLCMsg_Move> { using Type = CNetMessagePB<21, CCLCMsg_Move, SG_MOVE, BUF_UNRELIABLE, false>; };
template<> struct CNetMessagePBTraits<CSVCMsg_UserCommands> { using Type = CNetMessagePB<76, CSVCMsg_UserCommands, SG_HLTVREPLAY, BUF_RELIABLE, false>; };
template<> struct CNetMessagePBTraits<CMsgSource1LegacyGameEvent> { using Type = CNetMessagePB<207, CMsgSource1LegacyGameEvent, SG_EVENTS, BUF_RELIABLE, true>; };

template<typename PB_OBJECT_TYPE>
using CNetMessagePBFor = typename CNetMessagePBTraits<PB_OBJECT_TYPE>::Type;

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

	// Helper function to cast down the abstract message to its concrete CNetMessagePBFor<T> type.
	// Doesn't do any validity checks itself!
	template<typename T>
	const CNetMessagePBFor<T> *ToPB() const
	{
		return static_cast<const CNetMessagePBFor<T> *>(this);
	}

	template<typename T>
	CNetMessagePBFor<T> *ToPB()
	{
		return static_cast<CNetMessagePBFor<T> *>(this);
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

// AMNOTE: Only the engine can construct these, since the CNetMessage implementation and the proto binding
// (message id, group, buffer type) live in the engine's instantiation of the template.
// So to allocate the message yourself use INetworkMessageInternal::AllocateMessage() or INetworkMessages::AllocateNetMessageAbstract()
// functions instead of direct initialization (they both are equivalent)!
// Example usage:
// CNetMessagePBFor<ProtoClass> *msg = INetworkMessageInternal::AllocateMessage()->ToPB<ProtoClass>();
// msg->field1( 2 );
// msg->field2( 3 );
// IGameEventSystem::PostEventAbstract( ..., msg, ... );
template<int msgType, typename PB_OBJECT_TYPE, SignonGroup_t groupType, NetChannelBufType_t bufType, bool bOkayToRedispatch>
class CNetMessagePB : public CNetMessage, public PB_OBJECT_TYPE
{
public:
	// Prevent manual construction of this object as per the comment above
	CNetMessagePB() = delete;
	CNetMessagePB( const CNetMessagePB & ) = delete;
};

// AMNOTE: Not an engine class, stands in for the CNetMessagePB of a proto that has no CNetMessagePBTraits specialization.
// Every CNetMessagePB of PB_OBJECT_TYPE has this layout, whatever its other template arguments are.
template<typename PB_OBJECT_TYPE>
class CNetMessagePBView : public CNetMessage, public PB_OBJECT_TYPE
{
public:
	CNetMessagePBView() = delete;
	CNetMessagePBView( const CNetMessagePBView & ) = delete;
};

#endif // NETMESSAGE_H

