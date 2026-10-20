//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: This header should never be used directly from leaf code!!!
// Instead, just add the file memoverride.cpp into your project and all this
// will automagically be used
//
// $NoKeywords: $
//=============================================================================//

#ifndef TIER0_MEMALLOC_H
#define TIER0_MEMALLOC_H

#ifdef _WIN32
#pragma once
#endif

#include "platform.h"
#include <stddef.h>
#if defined( OSX )
#include <malloc/malloc.h>
#endif

#ifdef TIER0_DLL_EXPORT
#  define MEM_INTERFACE DLL_EXPORT
#else
#  define MEM_INTERFACE DLL_IMPORT
#endif

#if !defined(STEAM) && !defined(NO_MALLOC_OVERRIDE)

// IMemAlloc::Alloc and Free
#define MEMALLOC_REGION_ALLOC_DEFAULT			'4'
#define MEMALLOC_REGION_FREE_DEFAULT			'5'

// IMemAlloc::AllocAligned and FreeAligned
#define MEMALLOC_REGION_ALLOC_ALIGNED			'6'
#define MEMALLOC_REGION_FREE_ALIGNED			'7'

// The tier0 heap allocator's aligned methods, internal to tier0
#define MEMALLOC_REGION_ALLOC_HEAP_ALIGNED		'8'
#define MEMALLOC_REGION_FREE_HEAP_ALIGNED		'9'

// KeyValues3 clusters and metadata
#define MEMALLOC_REGION_ALLOC_KV3				':'
#define MEMALLOC_REGION_FREE_KV3				';'

// Aligned operator new and delete
#define MEMALLOC_REGION_ALLOC_NEW_ALIGNED		'<'
#define MEMALLOC_REGION_FREE_NEW_ALIGNED		'='

#define MEMALLOC_REGION_ALLOC_UNK6				'>'
#define MEMALLOC_REGION_FREE_UNK6				'?'

// Aligned operator new[] and delete[]
#define MEMALLOC_REGION_ALLOC_NEW_ARRAY_ALIGNED	'@'
#define MEMALLOC_REGION_FREE_NEW_ARRAY_ALIGNED	'A'

enum MemAllocAttribute_t
{
	MemAllocAttribute_Unk0 = 0,
	MemAllocAttribute_Unk1 = 1,
	MemAllocAttribute_Unk2 = 2
};

enum MemoryState
{
	MemoryState_UnexpectedlyAllocated = 0,
	MemoryState_UnexpectedlyFreed = 1,
	MemoryState_UnexpectedlyUnrecognized = 2,
	MemoryState_Corrupt = 3,
	MemoryState_Invalid = 4,
	MemoryState_Operational = 5,
	MemoryState_Unknown = 6
};

enum MemoryDebugType : int16
{
	MemoryDebugType_None = 0,
	MemoryDebugType_Light = 1,
	MemoryDebugType_Full = 2,
	// MemSetDbg fills with 0xD4 bytes
	MemoryDebugType_Unk3 = 3,
	// Reported by the Windows process heap allocator
	MemoryDebugType_Unk4 = 4
};

struct MemoryInfoState
{
	MemoryInfoState( MemoryDebugType type, bool debug = false ) : m_MemType( type ), m_bDebug( debug ) { }

	MemoryDebugType m_MemType;
	bool m_bDebug;
};

class CBufferString;

#define MEMALLOC_VERSION 1

typedef size_t (*MemAllocFailHandler_t)( size_t );

PLATFORM_INTERFACE void CMemAllocSystemInitialize();

