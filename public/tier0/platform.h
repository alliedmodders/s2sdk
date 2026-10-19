//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose:
//
// $NoKeywords: $
//
//===========================================================================//

#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef COMPILER_MSVC
#pragma once
#endif

#include "tier0/platform_compile_base.h"

class CBufferString;

//-----------------------------------------------------------------------------
// Message Box
//-----------------------------------------------------------------------------
#if defined( PLATFORM_WINDOWS_PC )
PLATFORM_INTERFACE void Plat_MessageBox( const char *pTitle, const tchar *pMessage, HWND hwnd = nullptr );
#else
#define Plat_MessageBox( ... ) ((void)0)
#endif


PLATFORM_INTERFACE void Plat_Printf ( const tchar *pFmt, ... ) FMTFUNCTION( 1, 2 );

//-----------------------------------------------------------------------------
// Posix platform helpers
//-----------------------------------------------------------------------------
#ifdef PLATFORM_POSIX

// Visual Studio likes to put an underscore in front of anything that looks like a portable function.
#define _strupr strupr
#define _getcwd getcwd
#define _open open
#define _lseek lseek
#define _read read
#define _close close
#define _vsnprintf vsnprintf
#define _stat stat
#define _O_RDONLY O_RDONLY
#define _stricmp strcasecmp
#define stricmp strcasecmp
#define _strnicmp strncasecmp
#define strnicmp strncasecmp
#define strcmpi strcasecmp
#define _alloca alloca
#define _finite finite
#define _unlink unlink
#define _putenv putenv
#define _chdir chdir
#define _access access
#define _strtoi64 strtoll

#if !defined( _snprintf )	// some vpc's define this on the command line
#define _snprintf snprintf
#endif

#include <alloca.h>
#include <unistd.h>											// get unlink
#include <signal.h>
#include <errno.h>

#endif // PLATFORM_POSIX


//-----------------------------------------------------------------------------
// Generally useful platform-independent macros (move to another file?)
//-----------------------------------------------------------------------------

// need macro for constant expression
#define ALIGN_VALUE( val, alignment ) ( ( val + alignment - 1 ) & ~( alignment - 1 ) )

#if ( defined(__SANITIZE_ADDRESS__) && __SANITIZE_ADDRESS__ )
	#define NO_ASAN __attribute__((no_sanitize("address")))
	#define NO_ASAN_FORCEINLINE NO_ASAN inline
#else
	#define NO_ASAN
	#define NO_ASAN_FORCEINLINE FORCEINLINE
#endif

// Force a function call site -not- to inlined. (useful for profiling)
#define DONT_INLINE(a) (((int)(a)+1)?(a):(a))

// Marks the codepath from here until the next branch entry point as unreachable,
// and asserts if any attempt is made to execute it.
#define UNREACHABLE() { Assert(0); HINT(0); }

// In cases where no default is present or appropriate, this causes MSVC to generate
// as little code as possible, and throw an assertion in debug.
#define NO_DEFAULT default: UNREACHABLE();

#define MAX_FILEPATH 512 

// Defines MAX_PATH
#ifndef MAX_PATH
	#define MAX_PATH  260
#endif

#ifdef _WIN32
#define MAX_UNICODE_PATH 32767
#else
#define MAX_UNICODE_PATH MAX_PATH
#endif

#define MAX_UNICODE_PATH_IN_UTF8 MAX_UNICODE_PATH*4

//-----------------------------------------------------------------------------
// FP exception handling
//-----------------------------------------------------------------------------
//#define CHECK_FLOAT_EXCEPTIONS		1

#if defined( COMPILER_MSVC64 )

	inline void SetupFPUControlWord()
	{
	}

#elif defined ( COMPILER_MSVC32 )

	inline void SetupFPUControlWordForceExceptions()
	{
		// use local to get and store control word
		uint16 tmpCtrlW;
		__asm
		{
			fnclex						/* clear all current exceptions */
			fnstcw word ptr [tmpCtrlW]	/* get current control word */
			and [tmpCtrlW], 0FCC0h		/* Keep infinity control + rounding control */
			or [tmpCtrlW], 0230h		/* set to 53-bit, mask only inexact, underflow */
			fldcw word ptr [tmpCtrlW]	/* put new control word in FPU */
		}
	}

	#ifdef CHECK_FLOAT_EXCEPTIONS

		inline void SetupFPUControlWord()
		{
			SetupFPUControlWordForceExceptions();
		}

	#else

		inline void SetupFPUControlWord()
		{
			// use local to get and store control word
			uint16 tmpCtrlW;
			__asm
			{
				fnstcw word ptr [tmpCtrlW]	/* get current control word */
				and [tmpCtrlW], 0FCC0h		/* Keep infinity control + rounding control */
				or [tmpCtrlW], 023Fh		/* set to 53-bit, mask only inexact, underflow */
				fldcw word ptr [tmpCtrlW]	/* put new control word in FPU */
			}
		}

	#endif

