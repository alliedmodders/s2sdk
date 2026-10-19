#ifndef PLATFORM_COMPILE_BASE_H
#define PLATFORM_COMPILE_BASE_H

#if defined(__x86_64__) || defined(_WIN64)
#define PLATFORM_64BITS 1
#endif

#ifdef COMPILER_MSVC
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <stddef.h>
#include <cstdlib>

#include "wchartypes.h"
#include "tier0/valve_off.h"

#ifdef POSIX
// need this for _alloca
#include <alloca.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <stdarg.h>
#else
#endif

#include <malloc.h>

#include <stdio.h>
#include <string.h>

#ifdef COMPILER_GCC
#include <new>
#else
#include <new.h>
#endif
#include <utility>

// Pull in the /analyze code annotations.
#include "annotations.h"

//-----------------------------------------------------------------------------
// Old-school defines we don't want to use moving forward
//-----------------------------------------------------------------------------
#if CROSS_PLATFORM_VERSION < 1

//-----------------------------------------------------------------------------
// This macro predates universal static_assert support in our toolchains
#define COMPILE_TIME_ASSERT( pred ) static_assert( pred, "Compile time assert constraint is not true: " #pred )

// ASSERT_INVARIANT used to be needed in order to allow COMPILE_TIME_ASSERTs at global
// scope. However the new COMPILE_TIME_ASSERT macro supports that by default.
#define ASSERT_INVARIANT( pred )	COMPILE_TIME_ASSERT( pred )

// feature enables
#define NEW_SOFTWARE_LIGHTING
#define SUPPORT_PACKED_STORE

// C functions for external declarations that call the appropriate C++ methods
#ifndef EXPORT
	#ifdef _WIN32
		#define EXPORT	_declspec( dllexport )
	#else
		#define EXPORT	/* */
	#endif
#endif

#ifdef PLATFORM_POSIX
typedef unsigned int DWORD;
typedef unsigned short WORD;
typedef void * HINSTANCE;
#define _MAX_PATH PATH_MAX
#endif

#endif // CROSS_PLATFORM_VERSION < 1


//-----------------------------------------------------------------------------
// NOTE: All compiler defines are set up in the base VPC scripts
// COMPILER_MSVC, COMPILER_MSVC32, COMPILER_MSVC64
// COMPILER_GCC
// The rationale for this is that we need COMPILER_MSVC for the pragma blocks
// #pragma once that occur at the top of all header files, therefore we can't
// place the defines for these in here.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Set up platform defines.
//-----------------------------------------------------------------------------
#ifdef _WIN32
	#define IsPlatformLinux()	false
	#define IsPlatformPosix()	false
	#define IsPlatformOSX()		false
	#define IsPlatformWindows() true
	#ifndef PLATFORM_WINDOWS
	#define PLATFORM_WINDOWS	1
	#endif

	#define IsPlatformWindowsPC() true
	#define PLATFORM_WINDOWS_PC 1

	#ifdef _WIN64
		#define IsPlatformWindowsPC64() true
		#define IsPlatformWindowsPC32() false
		#define PLATFORM_WINDOWS_PC64 1
	#else
		#define IsPlatformWindowsPC64() false
		#define IsPlatformWindowsPC32() true
		#define PLATFORM_WINDOWS_PC32 1
	#endif

#elif defined(POSIX)
	#define IsPlatformWindows()		false
	#define IsPlatformWindowsPC()	false
	#define IsPlatformWindowsPC64()	false
	#define IsPlatformWindowsPC32()	false
	#define IsPlatformPosix()		true
	#ifndef PLATFORM_POSIX
	#define PLATFORM_POSIX 1
	#endif

	#if defined( LINUX )
		#define IsPlatformLinux() true
		#define IsPlatformOSX() false
		#ifndef PLATFORM_LINUX
		#define PLATFORM_LINUX 1
		#endif
	#elif defined ( OSX )
		#define IsPlatformLinux() false
		#define IsPlatformOSX() true
		#ifndef PLATFORM_OSX
		#define PLATFORM_OSX 1
		#endif
	#else
		#define IsPlatformLinux() false
		#define IsPlatformOSX() false
	#endif

#else
	#error
#endif


//-----------------------------------------------------------------------------
// Old-school defines we're going to support since much code uses them
//-----------------------------------------------------------------------------
#if CROSS_PLATFORM_VERSION < 2