//-----------------------------------------------------------------------------
// NOTE! This should never be called directly from leaf code
// Just use new,delete,malloc,free etc. They will call into this eventually
//-----------------------------------------------------------------------------
abstract_class IMemAlloc
{
	// AMNOTE: A lot of functions in here might be stubbed out and not do what their description tells
	// this heavily depends on the allocator implementation and should be taken into account when using this directly!

private:
	virtual ~IMemAlloc() = 0;

public:
	// Release versions
	virtual void *Alloc( size_t nSize ) = 0;
	virtual void *Realloc( void *pMem, size_t nSize ) = 0;
	virtual void Free( void *pMem ) = 0;

	virtual void *AllocAligned( size_t nSize, size_t align ) = 0;
	virtual void *ReallocAligned( void *pMem, size_t nSize, size_t align ) = 0;
	virtual void FreeAligned( void *pMem ) = 0;

	inline void *IndirectAlloc( size_t nSize ) { return Alloc( nSize ); }
	inline void *IndirectAllocAligned( size_t nSize, size_t align ) { return AllocAligned( nSize, align ); }

	// AMNOTE: It's unclear if the functions below are actually a debug variants, as in binaries they are
	// absolutely the same to the above functions, but with a different name as they aren't merged together
	// in the vtable in the win binaries. So it's mostly a guess, same goes to their params.
	// ===============================================================
	virtual void *AllocDbg( size_t nSize ) = 0;
	virtual void *ReallocDbg( void *pMem, size_t nSize ) = 0;
	virtual void FreeDbg( void *pMem ) = 0;

	virtual void *AllocAlignedDbg( size_t nSize, size_t align ) = 0;
	virtual void *ReallocAlignedDbg( void *pMem, size_t nSize, size_t align ) = 0;
	virtual void FreeAlignedDbg( void *pMem ) = 0;
	// ===============================================================

	// Region-based allocations
	// Use MEMALLOC_REGION_* defines for the region arg
	// AMNOTE: Region name is mostly a guess!
	virtual void *RegionAlloc( uint8 region, size_t nSize ) = 0;
	virtual void RegionFree( uint8 region, void *pMem ) = 0;

	virtual void *RegionAllocAligned( uint8 region, size_t nSize, size_t align ) = 0;
	virtual void RegionFreeAligned( uint8 region, void *pMem ) = 0;

	// Returns size of a particular allocation
	virtual size_t GetSize( void *pMem ) = 0;
	virtual size_t GetSizeAligned( void *pMem ) = 0;

	// Writes the stats to "<out_path>.txt" next to the executable, or to "memstats_<n>.txt" when out_path is NULL,
	// "<stdout>" prints them to the console instead. Does nothing with the default pass-through allocator
	virtual void DumpStats( const char *out_path ) = 0;

	// AMNOTE: Stub, returns -1
	virtual int unk001() = 0;

	// AMNOTE: Stub
	virtual void unk002() = 0;

	// AMNOTE: Used by the CMemoryStack::Init
	virtual void *LockAllocStatsForCurrentStack( uint32 &allocid, bool ) = 0;
	virtual void *LockAllocStats( uint32 allocid ) = 0;
	virtual void UnlockAllocStats( uint32 allocid ) = 0;

	virtual void CompactOnFail() = 0;

	// Logs the out of memory message, breaks in the debugger and exits the process
	virtual void ReportFailedAllocation( size_t nSize ) = 0;

	// memset's at the pMem location with nSize bytes of specified type, where:
	// MemoryDebugType_Light - would memset with 0xDD bytes;
	// MemoryDebugType_Full - would memset with 0xD8 bytes;
	// MemoryDebugType_Unk3 - would memset with 0xD4 bytes;
	// the input pMem is returned
	//
	// NOTE: This would do nothing unless state has m_bDebug set or the allocator is in the debug memory mode
	virtual void *MemSetDbg( void *pMem, size_t nSize, MemoryInfoState state ) = 0;

	// Returns true if the underlying allocator is using DebugMemoryType different to DebugMemoryType::DebugMemoryType_None
	// would also return true if the state argument has m_bDebug set as true
	virtual bool IsInDebugMode( MemoryInfoState state ) = 0;

	// If memory is not in the debug mode, returns either MemoryState_Invalid or MemoryState_Operational
	// Otherwise a more deep check would be performed
	virtual MemoryState GetMemoryState( void *pMem ) = 0;

	// Logs a warning and breaks in a debugger if the pMem state doesn't match provided state,
	// MemoryState_Unknown matches any state, but MemoryState_Corrupt and MemoryState_Invalid are
	// always reported. Returns the state pMem is in
	virtual MemoryState ReportBadMemory( void *pMem, MemoryState state = MemoryState_Unknown ) = 0;

	// Returns memory debug type of this allocator
	virtual MemoryDebugType GetMemoryDebugType() = 0;

	// Returns previous total allocation size that was used
	// AMNOTE: Likely unused, the allocators only store the value
	virtual size_t SetTotalAllocationSize( size_t new_total_alloc_size ) = 0;

	// Returns previous allocation limit size that was used
	// AMNOTE: Likely unused, the allocators only store the value
	virtual size_t SetAllocationLimitSize( size_t new_alloc_limit_size ) = 0;

	// Writes detailed info about this allocator and settings used
	// Example output of a non debug allocator:
	// Heap: standard allocator pass-through to low-level
	// Low-level allocator: mimalloc
	//
	// Example output of a debug allocator with custom settings:
	// Heap: standard allocator + mem init + stackstats + light verifier + full mem debug
	// Low-level allocator: mimalloc
	virtual void GetAllocatorDescription( CBufferString &buf ) = 0;

	// Returns true if stackstats is enabled and -memstackstats_disable_pools launch option was used
	virtual bool IsStackStatsPoolsDisabled() = 0;

	// Returns true if stackstats is enabled for this allocator
	virtual bool IsStackStatsEnabled() = 0;

	virtual int GetAllocAttributes( void *pMem ) = 0;
	virtual void SetAllocAttributes( void *pMem, MemAllocAttribute_t alloc_attribute, int unk ) = 0;

	// AMNOTE: Copies data to an unknown struct of byte size 56
	// Returns true if data was written, false otherwise
	virtual bool unk101( void *out_val ) = 0;

	// Calls the lower-level allocator functions directly
	virtual void *AllocRaw( size_t nSize ) = 0;
	virtual void *ReallocRaw( void *pMem, size_t nSize ) = 0;
	virtual void FreeRaw( void *pMem ) = 0;
	virtual size_t GetSizeRaw( void *pMem ) = 0;
};

