//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#ifndef FASTTIMER_H
#define FASTTIMER_H
#ifdef _WIN32
#pragma once
#endif

#include <assert.h>
#include "tier0/platform.h"

#ifdef COMPILER_MSVC64
extern "C"
{
	unsigned __int64 __rdtsc();
}

#pragma intrinsic(__rdtsc)
#endif

class CCycleCount
{
	friend class CFastTimer;

public:
					CCycleCount();
					CCycleCount( uint64 cycles );

	void			Sample();	// Sample the clock. This takes about 34 clocks to execute (or 26,000 calls per millisecond on a P900).

	void			Init();		// Set to zero.
	void			Init( uint64 cycles );
	bool			IsLessThan( CCycleCount const &other ) const;					// Compare two counts.

	// Convert to other time representations. These functions are slow, so it's preferable to call them
	// during display rather than inside a timing block.
	unsigned long	GetCycles()  const;
	uint64			GetLongCycles() const;

	double			GetMillisecondsF() const;

	CCycleCount&	operator+=( CCycleCount const &other );

	// dest = rSrc1 + rSrc2
	static void		Add( CCycleCount const &rSrc1, CCycleCount const &rSrc2, CCycleCount &dest );	// Add two samples together.
	
	// dest = rSrc1 - rSrc2
	static void		Sub( CCycleCount const &rSrc1, CCycleCount const &rSrc2, CCycleCount &dest );	// Add two samples together.

	static uint64	GetTimestamp();

	uint64			m_Int64;
};

class CFastTimer
{
public:
	// These functions are fast to call and should be called from your sampling code.
	void				Start();
	void				End();

	const CCycleCount &	GetDuration() const;	// Get the elapsed time between Start and End calls.
	CCycleCount 		GetDurationInProgress() const; // Call without ending. Not that cheap.

private:
	CCycleCount	m_Duration;
#ifdef DEBUG_FASTTIMER
	bool m_bRunning;		// Are we currently running?
#endif
};


// -------------------------------------------------------------------------- // 
// CCycleCount inlines.
// -------------------------------------------------------------------------- // 

inline CCycleCount::CCycleCount()
{
	Init( (uint64)0 );
}

inline CCycleCount::CCycleCount( uint64 cycles )
{
	Init( cycles );
}

inline void CCycleCount::Init()
{
	Init( (uint64)0 );
}

inline void CCycleCount::Init( uint64 cycles )
{
	m_Int64 = cycles;
}

#ifdef COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 4189) // warning C4189: local variable is initialized but not referenced
#endif

inline void CCycleCount::Sample()
{
#ifdef COMPILER_MSVC64
	unsigned __int64* pSample = (unsigned __int64*)&m_Int64;
	*pSample = __rdtsc(); 
	//		Msg( "Sample = %I64x", pSample ); 
#elif defined( _X360 )
	// only need lower 32 bits, avoids doc'd read bug and 32 bit rollover is in 85 seconds
	m_Int64 = (uint64)__mftb32();
	// scale back up, needs to be viewed as 1 cycle/clock
#elif defined( __GNUC__ )
	unsigned long* pSample = (unsigned long *)&m_Int64;
	__asm__ __volatile__ (  
		"rdtsc\n\t"
		"movl %%eax,  (%0)\n\t"
		"movl %%edx, 4(%0)\n\t"
		: /* no output regs */
	: "D" (pSample)
		: "%eax", "%edx" );
#elif defined( _WIN32 )
	unsigned long* pSample = (unsigned long *)&m_Int64;
	__asm
	{
		// force the cpu to synchronize the instruction queue
		// NJS: CPUID can really impact performance in tight loops.
		//cpuid
		//cpuid
		//cpuid
		mov		ecx, pSample
		rdtsc
		mov		[ecx], eax
		mov		[ecx+4], edx
	}
#elif defined( POSIX )
	unsigned long* pSample = (unsigned long *)&m_Int64;
    __asm__ __volatile__ (  
		"rdtsc\n\t"
		"movl %%eax,  (%0)\n\t"
		"movl %%edx, 4(%0)\n\t"
		: /* no output regs */
		: "D" (pSample)
		: "%eax", "%edx" );
#endif
}

#ifdef COMPILER_MSVC
#pragma warning(pop)
#endif


inline CCycleCount& CCycleCount::operator+=( CCycleCount const &other )
{
	m_Int64 += other.m_Int64;
	return *this;
}


inline void CCycleCount::Add( CCycleCount const &rSrc1, CCycleCount const &rSrc2, CCycleCount &dest )
{
	dest.m_Int64 = rSrc1.m_Int64 + rSrc2.m_Int64;
}

inline void CCycleCount::Sub( CCycleCount const &rSrc1, CCycleCount const &rSrc2, CCycleCount &dest )
{
	dest.m_Int64 = rSrc1.m_Int64 - rSrc2.m_Int64;
}

inline uint64 CCycleCount::GetTimestamp()
{
	CCycleCount c;
	c.Sample();
	return c.GetLongCycles();
}

inline bool CCycleCount::IsLessThan(CCycleCount const &other) const
{
	return m_Int64 < other.m_Int64;
}


inline unsigned long CCycleCount::GetCycles() const
{
	return (unsigned long)m_Int64;
}

inline uint64 CCycleCount::GetLongCycles() const
{
	return m_Int64;
}


inline double CCycleCount::GetMillisecondsF() const
{
	return (double)m_Int64 * 1000.0 / (double)Plat_CPUTickFrequency();
}


// -------------------------------------------------------------------------- // 
// CFastTimer inlines.
// -------------------------------------------------------------------------- // 
inline void CFastTimer::Start()
{
	m_Duration.Sample();
#ifdef DEBUG_FASTTIMER
	m_bRunning = true;
#endif
}


inline void CFastTimer::End()
{
	CCycleCount cnt;
	cnt.Sample();
	if ( IsX360() )
	{
		// have to handle rollover, hires timer is only accurate to 32 bits
		// more than one overflow should not have occurred, otherwise caller should use a slower timer
		if ( (uint64)cnt.m_Int64 <= (uint64)m_Duration.m_Int64 )
		{
			// rollover occurred	
			cnt.m_Int64 += 0x100000000LL;	
		}
	}

	m_Duration.m_Int64 = cnt.m_Int64 - m_Duration.m_Int64;

#ifdef DEBUG_FASTTIMER
	m_bRunning = false;
#endif
}

inline CCycleCount CFastTimer::GetDurationInProgress() const
{
	CCycleCount cnt;
	cnt.Sample();
	if ( IsX360() )
	{
		// have to handle rollover, hires timer is only accurate to 32 bits
		// more than one overflow should not have occurred, otherwise caller should use a slower timer
		if ( (uint64)cnt.m_Int64 <= (uint64)m_Duration.m_Int64 )
		{
			// rollover occurred	
			cnt.m_Int64 += 0x100000000LL;	
		}
	}

	CCycleCount result;
	result.m_Int64 = cnt.m_Int64 - m_Duration.m_Int64;
	
	return result;
}


inline CCycleCount const& CFastTimer::GetDuration() const
{
#ifdef DEBUG_FASTTIMER
	assert( !m_bRunning );
#endif
	return m_Duration;
}


#endif // FASTTIMER_H