#define IsLinux()	IsPlatformLinux()
#define IsOSX()		IsPlatformOSX()
#define IsPosix()	IsPlatformPosix()

// Setup platform defines.
#ifdef COMPILER_MSVC
#define MSVC 1
#endif

#ifdef COMPILER_GCC
#define GNUC 1
#endif

#if defined( _WIN32 )
#define _WINDOWS 1
#endif

#ifdef PLATFORM_WINDOWS_PC
#define IS_WINDOWS_PC 1
#endif

#endif // CROSS_PLATFORM_VERSION < 2


//-----------------------------------------------------------------------------
// Set up build configuration defines.
//-----------------------------------------------------------------------------
#ifdef _CERT
#define IsCert() true
#else
#define IsCert() false
#endif

#ifdef _DEBUG
#define IsRelease() false
#define IsDebug() true
#else
#define IsRelease() true
#define IsDebug() false
#endif

#ifdef _RETAIL
#define IsRetail() true
#else
#define IsRetail() false
#endif


//-----------------------------------------------------------------------------
// Portable data types
//-----------------------------------------------------------------------------
typedef unsigned char				uint8;
typedef signed char					int8;

#if defined( COMPILER_MSVC )

	typedef __int16					int16;
	typedef unsigned __int16		uint16;
	typedef __int32					int32;
	typedef unsigned __int32		uint32;
	typedef __int64					int64;
	typedef unsigned __int64		uint64;

	// intp is an integer that can accomodate a pointer
	// (ie, sizeof(intp) >= sizeof(int) && sizeof(intp) >= sizeof(void *)
	typedef intptr_t				intp;
	typedef uintptr_t				uintp;

#else // !COMPILER_MSVC

	typedef short					int16;
	typedef unsigned short			uint16;
	typedef int						int32;
	typedef unsigned int			uint32;
	typedef long long				int64;
	typedef unsigned long long		uint64;
	#ifdef PLATFORM_64BITS
		typedef long long			intp;
		typedef unsigned long long	uintp;
	#else
		typedef int					intp;
		typedef unsigned int		uintp;
	#endif
	typedef void *HWND;

#endif // else COMPILER_MSVC

// From steam/steamtypes.h
// RTime32
// We use this 32 bit time representing real world time.
// It offers 1 second resolution beginning on January 1, 1970 (Unix time)
typedef uint32 RTime32;

typedef float				float32;
typedef double				float64;

// for when we don't care about how many bits we use
typedef unsigned int		uint;

#ifdef _WIN32
typedef wchar_t ucs2; // under windows wchar_t is ucs2
#else
typedef unsigned short ucs2;
#endif

typedef float vec_t;

typedef void *HANDLE;

// A file handle of the OS: a HANDLE on Windows, a file descriptor elsewhere
#ifdef _WIN32
typedef void *OSFILEHANDLE;
#else
typedef int OSFILEHANDLE;
#endif

#if !defined( offsetof )
	#ifdef __GNUC__
		#define offsetof( type, var ) __builtin_offsetof( type, var )
	#else
		#define offsetof(s,m)	(size_t)&(((s *)0)->m)
	#endif
#endif // !defined( offsetof )

//-----------------------------------------------------------------------------
// Long is evil because it's treated differently by different compilers
// Preventing its use is nasty however. This #define, which should be
// turned on in individual VPC files, causes you to include tier0/valve_off.h
// before standard C + windows headers, and include tier0/valve_on.h after
// standard C + windows headers. So, there's some painful overhead to disabling long
//-----------------------------------------------------------------------------
#ifdef DISALLOW_USE_OF_LONG
	#define long			long_is_the_devil_stop_using_it_use_int32_or_int64
#endif


