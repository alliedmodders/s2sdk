//===== Copyright (c) 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose:
//
//===========================================================================//

#ifndef INETWORKSYSTEM_H
#define INETWORKSYSTEM_H
#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"
#include "appframework/IAppSystem.h"
#include "inetchannel.h"
#include "networksystem/inetworkserializer.h"
#include "tier1/bitbuf.h"

class IConnectionlessPacketHandler;

class INetworkConfigChanged;
class INetworkPacketFilter;
class INetworkFileDownloadFilter;
class INetworkFileSendCompleted;
class INetworkPrepareStartupParams;
class IPeerToPeerCallbacks;
class ISteamP2PAllowConnection;
class INetworkChannelNotify;
class NetScratchBuffer_t;
class CMsgSteamDatagramGameServerAuthTicket;
class CUtlStringToken;
class CPeerToPeerAddress;
class ISteamNetworkingUtils;
class ISteamNetworkingSockets;
class ISteamNetworkingMessages;

enum ENSAddressType
{
	kAddressDirect,
	kAddressP2P,
	kAddressProxiedGameServer,
	kAddressProxiedClient,

	kAddressMax
};

class ns_address
{
public:
	const netadr_t &GetAddress() const { return m_Address; }
	const CSteamID& GetSteamID() const { return m_ID; }
	const uint16 GetRemotePort() const { return m_nRemotePort; }
	ENSAddressType GetAddressType() const { return m_AddressType; }
private:
	netadr_t m_Address;
	CSteamID m_ID;
	uint16 m_nRemotePort;
	int m_Unknown;
	ENSAddressType m_AddressType;
};

class IConnectionlessPacketHandler
{
public:
	virtual	~IConnectionlessPacketHandler( void ) {};

	virtual bool ProcessConnectionlessPacket( const ns_address *addr, bf_read *bf ) = 0;	// process a connectionless packet
};

enum
{
	NS_CLIENT = 0,	// client socket
	NS_SERVER,	// server socket
	NS_HLTV,
	NS_P2P,
	MAX_SOCKETS
};

enum ESteamP2PConnectionOwner {};

// Reverse engineered interface: return types may be wrong

abstract_class INetworkSystem : public IAppSystem
{
public:
	virtual void InitGameServer() = 0;
	virtual void ShutdownGameServer() = 0;

	virtual int CreateSocket( int, int, int, int, int, const char * ) = 0;
	virtual bool OpenSocket( int sock ) = 0;
	virtual bool ConnectSocket( int sock, const ns_address &adr ) = 0;
	virtual bool IsSocketOpen( int sock ) = 0;
	virtual void CloseSocket( int sock ) = 0;
	virtual bool ConnectLoopback( int sock1, int sock2 ) = 0;
	virtual void SetDefaultBroadcastPort( int port ) = 0;
	virtual void PollSocket( int sock, IConnectionlessPacketHandler * ) = 0;

	virtual void ProcessSocketMessages( int sock ) = 0;

	virtual INetChannel *CreateNetChannel( int sock, const ns_address *adr, HSteamNetConnection hConn, const char *pszName, NetworkCategoryId nSendCategory, NetworkCategoryId nRecvCategory, bool bPlayback ) = 0;
	// AMNOTE: The bool makes the Steam connection close with an app exception end reason and is passed to INetworkChannelNotify::OnShutdownChannel
	virtual void RemoveNetChannel( INetChannel *netchan, bool ) = 0;
	virtual bool RemoveNetChannelByAddress( int sock, const CPeerToPeerAddress &adr ) = 0;

	virtual void SetTime( double time ) = 0;
	virtual void SetTimeScale( float scale ) = 0;
	virtual double GetNetTime() const = 0;

	virtual const char *DescribeSocket( int sock ) = 0;
	virtual bool IsValidSocket( int sock ) = 0;

	virtual bool BufferToBufferCompress( uint8 *pDest, unsigned int &nDestSize, uint8 *pIn, unsigned int nInSize ) = 0;
	virtual bool BufferToBufferDecompress( uint8 *pDest, unsigned int &nDestSize, uint8 *pIn, unsigned int nInSize ) = 0;

	virtual netadr_t &GetPublicAdr() = 0;
	virtual netadr_t &GetLocalAdr() = 0;
	virtual uint16 GetUDPPort( int sock ) = 0;
	virtual uint16 GetUDPPortWithFallback( int sock ) = 0;

	virtual void AddNetworkChannelNotifyCallback( INetworkChannelNotify *pNotify ) = 0;
	virtual void RemoveNetworkChannelNotifyCallback( INetworkChannelNotify *pNotify ) = 0;

	virtual void CloseAllSockets() = 0;

	virtual NetScratchBuffer_t *GetScratchBuffer( void ) = 0;
	virtual void PutScratchBuffer( NetScratchBuffer_t * ) = 0;

	// Plugins must use these instead of SteamGameServerNetworkingSockets(), the engine may use the game's own networking library rather than steamclient
	// Returns SteamNetworkingUtils004 interface
	virtual ISteamNetworkingUtils *GetSteamNetworkUtils() = 0;

	// Returns User SteamNetworkingSockets012 interface
	virtual ISteamNetworkingSockets *GetSteamUserNetworkingSockets() = 0;

	// Returns GameServer SteamNetworkingSockets012 interface
	virtual ISteamNetworkingSockets *GetSteamGameServerNetworkingSockets() = 0;

	// Returns either User or GameServer SteamNetworkingSockets012 interface
	virtual ISteamNetworkingSockets *GetSteamNetworkingSockets() = 0;

	// Returns SteamNetworkingMessages002 interface
	virtual ISteamNetworkingMessages *GetSteamNetworkingMessages() = 0;

	virtual HSteamNetConnection GetSteamNetConnectionForSocket( int sock ) = 0;
	// AMNOTE: Maps the connection's end reason to a disconnection reason: App/AppException codes minus 1000/2000, Local_*/Remote_* codes to LOCALPROBLEM_*/REMOTE_*, eOldState picks the *_CONNECTING variants
	virtual ENetworkDisconnectionReason unk101( const SteamNetConnectionInfo_t *pInfo, ESteamNetworkingConnectionState eOldState ) = 0;

	// The connection is closed with k_ESteamNetConnectionEnd_AppException_Min + reason
	virtual void RejectConnection( HSteamNetConnection hConn, ENetworkDisconnectionReason reason, const char *pszDebug = nullptr ) = 0;

	virtual void RunCallbacks( bool bGameServer, void *pContext, void *pfnCallback ) = 0;
	// AMNOTE: Calls RunCallbacks with the network system's own connection status handler
	virtual void unk201( bool bGameServer ) = 0;

	virtual void InitSteamNetworking() = 0;

	virtual bool IsNetGraphEnabled() = 0;
	virtual HSteamNetConnection EstablishCacheableSharedNetConnection( const ns_address &adr, SteamNetworkingMicroseconds usecKeepAlive ) = 0;

	virtual ~INetworkSystem() = 0;
};

DECLARE_TIER2_INTERFACE( INetworkSystem, g_pNetworkSystem );

#endif // INETWORKSYSTEM_H
