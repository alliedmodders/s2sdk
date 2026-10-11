#ifndef FLATTENEDSERIALIZERS_H
#define FLATTENEDSERIALIZERS_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/basetypes.h"
#include "tier0/utldelegate.h"
#include "tier0/utlleanvector.h"
#include "tier0/utlstring.h"
#include "tier0/utlsymbollarge.h"
#include "tier0/utlvector.h"
#include "tier2/fieldpath.h"
#include "networksystem/inetworkserializer.h"
#include "networksystem/networksystemtypes.h"
#include "playerslot.h"

struct FlattenedSerializerDesc_t
{
	CUtlSymbolLarge m_name;
	FlattenedSerializerHandle_t m_handle;
};

struct BuildFlattenedSerializerInfo_t;
class bf_read;
class bf_write;
class CSerializedEntity;
class CSVCMsg_FlattenedSerializer;
class IFlattenedSerializerSpewFunc;
class IFlattenedSerializerSpewListener;
class INetworkFieldChangedEventQueue;
class IPolymorphicMetadataHelper;

// AMNOTE: Names array elements in field path spew instead of "[%d]" when it returns a non-empty string
typedef CUtlDelegate<CUtlString ( int, const char *pszName, int nArrayIndex )> ArrayElementNameFormatterDelegate_t;

