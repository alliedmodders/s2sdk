#ifndef SCHEMAENUMINFO_DATA_H
#define SCHEMAENUMINFO_DATA_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"
#include "schemalib/schemametadata_data.h"

class CSchemaEnumInfo;
class CSchemaSystemTypeScope;

enum SchemaEnumFlags1_t : uint8
{
	SCHEMA_EF1_INFO_IS_INITIALIZED = (1 << 0),
	SCHEMA_EF1_MODULE_LOCAL_TYPE_SCOPE = (1 << 1),
	SCHEMA_EF1_GLOBAL_TYPE_SCOPE = (1 << 2),
	SCHEMA_EF1_UNSIGNED_STORAGE_TYPE = (1 << 3),
	SCHEMA_EF1_IS_FLAGS_ENUM = (1 << 4),
	SCHEMA_EF1_IS_SCOPED_ENUM = (1 << 5)
};

struct SchemaEnumeratorInfoData_t
{
	const char* m_pName;
	
	int64 m_nValue;
	
	uint16 m_nNumMetadata;
	SchemaMetadataEntryData_t* m_pMetadata;
};

struct SchemaEnumInfoData_t
{
	CSchemaEnumInfo* m_pPrimaryBinding;
	
	const char* m_pName;
	const char* m_pProjectName;
	
	uint8 m_nSizeOf;
	uint8 m_nAlignOf;
	
	uint16 m_nEnumFlags1;

	uint16 m_nNumEnumerators;
	uint16 m_nNumMetadata;
	
	SchemaEnumeratorInfoData_t* m_pEnumerators;
	SchemaMetadataEntryData_t* m_pMetadata;
	
	CSchemaSystemTypeScope* m_pOuterTypeScope;
	
	int64 m_nMinValue;
	int64 m_nMaxValue;
};

#endif // SCHEMAENUMINFO_DATA_H
