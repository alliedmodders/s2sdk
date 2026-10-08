#ifndef NETWORKSYSTEMTYPES_H
#define NETWORKSYSTEMTYPES_H

#ifdef _WIN32
#pragma once
#endif

#include "tier1/utlhashtable.h"
#include "tier1/utlleanvector.h"
#include "tier1/utlvector.h"

struct OffsetIgnore_t
{
	typedef uint16 OffsetIgnoreValueType_t;

	CUtlHashtable<uint16, empty_t> m_OffsetHashtable;
	OffsetIgnoreValueType_t m_unMaxOffset;
};

struct ChangeAccessorFieldPathIndex_t
{
	ChangeAccessorFieldPathIndex_t() { m_Value = -1; }
	ChangeAccessorFieldPathIndex_t( int32 value ) { m_Value = value; }

	int32 m_Value;
};

struct ChangeAccessorFieldPathIndexInfo_t
{
	struct IgnoreCache_t
	{
		uint16 m_Offsets[16];
		ChangeAccessorFieldPathIndex_t m_FieldPaths[16];
		int m_nCount;
		int m_nFirstElement;
	};

	typedef CUtlLeanVectorFixedGrowable<uint32> PackedFieldPathVec_t;

	PackedFieldPathVec_t m_ChangeAccessorFieldPathIdList;
	const OffsetIgnore_t *m_pBaseOffsetToIgnore;
	CUtlVectorFixedGrowable<const OffsetIgnore_t *, 4> m_OffsetsToIgnoreForPaths;
	IgnoreCache_t m_ShouldIgnore;
	IgnoreCache_t m_ShouldNotIgnore;
};

struct VarChangeInfo_t
{
	ChangeAccessorFieldPathIndex_t m_nRootPathIndex;
	int16 m_nArrayIndex;
	uint32 m_nFieldOffset : 31;
	uint32 m_bResolved : 1;
};

#endif // NETWORKSYSTEMTYPES_H
