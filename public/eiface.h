//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose:
//
// $NoKeywords: $
//
//===========================================================================//

#ifndef EIFACE_H
#define EIFACE_H

#ifdef _WIN32
#pragma once
#endif

#include "tier1/convar.h"
#include "icvar.h"
#include "edict.h"
#include "globalvars.h"
#include "mathlib/vplane.h"
#include "soundflags.h"
#include "bitvec.h"
#include "tier1/bitbuf.h"
#include "tier1/utlmap.h"
#include "tier1/utlstring.h"
#include "tier1/bufferstring.h"
#include <steam/steamclientpublic.h>
#include "playerslot.h"
#include <iloopmode.h>
#include "network_connection.pb.h"
#include "entity2/entityidentity.h"
#include "checktransmitinfo.h"
#include "engine2/iscreenshotcallback.h"
#include "networksystem/inetworksystem.h"
#include "resourcefile/resourcetype.h"

//-----------------------------------------------------------------------------
// forward declarations
//-----------------------------------------------------------------------------
class	ServerClass;
class	CGameTrace;
typedef	CGameTrace trace_t;
struct	typedescription_t;
class	CSaveRestoreData;
struct	datamap_t;
struct	studiohdr_t;
class	CBaseEntity;
class	CRestore;
class	CSave;
struct	vcollide_t;
class	IRecipientFilter;
class	CBaseEntity;
class	ITraceFilter;
class	INetChannelInfo;
class	ISpatialPartition;
class IScratchPad3D;
class CStandardSendProxies;
class IAchievementMgr;
class CGamestatsData;
class CSteamID;
class ISPSharedMemory;
class CGamestatsData;
class CEngineHltvInfo_t;
class INetworkStringTable;
class CEntityLump;
class IPVS;
class IHLTVDirector;
struct SpawnGroupDesc_t;
class IClassnameForMapClassCallback;
struct Entity2Networkable_t;
class CCreateGameServerLoadInfo;
class INavListener;
class CNavData;
class CEntityHandle;
struct RenderDeviceInfo_t;

enum RenderMultisampleType_t : uint8
{
	RENDER_MULTISAMPLE_INVALID = 0xFF,
	RENDER_MULTISAMPLE_NONE = 0,
	RENDER_MULTISAMPLE_2X,
	RENDER_MULTISAMPLE_4X,
	RENDER_MULTISAMPLE_6X,
	RENDER_MULTISAMPLE_8X,
	RENDER_MULTISAMPLE_16X,
	RENDER_MULTISAMPLE_TYPE_COUNT
};

class GameSessionConfiguration_t;
struct StringTableDef_t;
class ILoopModePrerequisiteRegistry;
struct URLArgument_t;
struct vis_info_t;
class IHLTVServer;
class CCompressedResourceManifest;
class ILoadingSpawnGroup;
class KeyValues3;
struct SaveGameParams_t;
class IToolGameSimulationAPI;
class CCLCMsg_Move;
template <typename T>
class CNetMessagePB;
class CCLCMsg_Diagnostic;
class CUtlBuffer;
class ISceneViewDebugOverlays;
class CEntityClass;
class CSVCMsg_UserCommands;
class INetChannel;
class INetworkGameServer;
struct FlattenedSerializerSpewField_t;
class INetworkMessageInternal;
class CNetMessage;
struct NetMessageInfo_t;
class CGameInfo;
enum SignonState_t : int;

namespace google
{
	namespace protobuf
	{
		class Message;
	}
}

//-----------------------------------------------------------------------------
// defines
//-----------------------------------------------------------------------------

#ifdef _WIN32
#define DLLEXPORT __stdcall
#else
#define DLLEXPORT /* */
#endif

#define INTERFACEVERSION_VENGINESERVER	"Source2EngineToServer001"

struct bbox_t
{
	Vector mins;
	Vector maxs;
};

class CPlayerUserId
{
public:
	CPlayerUserId( int index )
	{
		_index = index;
	}

	int Get() const
	{
		return _index;
	}

	bool operator==( const CPlayerUserId &other ) const { return other._index == _index; }
	bool operator!=( const CPlayerUserId &other ) const { return other._index != _index; }

private:
	unsigned short _index;
};

// Times are in seconds, relative to the current time
struct HltvReplayParams_t
{
	int m_nPrimaryTargetEntIndex = -1;
	float m_flDelay = 0.0f;
	float m_flStopAt = 0.0f;
	float m_flPlaybackSpeed = 1.0f;
	float m_flSlowdownBeginAt = 0.0f;
	float m_flSlowdownEndAt = 0.0f;
	float m_flSlowdownRate = 1.0f;
	bool m_bAbortCurrentReplay = false;
	int m_nReason = 0;
	// AMNOTE: Flags: 1 = replay the stash m_nStashId, 2 = replay all of the stash instead of its last m_flDelay seconds
	uint32 m_unk101 = 0;
	uint32 m_nStashId = 0;
};

