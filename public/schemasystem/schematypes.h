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

class CSchemaSystemTypeScope;
class CSchemaClassInfo;
class CSchemaEnumInfo;
class CBufferString;
struct SchemaAtomicTypeInfo_t;
struct datamap_t;

enum SchemaClassFlags1_t : uint32
{
	SCHEMA_CF1_HAS_VIRTUAL_MEMBERS = (1 << 0),
	SCHEMA_CF1_IS_ABSTRACT = (1 << 1),
	SCHEMA_CF1_HAS_TRIVIAL_CONSTRUCTOR = (1 << 2),
	SCHEMA_CF1_HAS_TRIVIAL_DESTRUCTOR = (1 << 3),
	SCHEMA_CF1_LIMITED_METADATA = (1 << 4),
	SCHEMA_CF1_INHERITANCE_DEPTH_CALCULATED = (1 << 5),
	SCHEMA_CF1_MODULE_LOCAL_TYPE_SCOPE = (1 << 6),
	SCHEMA_CF1_GLOBAL_TYPE_SCOPE = (1 << 7),
	SCHEMA_CF1_CONSTRUCT_ALLOWED = (1 << 8),
	SCHEMA_CF1_CONSTRUCT_DISALLOWED = (1 << 9),
	SCHEMA_CF1_INFO_TAG_MNetworkAssumeNotNetworkable = (1 << 10),
	SCHEMA_CF1_INFO_TAG_MNetworkNoBase = (1 << 11),
	SCHEMA_CF1_INFO_TAG_MIgnoreTypeScopeMetaChecks = (1 << 12),
	SCHEMA_CF1_INFO_TAG_MDisableDataDescValidation = (1 << 13),
	SCHEMA_CF1_INFO_TAG_MClassHasEntityLimitedDataDesc = (1 << 14),
	SCHEMA_CF1_INFO_TAG_MClassHasCustomAlignedNewDelete = (1 << 15),
	SCHEMA_CF1_INFO_TAG_MNonConstructibleClassBase = (1 << 16),
	SCHEMA_CF1_INFO_TAG_MConstructibleClassBase = (1 << 17),
	SCHEMA_CF1_INFO_TAG_MHasKV3TransferPolymorphicClassname = (1 << 18),
};

enum SchemaClassFlags2_t : uint32 {};

enum SchemaEnumFlags_t : uint8
{
	SCHEMA_EF_IS_REGISTERED = (1 << 0),
	SCHEMA_EF_MODULE_LOCAL_TYPE_SCOPE = (1 << 1),
	SCHEMA_EF_GLOBAL_TYPE_SCOPE = (1 << 2),
	SCHEMA_EF_UNK003 = (1 << 3),
	SCHEMA_EF_UNK004 = (1 << 4), 
	SCHEMA_EF_UNK005 = (1 << 5)
};

// Works in combination with SchemaClassManipulatorFn_t
enum SchemaClassManipulatorAction_t
{
	// Registers pObject in a schemasystem
	SCHEMA_CLASS_MANIPULATOR_ACTION_REGISTER = 0,
	SCHEMA_CLASS_MANIPULATOR_ACTION_REGISTER_PRE,

	// Allocates object on the heap and constructs it in place, pObject is unused
	SCHEMA_CLASS_MANIPULATOR_ACTION_ALLOCATE,
	// Deallocates pObject
	SCHEMA_CLASS_MANIPULATOR_ACTION_DEALLOCATE,

	// Constructs pObject in place
	SCHEMA_CLASS_MANIPULATOR_ACTION_CONSTRUCT_IN_PLACE,
	// Destructs pObject in place
	SCHEMA_CLASS_MANIPULATOR_ACTION_DESCTRUCT_IN_PLACE,

	// Returns schema binding of pObject
	SCHEMA_CLASS_MANIPULATOR_ACTION_GET_SCHEMA_BINDING,
};

typedef void *(*SchemaClassManipulatorFn_t)(SchemaClassManipulatorAction_t eAction, void *pObject);

template <class K, class V>
class CSchemaPtrMap
{
public:
	CUtlOrderedMap<K, V, CDefLess<K>, unsigned short> m_Map;
	CThreadFastMutex m_Mutex;
};

struct SchemaMetadataEntryData_t
{
	const char* m_pszName;
	void* m_pData;
};

struct SchemaClassFieldData_t
{
	const char* m_pszName;
	
	CSchemaType* m_pType;
	
	int m_nSingleInheritanceOffset;
	
	uint16 m_nStaticMetadataCount;
	SchemaMetadataEntryData_t* m_pStaticMetadata;
};

struct SchemaStaticFieldData_t
{
	const char* m_pszName;
	
	CSchemaType* m_pType;
	
	void* m_pInstance;
	
	uint16 m_nStaticMetadataCount;
	SchemaMetadataEntryData_t* m_pStaticMetadata;
};

struct SchemaBaseClassInfoData_t
{
	uint m_nOffset;
	CSchemaClassInfo* m_pClass;
};

struct SchemaClassInfoData_t
{
	CSchemaClassInfo* m_pSchemaBinding;
	
	const char* m_pszName;
	const char* m_pszProjectName;
	const char* m_pszCPPName;

	int m_nSize;
	
	uint16 m_nFieldCount;
	uint16 m_nStaticMetadataCount;
	
	uint8 m_nAlignment;
	uint8 m_nBaseClassCount;
	
	uint16 m_nMultipleInheritanceDepth;
	uint16 m_nSingleInheritanceDepth;
	
	SchemaClassFieldData_t* m_pFields;
	SchemaBaseClassInfoData_t* m_pBaseClasses;
	datamap_t* m_pDataDescMap;
	SchemaMetadataEntryData_t* m_pStaticMetadata;
	
	CSchemaSystemTypeScope* m_pTypeScope;
	CSchemaType_DeclaredClass* m_pDeclaredClass;
	
	uint32 m_nFlags1;
	uint32 m_nFlags2;
	
	SchemaClassManipulatorFn_t m_pfnManipulator;
};

class CSchemaClassInfo : public SchemaClassInfoData_t
{
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
