#ifndef TIER1_ERRORLISTENER_H
#define TIER1_ERRORLISTENER_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"
#include "tier0/buffer_string.h"
#include "tier0/utlstring.h"

// AMNOTE: Names are guesses
enum ErrorListenerSeverity_t
{
	ERROR_LISTENER_SEVERITY_MESSAGE = 0,
	ERROR_LISTENER_SEVERITY_WARNING = 1,
	ERROR_LISTENER_SEVERITY_ERROR = 2,
};

// AMNOTE: Only the virtual destructor is known
class IBaseErrorContext
{
public:
	virtual ~IBaseErrorContext() {}
};

struct ErrorListenerMessage_t
{
	ErrorListenerMessage_t() : m_nSeverity( ERROR_LISTENER_SEVERITY_MESSAGE ), m_nLine( 0 ), m_nColumn( 0 ), m_pContext( nullptr ) {}
	~ErrorListenerMessage_t() { delete m_pContext; }

	ErrorListenerMessage_t( const ErrorListenerMessage_t & ) = delete;
	ErrorListenerMessage_t &operator=( const ErrorListenerMessage_t & ) = delete;

	CBufferString m_sMessage;
	int m_nSeverity; // ErrorListenerSeverity_t
	CUtlString m_sLocation;
	int m_nLine;
	int m_nColumn;
	IBaseErrorContext *m_pContext;
};

// AMNOTE: Name is a guess
struct ErrorContextHandle_t
{
	int m_nContextLength;
};

// No virtual destructor
abstract_class IErrorListener
{
public:
	virtual void ReportMessage( const ErrorListenerMessage_t &message ) = 0;
	virtual bool FormatMessageLocation( const ErrorListenerMessage_t &message, CBufferString &sOutLocation, const char *pszSuffix ) = 0;
	virtual const char *GetSourceName( int *pLineAndColumn ) = 0;
	virtual bool AppendContextPath( CBufferString &sOutContextPath ) = 0;
	virtual ErrorContextHandle_t PushContext( PRINTF_FORMAT_STRING const char *pszFormat, ... ) FMTFUNCTION( 2, 3 ) = 0;

	// A negative length does nothing
	virtual void PopContext( int nContextLength ) = 0;
};

#endif // TIER1_ERRORLISTENER_H
