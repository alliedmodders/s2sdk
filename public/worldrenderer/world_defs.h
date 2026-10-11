#ifndef WORLD_DEFS_H
#define WORLD_DEFS_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"

// AMNOTE: Index into the engine's world group table, with the high bit set for client-side groups; -1 is invalid
class WorldGroupId_t
{
public:
	WorldGroupId_t( uint32 nId = 0 ) : m_nId( nId ) {}

	bool operator==( const WorldGroupId_t &other ) const { return m_nId == other.m_nId; }
	bool operator!=( const WorldGroupId_t &other ) const { return m_nId != other.m_nId; }
	bool operator<( const WorldGroupId_t &other ) const { return m_nId < other.m_nId; }

	uint32 m_nId;
};

#endif // WORLD_DEFS_H