//-----------------------------------------------------------------------------
// Various compiler-specific keywords
//-----------------------------------------------------------------------------
#ifdef COMPILER_MSVC

	#ifdef FORCEINLINE
		#undef FORCEINLINE
	#endif
	#define STDCALL					__stdcall
	#ifndef FASTCALL
		#define  FASTCALL			__fastcall
	#endif
	#define FORCEINLINE				__forceinline
	#define FORCEINLINE_TEMPLATE	__forceinline
	#define NULLTERMINATED			__nullterminated

	// This can be used to ensure the size of pointers to members when declaring
	// a pointer type for a class that has only been forward declared
	#define SINGLE_INHERITANCE		__single_inheritance
	#define MULTIPLE_INHERITANCE	__multiple_inheritance
	#define EXPLICIT				explicit
	#define NO_VTABLE				__declspec( novtable )

	// gcc doesn't allow storage specifiers on explicit template instatiation, but visual studio needs them to avoid link errors.
	#define TEMPLATE_STATIC			static

	// Used for dll exporting and importing
	#define DLL_EXPORT				extern "C" __declspec( dllexport )
	#define DLL_IMPORT				extern "C" __declspec( dllimport )

	// Can't use extern "C" when DLL exporting a class
	#define DLL_CLASS_EXPORT		__declspec( dllexport )
	#define DLL_CLASS_IMPORT		__declspec( dllimport )

	// Can't use extern "C" when DLL exporting a global
	#define DLL_GLOBAL_EXPORT		extern __declspec( dllexport )
	#define DLL_GLOBAL_IMPORT		extern __declspec( dllimport )

	// Pass hints to the compiler to prevent it from generating unnessecary / stupid code
	// in certain situations.  Several compilers other than MSVC also have an equivilent
	// construct.
	//
	// Essentially the 'Hint' is that the condition specified is assumed to be true at
	// that point in the compilation.  If '0' is passed, then the compiler assumes that
	// any subsequent code in the same 'basic block' is unreachable, and thus usually
	// removed.
	#define HINT(THE_HINT)			__assume((THE_HINT))

	// decls for aligning data
	#define DECL_ALIGN(x)			__declspec( align( x ) )

	// GCC had a few areas where it didn't construct objects in the same order
	// that Windows does. So when CVProfile::CVProfile() would access g_pMemAlloc,
	// it would crash because the allocator wasn't initalized yet.
	#define CONSTRUCT_EARLY

	#define SELECTANY				__declspec(selectany)

	#define RESTRICT				__restrict
	#define RESTRICT_FUNC			__declspec(restrict)
	#define FMTFUNCTION( a, b )
	#define NOINLINE

#if !defined( NO_THREAD_LOCAL )
	#define DECL_THREAD_LOCAL		__declspec(thread)
#endif

	#define DISABLE_VC_WARNING( x ) __pragma(warning(disable:4310) )
	#define DEFAULT_VC_WARNING( x ) __pragma(warning(default:4310) )


#elif defined ( COMPILER_GCC )

	#if (CROSS_PLATFORM_VERSION >= 1) && !defined( PLATFORM_64BITS )
		#define  STDCALL			__attribute__ ((__stdcall__))
	#else
		#define  STDCALL
		#define  __stdcall			__attribute__ ((__stdcall__))
	#endif

	#define  FASTCALL
	#ifdef _LINUX_DEBUGGABLE
		#define  FORCEINLINE
	#else
		#define  FORCEINLINE		inline
	#endif

	// GCC 3.4.1 has a bug in supporting forced inline of templated functions
	// this macro lets us not force inlining in that case
	#define FORCEINLINE_TEMPLATE	inline
	#define SINGLE_INHERITANCE
	#define MULTIPLE_INHERITANCE
	#define EXPLICIT
	#define NO_VTABLE

	#define NULLTERMINATED

	#define TEMPLATE_STATIC

	// Used for dll exporting and importing
	#define DLL_EXPORT				extern "C" __attribute__ ((visibility("default")))
	#define DLL_IMPORT				extern "C"

	// Can't use extern "C" when DLL exporting a class
	#define DLL_CLASS_EXPORT		__attribute__ ((visibility("default")))
	#define DLL_CLASS_IMPORT

	// Can't use extern "C" when DLL exporting a global
	#define DLL_GLOBAL_EXPORT		__attribute__((visibility("default")))
	#define DLL_GLOBAL_IMPORT		extern

	#define HINT(THE_HINT)			0
	#define DECL_ALIGN(x)			__attribute__( ( aligned( x ) ) )
	#define CONSTRUCT_EARLY			__attribute__((init_priority(101)))
	#define SELECTANY				__attribute__((weak))
	#define RESTRICT
	#define RESTRICT_FUNC
	#define FMTFUNCTION( fmtargnumber, firstvarargnumber ) __attribute__ (( format( printf, fmtargnumber, firstvarargnumber )))
	#define NOINLINE				__attribute__ ((noinline))

#if !defined( NO_THREAD_LOCAL )
	#define DECL_THREAD_LOCAL		__thread
#endif

	#define DISABLE_VC_WARNING( x )
	#define DEFAULT_VC_WARNING( x )