//-----------------------------------------------------------------------------
// Purpose: Interface the engine exposes to the game DLL and client DLL
//-----------------------------------------------------------------------------
abstract_class ISource2Engine : public IAppSystem
{
public:
	// Is the game paused?
	virtual bool		IsPaused() = 0;

	// What is the game timescale multiplied with the host_timescale?
	virtual float		GetTimescale( void ) const = 0;

	virtual void		*FindOrCreateWorldSession( const char *pszWorldName, const char *, void * ) = 0;

	virtual CEntityLump	*GetEntityLumpForTemplate( const char *, bool, const char *, const char *, bool ) = 0;

	virtual uint32		GetStatsAppID() const = 0;

	virtual void		WriteJpegScreenshot( const char *pszFilename, int nQuality, int nWidth, int nHeight, bool bAsyncWrite ) = 0;
	virtual void		RequestScreenshot( IScreenshotCallback *pCallback, void *pContext ) = 0;
};

//-----------------------------------------------------------------------------
// Purpose: Interface the engine exposes to the game DLL
//-----------------------------------------------------------------------------
abstract_class IVEngineServer2 : public ISource2Engine
{
public:
	virtual EUniverse	GetSteamUniverse() const = 0;

	virtual WorldGroupId_t	FindWorldGroupByName( const char *pszName ) = 0;
	virtual const char	*GetWorldGroupName( WorldGroupId_t hWorldGroup ) = 0;
	virtual WorldGroupId_t	GetFirstWorldGroupId( bool bClient ) = 0;
	virtual WorldGroupId_t	GetLastWorldGroupId( bool bClient ) = 0;
	virtual bool		IsWorldGroupValid( WorldGroupId_t hWorldGroup ) = 0;

	// Points to two floats for the last main loop iteration: the time it took before running its frame, and the frame's time
	virtual float		*GetMainLoopTimes() = 0;

	virtual void		SetFrameTimeAmnesty( const char *amnesty, int, float frametime ) = 0;
	virtual const char *GetFrameTimeAmnesty( bool check_cvar ) = 0;

	virtual void		SetFramePerformanceTag( const char *pszTag, int nValue, int, float flDuration ) = 0;

	virtual void		ShowFrameTimeReport( void *, bool, LoggingChannelID_t channel = -1 ) = 0;

	virtual void		DumpNetStats( void *, void (*pfnPrint)( const char * ) ) = 0;

	virtual bool		ShouldForceMaxFrametimeToTickInterval() = 0;
	virtual uint32		GetLongFrameCount() = 0;

	// Tell engine to change level ( "changelevel s1\n" or "changelevel2 s1 s2\n" )
	virtual void		ChangeLevel( const char *s1, const char *s2 ) = 0;

	// Ask engine whether the specified map is a valid map file (exists and has valid version number).
	virtual int			IsMapValid( const char *filename ) = 0;

	// Is this a dedicated server?
	virtual bool		IsDedicatedServer( void ) = 0;

	// Is this an HLTV relay?
	virtual bool		IsHLTVRelay( void ) = 0;

	// Is server only accepting local connections?
	virtual bool		IsServerLocalOnly( void ) = 0;

	virtual int			PrecacheGeneric( const char *s, bool preload = false ) = 0;
	virtual bool		IsGenericPrecached( char const *s ) const = 0;

	// Returns the server assigned userid for this player.  Useful for logging frags, etc.
	//  returns -1 if the edict couldn't be found in the list of players.
	virtual CPlayerUserId GetPlayerUserId( CPlayerSlot nSlot ) = 0;
	virtual const char	*GetPlayerNetworkIDString( CPlayerSlot nSlot ) = 0;
	// Get stats info interface for a client netchannel
	virtual INetChannelInfo* GetPlayerNetInfo( CPlayerSlot nSlot ) = 0;

	// AMNOTE: Returns a pointer into the client's frame for its acknowledged delta tick, or nullptr
	virtual void		*unk101( CPlayerSlot nSlot ) = 0;
	// Returns -1 for an invalid slot
	virtual int			GetClientDeltaTick( CPlayerSlot nSlot ) = 0;

	// Given the current PVS(or PAS) and origin, determine which players should hear/receive the message
	virtual void		Message_DetermineMulticastRecipients( bool usepas, const Vector& origin, CPlayerBitVec& playerbits ) = 0;

	// Issue a command to the command parser as if it was typed at the server console.
	virtual void		ServerCommand( const char *str ) = 0;
	// Issue the specified command to the specified client (mimics that client typing the command at the console).
	virtual void		ClientCommand( CPlayerSlot nSlot, const char *szFmt, ... ) FMTFUNCTION( 3, 4 ) = 0;

	// Print szMsg to the client console.
	virtual void		ClientPrintf( CPlayerSlot nSlot, const char *szMsg ) = 0;

	virtual bool		IsLowViolence() = 0;
	virtual void		SetHLTVChatBan( const CSteamID &steamID, bool bBanned ) = 0;
	virtual bool		IsAnyClientLowViolence() = 0;

	// Get the current game directory (hl2, tf2, hl1, cstrike, etc.)
	virtual void        GetGameDir( CBufferString &gameDir ) = 0;

	// Create a bot with the given name.  Player index is -1 if fake client can't be created
	virtual CPlayerSlot	CreateFakeClient( const char *netname ) = 0;

	// Get a convar keyvalue for s specified client
	virtual const char	*GetClientConVarValue( CPlayerSlot nSlot, const char *name ) = 0;

	// Print a message to the server log file
	virtual void		LogPrint( const char *msg ) = 0;
	virtual bool		IsLogEnabled() = 0;

	virtual bool IsSplitScreenPlayer( CPlayerSlot nSlot ) = 0;
	virtual CPlayerSlot GetSplitScreenPlayerAttachToEdict( CPlayerSlot nSlot ) = 0;
	virtual CPlayerSlot GetSplitScreenPlayerForEdict( CPlayerSlot nSlot, int nSplitScreenSlot ) = 0;

	// Ret types might be all wrong for these. Haven't researched yet.
	virtual void	UnloadSpawnGroup( SpawnGroupHandle_t spawnGroup, /*ESpawnGroupUnloadOption*/ int) = 0;
	virtual SpawnGroupHandle_t LoadSpawnGroup( const SpawnGroupDesc_t & ) = 0;
	virtual void	SetSpawnGroupDescription( SpawnGroupHandle_t spawnGroup, const char *pszDescription ) = 0;
	virtual bool	IsSpawnGroupLoaded( SpawnGroupHandle_t spawnGroup ) const = 0;
	virtual bool	IsSpawnGroupLoading( SpawnGroupHandle_t spawnGroup ) const = 0;
	virtual void	MakeSpawnGroupActive( SpawnGroupHandle_t spawnGroup ) = 0;
	virtual void	SynchronouslySpawnGroup( SpawnGroupHandle_t spawnGroup ) = 0;
	virtual void	SynchronizeAndBlockUntilLoaded( SpawnGroupHandle_t spawnGroup ) = 0;

	virtual void SetTimescale( float flTimescale ) = 0;

	virtual uint32		GetAppID() = 0;

	// Returns the SteamID of the specified player. It'll be NULL if the player hasn't authenticated yet.
	virtual const CSteamID	*GetClientSteamID( CPlayerSlot nSlot ) = 0;

	// Methods to set/get a gamestats data container so client & server running in same process can send combined data
	virtual void SetGamestatsData( CGamestatsData *pGamestatsData ) = 0;
	virtual CGamestatsData *GetGamestatsData() = 0;

	// Send a client command keyvalues
	// keyvalues are deleted inside the function
	virtual void ClientCommandKeyValues( CPlayerSlot nSlot, KeyValues *pCommand ) = 0;

	// This makes the host run 1 tick per frame instead of checking the system timer to see how many ticks to run in a certain frame.
	// i.e. it does the same thing timedemo does.
	virtual void SetDedicatedServerBenchmarkMode( bool bBenchmarkMode ) = 0;

	// Returns true if this client has been fully authenticated by Steam
	virtual bool IsClientFullyAuthenticated( CPlayerSlot nSlot ) = 0;

	virtual CGlobalVars	*GetServerGlobals() = 0;

	// Sets a USERINFO client ConVar for a fakeclient
	virtual void		SetFakeClientConVarValue( CPlayerSlot nSlot, const char *cvar, const char *value ) = 0;

	virtual CSharedEdictChangeInfo* GetSharedEdictChangeInfo() = 0;

	virtual void SetAchievementMgr( IAchievementMgr *pAchievementMgr ) =0;
	virtual IAchievementMgr *GetAchievementMgr() = 0;

	// Fill in the player info structure for the specified player index (name, model, etc.)
	virtual bool GetPlayerInfo( CPlayerSlot nSlot, google::protobuf::Message &info ) = 0;

	// Returns the XUID of the specified player. It'll be NULL if the player hasn't connected yet.
	virtual uint64 GetClientXUID( CPlayerSlot nSlot ) = 0;

	virtual void				*GetPVSForSpawnGroup( SpawnGroupHandle_t spawnGroup ) = 0;
	virtual SpawnGroupHandle_t	FindSpawnGroupByName( const char *szName ) = 0;

	// Returns the SteamID of the game server
	virtual CSteamID	GetGameServerSteamID() = 0;

	virtual int GetBuildVersion( void ) const = 0;

	virtual bool IsClientLowViolence( CPlayerSlot nSlot ) = 0;

	// Kicks the slot with the specified NetworkDisconnectionReason
	virtual void DisconnectClient( CPlayerSlot nSlot, ENetworkDisconnectionReason reason, const char *szInternalReason = nullptr ) = 0;

	virtual void DisconnectAllClients( ENetworkDisconnectionReason reason ) = 0;
	virtual void GetAllSpawnGroupsWithPVS( CUtlVector<SpawnGroupHandle_t> *spawnGroups, CUtlVector<IPVS *> *pOut ) = 0;

	// Use these to setup who can hear whose voice.
	// Pass in client indices (which are their ent indices - 1).
	virtual bool GetClientListening(CPlayerSlot iReceiver, CPlayerSlot iSender) = 0;
	virtual bool SetClientListening(CPlayerSlot iReceiver, CPlayerSlot iSender, bool bListen) = 0;
	virtual bool SetClientProximity(CPlayerSlot iReceiver, CPlayerSlot iSender, bool bUseProximity) = 0;

	// AMNOTE: Creates or reuses a client without a connection, returns its slot
	virtual CPlayerSlot CreateClient( CPlayerSlot nSlot, CSteamID steamID, const char *pszName ) = 0;
	// AMNOTE: Sets a client state to !bool unless it is 2 or higher, which CreateClient sets
	virtual void unk201( CPlayerSlot nSlot, bool ) = 0;
	virtual SignonState_t GetClientSignonState( CPlayerSlot nSlot ) = 0;

	virtual void KickClient( CPlayerSlot nSlot, const char *szInternalReason, ENetworkDisconnectionReason reason ) = 0;
	virtual void BanClient( CPlayerSlot nSlot, float flDuration, bool bKick ) = 0;
	virtual void BanClient( CSteamID steamId, float flDuration, bool bKick ) = 0;

	virtual bool StartHltvReplay( CPlayerSlot nSlot, const HltvReplayParams_t &params ) = 0;
	virtual void ForceStopHltvReplay( CPlayerSlot nSlot ) = 0;
	virtual void StopAllHltvReplays() = 0;
	virtual int GetClientHltvReplayDelay( CPlayerSlot nSlot ) = 0;
	virtual bool IsHltvReplayBufferAvailable() = 0;
	virtual bool CanStartHltvReplay( CPlayerSlot nSlot, int nDelay ) = 0;
	virtual void ResetHltvReplayRequestTime( CPlayerSlot nSlot ) = 0;
	virtual bool IsAnyHltvReplayActive() = 0;

	virtual void SetClientUpdateRate( CPlayerSlot nSlot, float flUpdateRate ) = 0;

	virtual void SetClientJitterBadThresholdUp( CPlayerSlot nSlot, float flThreshold ) = 0;
	virtual bool StashHltvReplay( uint32 nStashId, float flSeconds ) = 0;
	virtual void AddHltvRelayProxyWhitelist( uint32 a, uint32 b, uint32 c, uint32 d, uint32 numbits ) = 0;
	virtual bool WasShutDownRequested() const = 0;
	// AMNOTE: Extends the queued matchmaking reservation timeout and sends the data in a connectionless packet to every reservation entry, or only to the entry matching the int when it is non-zero
	virtual void unk301( int, uint8, uint8, uint32 nDataSize, const void *pData ) = 0;
	virtual bool IsTvRecording() = 0;
	virtual void StartAutoRecording() = 0;
	virtual void RecordDemo( const char *pszFilename ) = 0;
	virtual void StopRecordingDemo( const CGameInfo *pGameInfo ) = 0;
	virtual void BroadcastEvent( INetworkMessageInternal *pEvent, const CNetMessage *pData ) = 0;
	// Returns an empty string when not recording
	virtual const char *GetTvRecordingDemoFilename() = 0;
	virtual const char *GetMapName() = 0;
	virtual ConVarUserInfoSet_t GetClientUserInfo( CPlayerSlot nSlot ) = 0;
	virtual bool IsRecordingDemo() = 0;
};

