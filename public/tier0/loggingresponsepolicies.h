#ifndef LOGGINGRESPONSEPOLICIES_H
#define LOGGINGRESPONSEPOLICIES_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/loggingtypes.h"
#include "icommandline.h"

//-----------------------------------------------------------------------------
// Default logging response policy used when one is not specified.
//-----------------------------------------------------------------------------
class CDefaultLoggingResponsePolicy : public ILoggingResponsePolicy
{
public:
	virtual LoggingResponse_t OnLog( const LoggingContext_t *pContext )
	{
		if ( pContext->m_Severity == LS_ASSERT && !CommandLine()->FindParm( "-noassert" ) ) 
		{
			return LR_DEBUGGER;
		}
		else if ( pContext->m_Severity == LS_ERROR )
		{
			return LR_ABORT;
		}
		else
		{
			return LR_CONTINUE;
		}
	}
};

//-----------------------------------------------------------------------------
// A logging response policy which never terminates the process, even on error.
//-----------------------------------------------------------------------------
class CNonFatalLoggingResponsePolicy : public ILoggingResponsePolicy
{
public:
	virtual LoggingResponse_t OnLog( const LoggingContext_t *pContext )
	{
		if ( ( pContext->m_Severity == LS_ASSERT && !CommandLine()->FindParm( "-noassert" ) ) || pContext->m_Severity == LS_ERROR )
		{
			return LR_DEBUGGER;
		}
		else
		{
			return LR_CONTINUE;
		}
	}
};

#endif // LOGGINGRESPONSEPOLICIES_H