#else

	#define DECL_ALIGN(x)			/* */
	#define SELECTANY				static

#endif

#if defined( GNUC )
// gnuc has the align decoration at the end
#define ALIGN4
#define ALIGN8
#define ALIGN16
#define ALIGN32
#define ALIGN128
#define ALIGN_N( _align_ )

#define ALIGN4_POST DECL_ALIGN(4)
#define ALIGN8_POST DECL_ALIGN(8)
#define ALIGN16_POST DECL_ALIGN(16)
#define ALIGN32_POST DECL_ALIGN(32)
#define ALIGN128_POST DECL_ALIGN(128)
#define ALIGN_N_POST( _align_ ) DECL_ALIGN( _align_ )
#else
// MSVC has the align at the start of the struct
#define ALIGN4 DECL_ALIGN(4)
#define ALIGN8 DECL_ALIGN(8)
#define ALIGN16 DECL_ALIGN(16)
#define ALIGN32 DECL_ALIGN(32)
#define ALIGN128 DECL_ALIGN(128)
#define ALIGN_N( _align_ ) DECL_ALIGN( _align_ )

#define ALIGN4_POST
#define ALIGN8_POST
#define ALIGN16_POST
#define ALIGN32_POST
#define ALIGN128_POST
#define ALIGN_N_POST( _align_ )
#endif

// !!! NOTE: if you get a compile error here, you are using VALIGNOF on an abstract type :NOTE !!!
#define VALIGNOF_PORTABLE( type ) ( sizeof( AlignOf_t<type> ) - sizeof( type ) )

#if defined( COMPILER_GCC ) || defined( COMPILER_MSVC )
#define VALIGNOF( type ) __alignof( type )
#define VALIGNOF_TEMPLATE_SAFE( type ) VALIGNOF_PORTABLE( type )
#else
#error "PORT: Code only tested with MSVC! Must validate with new compiler, and use built-in keyword if available."
#endif

//-----------------------------------------------------------------------------
// Convert int<-->pointer, avoiding 32/64-bit compiler warnings:
//-----------------------------------------------------------------------------
#define INT_TO_POINTER( i ) (void *)( ( i ) + (char *)NULL )
#define POINTER_TO_INT( p ) ( (int)(uintp)( p ) )


// This can be used to declare an abstract (interface only) class.
// Classes marked abstract should not be instantiated.  If they are, and access violation will occur.
//
// Example of use:
//
// abstract_class CFoo
// {
//      ...
// }
//
// MSDN __declspec(novtable) documentation: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclang/html/_langref_novtable.asp
//
// Note: NJS: This is not enabled for regular PC, due to not knowing the implications of exporting a class with no no vtable.
//       It's probable that this shouldn't be an issue, but an experiment should be done to verify this.
//
	#define abstract_class class

// MSVC CRT uses 0x7fff while gcc uses MAX_INT, leading to mismatches between platforms
// As a result, we pick the least common denominator here.  This should be used anywhere
// you might typically want to use RAND_MAX
#define VALVE_RAND_MAX 0x7fff


//-----------------------------------------------------------------------------
// Why do we need this? It would be nice to make it die die die
//-----------------------------------------------------------------------------
// Alloca defined for this platform
#if defined( COMPILER_MSVC ) && !defined( WINDED )
	#if defined(_M_IX86)
		#define __i386__	1
	#endif
#endif

#if defined __i386__ && !defined __linux__
	#define id386	1
#else
	#define id386	0
#endif  // __i386__


//-----------------------------------------------------------------------------
// Disable annoying unhelpful warnings
//-----------------------------------------------------------------------------
#ifdef COMPILER_MSVC
// Remove warnings from warning level 4.
#pragma warning(disable : 4514) // warning C4514: 'acosl' : unreferenced inline function has been removed
#pragma warning(disable : 4100) // warning C4100: 'hwnd' : unreferenced formal parameter
#pragma warning(disable : 4127) // warning C4127: conditional expression is constant
#pragma warning(disable : 4512) // warning C4512: 'InFileRIFF' : assignment operator could not be generated
#pragma warning(disable : 4611) // warning C4611: interaction between '_setjmp' and C++ object destruction is non-portable
#pragma warning(disable : 4710) // warning C4710: function 'x' not inlined
#pragma warning(disable : 4702) // warning C4702: unreachable code
#pragma warning(disable : 4505) // unreferenced local function has been removed
#pragma warning(disable : 4239) // nonstandard extension used : 'argument' ( conversion from class Vector to class Vector& )
#pragma warning(disable : 4097) // typedef-name 'BaseClass' used as synonym for class-name 'CFlexCycler::CBaseFlex'
#pragma warning(disable : 4324) // Padding was added at the end of a structure
#pragma warning(disable : 4244) // type conversion warning.
#pragma warning(disable : 4305)	// truncation from 'const double ' to 'float '
#pragma warning(disable : 4786)	// Disable warnings about long symbol names
#pragma warning(disable : 4250) // 'X' : inherits 'Y::Z' via dominance
#pragma warning(disable : 4201) // nonstandard extension used : nameless struct/union

