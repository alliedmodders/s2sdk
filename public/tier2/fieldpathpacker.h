#ifndef FIELDPATHPACKER_H
#define FIELDPATHPACKER_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"
#include "tier0/threadtools.h"
#include "tier0/utlhashtable.h"
#include "tier0/logging.h"
#include "tier2/fieldpath.h"

// AMNOTE: Packs field paths into PackedFieldPath_t. The engine's is ISerializedEntities::GetFieldPathPacker;
// A path that doesn't fit in 32 bits is added to its list on first use and packed as 0x80000000 | its index, for the packer's lifetime
class CFieldPathPacker
{
public:
	static constexpr int s_nMaxDepth = 3;
	// The largest component each depth stores
	static constexpr int16 s_nDepthMax[ s_nMaxDepth ] = { 510, 2046, 2046 };
	static constexpr int s_nDepthShift[ s_nMaxDepth ] = { 22, 11, 0 };
	static constexpr uint32 s_nDepthMask[ s_nMaxDepth ] = { 0x7FC00000, 0x3FF800, 0x7FF };

	// A complex index the packer doesn't have, or a path deeper than DEFAULT_MAX_PATH_DEPTH (where the engine stops
	// with a fatal error), gives an empty path
	CFieldPath UnpackPath( PackedFieldPath_t nPath ) const;
	CFieldPath UnpackComplex( PackedFieldPath_t nPath ) const;

private:
	mutable CThreadRWLock_FastRead m_Lock;
	CUtlStableHashtable<CFieldPath, empty_t, DefaultHashFunctor<CFieldPath>, DefaultEqualFunctor<CFieldPath>, uint32> m_ComplexPaths;
	LoggingChannelID_t m_nLogChannel;
};

inline CFieldPath CFieldPathPacker::UnpackPath( PackedFieldPath_t nPath ) const
{
	if ( (int32)nPath < -1 )
		return UnpackComplex( nPath );

	CFieldPath path;
	path.m_nCount = 0;
	path.m_bUsingExternalPaths = false;

	if ( (int32)nPath == -1 )
	{
		path.m_FixedPaths[ path.m_nCount++ ] = 0x7FFF;
		return path;
	}

	for ( int i = 0; i < s_nMaxDepth && ( nPath & s_nDepthMask[ i ] ); i++ )
		path.m_FixedPaths[ path.m_nCount++ ] = (int16)( ( ( nPath & s_nDepthMask[ i ] ) >> s_nDepthShift[ i ] ) - 1 );

	return path;
}

inline CFieldPath CFieldPathPacker::UnpackComplex( PackedFieldPath_t nPath ) const
{
	CFieldPath path;
	path.m_nCount = 0;
	path.m_bUsingExternalPaths = false;

	m_Lock.LockForRead();

	const UtlHashHandle_t hPath = nPath & 0x7FFFFFFF;
	if ( m_ComplexPaths.IsValidHandle( hPath ) && m_ComplexPaths.Key( hPath ).GetDepth() <= DEFAULT_MAX_PATH_DEPTH )
	{
		const CFieldPath &complexPath = m_ComplexPaths.Key( hPath );
		path.m_nCount = (int16)complexPath.GetDepth();
		memcpy( path.m_FixedPaths, complexPath.GetPathData(), path.m_nCount * sizeof( int16 ) );
	}

	m_Lock.UnlockRead();

	return path;
}

#endif /* FIELDPATHPACKER_H */
