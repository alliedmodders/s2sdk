#ifndef VECTORALIGNED_H
#define VECTORALIGNED_H

#ifdef _WIN32
#pragma once
#endif

#include "mathlib/vector.h"
#include "tier0/memalloc.h"

//-----------------------------------------------------------------------------
// Here's where we add all those lovely SSE optimized routines
//-----------------------------------------------------------------------------

class ALIGN16 VectorAligned : public Vector
{
public:
	inline VectorAligned(void) {};
	inline VectorAligned(vec_t X, vec_t Y, vec_t Z) 
	{
		Init(X,Y,Z);
	}

#ifdef VECTOR_NO_SLOW_OPERATIONS

private:
	// No copy constructors allowed if we're in optimal mode
	VectorAligned(const VectorAligned& vOther);
	VectorAligned(const Vector &vOther);

#else
public:
	explicit VectorAligned(const Vector &vOther) 
	{
		Init(vOther.x, vOther.y, vOther.z);
	}
	
	VectorAligned& operator=(const Vector &vOther)	
	{
		Init(vOther.x, vOther.y, vOther.z);
		return *this;
	}

	VectorAligned& operator=(const VectorAligned &vOther)
	{
		// we know we're aligned, so use simd
		// we can't use the convenient abstract interface coz it gets declared later
#if _WIN32
		_mm_store_ps(Base(), _mm_load_ps( vOther.Base() ));
#else
		Init(vOther.x, vOther.y, vOther.z);
#endif
		return *this;
	}

	
#endif
	float w;	// this space is used anyway

#if !defined(NO_MALLOC_OVERRIDE)
	void* operator new[] ( size_t nSize)
	{
		return MemAlloc_AllocAligned(nSize, 16);
	}

	void operator delete[] ( void* p) 
	{
		MemAlloc_FreeAligned(p);
	}

	// please don't allocate a single quaternion...
	void* operator new   ( size_t nSize )
	{
		return MemAlloc_AllocAligned(nSize, 16);
	}
	void operator delete ( void* p) 
	{
		MemAlloc_FreeAligned(p);
	}
#endif
} ALIGN16_POST;

#endif // VECTORALIGNED_H