//-----------------------------------------------------------------------------
// Singleton interface
//-----------------------------------------------------------------------------
MEM_INTERFACE IMemAlloc *g_pMemAlloc;

// g_pMemAlloc's methods as plain functions, for code that takes allocation function pointers
MEM_INTERFACE void *MemAlloc_AllocFunc( size_t nSize );
MEM_INTERFACE void *MemAlloc_CallocFunc( size_t nCount, size_t nSize );
MEM_INTERFACE void *MemAlloc_ReallocFunc( void *pMem, size_t nSize );
MEM_INTERFACE void MemAlloc_FreeFunc( void *pMem );
MEM_INTERFACE void *MemAlloc_AllocAlignedFunc( size_t nSize, size_t align );
MEM_INTERFACE void MemAlloc_FreeAlignedFunc( void *pMem );
MEM_INTERFACE size_t MemAlloc_GetSizeFunc( void *pMem );
MEM_INTERFACE int MemAlloc_GetAllocAttributesFunc( void *pMem );
MEM_INTERFACE void MemAlloc_SetAllocAttributesFunc( void *pMem, MemAllocAttribute_t alloc_attribute, int unk );
// Return nullptr for a nullptr string
MEM_INTERFACE char *MemAlloc_StrDupFunc( const char *pString );
MEM_INTERFACE char *MemAlloc_StrDupWrapperFunc( const char *pString );
MEM_INTERFACE wchar_t *MemAlloc_WcStrDupFunc( const wchar_t *pString );

//-----------------------------------------------------------------------------

inline void *MemAlloc_Alloc( size_t nSize )
{ 
	return g_pMemAlloc->IndirectAlloc( nSize );
}

inline void *MemAlloc_Alloc( size_t nSize, const char *pFileName, int nLine )
{ 
	return g_pMemAlloc->IndirectAlloc( nSize /*, pFileName, nLine*/ );
}
inline void MemAlloc_Free( void *ptr )
{
	g_pMemAlloc->Free( ptr );
}
inline void MemAlloc_Free( void *ptr, const char *pFileName, int nLine )
{
	g_pMemAlloc->Free( ptr /*, pFileName, nLine*/ );
}

//-----------------------------------------------------------------------------