#elif defined ( COMPILER_GCC )

	inline void SetupFPUControlWord()
	{
		__volatile unsigned short int __cw;
		__asm __volatile ("fnstcw %0" : "=m" (__cw));
		__cw = __cw & 0x0FCC0;	// keep infinity control, keep rounding mode
		__cw = __cw | 0x023F;	// set 53-bit, no exceptions
		__asm __volatile ("fldcw %0" : : "m" (__cw));
	}

#endif


//-----------------------------------------------------------------------------
// Purpose: Standard functions for handling endian-ness
//-----------------------------------------------------------------------------

//-------------------------------------
// Basic swaps
//-------------------------------------

template <typename T>
inline T WordSwapC( T w )
{
   uint16 temp;

   COMPILE_TIME_ASSERT( sizeof( T ) == sizeof(uint16) );

   temp  = ((*((uint16 *)&w) & 0xff00) >> 8);
   temp |= ((*((uint16 *)&w) & 0x00ff) << 8);

   return *((T*)&temp);
}

template <typename T>
inline T DWordSwapC( T dw )
{
   uint32 temp;

   COMPILE_TIME_ASSERT( sizeof( T ) == sizeof(uint32) );

   temp  =   *((uint32 *)&dw) 				>> 24;
   temp |= ((*((uint32 *)&dw) & 0x00FF0000) >> 8);
   temp |= ((*((uint32 *)&dw) & 0x0000FF00) << 8);
   temp |= ((*((uint32 *)&dw) & 0x000000FF) << 24);

   return *((T*)&temp);
}

template <typename T>
inline T QWordSwapC( T dw )
{
	// Assert sizes passed to this are already correct, otherwise
	// the cast to uint64 * below is unsafe and may have wrong results 
	// or even crash.
	COMPILE_TIME_ASSERT( sizeof( dw ) == sizeof(uint64) );

	uint64 temp;

	temp  =   *((uint64 *)&dw) 				         >> 56;
	temp |= ((*((uint64 *)&dw) & 0x00FF000000000000ull) >> 40);
	temp |= ((*((uint64 *)&dw) & 0x0000FF0000000000ull) >> 24);
	temp |= ((*((uint64 *)&dw) & 0x000000FF00000000ull) >> 8);
	temp |= ((*((uint64 *)&dw) & 0x00000000FF000000ull) << 8);
	temp |= ((*((uint64 *)&dw) & 0x0000000000FF0000ull) << 24);
	temp |= ((*((uint64 *)&dw) & 0x000000000000FF00ull) << 40);
	temp |= ((*((uint64 *)&dw) & 0x00000000000000FFull) << 56);

	return *((T*)&temp);
}

//-------------------------------------
// Fast swaps
//-------------------------------------

//-------------------------------------
// Fast swaps
//-------------------------------------

#if defined _MSC_VER		// MSVC (What about MinGW and Clang for Windows)

#define WordSwap(d) _byteswap_ushort(d)
#define DWordSwap(d) ((uint32)(_byteswap_ulong( (unsigned long) d)))
#define QWordSwap(d) _byteswap_uint64(d)

#elif defined __GNUC__		// GCC or Clang

#define WordSwap(d) __builtin_bswap16(d)
#define DWordSwap(d) __builtin_bswap32(d)
#define QWordSwap(d) __builtin_bswap64(d)

#else						// N/A, native code

#pragma message( "TODO: Using non-intrinsic byteswap functions..." )

#define WordSwap WordSwapC
#define DWordSwap DWordSwapC
#define QWordSwap QWordSwapC

#endif

//-------------------------------------
// The typically used methods.
//-------------------------------------

#define PLAT_LITTLE_ENDIAN 1


// If a swapped float passes through the fpu, the bytes may get changed.
// Prevent this by swapping floats as DWORDs.
#define SafeSwapFloat( pOut, pIn )	(*((uint*)pOut) = DWordSwap( *((uint*)pIn) ))

#if defined(PLAT_LITTLE_ENDIAN)
#define BigShort( val )				WordSwap( val )
#define BigWord( val )				WordSwap( val )
#define BigLong( val )				DWordSwap( val )
#define BigDWord( val )				DWordSwap( val )
#define LittleShort( val )			( val )
#define LittleWord( val )			( val )
#define LittleLong( val )			( val )
#define LittleDWord( val )			( val )
#define SwapShort( val )			BigShort( val )
#define SwapWord( val )				BigWord( val )
#define SwapLong( val )				BigLong( val )
#define SwapDWord( val )			BigDWord( val )

// Pass floats by pointer for swapping to avoid truncation in the fpu
#define BigFloat( pOut, pIn )		SafeSwapFloat( pOut, pIn )
#define LittleFloat( pOut, pIn )	( *pOut = *pIn )
#define SwapFloat( pOut, pIn )		BigFloat( pOut, pIn )

#elif defined(PLAT_BIG_ENDIAN)

