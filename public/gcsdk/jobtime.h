#ifndef JOBTIME_H
#define JOBTIME_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"

namespace GCSDK
{

// A time in microseconds on the GC client's job clock
class CJobTime
{
public:
	uint64 LTime() const { return m_lTime; }
	void SetLTime( uint64 lTime ) { m_lTime = lTime; }

private:
	uint64 m_lTime;
};

} // namespace GCSDK

#endif // JOBTIME_H
