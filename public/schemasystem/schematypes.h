#ifndef SCHEMATYPES_H
#define SCHEMATYPES_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/logging.h"
#include "tier0/threadtools.h"
#include "tier0/generichash.h"
#include "tier0/utlmap.h"
#include "tier0/utlstring.h"
#include "schemasystem/baseschema.h"
#include "schemasystem/minschema.h"
#include "schemalib/schematype.h"
#include "schemalib/schemametadata_data.h"
#include "schemalib/schemaclassinfo_data.h"
#include "schemalib/schemaclassinfo.h"

class CSchemaSystemTypeScope;
class CSchemaClassInfo;
class CSchemaEnumInfo;
class CBufferString;
struct SchemaAtomicTypeInfo_t;
struct datamap_t;

enum SchemaEnumFlags_t : uint8
{
	SCHEMA_EF_IS_REGISTERED = (1 << 0),
	SCHEMA_EF_MODULE_LOCAL_TYPE_SCOPE = (1 << 1),
	SCHEMA_EF_GLOBAL_TYPE_SCOPE = (1 << 2),
	SCHEMA_EF_UNK003 = (1 << 3),
	SCHEMA_EF_UNK004 = (1 << 4), 
	SCHEMA_EF_UNK005 = (1 << 5)
};

template <class K, class V>
class CSchemaPtrMap
{
public:
	CUtlOrderedMap<K, V, CDefLess<K>, unsigned short> m_Map;
	CThreadFastMutex m_Mutex;
};

struct SchemaEnumeratorInfoData_t
{
	const char* m_pszName;
	
	int64 m_nValue;
	
	uint16 m_nStaticMetadataCount;
	SchemaMetadataEntryData_t* m_pStaticMetadata;
};

struct SchemaEnumInfoData_t
{
	CSchemaEnumInfo* m_pSchemaBinding;
	
	const char* m_pszName;
	const char* m_pszProjectName;
	
	uint8 m_nSize;
	uint8 m_nAlignment;
	
	uint16 m_nFlags;

	uint16 m_nEnumeratorCount;
	uint16 m_nStaticMetadataCount;
	
	SchemaEnumeratorInfoData_t* m_pEnumerators;
	SchemaMetadataEntryData_t* m_pStaticMetadata;
	
	CSchemaSystemTypeScope* m_pTypeScope;
	
	int64 m_nMinEnumeratorValue;
	int64 m_nMaxEnumeratorValue;
};

class CSchemaEnumInfo : public SchemaEnumInfoData_t
{
};

struct SchemaAtomicTypeInfo_t
{
	const char* m_pszName;
	const char* m_pszTokenName;
	
	int m_nAtomicID;
	
	int m_nStaticMetadataCount;
	SchemaMetadataEntryData_t* m_pStaticMetadata;
};

struct AtomicTypeInfo_T_t
{
	bool operator<( const AtomicTypeInfo_T_t& rhs ) const 
	{ 
		if ( m_nAtomicID != rhs.m_nAtomicID )
			return m_nAtomicID < rhs.m_nAtomicID;
		
		if ( m_pTemplateType != rhs.m_pTemplateType )
			return m_pTemplateType < rhs.m_pTemplateType;
		
		return (void *)m_pfnManipulator < (void *)rhs.m_pfnManipulator;
	}
	
	int m_nAtomicID;
	CSchemaType* m_pTemplateType;
	SchemaCollectionManipulatorFn_t m_pfnManipulator;
};

struct AtomicTypeInfo_CollectionOfT_t
{
	bool operator<( const AtomicTypeInfo_CollectionOfT_t &rhs ) const
	{
		if(m_nAtomicID != rhs.m_nAtomicID)
			return m_nAtomicID < rhs.m_nAtomicID;

		if(m_pTemplateType != rhs.m_pTemplateType)
			return m_pTemplateType < rhs.m_pTemplateType;

		if(m_nFixedBufferCount != rhs.m_nFixedBufferCount)
			return m_nFixedBufferCount < rhs.m_nFixedBufferCount;

		return (void *)m_pfnManipulator < (void *)rhs.m_pfnManipulator;
	}

	int m_nAtomicID;
	CSchemaType *m_pTemplateType;
	uint64 m_nFixedBufferCount;
	SchemaCollectionManipulatorFn_t m_pfnManipulator;
};

struct AtomicTypeInfo_TT_t
{
	bool operator<( const AtomicTypeInfo_TT_t& rhs ) const 
	{ 
		if ( m_nAtomicID != rhs.m_nAtomicID )
			return m_nAtomicID < rhs.m_nAtomicID;
		
		if ( m_pTemplateType != rhs.m_pTemplateType )
			return m_pTemplateType < rhs.m_pTemplateType;
		
		return m_pTemplateType2 < rhs.m_pTemplateType2;
	}
	
	int m_nAtomicID;
	CSchemaType* m_pTemplateType;
	CSchemaType* m_pTemplateType2;
};

struct AtomicTypeInfo_I_t
{
	bool operator<( const AtomicTypeInfo_I_t& rhs ) const 
	{ 
		if ( m_nAtomicID != rhs.m_nAtomicID )
			return m_nAtomicID < rhs.m_nAtomicID;

		return m_nInteger < rhs.m_nInteger;
	}
	
	int m_nAtomicID;
	int m_nInteger;
};

struct TypeAndCountInfo_t
{
	bool operator<( const TypeAndCountInfo_t& rhs ) const 
	{ 
		if ( m_nElementCount != rhs.m_nElementCount )
			return m_nElementCount < rhs.m_nElementCount;
		
		return m_pElementType < rhs.m_pElementType;
	}
	
	int m_nElementCount;
	CSchemaType* m_pElementType;
};

#endif // SCHEMATYPES_H