#define BigShort( val )				( val )
#define BigWord( val )				( val )
#define BigLong( val )				( val )
#define BigDWord( val )				( val )
#define LittleShort( val )			WordSwap( val )
#define LittleWord( val )			WordSwap( val )
#define LittleLong( val )			DWordSwap( val )
#define LittleDWord( val )			DWordSwap( val )
#define SwapShort( val )			LittleShort( val )
#define SwapWord( val )				LittleWord( val )
#define SwapLong( val )				LittleLong( val )
#define SwapDWord( val )			LittleDWord( val )

// Pass floats by pointer for swapping to avoid truncation in the fpu
#define BigFloat( pOut, pIn )		( *pOut = *pIn )
#define LittleFloat( pOut, pIn )	SafeSwapFloat( pOut, pIn )
#define SwapFloat( pOut, pIn )		LittleFloat( pOut, pIn )

#else

// @Note (toml 05-02-02): this technique expects the compiler to
// optimize the expression and eliminate the other path. On any new
// platform/compiler this should be tested.
inline short BigShort( short val )		{ int test = 1; return ( *(char *)&test == 1 ) ? WordSwap( val )  : val; }
inline uint16 BigWord( uint16 val )		{ int test = 1; return ( *(char *)&test == 1 ) ? WordSwap( val )  : val; }
inline long BigLong( long val )			{ int test = 1; return ( *(char *)&test == 1 ) ? DWordSwap( val ) : val; }
inline uint32 BigDWord( uint32 val )	{ int test = 1; return ( *(char *)&test == 1 ) ? DWordSwap( val ) : val; }
inline short LittleShort( short val )	{ int test = 1; return ( *(char *)&test == 1 ) ? val : WordSwap( val ); }
inline uint16 LittleWord( uint16 val )	{ int test = 1; return ( *(char *)&test == 1 ) ? val : WordSwap( val ); }
inline long LittleLong( long val )		{ int test = 1; return ( *(char *)&test == 1 ) ? val : DWordSwap( val ); }
inline uint32 LittleDWord( uint32 val )	{ int test = 1; return ( *(char *)&test == 1 ) ? val : DWordSwap( val ); }
inline uint64 LittleQWord( uint64 val )	{ int test = 1; return ( *(char *)&test == 1 ) ? val : QWordSwap( val ); }
inline short SwapShort( short val )					{ return WordSwap( val ); }
inline uint16 SwapWord( uint16 val )				{ return WordSwap( val ); }
inline long SwapLong( long val )					{ return DWordSwap( val ); }
inline uint32 SwapDWord( uint32 val )				{ return DWordSwap( val ); }

// Pass floats by pointer for swapping to avoid truncation in the fpu
inline void BigFloat( float *pOut, const float *pIn )		{ int test = 1; ( *(char *)&test == 1 ) ? SafeSwapFloat( pOut, pIn ) : ( *pOut = *pIn ); }
inline void LittleFloat( float *pOut, const float *pIn )	{ int test = 1; ( *(char *)&test == 1 ) ? ( *pOut = *pIn ) : SafeSwapFloat( pOut, pIn ); }
inline void SwapFloat( float *pOut, const float *pIn )		{ SafeSwapFloat( pOut, pIn ); }

#endif

FORCEINLINE uint32 LoadLittleDWord( const uint32 *base, unsigned int dwordIndex )
	{
		return LittleDWord( base[dwordIndex] );
	}

FORCEINLINE void StoreLittleDWord( uint32 *base, unsigned int dwordIndex, uint32 dword )
	{
		base[dwordIndex] = LittleDWord(dword);
	}

inline uint64 CastPtrToUint64( const void* p )
{
	return (uint64) ( (uintp) p );
}

inline int64 CastPtrToInt64( const void* p )
{
	return (int64) ( (uintp) p );
}

// Returns true if running in test mode.
// Test mode is turned on if -testmode is on the command line or the VALVE_TESTMODE environment variable is defined.
PLATFORM_INTERFACE bool				Plat_IsInTestMode();

PLATFORM_INTERFACE double			Plat_FloatTime();		// Returns time in seconds since the module was loaded.
PLATFORM_INTERFACE uint32			Plat_MSTime();			// Time in milliseconds.
PLATFORM_INTERFACE uint64			Plat_MSTime64();		// Time in milliseconds.
PLATFORM_INTERFACE uint64			Plat_USTime();			// Time in microseconds.
PLATFORM_INTERFACE char *			Plat_ctime64( const time_t *timep, char *buf, size_t bufsize );
PLATFORM_INTERFACE uint64			Plat_GetClockStart();	// Snapshot of the clock when app started.
PLATFORM_INTERFACE time_t			Plat_GetTime();			// Same as time().

// Get the local calendar time.
// Same as time() followed by localtime(), but non-crash-prone and threadsafe.
PLATFORM_INTERFACE void				Plat_GetLocalTime( struct tm *pNow );

// Convert a time_t (specified in nTime - seconds since Jan 1, 1970 UTC) to a local calendar time in a threadsafe and non-crash-prone way.
PLATFORM_INTERFACE void				Plat_ConvertToLocalTime( uint64 nTime, struct tm *pNow );

// Get a time string (same as ascstring, but threadsafe).
PLATFORM_INTERFACE void				Plat_GetTimeString( struct tm *pTime, char *pOut, int nMaxBytes );

