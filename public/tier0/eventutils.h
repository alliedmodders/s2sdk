#ifndef EVENTUTILS_H
#define EVENTUTILS_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"
#include "tier0/refcount.h"
#include "tier0/threadtools.h"

//-----------------------------------------------------------------------------
// A manual-reset event shared by reference, which goes back to its pool on its last Release()
//-----------------------------------------------------------------------------
class COneTimeEvent : public IRefCounted
{
public:
	// Returns a reset event with one reference
	DLL_CLASS_IMPORT static COneTimeEvent *Allocate();

	CThreadEvent &GetEvent() { return m_Event; }

private:
	CThreadEvent m_Event;
	CInterlockedInt m_nRefCount;
};

#endif // EVENTUTILS_H
