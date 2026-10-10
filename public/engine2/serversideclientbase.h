#ifndef SERVERSIDECLIENTBASE_H
#define SERVERSIDECLIENTBASE_H

#ifdef _WIN32
#pragma once
#endif

#include "bitvec.h"
#include "const.h"
#include "inetchannel.h"
#include "playerslot.h"
#include "entity2/entityidentity.h"
#include "networksystem/inetworksystem.h"
#include "networksystem/netmessage.h"
#include "steam/steamclientpublic.h"
#include "tier0/annotations.h"
#include "tier0/tslist.h"
#include "tier1/circularbuffer.h"
#include "tier1/convar.h"
#include "tier1/utlcommon.h"
#include "tier1/utldict.h"
#include "tier1/utlsignalslot.h"
#include "tier1/utlstring.h"
#include "tier1/utlvector.h"
#include "netmessages.pb.h"

class KeyValues;
class CClientFrame;
class CFrameSnapshot;
class CNetworkGameServerBase;
class INetworkStringTable;
struct HltvReplayParams_t;

class NetMessagePacketStart;
class NetMessagePacketEnd;
class NetMessageConnectionClosed;
class NetMessageConnectionCrashed;
class NetMessageSplitscreenUserChanged;

template <class T>
class CDelayedCallBase;

class CServerSideClientBase : public CUtlSlot, public INetworkChannelNotify, public INetworkMessageProcessingPreFilter
{
public:
	struct BuildServerInfoMessageAsync_t;

	// AMNOTE: Net spike trace, written to netspike.txt. Each sent net message adds a record of its description and size while m_unk001 isn't 0
	struct NetSpikeTrace_t
	{
		struct Record_t
		{
			CUtlString m_Description;
			int m_nBits;
		};

		CUtlVector<Record_t> m_Records;
		int m_unk001;
		int m_unk002;
		int m_unk003;
	};

	virtual ~CServerSideClientBase() = 0;

	// AMNOTE: The first argument is unused
	virtual void Connect( int, const char *pszName, CPlayerUserId nUserID, INetChannel *pNetChannel, uint8 nConnectionTypeFlags, uint32 nChallengeNumber ) = 0;
	virtual void Inactivate( const char *pszAddons ) = 0;
	virtual void Reactivate( CPlayerSlot nSlot ) = 0;
	virtual void SetServer( CNetworkGameServerBase *pServer ) = 0;
	virtual void Reconnect() = 0;
	virtual void Disconnect( ENetworkDisconnectionReason reason, const char *pszInternalReason ) = 0;
	virtual bool CheckConnect() = 0;

	// AMNOTE: Clears the client and gives it the SteamID and name without a connection, returns its slot
	virtual CPlayerSlot Create( CSteamID steamID, const char *pszName ) = 0;

	virtual void SetRate( int nRate ) = 0;
	virtual void SetUpdateRate( float flUpdateRate ) = 0;
	virtual int GetRate() = 0;

	virtual void Clear() = 0;

	// Returning false counts towards kicking the client for too many failed commands
	virtual bool ExecuteStringCommand( const CNetMessagePB<CNETMsg_StringCmd> &msg ) = 0;
	virtual bool SendNetMessage( const CNetMessage *pData, NetChannelBufType_t bufType = BUF_DEFAULT ) = 0;
	virtual bool FilterMessage( const CNetMessage *pData, INetChannel *pChannel ) override = 0;

	virtual void ClientPrintf( PRINTF_FORMAT_STRING const char *pszFormat, ... ) = 0;

	virtual bool IsFakeClient() const = 0;
	// Is an actual human player or split screen player (not a bot and not a HLTV slot)
	virtual bool IsHumanPlayer() const = 0;
	virtual bool IsHearingClient( CPlayerSlot nSlot ) const = 0;
	virtual bool IsProximityHearingClient( CPlayerSlot nSlot ) const = 0;
	virtual bool IsLowViolenceClient() const = 0;
	virtual bool IsSplitScreenUser() const = 0;