// converts a time_t to a struct tm without the local time conversion of ConvertToLocalTime
PLATFORM_INTERFACE struct tm*		Plat_gmtime64( const time_t *nTime, struct tm *pTime );
PLATFORM_INTERFACE time_t			Plat_timegm64( struct tm *timeptr );
PLATFORM_INTERFACE struct tm *		Plat_localtime64( const time_t *timep, struct tm *result );

PLATFORM_INTERFACE const char *		Plat_GetGameDirectory( int unknown=0 );

PLATFORM_INTERFACE bool				Plat_FileExists( const char *pFileName, int flags );
PLATFORM_INTERFACE bool				Plat_FileIsReadOnly( const char *pFileName );
// bDontFollowLinks checks a symbolic link itself rather than its target. Returns -1 when the file doesn't exist.
PLATFORM_INTERFACE int64			Plat_FileSize( const char *pFileName, bool bDontFollowLinks );
PLATFORM_INTERFACE bool				Plat_IsDirectory( const char *pPath, bool bDontFollowLinks );
PLATFORM_INTERFACE bool				Plat_IsFile( const char *pFileName, bool bDontFollowLinks );
PLATFORM_INTERFACE bool				Plat_IsSymlink( const char *pFileName );

// bWriteEveryone gives everyone write access on Linux
PLATFORM_INTERFACE bool				Plat_CreateDirectory( const char *pFileName, bool bWriteEveryone );
// Creates the missing directories of the path, or of the file's path
PLATFORM_INTERFACE bool				Plat_CreateDirectoryTree( const char *pPath, bool bWriteEveryone );
PLATFORM_INTERFACE bool				Plat_CreateDirectoryTreeForFile( const char *pFileName, bool bWriteEveryone );
// Creates the directory unless there is one, following a symbolic link to it
PLATFORM_INTERFACE bool				Plat_EnsureDirectory( const char *pPath, bool bWriteEveryone );
// bDontFollowLinks removes a symbolic link rather than its target
PLATFORM_INTERFACE bool				Plat_RemoveDirectory( const char *pPath, bool bDontFollowLinks );
// Removes the directory and everything in it; nFlags 1 removes a symbolic link at pPath rather than its target
PLATFORM_INTERFACE bool				Plat_RemoveDirectoryTree( const char *pPath, uint32 nFlags );
PLATFORM_INTERFACE bool				Plat_RemoveFile( const char *pFileName, bool bDontFollowLinks );
// Moves the file to the recycle bin on Windows; elsewhere removes it unless bRequireSafe
PLATFORM_INTERFACE bool				Plat_SafeRemoveFile( const char *pFileName, bool bRequireSafe );
PLATFORM_INTERFACE bool				Plat_RenameFile( const char *pFrom, const char *pTo );
PLATFORM_INTERFACE bool				Plat_SetFileReadOnly( const char *pFileName, bool bReadOnly );
// Sets the permission bits of nPermissionMask, which must have executable bits, to nPermissions; Windows only checks it's a file
PLATFORM_INTERFACE bool				Plat_SetFileExecutable( const char *pFileName, uint32 nPermissions, uint32 nPermissionMask );

// Returns 0 or the OS error code, see FioReadFileToMemory
PLATFORM_INTERFACE int				PlatC_ReadFileToMemory( const char *pFileName, void **pContent, int32 *pContentBytes, bool bZeroTerminate, int32 nMaxBytes );

PLATFORM_INTERFACE bool				Plat_GetTempDirectory( CBufferString *pOut );
PLATFORM_INTERFACE void				Plat_GetDocumentDirectory( CBufferString *pOut );
// Create a new temporary directory or file named from pPrefix, in pBaseDir or the temporary directory, and put its path in pOutPath
PLATFORM_INTERFACE bool				Plat_CreateTempDirectory( CBufferString *pOutPath, const char *pPrefix, bool bWriteEveryone, const char *pBaseDir );
// Returns -1 on failure
PLATFORM_INTERFACE OSFILEHANDLE		Plat_CreateOpenTempFile( CBufferString *pOutPath, const char *pPrefix, const char *pBaseDir );
// pMode must be a "w" mode of fopen
PLATFORM_INTERFACE FILE *			Plat_CreateFOpenTempFile( const char *pMode, CBufferString *pOutPath, const char *pPrefix, const char *pBaseDir );

PLATFORM_INTERFACE void				Plat_SetGameDirectory( const char *pGameDir );

// Unbuffers stdin, stdout and stderr
PLATFORM_INTERFACE void				Plat_DisableIOBuffering();

// nStdioFlags: 1 stdout, 2 stderr
PLATFORM_OVERLOAD bool				Plat_RedirectThisProcessStdioToFile( int nFile, uint32 nStdioFlags );
#ifdef PLATFORM_WINDOWS
PLATFORM_OVERLOAD bool				Plat_RedirectThisProcessStdioToFile( void *hFile, uint32 nStdioFlags );
#endif
// Moves the stdio streams redirected to nOldFile to nNewFile
PLATFORM_OVERLOAD bool				Plat_UpdateThisProcessStdioRedirect( int nOldFile, int nNewFile );
// The stdio streams redirected so far
PLATFORM_INTERFACE uint32			Plat_GetThisProcessStdioRedirectFlags();

