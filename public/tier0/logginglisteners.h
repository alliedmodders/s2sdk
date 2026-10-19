#ifndef LOGGINGLISTENERS_H
#define LOGGINGLISTENERS_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/loggingtypes.h"
#include <stdio.h>

// Used by CColorizedLoggingListener
#include "tier0/win32consoleio.h"

//-----------------------------------------------------------------------------
// A basic logging listener which prints to stdout and the debug channel.
//-----------------------------------------------------------------------------
class CSimpleLoggingListener : public ILoggingListener
{
public:
	CSimpleLoggingListener( bool bQuietPrintf = false, bool bQuietDebugger = false ) : 
	  m_bQuietPrintf( bQuietPrintf ), 
		  m_bQuietDebugger( bQuietDebugger ) 
	  { 
	  }

	  virtual void Log( const LoggingContext_t *pContext, const tchar *pMessage )
	  {
		  {
#ifndef _CERT
			  if ( !m_bQuietPrintf )
			  {
				  Plat_Printf( _T("%s"), pMessage );
			  }
#endif

#ifdef _WIN32
			  if ( !m_bQuietDebugger && Plat_IsInDebugSession() )
			  {
				  Plat_DebugString( pMessage );
			  }
#endif
		  }
	  }

	  // If set to true, does not print anything to stdout.
	  bool m_bQuietPrintf;
	  // If set to true, does not print anything to debugger.
	  bool m_bQuietDebugger;
};

//-----------------------------------------------------------------------------
// A basic logging listener for GUI applications
//-----------------------------------------------------------------------------
class CSimpleWindowsLoggingListener : public ILoggingListener
{
public:
	virtual void Log( const LoggingContext_t *pContext, const tchar *pMessage )
	{
		if ( Plat_IsInDebugSession() )
		{
			Plat_DebugString( pMessage );
		}
		if ( pContext->m_Severity == LS_ERROR )
		{
			if ( Plat_IsInDebugSession() )
				DebuggerBreak();
			
			if ( !Plat_IsInTestMode() )
				Plat_MessageBox( "Error", pMessage );
		}
	}
};


//-----------------------------------------------------------------------------
// ** NOTE FOR INTEGRATION **
// This was copied over from source 2 rather than integrated because 
// source 2 has more significantly refactored tier0 logging.
//
// A logging listener with Win32 console API color support which which prints 
// to stdout and the debug channel.
//-----------------------------------------------------------------------------
class CColorizedLoggingListener : public CSimpleLoggingListener
{
public:
	CColorizedLoggingListener( bool bQuietPrintf = false, bool bQuietDebugger = false ) : CSimpleLoggingListener( bQuietPrintf, bQuietDebugger )
	{
		InitWin32ConsoleColorContext( &m_ColorContext );
	}

	virtual void Log( const LoggingContext_t *pContext, const tchar *pMessage )
	{
		if ( !m_bQuietPrintf )
		{
			int nPrevColor = -1;

			if ( pContext->m_Color != UNSPECIFIED_LOGGING_COLOR )
			{
				nPrevColor = SetWin32ConsoleColor( &m_ColorContext,
					pContext->m_Color.r(), pContext->m_Color.g(), pContext->m_Color.b(), 
					MAX( MAX( pContext->m_Color.r(), pContext->m_Color.g() ), pContext->m_Color.b() ) > 128 );
			}

			Plat_Printf( _T("%s"), pMessage );

			if ( nPrevColor >= 0 )
			{
				RestoreWin32ConsoleColor( &m_ColorContext, nPrevColor );
			}
		}

#ifdef _WIN32
		if ( !m_bQuietDebugger && Plat_IsInDebugSession() )
		{
			Plat_DebugString( pMessage );
		}
#endif
	}

	Win32ConsoleColorContext_t m_ColorContext;
};

#endif // LOGGINGLISTENERS_H
