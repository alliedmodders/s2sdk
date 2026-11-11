//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#ifndef SERVER_CLASS_H
#define SERVER_CLASS_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/dbg.h"
#include "networkstringtabledefs.h"
#include "networksystem/iflattenedserializers.h"

class CEntityClass;

class ServerClass
{
public:
	const char*	GetName()		{ return m_pNetworkName; }

public:
	const char *m_pDLLClassName; // On the server this matches the m_pNetworkName, on the client it is prefixed with C_*
	const char *m_pNetworkName;
	
	CEntityClass *m_pEntityClass;

	FlattenedSerializerDesc_t m_flattenedSerializer;

	int m_ClassID;

	// This is an index into the network string table (sv.GetInstanceBaselineTable()).
	int m_InstanceBaselineIndex; // INVALID_STRING_INDEX if not initialized yet.

	bool m_bFakeClassForSourceTVRelay;
	bool m_bHasAlternateBaselines;
	bool m_bAlternateBaselinesUseRecipientsFilters;
	bool m_bFakeClassForDemoPlayback;
	bool m_bHasMixedOutOfPVSUpdateFields;
};

// Registered by DECLARE_NETWORKCLASS
struct NetworkClassRegistration_t
{
	const char *m_pDLLClassName;
	const char *m_pNetworkName;
	NetworkClassRegistration_t *m_pNext;
};

#endif
