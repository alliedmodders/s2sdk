#ifndef GAMESHAREDDEFS_H
#define GAMESHAREDDEFS_H

#ifdef _WIN32
#pragma once
#endif

#include "entity2/globalvars_base.h"
#include "string_t.h"


//-----------------------------------------------------------------------------
// Purpose: Defines the ways that a map can be loaded.
//-----------------------------------------------------------------------------
enum MapLoadType_t
{
	MapLoad_NewGame = 0,
	MapLoad_LoadGame,
	MapLoad_Transition,
	MapLoad_Background,
};

// Entities can span this many clusters before we revert to a slower area checking algorithm
#define	MAX_FAST_ENT_CLUSTERS	4
#define	MAX_ENT_CLUSTERS	64
#define MAX_WORLD_AREAS		8

//-----------------------------------------------------------------------------
// Stores information necessary to perform PVS testing.
//-----------------------------------------------------------------------------
struct PVSInfo_t
{
	// headnode for the entity's bounding box
	short		m_nHeadNode;			

	// number of clusters or -1 if too many
	short		m_nClusterCount;		

	// cluster indices
	unsigned short *m_pClusters;	

	// For dynamic "area portals"
	short		m_nAreaNum;
	short		m_nAreaNum2;

	// current position
	float		m_vCenter[3];

private:
	unsigned short m_pClustersInline[MAX_FAST_ENT_CLUSTERS];
};


//-----------------------------------------------------------------------------
// Purpose: Global variables shared between the engine and the game .dll
//-----------------------------------------------------------------------------
class CGlobalVars : public CGlobalVarsBase
{	
public:

	CGlobalVars();

public:
	// Current map
	string_t		mapname;
	string_t		startspot;
	MapLoadType_t	eLoadType;		// How the current map was loaded
	bool mp_teamplay;

	// current maxentities
	int				maxEntities;

	int				serverCount;
};

inline CGlobalVars::CGlobalVars() : 
	CGlobalVarsBase()
{
	serverCount = 0;
}

#endif // GAMESHAREDDEFS_H
