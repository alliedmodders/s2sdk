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
#include "engine2/isource2engine.h"
#include "engine2/ivengineserver2.h"
#include "engine2/isource2server.h"

//-----------------------------------------------------------------------------
// forward declarations
//-----------------------------------------------------------------------------
class	ServerClass;
class	IRecipientFilter;
class	INetChannelInfo;
class IAchievementMgr;
class CGamestatsData;
class CSteamID;
class INetworkStringTable;
class CEntityLump;
class IPVS;
class IHLTVDirector;
struct SpawnGroupDesc_t;
struct Entity2Networkable_t;
class CCreateGameServerLoadInfo;
class INavListener;
class CNavData;

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

	// When true, the map command changes level instead of loading the map while a server is running
	virtual bool		ShouldMapCommandChangeLevel() = 0;
	// When true, a connecting client gets a new client object while below the limit instead of reusing a free one
	virtual bool		ShouldAllocateNewClients() = 0;
	virtual bool		Uses64TickInterval() = 0;
	virtual bool		InitGameEvents( CreateInterfaceFn factory ) = 0;
};


#endif // EIFACE_H