abstract_class IServerGCLobby
{
public:
	virtual bool HasLobby() const = 0;
	virtual bool SteamIDAllowedToConnect( const CSteamID &steamId ) const = 0;
	virtual void UpdateServerDetails( void ) = 0;
	virtual bool ShouldHibernate() = 0;
	virtual bool SteamIDAllowedToP2PConnect( const CSteamID &steamId ) const = 0;
	virtual bool LobbyAllowsCheats( void ) const = 0;
};

#define INTERFACEVERSION_SERVERGAMEDLL				"Source2Server001"

//-----------------------------------------------------------------------------
// Purpose: These are the interfaces that the game .dll exposes to the engine
//-----------------------------------------------------------------------------
abstract_class ISource2Server : public IAppSystem
{
public:
	virtual bool			IsValveDS() const = 0;

	// Returns the previous globals, or nullptr if they were the defaults
	virtual CGlobalVars		*SetGlobals( CGlobalVars *pGlobals ) = 0;

	// Let the game .dll allocate it's own network/shared string tables
	virtual void			GameCreateNetworkStringTables( void ) = 0;

	virtual void			WriteSignonMessages( const bf_write &buf ) = 0;

	virtual void			PreWorldUpdate( bool simulating ) = 0;

	virtual CUtlOrderedMap<int, Entity2Networkable_t, CDefLess<int>, unsigned short>	*GetEntity2Networkables( void ) const = 0;

	virtual bool			GetEntity2Networkable( CEntityIndex nIndex, Entity2Networkable_t *pOut ) = 0;

	// Called to apply lobby settings to a dedicated server
	virtual void			ApplyGameSettings( KeyValues *pKV ) = 0;

	// The server should run physics/think on all edicts
	// One of these bools is 'simulating'... probably
	virtual void			GameFrame( bool simulating, bool bFirstTick, bool bLastTick ) = 0;

	// Returns true if the game DLL wants the server not to be made public.
	// Used by commentary system to hide multiplayer commentary servers from the master.
	virtual bool			ShouldHideFromMasterServer( bool bServerHasPassword ) = 0;

	virtual void			GetMatchmakingTags( CBufferString &buf ) = 0;

	virtual void			ServerHibernationUpdate( bool bHibernating ) = 0;

	virtual IServerGCLobby	*GetServerGCLobby() = 0;

	virtual void			GetMatchmakingGameData( CBufferString &buf ) = 0;

	// return true to disconnect client due to timeout (used to do stricter timeouts when the game is sure the client isn't loading a map)
	virtual bool			ShouldTimeoutClient( int nUserID, float flTimeSinceLastReceived ) = 0;

	virtual void			PrintStatus( CEntityIndex nPlayerEntityIndex, CBufferString &output ) = 0;

	virtual int				GetServerGameDLLFlags( void ) const = 0;

	// Get the list of cvars that require tags to show differently in the server browser
	virtual void			GetTaggedConVarList( KeyValues *pCvarTagList ) = 0;

	// Give the list of datatable classes to the engine.  The engine matches class names from here with
	//  edict_t::classname to figure out how to encode a class's data for networking
	virtual CUtlVector<ServerClass*> *GetAllServerClasses( void ) = 0;

	virtual const char		*GetActiveWorldName( void ) const = 0;

	virtual bool			IsPaused( void ) const = 0;

	virtual bool			GetNavMeshData( CNavData *pNavMeshData ) = 0;
	virtual void			SetNavMeshData( const CNavData *navMeshData ) = 0;
	virtual void			RegisterNavListener( INavListener *pNavListener ) = 0;
	virtual void			UnregisterNavListener( INavListener *pNavListener ) = 0;
	virtual bool			GetBugReportAttachment( int nIndex, CUtlBuffer &buf, CUtlString &sName, CUtlString &sDescription ) = 0;

	virtual IToolGameSimulationAPI *GetToolGameSimulationAPI( void ) = 0;
	virtual void			GetAnimationActivityList( CUtlVector<CUtlString> &activityList ) = 0;
	virtual void			GetAnimationEventList( CUtlVector<CUtlString> &eventList ) = 0;
	virtual void			FilterPlayerCounts( int *pInOutHumans, int *pInOutHumansSlots, int *pInOutBots ) = 0;

	// Called after the steam API has been activated post-level startup
	virtual void			GameServerSteamAPIActivated( void ) = 0;

	virtual void			GameServerSteamAPIDeactivated( void ) = 0;

	virtual void			OnHostNameChanged( const char *pHostname ) = 0;
	virtual void			PreFatalShutdown( void ) const = 0;
	virtual void			UpdateWhenNotInGame( float flFrameTime ) = 0;

	virtual void			*GetEconItemSystem( void ) = 0;

	virtual void			ServerConVarChanged( const char *pVarName, const char *pValue ) = 0;

	// Returns a list of values and names corresponding to HitGroup_t enum
	virtual void			GetHitGroupEnumInfo( CUtlVector<int> &values, CUtlVector<CUtlString> &names ) = 0;

	// Adds the game's fields to the server section of the status_json output
	virtual void			WriteStatusJson( KeyValues3 *pServer ) = 0;

	virtual bool			SaveGame_CalcFileName( const char *pszSaveName, CUtlString &fileName ) = 0;
	virtual bool			GetRequiredAddonsFromSaveFile( const char *pszSaveName, CUtlString &requiredAddons ) = 0;
	virtual void			GetLevelsFromSaveFile( const char *pszSaveName, CUtlVector<CCreateGameServerLoadInfo> &levels, bool bWipeAndExtract, int, CUtlString *pComment ) = 0;
	virtual void			ClearSaveDirectory( void ) = 0;
	virtual void			PreSaveGameLoaded( const char *pszSaveName ) = 0;
	virtual void			AppendSaveGameResources( HGameResourceManifest hManifest, ILoadingSpawnGroup *pLoadingSpawnGroup, SpawnGroupHandle_t hSpawnGroup, const void * ) = 0;
	virtual void			AppendTransitionResources( HGameResourceManifest hManifest, ILoadingSpawnGroup *pLoadingSpawnGroup, SpawnGroupHandle_t hSpawnGroup, const void * ) = 0;
	virtual /*SaveGameResult_t*/ int SaveGame( const SaveGameParams_t &params ) = 0;
	virtual bool			IsAsyncSaveInProgress( void ) = 0;
	virtual bool			ProcessPendingSaveRequest( void ) = 0;
	virtual bool			HasPendingSaveRequest( void ) = 0;
	virtual void			FinishAsyncSave( void ) = 0;

	virtual const char		*GetEntityUniqueHammerID( CEntityIndex nEntityIndex ) = 0;

	// AMNOTE: A scope is one subclass VData file, matched by its file path or, in the ByDataType variants,
	// by its generic data type. A null or empty scope matches any scope.
	virtual const char		*GetVDataClassName( const char *pszName, const char *pszScopeFile ) = 0;
	virtual const char		*GetVDataClassNameByDataType( const char *pszName, const char *pszGenericDataType ) = 0;
	virtual const CUtlVector<CUtlString> &GetSubclassNamesInScope( const char *pszScopeFile ) = 0;
	virtual const CUtlVector<CUtlString> &GetSubclassNamesInScopeByDataType( const char *pszGenericDataType ) = 0;
	virtual void			GetAllSubclassNames( CUtlVector<CUtlString> &names ) = 0;
	virtual const char		*GetSubclassDesignerName( const char *pszSubclassName ) = 0;

	virtual void			UpdateGCInformation( bool, bool ) = 0;

	virtual const CUtlVector<CUtlString> &GetDesignerNamesForClass( const char *pszClassName ) = 0;
	// AMNOTE: Does nothing in CS2
	virtual void			unk_101( void *, void * ) = 0;

	virtual bool			ReportGCQueuedMatchStart( int32 iReservationStage, uint32 *puiConfirmedAccounts, int numConfirmedAccounts ) = 0;

	// AMNOTE: Forwards to the game rules, which do nothing with it in CS2
	virtual void			unk_201( void * ) = 0;
	virtual ISceneViewDebugOverlays *GetDebugOverlays( void ) = 0;

	virtual const char		*GetNativeClassForScriptClass( const char *pszScriptClassName ) = 0;
	virtual CEntityClass	*GetScriptClassForDesignerName( const char *pszDesignerName ) = 0;
	virtual bool			IsScriptClassDerivedFrom( const char *pszDesignerName, const char *pszBaseName ) = 0;

	virtual bool			ShouldHoldGameServerReservation( float flTimeElapsedWithoutClients ) = 0;

	virtual void			OnBroadcastRelayRequestSucceeded( void * ) = 0;
	virtual void			SendServerFrameTime( float flFrameTime ) = 0;
	virtual void			OnClientHltvReplayStart( CPlayerSlot slot, int ) = 0;
	virtual void			OnClientHltvReplayStop( CPlayerSlot slot ) = 0;

	virtual bool			FormatSerializerFieldValue( CEntityIndex nEntityIndex, FlattenedSerializerSpewField_t &field ) = 0;

	virtual void			CaptureUserCommands( void ) = 0;
	virtual CNetMessagePB<CSVCMsg_UserCommands> *CreateUserCommandsMessage( void ) = 0;

	virtual bool			ProcessClientStringCommand( CPlayerSlot slot, const CCommand &args, uint32 nPredictionSync ) = 0;
	virtual void			OnPreMatchInterfaceCommand( uint32 uiAccountID, int, const char *pszCommand ) = 0;
	virtual void			SetPlayerTeammatePreferredColor( uint32 uiAccountID, int nColor ) = 0;
	virtual void			UpdateCompTeammateColors( void ) = 0;

	// A reason other than NETWORK_DISCONNECT_INVALID rejects the connecting client
	virtual ENetworkDisconnectionReason GetClientConnectRejectReason( const CSteamID &steamID ) = 0;
	virtual const char		*ClientConnectionValidatePreNetChan( const CSteamID &steamID, const char *pszPlayerName ) = 0;

	virtual bool			LogForHTTPListeners( const char *pszLogLine ) = 0;

	// AMNOTE: These four do nothing in CS2
	virtual void			unk_301( void ) = 0;
	virtual void			unk_302( void ) = 0;
	virtual void			unk_303( void ) = 0;
	virtual void			unk_304( void ) = 0;

	virtual void			RegisterClientNetMessageHandlers( INetChannel *pNetChannel, CPlayerSlot slot, int nAction ) = 0;
	virtual bool			GetAddonForMap( const char *pszMapName, CUtlString &addonName ) = 0;
	virtual uint64			GetMatchID( void ) = 0;
	virtual void			OnSteamAuthWarning( const char *pszMessage ) = 0;
	virtual void			OnNetworkGameServerActivated( INetworkGameServer *pNetworkGameServer ) = 0;
	virtual uint32			GetSteamGroupAccountID( void ) = 0;

#ifdef PLATFORM_LINUX
	// AMNOTE: Does nothing in CS2
	virtual void			unk_401( void ) = 0;
#endif
};

