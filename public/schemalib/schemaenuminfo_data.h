#ifndef SCHEMAENUMINFO_DATA_H
#define SCHEMAENUMINFO_DATA_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"
#include "schemalib/schemametadata_data.h"

class CSchemaEnumInfo;
class CSchemaSystemTypeScope;

enum SchemaEnumFlags_t : uint8
{
	SCHEMA_EF_IS_REGISTERED = (1 << 0),
	SCHEMA_EF_MODULE_LOCAL_TYPE_SCOPE = (1 << 1),
	SCHEMA_EF_GLOBAL_TYPE_SCOPE = (1 << 2),
	SCHEMA_EF_UNK003 = (1 << 3),
	SCHEMA_EF_UNK004 = (1 << 4), 
	SCHEMA_EF_UNK005 = (1 << 5)
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

#endif // SCHEMAENUMINFO_DATA_H