// AMNOTE: An HKEY on Windows, always nullptr elsewhere
typedef void *PlatRegistryKey_t;
PLATFORM_INTERFACE bool				Plat_RegistryDirectoryOpenOrCreate( const char *rootKey, PlatRegistryKey_t &hKey );
PLATFORM_INTERFACE void				Plat_RegistryDirectoryClose( PlatRegistryKey_t &hKey );

PLATFORM_INTERFACE char const *		Plat_GetEnv( char const *pEnvVarName );

PLATFORM_INTERFACE void				Plat_ExitProcess( int nCode );

PLATFORM_INTERFACE bool				Plat_ShouldCollectMiniDumpsForFatalErrors();

// b/w compatibility
#define Sys_FloatTime Plat_FloatTime

// Protect against bad auto operator=
#define DISALLOW_OPERATOR_EQUAL( _classname )			\
	private:											\
		_classname &operator=( const _classname & );	\
	public:

// Define a reasonable operator=
#define IMPLEMENT_OPERATOR_EQUAL( _classname )			\
	public:												\
		_classname &operator=( const _classname &src )	\
		{												\
			memcpy( this, &src, sizeof(_classname) );	\
			return *this;								\
		}

// Processor Information:
struct CPUInformation
{
	int	 m_Size;		// Size of this structure, for forward compatability.

	uint16 m_nLogicalProcessors;	// Number op logical processors.
	uint16 m_nPhysicalProcessors;	// Number of physical processors

	uint16 m_nECores; // Number of E-Cores
	uint16 m_nPCores; // Number of P-Cores

	bool m_bRDTSC : 1;	// Is RDTSC supported?
	bool m_bRDTSCP : 1;	// Is RDTSCP supported?
	bool m_bInvariantTSC : 1;
	bool m_bCMOV  : 1;  // Is CMOV supported?
	bool m_bFCMOV : 1;  // Is FCMOV supported?
	bool m_bSSE	  : 1;	// Is SSE supported?
	bool m_bSSE2  : 1;	// Is SSE2 Supported?
	bool m_b3DNow : 1;	// Is 3DNow! Supported?
	bool m_bMMX   : 1;	// Is MMX supported?
	bool m_bHT	  : 1;	// Is HyperThreading supported?
	bool m_bSSE3 : 1;
	bool m_bPCLMULQDQ : 1;
	bool m_bSSSE3 : 1;
	bool m_bSSE4a : 1;
	bool m_bSSE41 : 1;
	bool m_bSSE42 : 1;
	bool m_bAVX   : 1;		// Is AVX supported?
	bool m_bAVXDisabled : 1;// Is AVX disabled by the system requirements?
	bool m_bAVX2 : 1;
	bool m_bAVX512 : 1;		// Is AVX-512 F/DQ/VL with VPCLMULQDQ supported?
	bool m_bECores : 1;		// Has E-Cores?
	bool m_bHYBRID : 1;		// Is using Hybrid Architecture?
	
	char* m_szProcessorID;		// Processor vendor Identification.
	char* m_szProcessorBrand;	// Processor brand string, if available

	uint32 m_nModel;
	uint32 m_nFeatures[ 3 ];

	uint32 m_nL1CacheSizeKb;
	uint32 m_nL1CacheDesc;
	uint32 m_nL2CacheSizeKb;
	uint32 m_nL2CacheDesc;
	uint32 m_nL3CacheSizeKb;
	uint32 m_nL3CacheDesc;

	struct ProcessorInfo
	{
		uint16 m_PhysicalId;
		uint8 m_CoreId;

		uint8 m_HyperThreadingIdx;

		uint16 m_Group;
		uint8 m_GroupIdx;

		enum EfficiencyType : uint8
		{
			ET_ECORE = 0,
			ET_PCORE = 1
		};

		EfficiencyType m_EfficiencyType;

		// The max frequency in Hz instead on Linux
		uint64 m_EfficiencyClass;
	};

	ProcessorInfo *m_pProcessorInfo;

	CPUInformation()
	{
		memset( (void *)this, 0, sizeof( *this ) );
	}

	enum
	{
		CPUDESC_NONE = 0,
		CPUDESC_INCLUDE_PROCESSOR_BRAND	= (1 << 0),
		CPUDESC_INCLUDE_CORE_INFO		= (1 << 1),
		CPUDESC_EXCLUDE_FEATURE_SUPPORT	= (1 << 2)
	};

	// flags are CPUDESC_* flags
	PLATFORM_CLASS const char *GetDescription( CBufferString *buf, uint32 flags = CPUDESC_NONE ) const;

	PLATFORM_CLASS static bool GetIAFamilyModelStepping( uint32 *family_id, uint32 *model_id = nullptr, uint32 *stepping_id = nullptr );

