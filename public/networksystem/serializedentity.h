#ifndef SERIALIZEDENTITY_H
#define SERIALIZEDENTITY_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"
#include "tier0/mempool.h"
#include "tier0/threadinterlocks.h"
#include "tier0/utlleanvector.h"
#include "tier0/utlstring.h"
#include "tier2/fieldpath.h"
#include "networksystem/iflattenedserializers.h"
#include "networksystem/networksystemtypes.h"

class CFieldPathPacker;

// AMNOTE: The header of the bit offset of each field's data in an entity's data, one per packed field path, so
// CSerializedEntity::m_nFieldCount of them (CSerializedFieldStorage<PackedFieldOffsets_t, int>)
struct PackedFieldOffsets_t
{
	CInterlockedInt m_nRefs;
};

// AMNOTE: The header of the array counts and polymorphic ids of an entity's fields
// (CSerializedFieldStorage<FieldMetadataList_t, PolymorphicMetadataEntry_t>)
struct FieldMetadataList_t
{
	int Count() const { return m_nCount; }

	CInterlockedInt m_nRefs;
	int m_nCount;
};

// AMNOTE: The encoded field values of an entity. Only ISerializedEntities creates, copies and releases them: the field lists
// are shared between copies
class CSerializedEntity
{
public:
	CSerializedEntity( const CSerializedEntity & ) = delete;
	CSerializedEntity &operator=( const CSerializedEntity & ) = delete;

	CSerializedFieldStorage<PackedFieldPathList_t, PackedFieldPath_t> *m_pFieldPaths;
	CSerializedFieldStorage<PackedFieldOffsets_t, int> *m_pFieldOffsets;
	CSerializedFieldStorage<FieldMetadataList_t, PolymorphicMetadataEntry_t> *m_pFieldMetadata;

	// AMNOTE: m_nDataBits bits of field data, in m_pFieldPaths order
	uint8 *m_pData;

	FlattenedSerializerDesc_t m_AssociatedSerializer;
	int m_nFieldCount;
	int m_nDataBits;
	int m_nDataBufferSize;

	// AMNOTE: Clearing keeps m_pData for reuse, and it grows in 128 byte steps
	bool m_bReuseDataBuffer;
};

abstract_class ISerializedEntities
{
public:
	// AMNOTE: A data buffer of nDataBufferSize bytes is allocated up front, rounded up to a multiple of 128 when
	// bReuseDataBuffer is set (see CSerializedEntity::m_bReuseDataBuffer)
	virtual CSerializedEntity *AllocateSerializedEntity( const FlattenedSerializerDesc_t &serializer, bool bReuseDataBuffer = false, int nDataBufferSize = 0 ) = 0;
	virtual void ReleaseSerializedEntity( CSerializedEntity *pEntity ) = 0;
	virtual CSerializedEntity *CopySerializedEntity( const CSerializedEntity *pEntity ) = 0;
	virtual void ReplaceSerializedEntity( CSerializedEntity *pDest, const CSerializedEntity *pSource ) = 0;

	// AMNOTE: Appends the entity's packed field paths to pOutFieldPaths if set
	virtual int GetSerializedEntityFieldCount( const CSerializedEntity *pEntity, CUtlLeanVectorFixedGrowable<uint32> *pOutFieldPaths ) = 0;

	// AMNOTE: A blob is a CSerializedEntity packed into one allocation; only the fields in pFieldPaths are packed if set. With
	// pPool the blob is allocated from the pool and only gets ReleaseSerializedEntityBlobResources, without it the blob
	// is heap allocated and gets FreeSerializedEntityBlob, which also releases its resources
	virtual void *CreateSerializedEntityBlob( const CSerializedEntity *pEntity, const CUtlLeanVectorFixedGrowable<uint32> *pFieldPaths, CUtlScratchMemoryPool *pPool ) = 0;
	virtual void *AllocSerializedEntityBlob( int nSize ) = 0;
	virtual void ReleaseSerializedEntityBlobResources( void *pBlob ) = 0;
	virtual void FreeSerializedEntityBlob( void *pBlob ) = 0;
	virtual size_t GetSerializedEntityBlobSize( const void *pBlob ) = 0;

	// AMNOTE: Logs each difference
	virtual bool CompareSerializedEntities( const CSerializedEntity *pTo, const CSerializedEntity *pFrom, NetworkSerializationMode_t nMode ) = 0;
	virtual bool AreSerializedEntitiesEqual( const CSerializedEntity *pEntity, const CSerializedEntity *pOther ) = 0;

	virtual void ReportMemoryUsage() = 0;
	virtual CUtlString DescribeSerializedEntity( const CSerializedEntity *pEntity ) = 0;
	virtual CFieldPathPacker *GetFieldPathPacker() = 0;
	virtual void SetReuseDataBuffer( CSerializedEntity *pEntity, bool bReuseDataBuffer ) = 0;

	// AMNOTE: Array counts of 100000 and more are invalid. ValidateFieldArrayCount logs a count above 100000 with the field's name
	// and writes a minidump, the first four times
	virtual bool IsValidFieldArrayCount( uint32 nCount ) = 0;
	virtual void ValidateFieldArrayCount( uint32 nCount, const char *pszFieldName ) = 0;

	// AMNOTE: The bytes the entity takes up, counting the field lists it shares with entities already in pCountedLists only once
	virtual size_t GetMemoryUsage( void *pCountedLists, const CSerializedEntity *pEntity ) = 0;
	// AMNOTE: Adds the bytes and bits of each field of the entity to pFieldStats
	virtual void GatherFieldMemoryUsage( void *pFieldStats, void *pCountedLists, const CSerializedEntity *pEntity ) = 0;

	virtual FlattenedSerializerDesc_t GetAssociatedSerializer( const CSerializedEntity *pEntity ) = 0;

	virtual const PolymorphicMetadataEntry_t *FindFieldMetadata( const CSerializedEntity *pEntity, PackedFieldPath_t nFieldPath ) = 0;

	// AMNOTE: Appends the entity's metadata entries of type nType (see PolymorphicMetadataEntry_t::m_nType) to pOut.
	// Expects caller to free it.
	virtual void GetFieldMetadata( const CSerializedEntity *pEntity, PolymorphicMetadata_t *pOut, uint8 nType ) = 0;
};

#endif // SERIALIZEDENTITY_H
