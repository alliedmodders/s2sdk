#ifndef RESOURCETYPE_H
#define RESOURCETYPE_H

#ifdef COMPILER_MSVC
#pragma once
#endif

#include <tier0/platform.h>
#include <tier0/dbg.h>
#include <tier0/threadtools.h>
#include <tier0/buffer_string.h>
#include <tier0/generichash.h>
#include <tier0/tier0_strtools.h>
#include <tier0/utlsymbollarge.h>

#include <initializer_list>

enum ResourceBindingFlags_t
{
	RESOURCE_BINDING_LOADED = 0x1,
	RESOURCE_BINDING_ERROR = 0x2,
	RESOURCE_BINDING_UNLOADABLE = 0x4,
	RESOURCE_BINDING_PROCEDURAL = 0x8,
	RESOURCE_BINDING_TRACKLEAKS = 0x20,
	RESOURCE_BINDING_IS_ERROR_BINDING_FOR_TYPE = 0x40,
	RESOURCE_BINDING_HAS_EVER_BEEN_LOADED = 0x80,
	RESOURCE_BINDING_ANONYMOUS = 0x100,

	RESOURCE_BINDING_FIRST_UNUSED_FLAG = 0x200
};

struct ResourceId_t
{
	ResourceId_t() : m_Value( 0 ) {}
	ResourceId_t( uint64 nValue ) : m_Value( nValue ) {}

	uint64 m_Value;
};

struct ResourceNameInfo_t
{
	CUtlSymbolLarge m_ResourceNameSymbol;
};

typedef const ResourceNameInfo_t *ResourceNameHandle_t;
typedef uint16 LoadingResourceIndex_t;
typedef uint16 StreamingResourceDataIndex_t;
typedef char ResourceTypeIndex_t;
typedef uint32 ExtRefIndex_t;

struct ResourceBindingBase_t
{
	void* m_pData;
	ResourceNameHandle_t m_Name;
	ResourceId_t m_ResourceId;
	uint16 m_nFlags;
	StreamingResourceDataIndex_t m_nStreamingResource;
	ResourceTypeIndex_t m_nResourceType;
	uint8 m_nReloadCounter;
	LoadingResourceIndex_t m_nLoadingResource;
	CInterlockedInt m_nRefCount;
	ExtRefIndex_t m_nExtRefHandle;
};

typedef const ResourceBindingBase_t* ResourceHandle_t;
typedef void* HGameResourceManifest;
typedef uint64 ResourceType_t;

class CResourceName
{
public:
	CResourceName() : m_Type( 0 ) {}
	CResourceName( const char *pszName ) : CResourceName() { Init( pszName ); }
	CResourceName( const char *pszName, ResourceType_t nType ) : CResourceName() { InitAndForceType( pszName, nType ); }
	CResourceName( const CResourceName &other ) : CResourceName() { CopyFrom( other ); }

	CResourceName &operator=( const CResourceName &other )
	{
		if(this == &other)
			return *this;

		CopyFrom( other );
		return *this;
	}

	void CopyFrom( const CResourceName &other );

	// Returns false and leaves the name empty when FixupResourceName refuses it
	bool Init( const char *pszName ) { return InitAndForceType( pszName, 0 ); }
	bool InitAndForceType( const char *pszName, ResourceType_t nType );

	void Clear()
	{
		m_Name.Clear();
		m_Id = ResourceId_t();
		m_Type = 0;
	}

	bool IsEmpty() const { return m_Name.Length() == 0; }
	const char *GetName() const { return m_Name.Get(); }
	ResourceId_t GetId() const { return m_Id; }
	ResourceType_t GetType() const { return m_Type; }

protected:
	CPathBufferString m_Name;
	ResourceId_t m_Id;
	ResourceType_t m_Type;
};