	PLATFORM_CLASS static bool GetAMDFamily( uint32 *family_id );
	PLATFORM_CLASS static bool GetWinLevelRevision( uint16 *family_id, uint16 *model_id = nullptr );
};

PLATFORM_INTERFACE const CPUInformation *GetCPUInformation();
PLATFORM_INTERFACE int Plat_GetNumberCpusOnline();
PLATFORM_INTERFACE uint64 Plat_CPUTickFrequency();

PLATFORM_INTERFACE double Plat_GetCPUUsage();

PLATFORM_INTERFACE void Plat_GetCurrentDate( int *pDay, int *pMonth, int *pYear );


#define Plat_FastMemset memset
#define Plat_FastMemcpy memcpy

//-----------------------------------------------------------------------------
// Returns true if running on a 64 bit (windows) OS
//-----------------------------------------------------------------------------
PLATFORM_INTERFACE bool Plat_Is64BitOS();


//-----------------------------------------------------------------------------
// There is no requirement that a va_list be usable in multiple calls,
// but the Steam code does this.  Linux64 does not support reuse, whereas
// Windows does, so Linux64 breaks on code that was written and working
// on Windows.  Fortunately Linux has va_copy, which provides a simple
// way to let a va_list be used multiple times.  Unfortunately Windows
// does not have va_copy, so here we provide things to hide the difference.
//-----------------------------------------------------------------------------

class CReuseVaList
{
public:
	CReuseVaList( va_list List )
	{
#if defined(LINUX) || defined(OSX)
		va_copy( m_ReuseList, List );
#else
		m_ReuseList = List;
#endif
	}
	~CReuseVaList()
	{
#if defined(LINUX) || defined(OSX)
		va_end( m_ReuseList );
#endif
	}

	va_list m_ReuseList;
};

//-----------------------------------------------------------------------------
// Methods to invoke the constructor, copy constructor, and destructor
//-----------------------------------------------------------------------------

template <class T>
inline T* Construct( T* pMemory )
{
	return reinterpret_cast<T*>(::new( pMemory ) T);
}

template <class T, typename... ARGS>
inline T* Construct( T* pMemory, ARGS&&... args )
{
	return reinterpret_cast<T*>(::new( pMemory ) T( std::forward<ARGS>( args )... ));
}

template <class T, class P>
inline void ConstructOneArg( T* pMemory, P const& arg)
{
	::new( pMemory ) T(arg);
}

template <class T, class P1, class P2 >
inline void ConstructTwoArg( T* pMemory, P1 const& arg1, P2 const& arg2)
{
	::new( pMemory ) T(arg1, arg2);
}

template <class T, class P1, class P2, class P3 >
inline void ConstructThreeArg( T* pMemory, P1 const& arg1, P2 const& arg2, P3 const& arg3)
{
	::new( pMemory ) T(arg1, arg2, arg3);
}

template <class T>
inline T* CopyConstruct( T* pMemory, T const& src )
{
	return reinterpret_cast<T*>(::new( pMemory ) T(src));
}

template <class T>
inline T* MoveConstruct( T* pMemory, T&& src )
{
	return reinterpret_cast<T*>(::new( pMemory ) T(std::move(src)));
}

// nAllocatedSize is for objects allocated with fewer bytes than sizeof(T)
template <class T>
inline void Destruct( T* pMemory, size_t nAllocatedSize )
{
	pMemory->~T();

#ifdef _DEBUG
	memset( reinterpret_cast<void*>( pMemory ), 0xDD, nAllocatedSize );
#endif
}

template <class T>
inline void Destruct( T* pMemory )
{
	Destruct( pMemory, sizeof(T) );
}

// The above will error when binding to a type of: foo(*)[] -- there is no provision in c++ for knowing how many objects
// to destruct without preserving the count and calling the necessary destructors.
template <class T, size_t N>
inline void Destruct( T (*pMemory)[N] )
{
	for ( size_t i = 0; i < N; i++ )
	{
		(pMemory[i])->~T();
	}

#ifdef _DEBUG
	memset( reinterpret_cast<void*>( pMemory ), 0xDD, sizeof(*pMemory) );
#endif
}

template <typename T> struct RemoveReference_      { using Type = T; };
template <typename T> struct RemoveReference_<T&>  { using Type = T; };
template <typename T> struct RemoveReference_<T&&> { using Type = T; };

template <typename T>
using RemoveReference = typename RemoveReference_<T>::Type;

template <typename T>
constexpr RemoveReference<T>&& Move(T&& arg)
{
	return static_cast<RemoveReference<T>&&>(arg);
}

//--------------------------------------------------------------------------------------------------
// RunCodeAtScopeExit
//
// Example:
//	int *x = new int;
//	RunCodeAtScopeExit( delete x )
//--------------------------------------------------------------------------------------------------
template <typename LambdaType>
class CScopeGuardLambdaImpl
{
public:
	explicit CScopeGuardLambdaImpl( LambdaType&& lambda ) : m_lambda( Move( lambda ) ) { }
	~CScopeGuardLambdaImpl() { m_lambda(); }
private:
	LambdaType m_lambda;
};