//-----------------------------------------------------------------------------
// Just an interface version name for the random number interface
// See tier1/random.h for the interface definition
// NOTE: If you change this, also change VENGINE_CLIENT_RANDOM_INTERFACE_VERSION in cdll_int.h
//-----------------------------------------------------------------------------
#define VENGINE_SERVER_RANDOM_INTERFACE_VERSION	"VEngineRandom001"

#define INTERFACEVERSION_SERVERGAMEENTS			"Source2GameEntities001"
//-----------------------------------------------------------------------------
// Purpose: Interface to get at server entities
//-----------------------------------------------------------------------------
abstract_class ISource2GameEntities : public IAppSystem
{
public:
	virtual					~ISource2GameEntities() = 0;

	// This sets a bit in pInfo for each edict in the list that wants to be transmitted to the
	// client specified in pInfo.
	//
	// This is also where an entity can force other entities to be transmitted if it refers to them
	// with ehandles.
	virtual void			CheckTransmit( CCheckTransmitInfo **pInfoInfoList, int nInfoCount, CBitVec<16384> &unionTransmitEdicts,
										   CBitVec<16384> &, const Entity2Networkable_t **pNetworkables,
										   const uint16 *pEntityIndicies, int nEntityIndices ) = 0;
	
	// TERROR: Perform any PVS cleanup before a full update
	virtual void			PrepareForFullUpdate( CEntityIndex nPlayerEntityIndex ) = 0;

	virtual bool			ShouldClientReceiveStringTableUserData( const INetworkStringTable *pTable, int stringNumber, const CCheckTransmitInfo *pInfo ) = 0;
	
	virtual void			ResetChangeAccessorsSerialNumbersToZero() = 0;
	
	virtual bool			GetWorldspaceCenter( CEntityIndex nEntityIndex, Vector *pCenter ) const = 0;

	virtual void			OnPrePackEntities( const CUtlVector<Entity2Networkable_t *> &ents ) const = 0;
};