#if _MSC_VER >= 1300
#pragma warning(disable : 4511)	// Disable warnings about private copy constructors
#pragma warning(disable : 4121)	// warning C4121: 'symbol' : alignment of a member was sensitive to packing
#pragma warning(disable : 4530)	// warning C4530: C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc (disabled due to std headers having exception syntax)
#endif

#if _MSC_VER >= 1400
#pragma warning(disable : 4996)	// functions declared deprecated
#endif

// When we port to 64 bit, we'll have to resolve the int, ptr vs size_t 32/64 bit problems...
#if !defined( COMPILER_MSVC64 )
#if ( CROSS_PLATFORM_VERSION < 1 )
#pragma warning( disable : 4267 )	// conversion from 'size_t' to 'int', possible loss of data
#pragma warning( disable : 4311 )	// pointer truncation from 'char *' to 'int'
#pragma warning( disable : 4312 )	// conversion from 'unsigned int' to 'memhandle_t' of greater size
#endif
#endif

#endif


//-----------------------------------------------------------------------------
// Stack-based allocation related helpers
//-----------------------------------------------------------------------------
#if defined( COMPILER_GCC )

	#define stackalloc( _size )		alloca( ALIGN_VALUE( _size, 16 ) )

	#ifdef PLATFORM_OSX
		#define mallocsize( _p )	( malloc_size( _p ) )
	#else
		#define mallocsize( _p )	( malloc_usable_size( _p ) )
	#endif

#elif defined ( COMPILER_MSVC )

	#define stackalloc( _size )		_alloca( ALIGN_VALUE( _size, 16 ) )
	#define mallocsize( _p )		( _msize( _p ) )

#endif

#define  stackfree( _p )			0


//-----------------------------------------------------------------------------
// Used to break into the debugger
//-----------------------------------------------------------------------------
#if defined( PLATFORM_WINDOWS )
#define DebuggerBreak()  __debugbreak()
#else
	// On OSX, SIGTRAP doesn't really stop the thread cold when debugging.
	// So if being debugged, use INT3 which is precise.
#ifdef PLATFORM_OSX
#define DebuggerBreak()  if ( Plat_IsInDebugSession() ) { __asm ( "int $3" ); } else { raise(SIGTRAP); }
#else
#define DebuggerBreak()  raise(SIGTRAP)
#endif
#endif
#define	DebuggerBreakIfDebugging() if ( !Plat_IsInDebugSession() ) ; else DebuggerBreak()


//-----------------------------------------------------------------------------
// DLL export for platform utilities
//-----------------------------------------------------------------------------
#ifndef STATIC_TIER0

#ifdef TIER0_DLL_EXPORT
#define PLATFORM_INTERFACE	DLL_EXPORT
#define PLATFORM_OVERLOAD	DLL_GLOBAL_EXPORT
#define PLATFORM_CLASS		DLL_CLASS_EXPORT
#else
#define PLATFORM_INTERFACE	DLL_IMPORT
#define PLATFORM_OVERLOAD	DLL_GLOBAL_IMPORT
#define PLATFORM_CLASS		DLL_CLASS_IMPORT
#endif

#else	// BUILD_AS_DLL

#define PLATFORM_INTERFACE	extern
#define PLATFORM_OVERLOAD
#define PLATFORM_CLASS

#endif	// BUILD_AS_DLL


//-----------------------------------------------------------------------------
// Returns true if debugger attached, false otherwise
//-----------------------------------------------------------------------------
#if defined( PLATFORM_WINDOWS ) || defined( PLATFORM_LINUX ) || defined( PLATFORM_OSX )
PLATFORM_INTERFACE bool Plat_IsInDebugSession();
PLATFORM_INTERFACE void Plat_DebugString( const tchar * );
#else
inline bool Plat_IsInDebugSession() { return false; }
#define Plat_DebugString(s) ((void)0)
#endif