	virtual bool ProcessTick( const CNetMessagePB<CNETMsg_Tick> &msg ) = 0;
	virtual bool ProcessStringCmd( const CNetMessagePB<CNETMsg_StringCmd> &msg ) = 0;
	virtual bool ProcessSetConVar( const CNetMessagePB<CNETMsg_SetConVar> &msg ) = 0;
	virtual bool ProcessSignonState( const CNetMessagePB<CNETMsg_SignonState> &msg ) = 0;
	virtual bool ProcessSpawnGroup_LoadCompleted( const CNetMessagePB<CNETMsg_SpawnGroup_LoadCompleted> &msg ) = 0;
	virtual bool ProcessClientInfo( const CNetMessagePB<CCLCMsg_ClientInfo> &msg ) = 0;
	virtual bool ProcessBaselineAck( const CNetMessagePB<CCLCMsg_BaselineAck> &msg ) = 0;
	virtual bool ProcessLoadingProgress( const CNetMessagePB<CCLCMsg_LoadingProgress> &msg ) = 0;
	virtual bool ProcessSplitPlayerConnect( const CNetMessagePB<CCLCMsg_SplitPlayerConnect> &msg ) = 0;
	virtual bool ProcessSplitPlayerDisconnect( const CNetMessagePB<CCLCMsg_SplitPlayerDisconnect> &msg ) = 0;
	virtual bool ProcessCmdKeyValues( const CNetMessagePB<CCLCMsg_CmdKeyValues> &msg ) = 0;
	virtual bool ProcessRconServerDetails( const CNetMessagePB<CCLCMsg_RconServerDetails> &msg ) = 0;
	virtual bool ProcessDiagnostic( const CNetMessagePB<CCLCMsg_Diagnostic> &msg ) = 0;
	virtual bool ProcessMove( const CNetMessagePB<CCLCMsg_Move> &msg ) = 0;
	virtual bool ProcessVoiceData( const CNetMessagePB<CCLCMsg_VoiceData> &msg ) = 0;
	virtual bool ProcessRespondCvarValue( const CNetMessagePB<CCLCMsg_RespondCvarValue> &msg ) = 0;
	virtual bool ProcessPacketStart( const CNetMessagePB<NetMessagePacketStart> &msg ) = 0;
	virtual bool ProcessPacketEnd( const CNetMessagePB<NetMessagePacketEnd> &msg ) = 0;
	virtual bool ProcessConnectionClosed( const CNetMessagePB<NetMessageConnectionClosed> &msg ) = 0;
	virtual bool ProcessConnectionCrashed( const CNetMessagePB<NetMessageConnectionCrashed> &msg ) = 0;
	virtual bool ProcessSplitscreenUserChanged( const CNetMessagePB<NetMessageSplitscreenUserChanged> &msg ) = 0;
	virtual bool ProcessHltvReplay( const CNetMessagePB<CCLCMsg_HltvReplay> &msg ) = 0;
	virtual bool ProcessHltvFixupOperatorStatus( const CNetMessagePB<CSVCMsg_HltvFixupOperatorStatus> &msg ) = 0;
	virtual bool ProcessUserMessage( const CNetMessagePB<CSVCMsg_UserMessage> &msg ) = 0;

	// Registers the message handlers above on the channel
	virtual void ConnectionStart( INetChannel *pNetChannel ) = 0;

	// AMNOTE: Both unk001 are overloads of one name, as the Windows binary orders them in reverse.
	// The second one sends the server info built by a BuildServerInfoMessageAsync_t delayed call
	virtual void unk001() = 0;
	virtual void SendInitialSpawnGroups( const CUtlVector<void *> &spawnGroups, bool bAsync ) = 0;
	virtual void unk001( BuildServerInfoMessageAsync_t &info ) = 0;

	virtual bool UpdateAcknowledgedFramecount( int nTick ) = 0;
	virtual bool ShouldSendMessages() = 0;
	virtual void UpdateSendState() = 0;

	virtual const CMsgPlayerInfo &GetPlayerInfo() const = 0;

	// Applies the name, rate and other settings from the userinfo convars
	virtual void UpdateUserSettings() = 0;
	// AMNOTE: Sets the rate from the "rate" userinfo convar
	virtual void unk101() = 0;

	virtual CClientFrame *GetClientFrame( int nTick ) = 0;

	virtual void SendSignonData() = 0;
	virtual void SpawnPlayer() = 0;
	virtual void ActivatePlayer() = 0;

	virtual void SetName( const char *pszName ) = 0;
	virtual void SetUserCVar( const char *pszConVar, const char *pszValue ) = 0;

	virtual void FreeBaselines() = 0;

	virtual CServerSideClientBase *GetSplitScreenOwner() = 0;

	virtual bool unk201() = 0;

	virtual bool ShouldReceiveStringTableUserData( const INetworkStringTable *pTable, int nStringNumber ) = 0;

	// AMNOTE: Forwards to ISource2GameClients::unk101 with the client's slot
	virtual void unk301( void *pOut ) = 0;

	virtual bool StartHltvReplay( const HltvReplayParams_t &params ) = 0;
	virtual void StopHltvReplay() = 0;
	virtual int GetHltvReplayDelay() = 0;
	virtual bool CanStartHltvReplay( int nDelay ) = 0;
	virtual void ResetHltvReplayRequestTime() = 0;
	virtual const char *GetHltvReplayStats() = 0;

