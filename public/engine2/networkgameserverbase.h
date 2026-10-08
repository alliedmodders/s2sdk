//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#ifndef NETWORKGAMESERVERBASE_H
#define NETWORKGAMESERVERBASE_H

#ifdef _WIN32
#pragma once
#endif

#include "engine2/inetworkgameserver.h"
#include <steam/steamclientpublic.h>
#include "server_class.h"
#include "networksystem/inetworksystem.h"
#include "icvar.h"
#include "playerslot.h"

class KeyValues3;
class CServerSideClientBase;
class CSVCMsg_ServerInfo_t;
class C2S_CONNECT_Message;

typedef int ChallengeType_t;
typedef int PauseGroup_t;

abstract_class CNetworkGameServerBase : public INetworkGameServer, protected IConnectionlessPacketHandler, public IConVarListener
{
public:
	virtual ~CNetworkGameServerBase() = 0;

	virtual void	SetMaxClients( int nMaxClients ) = 0;
	
	// AMNOTE: Creates or reuses a client without a connection, returns its slot
	virtual CPlayerSlot CreateClient( CPlayerSlot slot, CSteamID steamID, const char *pszName ) = 0;

	virtual bool	ProcessConnectionlessPacket( const ns_address *addr, bf_read *bf ) = 0; // process a connectionless packet

	virtual void	OnConVarCreated( ConVarRefAbstract *pNewCvar ) = 0;
	virtual void	OnConCommandCreated( ConCommand *pNewCommand ) = 0;

	virtual CPlayerUserId GetPlayerUserId( CPlayerSlot slot ) = 0;
	virtual const char *GetPlayerNetworkIDString( CPlayerSlot slot ) = 0;
	
	// Returns udp port of this server instance
	virtual uint16		GetUDPPort() = 0;
	// Returns hostname of this server instance
	virtual const char *GetHostName() = 0;

	// AMNOTE: arg names are speculative and might be incorrect!
	// Sums up across all the connected players.
	virtual void	GetNetStats( float &inflow, float &outflow ) = 0;

	virtual void	FillKV3ServerInfo( KeyValues3 *out ) = 0;

	virtual bool	IsHLTV() = 0;
	
	virtual bool	IsPausable( PauseGroup_t ) = 0;

	// Returns sv_password cvar value, if it's set to "none" nullptr would be returned!
	virtual const char *GetPassword() = 0;

	virtual void	RemoveClientFromGame(CServerSideClientBase *, /*ENetworkDisconnectionReason*/ int ) = 0;

	virtual void	FillServerInfo( CSVCMsg_ServerInfo_t *pServerInfo ) = 0;
	virtual void	UserInfoChanged( CPlayerSlot slot ) = 0;

	// 2nd arg is unused.
	virtual void	GetClassBaseline( void *, ServerClass *pClass, intp *pOut ) = 0;

	virtual void	StartHLTVMaster() = 0;

	virtual CServerSideClientBase *ConnectClient( const char *pszName, ns_address *pAddr, uint32 steam_handle, C2S_CONNECT_Message *pConnectMsg,
												  const char *pszChallenge, const byte *pAuthTicket, int nAuthTicketLength, bool bIsLowViolence ) = 0;
	virtual CServerSideClientBase *CreateNewClient( CPlayerSlot slot ) = 0;
	
	virtual bool	FinishCertificateCheck( const ns_address *pAddr, int socket, byte ) = 0;

	virtual ChallengeType_t	GetChallengeType( const ns_address &addr ) = 0;
	virtual bool	CheckPassword( const ns_address &addr, const char *password ) = 0;

	virtual void	CalculateCPUUsage() = 0;
};

typedef CNetworkGameServerBase IServer;

#endif // NETWORKGAMESERVERBASE_H