PLATFORM_INTERFACE void				Plat_NonFatalErrorFunc( const tchar *pMsg, ... ) FMTFUNCTION( 1, 2 );

PLATFORM_INTERFACE void				Plat_FatalError( const tchar *pMsg, ... ) FMTFUNCTION( 1, 2 );

//-----------------------------------------------------------------------------
// Interface creation function
//-----------------------------------------------------------------------------
typedef void* (*CreateInterfaceFn)(const char *pName, int *pReturnCode);
typedef void* (*InstantiateInterfaceFn)();

// empty_t is the canonical "no-value" type which is fully defined but empty.
struct empty_t {};

// Use ValidateAlignment to sanity-check alignment usage when allocating arrays of an aligned type
#define ALIGN_ASSERT( pred ) { COMPILE_TIME_ASSERT( pred ); }
template< class T, int ALIGN >
inline void ValidateAlignmentExplicit(void)
{
	// Alignment must be a power of two
	ALIGN_ASSERT((ALIGN & (ALIGN - 1)) == 0);
	// Alignment must not imply gaps in the array (which the CUtlMemory pattern does not allow for)
	ALIGN_ASSERT(ALIGN <= sizeof(T));
	// Alignment must be a multiple of the size of the object type, or elements will *NOT* be aligned!
	ALIGN_ASSERT((sizeof(T) % ALIGN) == 0);
	// Alignment should be a multiple of the base alignment of T
	ALIGN_ASSERT((ALIGN % VALIGNOF(T)) == 0);
	// Alignment must not be bigger than the maximum declared alignment used by DECLARE_ALIGNED_BYTE_ARRAY
	// (if you hit this, just add more powers of 2 below and increase this limit)
	ALIGN_ASSERT( ALIGN <= 128 );
}
template< class T > inline void ValidateAlignment(void) { ValidateAlignmentExplicit<T, VALIGNOF(T)>(); }

// Portable alternative to __alignof
template<class T> struct AlignOf_t { AlignOf_t(){} AlignOf_t & operator=(const AlignOf_t &) { return *this; } byte b; T t; };

template < size_t NUM, class T, int ALIGN > struct AlignedByteArrayExplicit_t{};
template < size_t NUM, class T > struct AlignedByteArray_t : public AlignedByteArrayExplicit_t< NUM, T, VALIGNOF_TEMPLATE_SAFE(T) > {};

#define DECLARE_ALIGNED_BYTE_ARRAY( ALIGN ) \
	template < size_t NUM, class T > \
	struct ALIGN_N( ALIGN ) AlignedByteArrayExplicit_t< NUM, T, ALIGN > \
	{ \
		/* NOTE: verify alignment in the constructor (which may be wrong if this is heap-allocated, for ALIGN > MEMALLOC_MAX_AUTO_ALIGN) */ \
		AlignedByteArrayExplicit_t()	{ if ( (ALIGN-1) & (size_t)this ) { Plat_NonFatalErrorFunc( "AlignedByteArray not properly aligned\n" ); } } \
		T *			Base( void )		{ ValidateAlignmentExplicit<T,ALIGN>(); return (T *)&m_Data; } \
		const T *	Base( void ) const	{ ValidateAlignmentExplicit<T,ALIGN>(); return (const T *)&m_Data; } \
		size_t		Count() const		{ return NUM; } \
		T &operator[]( int i )				{ return Base()[i]; } \
		const T &operator[]( int i ) const	{ return Base()[i]; } \
	private: \
		byte m_Data[ NUM*sizeof( T ) ]; \
	} ALIGN_N_POST( ALIGN );

DECLARE_ALIGNED_BYTE_ARRAY(1);
DECLARE_ALIGNED_BYTE_ARRAY(2);
DECLARE_ALIGNED_BYTE_ARRAY(4);
DECLARE_ALIGNED_BYTE_ARRAY(8);
DECLARE_ALIGNED_BYTE_ARRAY(16);
DECLARE_ALIGNED_BYTE_ARRAY(32);
DECLARE_ALIGNED_BYTE_ARRAY(64);
DECLARE_ALIGNED_BYTE_ARRAY(128);

#endif // PLATFORM_COMPILE_BASE_H