// Lowercases the name and cleans up its slashes. Empties it and returns false for an absolute path or a name without an extension.
inline bool FixupResourceName( CBufferString &name )
{
	if ( name.Length() == 0 )
		return true;

	if ( V_IsAbsolutePath( name.Get() ) || name.Get()[0] == '/' )
	{
		Warning( "FixupResourceName: Illegal full path passed in (\"%s\")!\n", name.Get() );
		name.Clear();
		return false;
	}

	const char *pszExtension = V_GetFileExtension( name.Get() );
	if ( !pszExtension )
	{
		Warning( "FixupResourceName: Illegal path, missing extension passed in (\"%s\")!\n", name.Get() );
		name.Clear();
		return false;
	}

	const char *pszName = name.Get();
	int nLength = name.Length();

	CPathBufferString fixed;
	char *pszFixed = fixed.SetLength( nLength );
	int nFixedLength = 0;
	char chLast = '\0';
	bool bHasDotParts = false;

	for ( int i = 0; i < nLength; i++ )
	{
		char ch = pszName[i];
		if ( ch >= 'A' && ch <= 'Z' )
			ch += 'a' - 'A';

		if ( ch == '\\' || ch == '/' )
		{
			ch = '/';
			if ( chLast == '/' )
				continue;
		}
		else if ( ( ch == '.' || ch == ':' ) && &pszName[i] < pszExtension - 1 )
		{
			bHasDotParts = true;
		}

		pszFixed[nFixedLength++] = ch;
		chLast = ch;
	}

	pszFixed[nFixedLength] = '\0';

	if ( bHasDotParts )
	{
		V_RemoveDotSlashes( pszFixed, '/' );
		nFixedLength = (int)strlen( pszFixed );
	}

	name.Clear();
	name.Insert( 0, pszFixed, nFixedLength );
	return true;
}

// Gives a name without an extension the one of nType, and refuses a name that has a different one
inline bool FixupResourceName( ResourceType_t nType, CBufferString &name )
{
	if ( name.Length() == 0 )
		return true;

	if ( V_IsAbsolutePath( name.Get() ) || name.Get()[0] == '/' )
	{
		Warning( "FixupResourceName: Illegal full path passed in (\"%s\")!\n", name.Get() );
		name.Clear();
		return false;
	}

	char szTypeExtension[sizeof( ResourceType_t ) + 1] = {};
	memcpy( szTypeExtension, &nType, sizeof( ResourceType_t ) );

	const char *pszExtension = V_GetFileExtension( name.Get() );
	if ( !pszExtension )
	{
		name.SetExtension( szTypeExtension );
	}
	else if ( V_stricmp_fast( szTypeExtension, pszExtension ) != 0 )
	{
		Warning( "WARNING: Resource name \"%s\" has the incorrect extension \"%s\" for the specified resource type (expected \"%s\")!\n", name.Get(), pszExtension, szTypeExtension );
		name.Clear();
		return false;
	}

	name.FixupPathName( CORRECT_PATH_SEPARATOR );
	name.ToLowerFast( 0 );
	name.FixSlashes( '/' );
	return true;
}

// The lowercased extension up to its first underscore, as characters from the low byte up. 0 when it is longer than 8.
inline ResourceType_t DeduceResourceTypeFromResourceName( const char *pszName )
{
	const char *pszExtension = pszName ? V_GetFileExtension( pszName ) : nullptr;
	if ( !pszExtension )
		return 0;

	ResourceType_t nType = 0;

	for ( int i = 0; pszExtension[i] && pszExtension[i] != '_'; i++ )
	{
		if ( i == sizeof( ResourceType_t ) )
			return 0;

		char ch = pszExtension[i];
		if ( ch >= 'A' && ch <= 'Z' )
			ch += 'a' - 'A';

		nType |= (ResourceType_t)(uint8)ch << ( i * 8 );
	}

	return nType;
}

inline void CResourceName::CopyFrom( const CResourceName &other )
{
	m_Name.Clear();
	m_Name.Insert( 0, other.GetName() );
	m_Id = other.m_Id;
	m_Type = other.m_Type;
}

inline bool CResourceName::InitAndForceType( const char *pszName, ResourceType_t nType )
{
	Clear();
	m_Name.Insert( 0, pszName );

	if ( IsEmpty() )
		return true;

	bool bFixed = nType ? FixupResourceName( nType, m_Name ) : FixupResourceName( m_Name );
	if ( !bFixed )
		return false;

	m_Id = ResourceId_t( MurmurHash64( m_Name.Get(), m_Name.Length(), 0xEDABCDEF ) );
	m_Type = DeduceResourceTypeFromResourceName( m_Name.Get() );
	return true;
}

#endif // RESOURCETYPE_H