inline bool ValueIsPowerOfTwo( size_t value )			// don't clash with mathlib definition
{
	return (value & ( value - 1 )) == 0;
}


inline void *MemAlloc_AllocAlignedUnattributed( size_t size, size_t align )
{
	return g_pMemAlloc->IndirectAllocAligned( size, align );
}

#define MemAlloc_AllocAligned( s, a )	MemAlloc_AllocAlignedUnattributed( s, a )

inline void *MemAlloc_ReallocAligned( void *ptr, size_t size, size_t align )
{
	return g_pMemAlloc->ReallocAligned( ptr, size, align );
}

inline void MemAlloc_FreeAligned( void *pMemBlock )
{
	g_pMemAlloc->FreeAligned( pMemBlock );
}

inline size_t MemAlloc_GetSizeAligned( void *pMemBlock )
{
	return g_pMemAlloc->GetSizeAligned( pMemBlock );
}

struct aligned_tmp_t
{
	// empty base class
};

// template here to allow adding alignment at levels of hierarchy that aren't the base
template< int bytesAlignment = 16, class T = aligned_tmp_t >
class CAlignedNewDelete : public T
{
public:
	void *operator new( size_t nSize )
	{
		return MemAlloc_AllocAligned( nSize, bytesAlignment );
	}

	void operator delete(void *pData)
	{
		if ( pData )
		{
			MemAlloc_FreeAligned( pData );
		}
	}
};

//-----------------------------------------------------------------------------

#elif defined( POSIX )

#if defined( OSX )
// Mac always aligns allocs, don't need to call posix_memalign which doesn't exist in 10.5.8 which TF2 still needs to run on
//inline void *memalign(size_t alignment, size_t size) {void *pTmp=NULL; posix_memalign(&pTmp, alignment, size); return pTmp;}
inline void *memalign(size_t alignment, size_t size) {void *pTmp=NULL; pTmp = malloc(size); return pTmp;}
#endif

inline void *_aligned_malloc( size_t nSize, size_t align )															{ return memalign( align, nSize ); }
inline void _aligned_free( void *ptr )																				{ free( ptr ); }

inline void *MemAlloc_Alloc( size_t nSize, const char *pFileName = NULL, int nLine = 0 )							{ return malloc( nSize ); }
inline void MemAlloc_Free( void *ptr, const char *pFileName = NULL, int nLine = 0 )									{ free( ptr ); }

inline void *MemAlloc_AllocAligned( size_t size, size_t align )													{ return memalign( align, size ); }
inline void MemAlloc_FreeAligned( void *pMemBlock ) 																{ free( pMemBlock ); }

#if defined( OSX )
inline size_t _msize( void *ptr )																					{ return malloc_size( ptr ); }
#else
inline size_t _msize( void *ptr )																					{ return malloc_usable_size( ptr ); }
#endif

inline void *MemAlloc_ReallocAligned( void *ptr, size_t size, size_t align )
{
	void *ptr_new_aligned = memalign( align, size );

	if( ptr_new_aligned )
	{
		size_t old_size = _msize( ptr );
		size_t copy_size = ( size < old_size ) ? size : old_size;

		memcpy( ptr_new_aligned, ptr, copy_size );
		free( ptr );
	}

	return ptr_new_aligned;
}
#endif // !STEAM && !NO_MALLOC_OVERRIDE

//-----------------------------------------------------------------------------

#if !defined(STEAM)
#define MEM_ALLOC_CREDIT_(tag)	((void)0)
#define MEM_ALLOC_CREDIT()	MEM_ALLOC_CREDIT_(__FILE__)
#define MEM_ALLOC_CREDIT_CLASS()
#define MEM_ALLOC_CREDIT_FUNCTION()
#define MEM_ALLOC_CLASSNAME(type) NULL
#define MemAlloc_RegisterAllocation( pFileName, nLine, nLogicalSize, nActualSize, nTime ) ((void)0)
#define MemAlloc_RegisterDeallocation( pFileName, nLine, nLogicalSize, nActualSize, nTime ) ((void)0)
#endif // !STEAM

//-----------------------------------------------------------------------------

#endif /* TIER0_MEMALLOC_H */
