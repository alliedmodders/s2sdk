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
#include "engine2/inetworkserverservice.h"

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




#endif // ISERVER_H
