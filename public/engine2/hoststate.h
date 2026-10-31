#ifndef HOSTSTATE_H
#define HOSTSTATE_H
#ifdef _WIN32
#pragma once
#endif

#include "tier0/utlstring.h"
#include "tier0/keyvalues.h"
#include "appframework/iappsystem.h"
#include "engine2/tier4.h"
#include "engine2/isource2serverconfig.h"

struct ResourceManifestDesc_t;

enum HostStateRequestType_t
{
	HSR_IDLE = 1,
	HSR_GAME,
	HSR_SOURCETV_RELAY,
	HSR_QUIT
};

enum HostStateRequestMode_t
{
	HM_LEVEL_LOAD_SERVER = 1,
	HM_CONNECT,
	HM_CHANGE_LEVEL,
	HM_LEVEL_LOAD_LISTEN,
	HM_LOAD_SAVE,
	HM_PLAY_DEMO,
	HM_SOURCETV_RELAY,
	HM_ADDON_DOWNLOAD
};

struct CHostStateRequest
{
	HostStateRequestType_t m_iType;
	CUtlString m_LoopModeType;
	CUtlString m_Desc;
	bool m_bActive;
	unsigned int m_ID;
	HostStateRequestMode_t m_iMode;
	CUtlString m_LevelName;
	bool m_bChangelevel;
	CUtlString m_SaveGame;
	CUtlString m_Address;
	CUtlString m_DemoFile;
	bool m_bTimeDemo;
	CUtlString m_Addons;
	KeyValues *m_pKV;
};

class ISwitchLoopModeStatusNotify
{
	virtual void OnSwitchLoopModeFinished(const char *, uint32, bool) = 0;
};

class IHostStateMgr : public IAppSystem
{
public:
	virtual void RequestHS_Quit(void) = 0;
	virtual void RequestHS_Idle(KeyValues *) = 0;
	virtual void RequestHS_Connect(const char *, KeyValues *) = 0;
	virtual void RequestHS_ChangelevelReconnect(const char *, KeyValues *) = 0;
	virtual void RequestHS_LoadSpawnGroup(const char *, const char *, bool, KeyValues *) = 0;
	virtual void RequestHS_LoadSaveGame(const char *, const char *, KeyValues *) = 0;
	virtual void RequestHS_PlayDemo(const char *, const char *, bool, KeyValues *) = 0;
	virtual void RequestHS_PlayBroadcast(const char *, KeyValues *) = 0;
	virtual void RequestHS_DownloadAddons(KeyValues *) = 0;
	virtual void RequestHS_SourceTVRelay(const char *, KeyValues *) = 0;
	virtual void RequestHS_ReloadLastSaveGame(void) = 0;
	virtual void RequestHS_RestartSpawnGroups(void) = 0;
	virtual void unk001(void) = 0;
	virtual void unk002(void) = 0;
};

class CHostStateMgr : public CTier4AppSystem<IHostStateMgr>, public ISwitchLoopModeStatusNotify
{
public:
	struct LoopModeInfo_t
	{
		CUtlString m_LoopModeName;
		KeyValues *m_pLoopModeOptions;
	};

	CHostStateRequest *m_PendingRequest;
	CHostStateRequest *m_CurrentRequest;
	LoopModeInfo_t m_LoopModes[HOST_STATE_LOOP_MODE_COUNT];
	CUtlString m_SaveGame;
	CUtlString m_LevelName;
	CUtlString m_Addons;
	KeyValues *m_pKV;
	int m_ID;
	CUtlVector<CHostStateRequest *> m_QueuedRequests;
};

#endif // HOSTSTATE_H