//--------------------------------------------------------------------------------------------------
template <typename LambdaType>
CScopeGuardLambdaImpl< LambdaType > MakeScopeGuardLambda( LambdaType&& lambda )
{
	return CScopeGuardLambdaImpl< LambdaType >( Move( lambda ) );
}

//--------------------------------------------------------------------------------------------------
#define RunLambdaAtScopeExit2( VarName, ... )		const auto VarName( MakeScopeGuardLambda( __VA_ARGS__ ) ); (void)VarName
#define RunLambdaAtScopeExit( ... )					RunLambdaAtScopeExit2( UNIQUE_ID, __VA_ARGS__ )
#define RunCodeAtScopeExit( ... )					RunLambdaAtScopeExit( [&]() { __VA_ARGS__ ; } )

//
// GET_OUTER()
//
// A platform-independent way for a contained class to get a pointer to its
// owner. If you know a class is exclusively used in the context of some
// "outer" class, this is a much more space efficient way to get at the outer
// class than having the inner class store a pointer to it.
//
//	class COuter
//	{
//		class CInner // Note: this does not need to be a nested class to work
//		{
//			void PrintAddressOfOuter()
//			{
//				printf( "Outer is at 0x%x\n", GET_OUTER( COuter, m_Inner ) );
//			}
//		};
//
//		CInner m_Inner;
//		friend class CInner;
//	};

#define GET_OUTER( OuterType, OuterMember ) \
   ( ( OuterType * ) ( (uint8 *)this - offsetof( OuterType, OuterMember ) ) )


/*	TEMPLATE_FUNCTION_TABLE()

	(Note added to platform.h so platforms that correctly support templated
	 functions can handle portions as templated functions rather than wrapped
	 functions)

	Helps automate the process of creating an array of function
	templates that are all specialized by a single integer.
	This sort of thing is often useful in optimization work.

	For example, using TEMPLATE_FUNCTION_TABLE, this:

	TEMPLATE_FUNCTION_TABLE(int, Function, ( int blah, int blah ), 10)
	{
		return argument * argument;
	}

	is equivilent to the following:

	(NOTE: the function has to be wrapped in a class due to code
	generation bugs involved with directly specializing a function
	based on a constant.)

	template<int argument>
	class FunctionWrapper
	{
	public:
		int Function( int blah, int blah )
		{
			return argument*argument;
		}
	}

	typedef int (*FunctionType)( int blah, int blah );

	class FunctionName
	{
	public:
		enum { count = 10 };
		FunctionType functions[10];
	};

	FunctionType FunctionName::functions[] =
	{
		FunctionWrapper<0>::Function,
		FunctionWrapper<1>::Function,
		FunctionWrapper<2>::Function,
		FunctionWrapper<3>::Function,
		FunctionWrapper<4>::Function,
		FunctionWrapper<5>::Function,
		FunctionWrapper<6>::Function,
		FunctionWrapper<7>::Function,
		FunctionWrapper<8>::Function,
		FunctionWrapper<9>::Function
	};
*/

PLATFORM_INTERFACE bool vtune( bool resume );


#define TEMPLATE_FUNCTION_TABLE(RETURN_TYPE, NAME, ARGS, COUNT)			\
																		\