// AMNOTE: void pointers are engine structs whose layout isn't known yet
abstract_class IFlattenedSerializers
{
public:
	virtual void FindOrCreateCreateFlattenedSerializers( BuildFlattenedSerializerInfo_t *pInfos, int nCount ) = 0;
	virtual CUtlString GetFullFieldName( const FlattenedSerializerDesc_t &serializer, const CFieldPath &path, void *pObject, int nEntityIndex, NetworkSerializationMode_t nMode, bool ) = 0;
	virtual const char *GetFieldName( const FlattenedSerializerDesc_t &serializer, const CFieldPath &path, IPolymorphicMetadataHelper *pMetadataHelper, NetworkSerializationMode_t nMode, CUtlString *pOut ) = 0;
	virtual const char *GetFieldName( const FlattenedSerializerDesc_t &serializer, int nFieldPath, IPolymorphicMetadataHelper *pMetadataHelper, NetworkSerializationMode_t nMode, CUtlString *pOut ) = 0;
	virtual bool ReadFields( const FlattenedSerializerDesc_t &serializer, bf_read *pBuf, CSerializedEntity *pOut, int nEntityIndex, NetworkSerializationMode_t nMode, int ) = 0;
	virtual bool Encode( const FlattenedSerializerDesc_t &serializer, CSerializedEntity *pOut, void *pObject, int nEntityIndex, NetworkSerializationMode_t nMode, void * ) = 0;
	virtual bool GatherSendProxyResults( const FlattenedSerializerDesc_t &serializer, void *pObject, int nEntityIndex, NetworkSerializationMode_t nMode, void * ) = 0;
	virtual bool Decode( const FlattenedSerializerDesc_t &serializer, const CSerializedEntity *pFrom, void *pObject, int nEntityIndex, NetworkSerializationMode_t nMode, void *, int nFlags ) = 0;
	virtual int CalcDelta( const FlattenedSerializerDesc_t &serializer, const CSerializedEntity *pFrom, const CSerializedEntity *pTo, void *, int nEntityIndex, NetworkSerializationMode_t nMode, void * ) = 0;
	virtual CSerializedEntity *BuildDeltaProperties( const FlattenedSerializerDesc_t &serializer, void *, int nEntityIndex, NetworkSerializationMode_t nMode, const CSerializedEntity *pFrom, const CUtlLeanVectorFixedGrowable<uint32> *pFieldPaths, CUtlLeanVectorFixedGrowable<uint32> *pOutFieldPaths, void *, bool *, bool * ) = 0;
	virtual bool WriteFieldList( const FlattenedSerializerDesc_t &serializer, bf_write *pBuf, const CSerializedEntity *pEntity, int nEntityIndex, NetworkSerializationMode_t nMode, const CUtlLeanVectorFixedGrowable<uint32> *pFieldPaths, int ) = 0;
	virtual bool MergeDeltas( const FlattenedSerializerDesc_t &serializer, const CSerializedEntity *pOld, const CSerializedEntity *pDelta, CSerializedEntity *pOut, int nEntityIndex, NetworkSerializationMode_t nMode, CUtlLeanVectorFixedGrowable<uint32> *pOutFieldPaths ) = 0;
	virtual CSerializedEntity *BuildMergedSerializedEntity( const FlattenedSerializerDesc_t &serializer, void *pPackedDelta, CSerializedEntity *pBase, CUtlLeanVectorFixedGrowable<uint32> *pOutFieldPaths, bool bFreePackedDelta, int nEntityIndex ) = 0;
	virtual int CullFields( const FlattenedSerializerDesc_t &serializer, CPlayerSlot nSlot, void *, const CUtlLeanVectorFixedGrowable<uint32> *pFieldPaths, const void *, CUtlLeanVectorFixedGrowable<uint32> *pOutFieldPaths ) = 0;
	virtual int RemoveArrayElementsOutsideOfArrayMetadataBounds( const FlattenedSerializerDesc_t &serializer, const CSerializedEntity *pEntity, CUtlLeanVectorFixedGrowable<uint32> *pFieldPaths, int nEntityIndex, NetworkSerializationMode_t nMode ) = 0;
	virtual void SpewSerializer( const FlattenedSerializerDesc_t &serializer, NetworkSerializationMode_t nMode, IFlattenedSerializerSpewFunc *pSpew ) = 0;
	virtual bool MakeSerializersMatchByMeta( void *pMeta, const char *pszSerializerName, const FlattenedSerializerDesc_t &serializer, void *, void *, bool ) = 0;
	virtual bool CreateReplayCompatSerializerFromMeta( void *pMeta, const char *pszSerializerName, FlattenedSerializerDesc_t *pOut, void *, void * ) = 0;
	virtual void AssignChangeAccessorPathIds( const FlattenedSerializerDesc_t &serializer, IPolymorphicMetadataHelper *pMetadataHelper, NetworkSerializationMode_t nMode, void *pInfo, void * ) = 0;
	virtual void ResolveChanges( const FlattenedSerializerDesc_t &serializer, IPolymorphicMetadataHelper *pMetadataHelper, NetworkSerializationMode_t nMode, const CUtlLeanVectorFixedGrowable<uint32> *pFieldPaths, void *pChanges, CUtlLeanVectorFixedGrowable<uint32> *pOut, CUtlVector<int> * ) = 0;
	virtual CFieldPath FindFieldPathByAddress( const FlattenedSerializerDesc_t &serializer, const void *pObject, const void *pField, bool *pbFound ) = 0;
	virtual bool unk101( const FlattenedSerializerDesc_t &serializer, IPolymorphicMetadataHelper *pMetadataHelper, NetworkSerializationMode_t nMode, const CFieldPath &path, CUtlVector<int> * ) = 0;
	// AMNOTE: GatherSendProxyResults starting at path
	virtual bool GatherSendProxyResultsForPath( const FlattenedSerializerDesc_t &serializer, void *pObject, IPolymorphicMetadataHelper *pMetadataHelper, int nEntityIndex, NetworkSerializationMode_t nMode, const CFieldPath &path, void * ) = 0;
	virtual void *LookupOffsetsToIgnoreForPath( const FlattenedSerializerDesc_t &serializer, const CFieldPath &path, IPolymorphicMetadataHelper *pMetadataHelper, NetworkSerializationMode_t nMode ) = 0;
	virtual bool IsUnflattenedField( const FlattenedSerializerDesc_t &serializer, const char *pszFieldName ) = 0;
	virtual bool IsFieldNetworked( const FlattenedSerializerDesc_t &serializer, const char *pszFieldName ) = 0;
	virtual bool ValidateSerializedEntity( const FlattenedSerializerDesc_t &serializer, const CSerializedEntity *pEntity, int nEntityIndex, NetworkSerializationMode_t nMode ) = 0;
	virtual void DumpSerializedEntityToConsole( const FlattenedSerializerDesc_t &serializer, const char *pszLabel, const CSerializedEntity *pEntity, int nEntityIndex, NetworkSerializationMode_t nMode ) = 0;
	virtual void DumpSerializedEntityToLines( const FlattenedSerializerDesc_t &serializer, const char *pszLabel, const CSerializedEntity *pEntity, int nEntityIndex, NetworkSerializationMode_t nMode, CUtlVector<CUtlString> *pOutLines ) = 0;
	virtual void AddSpewListener( IFlattenedSerializerSpewListener *pListener ) = 0;
	virtual void RemoveSpewListener( IFlattenedSerializerSpewListener *pListener ) = 0;
	virtual bool WriteSerializerInfos( void *pSerializers, CSVCMsg_FlattenedSerializer *pMsg ) = 0;
	virtual void Report( NetworkSerializationMode_t nMode, bool bVerbose ) = 0;
	virtual void AddArrayElementNameFormatter( const ArrayElementNameFormatterDelegate_t &formatter ) = 0;
	virtual void RemoveArrayElementNameFormatter( const ArrayElementNameFormatterDelegate_t &formatter ) = 0;
	virtual INetworkFieldChangedEventQueue *CreateNetworkFieldChangedEventQueue( void *, void * ) = 0;
	// AMNOTE: Begin resets every serializer's matched meta variant, End reports the matches
	virtual void BeginSerializerMetaMatching( bool bTrack ) = 0;
	virtual void EndSerializerMetaMatching() = 0;
	// AMNOTE: Disabled fields are skipped when gathering send proxy results, building deltas and gathering polymorphic metadata
	virtual void SetFieldEnabled( const FlattenedSerializerDesc_t &serializer, const char *pszFieldName, bool bEnabled ) = 0;
	virtual void GatherPolymorphicMetadata( const FlattenedSerializerDesc_t &serializer, void *pObject, int nEntityIndex, NetworkSerializationMode_t nMode, void * ) = 0;
	// AMNOTE: Writes the variant class names of metadata gathered by GatherPolymorphicMetadata, the third argument is unused
	virtual void GetPolymorphicMetadataDescription( const FlattenedSerializerDesc_t &serializer, const void *pMetadata, void *, CUtlVector<CUtlString> *pOutLines ) = 0;
	virtual bool HasPolymorphicFields( const FlattenedSerializerDesc_t &serializer ) = 0;
	virtual void PurgeTemporaryData() = 0;
	virtual bool UserGroupHasFields( const FlattenedSerializerDesc_t &serializer, const char *pszUserGroup ) = 0;
	// AMNOTE: The fields MNetworkOutOfPVSUpdates opts in and out
	virtual bool GetOutOfPVSUpdateFields( const FlattenedSerializerDesc_t &serializer, void **ppOptIn, void **ppOptOut ) = 0;
};

#endif /* FLATTENEDSERIALIZERS_H */
