#ifndef FIELDPATH_H
#define FIELDPATH_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"

#define DEFAULT_MAX_PATH_DEPTH 10

class CFieldPath
{
public:
	union
	{
		// Used when m_bUsingExternalPaths is set
		int16 *m_pPaths;
		int16 m_FixedPaths[12];
	};

	int16 m_nCount;
	bool m_bUsingExternalPaths;
};

#endif /* FIELDPATH_H */
