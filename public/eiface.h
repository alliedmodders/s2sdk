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
#include "engine2/isource2serverconfig.h"
#include "rendersystem/rendersystemtypes.h"

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


#endif // EIFACE_H