#define INTERFACEVERSION_SERVERCONFIG			"Source2ServerConfig001"

abstract_class ISource2ServerConfig : public IAppSystem
{
public:
	// Returns string describing current .dll.  e.g., TeamFortress 2, Half-Life 2.
	//  Hey, it's more descriptive than just the name of the game directory
	virtual const char *GetGameDescription( void ) = 0;

	virtual int			GetNetworkVersion( void ) = 0;

	// Get server maxplayers and lower bound for same
	virtual void		GetPlayerLimits( int& minplayers, int& maxplayers, int &defaultMaxPlayers, bool &bIsMultiplayer ) const = 0;

	// Returns max splitscreen slot count ( 1 == no splits, 2 for 2-player split screen )
	virtual int			GetMaxSplitscreenPlayers( void ) = 0;

	// Return # of human slots, -1 if can't determine or don't care (engine will assume it's == maxplayers )
	virtual int			GetMaxHumanPlayers() = 0;

	virtual bool		ShouldNotifyLocalClientConnectionStateChanges() = 0;

	virtual void		OnClientFullyConnect( CEntityIndex nEntityIndex ) = 0;

	virtual void		GetHostStateLoopModeInfo( HostStateLoopModeType_t type, CUtlString &loopModeName, KeyValues **ppLoopModeOptions ) = 0;

	virtual bool		AllowDedicatedServers( EUniverse universe ) const = 0;

	virtual void		GetConVarPrefixesToResetToDefaults( CUtlString &sSemicolonDelimitedPrefixList ) const = 0;

	virtual bool		AllowSaveRestore() = 0;

	virtual bool		unk101() = 0;

	virtual bool		IsCommandQueueEnabled() = 0;
	virtual float		GetCommandQueueDilationPercentage() = 0;

	virtual bool		unk201() = 0;
	virtual bool		unk202() = 0;
	virtual bool		unk203() = 0;
	virtual bool 		unk204() = 0;
};

