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
#include "engine2/networkgameserverbase.h"

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



#endif // ISERVER_H