	// Waits for the client's pending send job
	virtual void Await() = 0;

	virtual void MarkToKick() = 0;
	virtual void UnmarkToKick() = 0;

	virtual bool ProcessSignonStateMsg( int nState, int nSpawnCount ) = 0;
	virtual void PerformDisconnection( ENetworkDisconnectionReason reason ) = 0;

public:
	CUtlString m_unk001;
	CUtlString m_Name;
	CPlayerSlot m_nClientSlot;
	CEntityIndex m_nEntityIndex;
	CNetworkGameServerBase *m_pServer;
	INetChannel *m_pNetChannel;
	uint8 m_nConnectionTypeFlags;

	// AMNOTE: 1 when Disconnect is queued as a delayed call instead of disconnecting right away
	uint8 m_unk101;

	bool m_bMarkedToKick;
	SignonState_t m_nSignonState;
	bool m_bSplitScreenUser;
	bool m_unk201;
	CSplitScreenSlot m_nSplitScreenPlayerSlot;
	CServerSideClientBase *m_SplitScreenUsers[4];
	CServerSideClientBase *m_pAttachedTo;
	bool m_unk301;

	// AMNOTE: Create sets it to 2
	int m_unk302;

	bool m_bFakePlayer;
	bool m_unk401;
	int m_unk402;
	CPlayerUserId m_UserID;
	bool m_bReceivedPacket;
	CSteamID m_SteamID;
	CSteamID m_unk501;
	CSteamID m_unk502;
	CSteamID m_FriendsID;
	ns_address m_Addr;
	ns_address m_unk601;
	KeyValues *m_pUserInfo;
	CUtlDict<empty_t, int> m_unk701;
	bool m_bConVarsChanged;
	bool m_unk801;
	bool m_bIsHLTV;
	bool m_unk901;

	// AMNOTE: The HLTV client's downstream TV sink, SendNetMessage forwards to it
	void *m_unk902;

	uint32 m_nSendTableCRC;
	uint32 m_nChallengeNumber;

	// AMNOTE: Server tick the server info was built at
	int m_unk1001;

	int m_nDeltaTick;
	int m_unk1101;
	int m_unk1102;
	int m_unk1103;
	CFrameSnapshot *m_pLastSnapshot;
	CUtlVector<SpawnGroupHandle_t> m_LoadedSpawnGroups;
	CMsgPlayerInfo m_PlayerInfo;
	CFrameSnapshot *m_pBaseline;

	// AMNOTE: Reset to -1 once the client acknowledges a later tick
	int m_unk1201;

	CBitVec<MAX_EDICTS> m_unk1202;
	int m_unk1203;
	int m_nLoadingProgress;
	int m_nForceWaitForTick;

	// AMNOTE: History of the acknowledged ticks, logged when they arrive out of order
	CCircularBuffer m_unk1301;

	bool m_bLowViolence;
	bool m_unk1401;
	bool m_unk1402;
	bool m_unk1403;
	bool m_unk1404;
	bool m_unk1405;
	float m_flNextMessageTime;
	float m_flSnapshotInterval;
	float m_unk1501;
	float m_unk1502;

	// AMNOTE: Holds a CNetMessagePB<CSVCMsg_PacketEntities>, which the SDK only declares as an abstract class
	alignas( CNetMessagePB<CSVCMsg_PacketEntities> ) uint8 m_unk1503[sizeof( CNetMessagePB<CSVCMsg_PacketEntities> )];

	CTSQueue<CDelayedCallBase<CServerSideClientBase> *> m_DelayedCalls;
	CUtlVector<CDelayedCallBase<CServerSideClientBase> *> m_unk1601;
	CUtlVector<CDelayedCallBase<CServerSideClientBase> *> m_unk1602;

	// ProcessStringCmd's rate limit: at most m_nMaxCommandsPerWindow commands
	// and m_nMaxBytesPerWindow bytes per m_nMaxTicksPerWindow ticks
	int m_nMaxTicksPerWindow;
	int m_nMaxCommandsPerWindow;
	uint64 m_nMaxBytesPerWindow;
	int m_nWindowStartTick;
	int m_nWindowCommands;
	uint64 m_nWindowBytes;

	NetSpikeTrace_t m_NetSpikeTrace;

	// Failed commands within a second of each other, the client is kicked after 16
	int m_nFailedCommands;
	int m_unk1701;
	double m_flLastFailedCommandTime;

	// AMNOTE: Set when the signon state goes back to connected, a connected signon state message is processed only while it's set
	bool m_unk1801;

	// Values of the client's userinfo convars
	ConVarUserInfoSet_t *m_pConVarUserInfoSet;
};

#endif // SERVERSIDECLIENTBASE_H
