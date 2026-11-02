#ifndef IPREREQUISITE_H
#define IPREREQUISITE_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"

class CPrerequisiteDescriptionHelper;

enum PrerequisiteStatus_t
{
	PS_NOT_SATISFIED = 0,
	PS_SATISFIED,
	PS_FAILED,
};

abstract_class IPrerequisite
{
public:
	IPrerequisite() : m_nRefCount( 0 ) {}
	virtual ~IPrerequisite() {}

	virtual void AddRef() = 0;
	virtual void Release() = 0;
	virtual PrerequisiteStatus_t GetPrerequisiteStatus( double flTimeoutTime ) = 0;
	virtual const char *GetName() const = 0;
	// AMNOTE: Adds the prerequisite's state to the helper's report, at depth nDepth
	virtual void Describe( int nDepth, CPrerequisiteDescriptionHelper *pHelper ) const = 0;

protected:
	int m_nRefCount;
};

#endif // IPREREQUISITE_H
