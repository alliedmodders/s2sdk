#ifndef RELIABLETIMER_H
#define RELIABLETIMER_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"

class CLimitTimer
{
public:
	explicit CLimitTimer( uint64 cMicroSecDuration ) { SetLimit( cMicroSecDuration ); }

	// Starts a period of cMicroSecDuration microseconds from now
	void SetLimit( uint64 cMicroSecDuration ) { m_nTickLimit = Plat_TicksAfterMicroSecs( cMicroSecDuration ); }

	// Whether the period has passed
	bool BLimitReached() const { return Plat_RelativeTicks() >= m_nTickLimit; }

private:
	uint64 m_nTickLimit;
};

#endif // RELIABLETIMER_H
