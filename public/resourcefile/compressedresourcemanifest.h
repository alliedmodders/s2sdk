#ifndef COMPRESSEDRESOURCEMANIFEST_H
#define COMPRESSEDRESOURCEMANIFEST_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/utlstring.h"
#include "tier0/utlstringmap.h"
#include "tier0/utlsymbol.h"
#include "tier0/utlsymboltable.h"

// A list of resource names, each split into a path and an extension shared through symbol tables
class CCompressedResourceManifest
{
public:
	struct CompressedResourceName_t
	{
		CUtlSymbol m_path; // In m_paths
		CUtlString m_baseFileName;
		CUtlSymbol m_extension; // In m_extensions
	};

	CUtlStringMap<CompressedResourceName_t> m_resources;
	CUtlSymbolTable m_paths;
	CUtlSymbolTable m_extensions;
};

#endif // COMPRESSEDRESOURCEMANIFEST_H
