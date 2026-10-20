#ifndef QUATERNIONALIGNED_H
#define QUATERNIONALIGNED_H

#ifdef _WIN32
#pragma once
#endif

#include "mathlib/vector.h"
#include "tier0/memalloc.h"

//-----------------------------------------------------------------------------
// Here's where we add all those lovely SSE optimized routines
//-----------------------------------------------------------------------------
class ALIGN16 QuaternionAligned : public Quaternion
{
public:
	inline QuaternionAligned(void) {};
	inline QuaternionAligned(vec_t X, vec_t Y, vec_t Z, vec_t W) 
	{
		Init(X,Y,Z,W);
	}

	operator Quaternion * () { return this; } 
	operator const Quaternion * () { return this; } 

#ifdef VECTOR_NO_SLOW_OPERATIONS

private:
	// No copy constructors allowed if we're in optimal mode
	QuaternionAligned(const QuaternionAligned& vOther);
	QuaternionAligned(const Quaternion &vOther);

#else
public:
	explicit QuaternionAligned(const Quaternion &vOther) 
	{
		Init(vOther.x, vOther.y, vOther.z, vOther.w);
	}

	QuaternionAligned& operator=(const Quaternion &vOther)	
	{
		Init(vOther.x, vOther.y, vOther.z, vOther.w);
		return *this;
	}

	QuaternionAligned& operator=(const QuaternionAligned &vOther)
	{
		// we know we're aligned, so use simd
		// we can't use the convenient abstract interface coz it gets declared later
#if _WIN32
		_mm_store_ps(Base(), _mm_load_ps( vOther.Base() ));
#else
		Init(vOther.x, vOther.y, vOther.z, vOther.w);
#endif
		return *this;
	}

#endif

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

#endif // QUATERNIONALIGNED_H
