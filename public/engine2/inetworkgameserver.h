//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#ifndef INETWORKGAMESERVER_H
#define INETWORKGAMESERVER_H

#ifdef _WIN32
#pragma once
#endif

#include "playerslot.h"
#include "tier0/utlstring.h"
#include "engine2/gameshareddefs.h"
#include "tier0/netadr.h"
#include "tier0/utlvector.h"
#include "const.h"
#include "resourcefile/resourcetype.h"

class GameSessionConfiguration_t;
class IGameSpawnGroupMgr;
struct EventServerAdvanceTick_t;
struct EventServerPollNetworking_t;
struct EventServerProcessNetworking_t;
struct EventServerBeginSimulate_t;
struct EventServerEndSimulate_t;
struct EventServerPostSimulate_t;
struct SpawnGroupDesc_t;
class CServerChangelevelState;
class ISceneViewDebugOverlays;
class INetworkMessageInternal;
class CNetMessage;
class IRecipientFilter;
class CSteamID;
class CMsgVoiceAudio;
enum SignonState_t : int;

enum server_state_t : int
{
	SS_Dead = 0,
	SS_WaitingForGameSessionManifest,
	SS_Loading,
	SS_Active,
	SS_Paused,
};

abstract_class INetworkGameServer 
{
public:
	virtual	void	Init( const GameSessionConfiguration_t &, const char * ) = 0;

	virtual void	SetGameSpawnGroupMgr( IGameSpawnGroupMgr * ) = 0;
	virtual void	SetGameSessionManifest( HGameResourceManifest ) = 0;

	virtual void	RegisterLoadingSpawnGroups( CUtlVector<unsigned int> & ) = 0;

	virtual void	Shutdown( void ) = 0;
	virtual void	AddRef( void ) = 0;
	virtual void	Release ( void ) = 0;

	virtual CGlobalVars *GetGlobals(void) = 0;

	virtual bool	IsActive( void ) const = 0;	
	virtual bool	IsPaused( void ) const = 0;	

	virtual void	SetServerTick( int tick ) = 0;
	// returns game world tick
	virtual int		GetServerTick( void ) const = 0;

	// returns current client limit
	virtual int		GetMaxClients( void ) const = 0;

	virtual void	ServerAdvanceTick( const EventServerAdvanceTick_t & ) = 0;
	virtual void	ServerPollNetworking( const EventServerPollNetworking_t & ) = 0;
	virtual void	ServerProcessNetworking( const EventServerProcessNetworking_t & ) = 0;

	virtual void	ServerBeginSimulate( const EventServerBeginSimulate_t & ) = 0;
	virtual void	ServerEndSimulate( const EventServerEndSimulate_t & ) = 0;
	virtual void	ServerPostSimulate( const EventServerPostSimulate_t & ) = 0;

	virtual SpawnGroupHandle_t LoadSpawnGroup( const SpawnGroupDesc_t & ) = 0;
	virtual void	AsyncUnloadSpawnGroup( unsigned int, /*ESpawnGroupUnloadOption*/ int ) = 0;
	virtual void	PrintSpawnGroupStatus( void ) const = 0;

	// returns the game time scale (multiplied in conjunction with host_timescale)
	virtual float	GetTimescale( void ) const = 0;

	virtual bool	IsSaveRestoreAllowed( CUtlString *pReason ) const = 0;

	virtual void	SetMapName( const char *pszNewName ) = 0;
	// current map name (BSP)
	virtual const char *GetMapName( void ) const = 0;
	virtual const char *GetAddonName( void ) const = 0;

	virtual bool	IsBackgroundMap( void ) const = 0;

	// returns game world time (GetServerTick() * tick_interval)
	virtual float	GetTime( void ) const = 0;

	virtual bool	ActivateServer( void ) = 0;
	virtual bool	PrepareForAssetLoad( void ) = 0;

	virtual netadr_t GetServerNetworkAddress( void ) = 0;

	virtual SpawnGroupHandle_t FindSpawnGroupByName( const char *pszName ) = 0;
	virtual void	MakeSpawnGroupActive( SpawnGroupHandle_t ) = 0;
	virtual void	SynchronouslySpawnGroup( SpawnGroupHandle_t ) = 0;

	virtual void	SetServerState( server_state_t eNewState ) = 0;
	virtual server_state_t GetServerState( void ) = 0;
	virtual bool	SpawnServer( const char * ) = 0;

	virtual int 	GetSpawnGroupLoadingStatus( SpawnGroupHandle_t ) = 0;
	virtual void	SetSpawnGroupDescription( SpawnGroupHandle_t, const char * ) = 0;

	virtual CServerChangelevelState *StartChangeLevel( const char *pszMap, const char *pszLandmark, void * ) = 0;
	virtual void	FinishChangeLevel( CServerChangelevelState * ) = 0;
	virtual bool	IsChangelevelPending( void ) const = 0;

	virtual void	GetAllLoadingSpawnGroups( CUtlVector<SpawnGroupHandle_t> *pOut ) = 0;

	virtual void	PreserveSteamID( void ) = 0;

	virtual GameSessionConfiguration_t *GetGameSessionConfig() = 0;

	virtual void	ReserveServerForQueuedGame( const char *pszReason ) = 0;

	virtual bool	IsReserved() = 0;
	// is_multiplayer of the GameSessionConfiguration_t the server was started with
	virtual bool	IsMultiplayer() = 0;
	virtual bool	IsPlayingSoloAgainstBots() = 0;

	virtual void	BroadcastPrintf( const char *pszFmt, ... ) FMTFUNCTION( 2, 3 ) = 0;

	// AMNOTE: Has no effect on clients made by CreateClient, which lock their slot
	virtual void	SetRecyclePlayerSlot( CPlayerSlot slot, bool bRecycle ) = 0;
	virtual SignonState_t GetClientSignonState( CPlayerSlot slot ) = 0;
	// Adds the server to the overlays' listeners to broadcast what they draw, or removes it
	virtual void	SetBroadcastDebugOverlays( ISceneViewDebugOverlays *pOverlays, bool bBroadcast ) = 0;

	virtual void	BroadcastMessage( INetworkMessageInternal *pNetMessage, const CNetMessage *pData, IRecipientFilter *filter ) = 0;
	virtual bool	IsRecordingDemo() = 0;

	virtual uint8	GetClientConnectionType( CPlayerSlot slot ) = 0;
	virtual bool	HasReplayDirector() = 0;
	virtual float	GetAverageFrameTime() = 0;

	virtual void	PrepareSendClientUpdatesMainThread() = 0;
	virtual void 	PrepareSendClientUpdatesAsync() = 0;

	virtual CSteamID GetGameServerSteamID() = 0;
	virtual void	BroadcastEntityVoice( int entity, CMsgVoiceAudio *data, uint64 xuid ) = 0;
};

#endif // INETWORKGAMESERVER_H