#define INTERFACEVERSION_SERVERGAMECLIENTS		"Source2GameClients001"

struct ClientReplayEventParams_t
{
	int m_nEventType = 0; // ReplayEventType_t
	float m_flSlowdownLength = 0.0f;
	float m_flSlowdownRate = 1.0f;
	int m_nPrimaryTargetEntIndex = -1;
	float m_flEventTime = 0.0f;
	bool m_bForceUseTheseSettings = false;
};

enum EncryptedMessageKeyType_t
{
	kEncryptedMessageKeyType_None = 0, // not encrypted
	kEncryptedMessageKeyType_Private = 1, // tv_encryptdata_key
	kEncryptedMessageKeyType_Public = 2, // tv_encryptdata_key_pub
};

//-----------------------------------------------------------------------------
// Purpose: Player / Client related functions
//-----------------------------------------------------------------------------
abstract_class ISource2GameClients : public IAppSystem
{
public:
	virtual void			OnClientConnected( CPlayerSlot slot, const char *pszName, uint64 xuid, const char *pszNetworkID, const char *pszAddress, bool bFakePlayer ) = 0;

	// Called when the client attempts to connect (doesn't get called for bots)
	// returning false would reject the connection with the pRejectReason message
	virtual bool			ClientConnect( CPlayerSlot slot, const char *pszName, uint64 xuid, const char *pszNetworkID, bool unk1, CBufferString *pRejectReason ) = 0;

	// Client is connected and should be put in the game
	// type values could be:
	// 0 - player
	// 1 - fake player (bot)
	// 2 - unknown
	virtual void			ClientPutInServer( CPlayerSlot slot, char const *pszName, int type, uint64 xuid ) = 0;

	// Client is going active
	// If bLoadGame is true, don't spawn the player because its state is already setup.
	virtual void			ClientActive( CPlayerSlot slot, bool bLoadGame, const char *pszName, uint64 xuid ) = 0;

	virtual void			ClientFullyConnect( CPlayerSlot slot ) = 0;

	// Client is disconnecting from server
	virtual void			ClientDisconnect( CPlayerSlot slot, ENetworkDisconnectionReason reason,
								const char *pszName, uint64 xuid, const char *pszNetworkID ) = 0;

	// Sets the client index for the client who typed the command into his/her console
	// virtual void			SetCommandClient( CPlayerSlot slot) = 0;

	// The client has typed a command at the console
	virtual void			ClientCommand( CPlayerSlot slot, const CCommand &args ) = 0;

	// Set the the client controller's userinfo cvar value set to the specified address.
	virtual void			ClientSetConVarUserInfoSet( CPlayerSlot slot, ConVarUserInfoSet_t pConVarUserInfoSet ) = 0;
	// A player changed one/several replicated cvars (name etc)
	virtual void			ClientSettingsChanged( CPlayerSlot slot ) = 0;

	// Determine PVS origin and set PVS for the player/viewentity
	virtual void			ClientSetupVisibility( CPlayerSlot slot, vis_info_t *visinfo ) = 0;

	// A block of CUserCmds has arrived from the user, decode them and buffer for execution during player simulation
	// Will be called when CNetworkGameServerBase::GetServerState() > SS_Loading
	// A "paused" argument equals CNetworkGameServerBase::GetServerState() == SS_Paused
	virtual void			ProcessUsercmds( CPlayerSlot slot, const CNetMessagePB<CCLCMsg_Move> &msg, bool paused ) = 0;

	virtual bool			IsPlayerSlotOccupied( CPlayerSlot slot ) = 0;

	virtual bool			IsPlayerAlive( CPlayerSlot slot) = 0;

	virtual int				GetPlayerScore( CPlayerSlot slot ) = 0;

	// Get the ear position for a specified client
	virtual void			ClientEarPosition( CPlayerSlot slot, Vector *pEarOrigin ) = 0;

	// Anything this game .dll wants to add to the bug reporter text (e.g., the entity/model under the picker crosshair)
	//  can be added here
	virtual void			GetBugReportInfo( CBufferString &buf ) = 0;

	// TERROR: A player sent a voice packet
	virtual void			ClientVoice( CPlayerSlot slot ) = 0;
	
	// The client has submitted a keyvalues command
	virtual void			ClientCommandKeyValues( CPlayerSlot slot, KeyValues *pKeyValues ) = 0;
	
	virtual void			ClientDiagnostic( CPlayerSlot slot, CCLCMsg_Diagnostic *pDiagnosticMsg ) = 0;
	
	virtual bool			ClientCanPause( CPlayerSlot slot ) = 0;

	virtual void			HLTVClientFullyConnect( int index, const CSteamID &steamID ) = 0;

	virtual bool			CanHLTVClientConnect( int index, const CSteamID &steamID, int *pRejectReason ) = 0;

	virtual void			StartHLTVServer( CEntityIndex index ) = 0;

	virtual void			SendHLTVStatusMessage( IHLTVServer *, bool, bool, const char *, int, int, int ) = 0;

	virtual IHLTVDirector	*GetHLTVDirector( void ) = 0;

	virtual uint32			GetPlayerTickBase( CPlayerSlot slot ) = 0;
	// AMNOTE: Fills a 60-byte structure from the player's controller
	virtual void			unk101( CPlayerSlot slot, void *pOut ) = 0;

	// Handles incoming usermessages from the client
	virtual void			ClientSvcUserMessage( CPlayerSlot slot, int um_type, uint32 size, const void *buf ) = 0;

	// Returns this player hltv delay in ticks
	virtual int				GetPlayerHltvDelay( CPlayerSlot slot, CEntityIndex &replay_ent ) = 0;
	virtual bool			ClientReplayEvent( CPlayerSlot slot, const ClientReplayEventParams_t &params ) = 0;

	// True when the server password is empty or "none", or matches the client's password
	virtual bool			CheckHltvPasswordMatch( const char *pszClientPassword, const char *pszServerPassword, const CSteamID &steamID, void * ) = 0;
	// While a client watches an HLTV replay, it is only sent the messages this returns true for:
	// chat, text, radio and raw audio messages, and user messages carrying radio, audio, rank, XP, quest progress or lobby disconnects
	virtual bool			ShouldSendMessageDuringHltvReplay( const NetMessageInfo_t *pInfo, const CNetMessage *pData ) = 0;
	virtual EncryptedMessageKeyType_t GetMessageEncryptionKey( const NetMessageInfo_t *pInfo, CNetMessage *pData ) = 0;
	// Returns false when the config text uses commands or convars that workshop configs may not use, listing them in pErrors
	virtual bool			ValidateConfigCommands( const char *pszConfig, CBufferString *pErrors ) = 0;
	// Returns false to not kick the client right away for failing Steam authentication; CS2 then stores the reason on its pawn
	virtual bool			ShouldKickClientForSteamAuthFailure( CPlayerSlot slot, ENetworkDisconnectionReason reason ) = 0;
};

typedef IVEngineServer2 IVEngineServer;
typedef ISource2Server IServerGameDLL;
typedef ISource2GameEntities IServerGameEnts;
typedef ISource2GameClients IServerGameClients;

#endif // EIFACE_H
