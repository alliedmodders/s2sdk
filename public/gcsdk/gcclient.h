//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Holds the CGCClient class
//
//=============================================================================

#ifndef GCCLIENT_H
#define GCCLIENT_H

#ifdef _WIN32
#pragma once
#endif

#include "steam/steam_api.h"
#include "steam/isteamgamecoordinator.h"
#include "tier0/threadtools.h"
#include "tier0/utlleanvector.h"
#include "tier0/utlmap.h"
#include "gcsdk/gcclient_sharedobjectcache.h"

class CTestEvent;

namespace GCSDK
{

// AMNOTE: Opaque, only its size matches the game's. Its work thread pool embeds a CThreadEvent, which
// is 112 bytes larger on Linux (pthread mutex and condition) than on Windows (a HANDLE)
class CJobMgr
{
protected:
	uint8 m_unk001[784];
	CThreadEvent m_unk002;
};

//-----------------------------------------------------------------------------
// Purpose: The game's connection to the GC and its shared object caches
//-----------------------------------------------------------------------------
class CGCClient
{
public:
	virtual ~CGCClient();

	// AMNOTE: Both are empty in CS2
	virtual void Test_AddEvent( CTestEvent *pEvent ) {}
	virtual void Test_CacheSubscribed( const SOID_t &owner ) {}

	ISteamGameCoordinator *GetSteamGameCoordinator() const { return m_pSteamGameCoordinator; }
	bool BIsGameserver() const { return m_bGameserver; }

	/// Find the shared object cache for the specified owner, NULL if there is none
	CGCClientSharedObjectCache *FindSOCache( const SOID_t &owner ) const
	{
		auto i = m_mapSOCache.Find( owner );
		return m_mapSOCache.IsValidIndex( i ) ? m_mapSOCache[i] : NULL;
	}

	/// Adds a listener to the shared object caches.  A call to re-add
	/// a listener that's already listening is harmlessly ignored.
	// AMNOTE: The listener gets no calls for the caches that are already subscribed
	void AddSOCacheListener( ISharedObjectListener *pListener )
	{
		if ( !m_vecSOCacheListeners.HasElement( pListener ) )
			m_vecSOCacheListeners.AddToTail( pListener );
	}

	/// Removes a listener for the shared object caches.  Returns true
	/// if we were listening and were successfully removed, false
	/// otherwise
	bool RemoveSOCacheListener( ISharedObjectListener *pListener ) { return m_vecSOCacheListeners.FindAndRemove( pListener ); }

	const CUtlVector<ISharedObjectListener *> &GetSOCacheListeners() const { return m_vecSOCacheListeners; }

protected:
	ISteamUser *m_pSteamUser;
	ISteamGameServer *m_pSteamGameServer;
	ISteamGameCoordinator *m_pSteamGameCoordinator;
	ISteamUtils *m_pSteamUtils;
	CUtlLeanVector<uint8> m_memMsg;

	// local job handling
	CJobMgr m_JobMgr;

	// Shared object caches
	CUtlOrderedMap<SOID_t, CGCClientSharedObjectCache *, CDefLess<SOID_t>, unsigned short> m_mapSOCache;

	// AMNOTE: The listeners of every cache, see ISharedObjectListener
	CUtlVector<ISharedObjectListener *> m_vecSOCacheListeners;

	uint8 m_unk001[72];
	bool m_bGameserver;
	uint8 m_unk101[47];

	CCallback<CGCClient, GCMessageAvailable_t, false> m_CallbackGCMessageAvailable;
	CCallback<CGCClient, SteamServersDisconnected_t, false> m_CallbackSteamServersDisconnected;
	CCallback<CGCClient, SteamServerConnectFailure_t, false> m_CallbackSteamServerConnectFailure;
	CCallback<CGCClient, SteamServersConnected_t, false> m_CallbackSteamServersConnected;
};

} // namespace GCSDK

#endif // GCCLIENT_H
