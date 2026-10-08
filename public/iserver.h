//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef ISERVER_H
#define ISERVER_H
#ifdef _WIN32
#pragma once
#endif

#include <edict.h>
#include <resourcefile/resourcetype.h>
#include <tier1/checksum_crc.h>
#include <engine/IEngineService.h>
#include "networksystem/inetworkmessages.h"
#include "icvar.h"
#include <netadr.h>
#include "networksystem/inetworksystem.h"
#include "entity2/entityidentity.h"
#include "playerslot.h"
#include "engine2/inetworkgameserver.h"

class IRecipientFilter;
class ServerClass;
class CGlobalVars;
class IGameSpawnGroupMgr;
struct EventServerAdvanceTick_t;
struct EventServerPollNetworking_t;
struct EventServerProcessNetworking_t;
struct EventServerBeginSimulate_t;
struct EventServerEndSimulate_t;
struct EventServerPostSimulate_t;
struct SpawnGroupDesc_t;
class IPrerequisite;
class CServerChangelevelState;
class ISource2WorldSession;
class INetworkGameClient;
class GameSessionConfiguration_t;
class KeyValues3;
class CSVCMsg_ServerInfo_t;
class CServerSideClientBase;
class C2S_CONNECT_Message;
class CMsgVoiceAudio;
class CSteamID;
class ISceneViewDebugOverlays;
class IEntityReport;
enum SignonState_t : int;

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

abstract_class INetworkServerService : public IEngineService
{
public:
	virtual ~INetworkServerService() {}
	virtual CNetworkGameServerBase	*GetIGameServer( void ) = 0;
	virtual bool	IsActiveInGame( void ) const = 0;
	virtual bool	IsMultiplayer( void ) const = 0;
	virtual void	StartupServer( const GameSessionConfiguration_t &config, ISource2WorldSession *pWorldSession, const char * ) = 0;
	virtual void	SetGameSpawnGroupMgr( IGameSpawnGroupMgr *pMgr ) = 0;
	virtual void	AddServerPrerequisites( const GameSessionConfiguration_t &, const char *, ILoopModePrerequisiteRegistry *, bool ) = 0;
	//virtual void	SetServerSocket( int ) = 0;
	virtual bool	IsServerRunning( void ) const = 0;
	virtual void	DisconnectGameNow( ENetworkDisconnectionReason reason ) = 0;
	virtual void	PrintSpawnGroupStatus( void ) const = 0;
	//virtual int		GetTickInterval( void ) const = 0;
	//virtual void	ProcessSocket( void ) = 0;
	virtual netadr_t GetServerNetworkAddress( void ) = 0;
	virtual bool	GameLoadFailed( void ) const = 0;
	virtual void	SetGameLoadFailed( bool bFailed ) = 0;
	virtual void	SetGameLoadStarted( void ) = 0;
	virtual void	StartChangeLevel( const char *, const char *pszLandmark, void * ) = 0;
	virtual bool	FinishChangeLevel( void ) = 0;
	virtual bool	IsChangelevelPending( void ) const = 0;
	virtual void	PreserveSteamID( void ) = 0;
	virtual CRC32_t	GetServerSerializersCRC( void ) = 0;
	virtual void	*GetServerSerializersMsg( void ) = 0;
	virtual IGameSpawnGroupMgr *GetGameSpawnGroupMgr( void ) = 0;
	virtual bool	IsSaveRestoreAllowed( CUtlString *pReason ) = 0;
	virtual bool	IsEntityReportActive( void ) = 0;
	// Returns nullptr unless the entity report is active and targets this slot or every slot
	virtual IEntityReport *GetEntityReport( int nSlot ) = 0;
	// AMNOTE: Does nothing
	virtual void	unk101( void ) = 0;
	// AMNOTE: Does nothing
	virtual void	unk102( void ) = 0;
	virtual bool	ThreadInPrimaryOrSecondaryMainThread( void ) = 0;
	virtual void	EnterSyncInterval( void ) = 0;
	// AMNOTE: The bool picks which of two parallel work modes clients enter
	virtual void	ExitSyncInterval( bool ) = 0;
};

typedef CNetworkGameServerBase IServer;


#endif // ISERVER_H
