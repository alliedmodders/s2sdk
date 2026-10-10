#ifndef IPVS_H
#define IPVS_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"
#include "mathlib/vector.h"
#include "scenesystem/iscenesystem.h"

struct AABB_t;

//-----------------------------------------------------------------------------
// AMNOTE: Purpose: The potentially visible set of a spawn group, from IVEngineServer2::GetPVSForSpawnGroup().
// AMNOTE: Positions are in world space unless a method says otherwise.
// AMNOTE: A vis_info_t with a cluster count of 0 sees every cluster.
//-----------------------------------------------------------------------------
abstract_class IPVS
{
public:
	// AMNOTE: Writes the origin's cluster to pClusters and returns 1; nMaxClusters is ignored, so provide at least one int
	virtual int GetClustersAtOrigin( int *pClusters, int nMaxClusters, const Vector &vecOrigin ) = 0;
	virtual int GetClustersInBox( int *pClusters, int nMaxClusters, const Vector &vecMins, const Vector &vecMaxs, bool bUnk ) = 0;
	// AMNOTE: Same as GetClustersInBox for an oriented box: Quaternion rotation, Vector center, Vector half extents
	virtual int unk001( int *pClusters, int nMaxClusters, const void *pOrientedBox, bool bUnk ) = 0;
	// AMNOTE: Same as GetClustersInBox for a view frustum
	virtual int unk002( int *pClusters, int nMaxClusters, const void *pFrustum, bool bUnk ) = 0;
	// AMNOTE: Always writes cluster 0 to pClusters and returns 1
	virtual int unk003( int *pClusters ) = 0;
	// AMNOTE: Keeps the clusters whose bounds are within flMaxDistance of vecOrigin, returns how many are left
	// AMNOTE: With missing or trivial vis data, writes cluster 0 and returns 1; provide at least one int
	virtual int FilterClustersByDistance( int *pClusters, int nClusters, const Vector &vecOrigin, float flMaxDistance ) = 0;

	virtual int GetClusterCount() = 0;
	// AMNOTE: Writes up to nMaxBounds entries in world space, but returns the total cluster count
	virtual int GetAllClusterBounds( AABB_t *pBounds, int nMaxBounds ) = 0;
	virtual int GetClusterForOrigin( const Vector &vecOrigin ) = 0;

	// AMNOTE: Returns the vis_debug_lock origin, or nullptr if it isn't locked
	virtual const Vector *unk101() = 0;
	// AMNOTE: Returns true if the vis data has exactly 2 clusters
	virtual bool unk102() = 0;

	virtual bool CheckOriginInPVS( const Vector &vecOrigin, const vis_info_t *pVisInfo ) = 0;
	virtual bool CheckBoxInPVS( const Vector &vecMins, const Vector &vecMaxs, const vis_info_t *pVisInfo ) = 0;
	virtual bool GetClusterBounds( int nCluster, Vector *pMins, Vector *pMaxs ) = 0;
	virtual bool CheckOriginInSkyPVS( const Vector &vecOrigin ) = 0;
	virtual bool CheckOriginInSunlightPVS( const Vector &vecOrigin ) = 0;

	// AMNOTE: Doesn't set the spawn group handle of pVisInfo; nCluster must not be negative
	virtual void GetPVSForCluster( int nCluster, vis_info_t *pVisInfo ) = 0;
	// AMNOTE: nClusters is the bit count, not the number of uint32 words; an empty set produces a full PVS
	virtual void GetPVSForClusters( const uint32 *pClusterBits, int nClusters, vis_info_t *pVisInfo ) = 0;

	// AMNOTE: Returns true if any cluster pVisInfo sees is in the sunlight PVS
	virtual bool CheckPVSInSunlightPVS( const vis_info_t *pVisInfo ) = 0;

	// AMNOTE: Clears the bits for AddOriginToPVS; with no vis data the zero count still means full visibility
	// AMNOTE: Does not set the spawn group handle
	virtual void ResetPVS( vis_info_t *pVisInfo ) = 0;
	// AMNOTE: Takes spawn-group-local coordinates (world position minus GetWorldOffset()); preserves the handle
	virtual void AddOriginToPVS( const Vector &vecLocalOrigin, vis_info_t *pVisInfo ) = 0;
	// AMNOTE: Takes spawn-group-local coordinates and unions all overlapping cluster PVS rows
	virtual void GetPVSForOrigin( vis_info_t *pVisInfo, const Vector &vecLocalOrigin ) = 0;
	// AMNOTE: The sky PVS, or the sunlight PVS with vis_sunlight_enable
	virtual void GetSunlightViewPVS( vis_info_t *pVisInfo ) = 0;
	// AMNOTE: Sets pVisInfo to see every cluster
	virtual void GetFullPVS( vis_info_t *pVisInfo ) = 0;

	// AMNOTE: Returns the total cluster count, not the number of set bits; missing or trivial vis returns 2 with bit 0 set
	virtual int GetClusterBitsInBox( CBitVec<4096> *pClusters, const Vector &vecMins, const Vector &vecMaxs, bool bUnk ) = 0;

	// AMNOTE: Returns true if the box is inside the vis data's bounds, without taking off the world offset
	virtual bool unk201( const AABB_t &box ) = 0;
	virtual bool CheckBoxInCluster( const AABB_t &box, int nCluster ) = 0;

	// AMNOTE: Returns the vis data
	virtual void *unk301() = 0;

	virtual Vector GetWorldOffset() = 0;
	// AMNOTE: Grows the box to the vis grid's cells, plus nExtraCells on each side
	virtual void SnapBoxToVisCells( AABB_t *pBox, int nExtraCells ) = 0;

	// AMNOTE: Same as GetClusterBitsInBox, but writes the non-empty 32-cluster words as 8-byte entries, returns their count
	virtual int unk401( void *pEntries, int nMaxEntries, const Vector &vecMins, const Vector &vecMaxs, bool bUnk ) = 0;
};

#endif // IPVS_H