typedef RETURN_TYPE (FASTCALL *__Type_##NAME) ARGS;						\
																		\
template<const int nArgument>											\
struct __Function_##NAME												\
{																		\
	static RETURN_TYPE FASTCALL Run ARGS;								\
};																		\
																		\
template <const int i>														\
struct __MetaLooper_##NAME : __MetaLooper_##NAME<i-1>					\
{																		\
	__Type_##NAME func;													\
	inline __MetaLooper_##NAME() { func = __Function_##NAME<i>::Run; }	\
};																		\
																		\
template<>																\
struct __MetaLooper_##NAME<0>											\
{																		\
	__Type_##NAME func;													\
	inline __MetaLooper_##NAME() { func = __Function_##NAME<0>::Run; }	\
};																		\
																		\
class NAME																\
{																		\
private:																\
	static const __MetaLooper_##NAME<COUNT> m;							\
public:																	\
	enum { count = COUNT };												\
	static const __Type_##NAME* functions;								\
};																		\
const __MetaLooper_##NAME<COUNT> NAME::m;								\
const __Type_##NAME* NAME::functions = (__Type_##NAME*)&m;				\
template<const int nArgument>													\
RETURN_TYPE FASTCALL __Function_##NAME<nArgument>::Run ARGS


#define LOOP_INTERCHANGE(BOOLEAN, CODE)\
	if( (BOOLEAN) )\
	{\
		CODE;\
	} else\
	{\
		CODE;\
	}


//-----------------------------------------------------------------------------
// Dynamic libs support
//-----------------------------------------------------------------------------
#if defined( PLATFORM_WINDOWS )

PLATFORM_INTERFACE void *Plat_GetProcAddress( const char *pszModule, const char *pszName );

template <typename FUNCPTR_TYPE>
class CDynamicFunction
{
public:
	CDynamicFunction( const char *pszModule, const char *pszName, FUNCPTR_TYPE pfnFallback = NULL )
	{
		m_pfn = pfnFallback;
		void *pAddr = Plat_GetProcAddress( pszModule, pszName );
		if ( pAddr )
		{
			m_pfn = (FUNCPTR_TYPE)pAddr;
		}
	}

	operator bool()			{ return m_pfn != NULL;	}
	bool operator !()		{ return !m_pfn;	}
	operator FUNCPTR_TYPE()	{ return m_pfn; }

private:
	FUNCPTR_TYPE m_pfn;
};
#endif


//-----------------------------------------------------------------------------
// What OS version are we?
//-----------------------------------------------------------------------------
enum EOSType
{
	k_eIOSUnknown = -600,

	k_eAndroidUnknown = -500,

	k_eAndroid50,
	k_eAndroid51,
	k_eAndroid60,
	k_eAndroid70,
	k_eAndroid711,
	k_eAndroid8,
	k_eAndroid8MR1,
	k_eAndroid9,
	k_eAndroid10,

	k_eAndroidMax,

	k_eOSUMQ = -400,

	k_eOSPS3 = -300,

	k_eLinuxUnknown = -203,

	k_eLinux22,
	k_eLinux24,
	k_eLinux26,
	k_eLinux32,
	k_eLinux35,
	k_eLinux36,
	k_eLinux310,
	k_eLinux40,
	k_eLinux45,
	k_eLinux418,
	k_eLinux50,

	k_eLinuxMax,

	k_eMacOSUnknown = -102,

	k_eMacOS104,
	k_eMacOS105,
	k_eMacOS1058,
	k_eMacOS106 = -95,
	k_eMacOS1063,
	k_eMacOS1064_slgu,
	k_eMacOS1067,
	k_eMacOS107 = -90,
	k_eMacOS108,
	k_eMacOS109,
	k_eMacOS1010,
	k_eMacOS1011,
	k_eMacOS1012,
	k_eMacOS1013,
	k_eMacOS1014,
	k_eMacOS1015,
	k_eMacOS1016,
	k_eMacOS11,
	k_eMacOS111,
	k_eMacOS1017,
	k_eMacOS12,
	k_eMacOS1018,
	k_eMacOS13,
	k_eMacOS14,
	k_eMacOS15,

	k_eMacOSMax,

	k_eWinUnknown = 0,

	k_eWin311,
	k_eWin95,
	k_eWin98,
	k_eWinME,
	k_eWinNT,
	k_eWin2000,
	k_eWinXP,
	k_eWin2003,
	k_eWinVista,
	k_eWin2008,
	k_eWindows7,
	k_eWin2008R2,
	k_eWin2012,
	k_eWindows8,
	k_eWindows81,
	k_eWin2012R2,
	k_eWindows10,
	k_eWin2016,
	k_eWin2019,
	k_eWin2022,
	k_eWindows11,

	k_eWinMAX,

	k_eOSUnknown = -1
};

PLATFORM_INTERFACE EOSType Plat_GetOSType();
PLATFORM_INTERFACE bool Plat_GetOSDescriptionString(char *buf, size_t size);
PLATFORM_INTERFACE const char *GetNameFromOSType( EOSType type );


//  Watchdog timer support. Call Plat_BeginWatchdogTimer( nn, issuer ) to kick the timer off.  if you don't call
// Plat_EndWatchdogTimer within nn seconds, the program will kick off an exception.  This is for making
// sure that hung dedicated servers abort (and restart) instead of staying hung. Calling
// Plat_EndWatchdogTimer more than once or when there is no active watchdog is fine. Only does anything
// under linux right now. It should be possible to implement this functionality in windows via a
// thread, if desired.

PLATFORM_INTERFACE void Plat_BeginWatchdogTimer( int nSecs, const char *szIssuer );
PLATFORM_INTERFACE void Plat_EndWatchdogTimer( const char *szIssuer );
PLATFORM_INTERFACE void Plat_SetWatchdogHandlerFunction( void (*handler)() );

#include "tier0/valve_on.h"

PLATFORM_INTERFACE void			V_tier0_memset( void *dest, int fill, size_t count );
PLATFORM_INTERFACE void			V_tier0_memcpy( void *dest, const void *src, size_t count );
PLATFORM_INTERFACE void			V_tier0_memmove( void *dest, const void *src, size_t count );
PLATFORM_INTERFACE int			V_tier0_memcmp( const void *m1, const void *m2, size_t count );

#define V_memset(dest, fill, count)		V_tier0_memset		((dest), (fill), (count))
#define V_memcpy(dest, src, count)		V_tier0_memcpy		((dest), (src), (count))
#define V_memmove(dest, src, count)		V_tier0_memmove		((dest), (src), (count))
#define V_memcmp(m1, m2, count)			V_tier0_memcmp		((m1), (m2), (count))

#define Q_memset				V_memset
#define Q_memcpy				V_memcpy
#define Q_memmove				V_memmove
#define Q_memcmp				V_memcmp

#endif /* PLATFORM_H */
