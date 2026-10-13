#ifndef MESSAGES_BASE_H
#define MESSAGES_BASE_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"

PLATFORM_INTERFACE void Msg( const tchar* pMsg, ... );
PLATFORM_INTERFACE void Warning( const tchar *pMsg, ... ) FMTFUNCTION( 1, 2 );
PLATFORM_INTERFACE void DevMsg( int level, const tchar* pMsg, ... ) FMTFUNCTION( 2, 3 );
PLATFORM_INTERFACE void DevWarning( int level, const tchar *pMsg, ... ) FMTFUNCTION( 2, 3 );
PLATFORM_OVERLOAD void DevMsg( const tchar* pMsg, ... ) FMTFUNCTION( 1, 2 );
PLATFORM_OVERLOAD void DevWarning( const tchar *pMsg, ... ) FMTFUNCTION( 1, 2 );
PLATFORM_OVERLOAD void ConMsg( const tchar* pMsg, ... ) FMTFUNCTION( 1, 2 );
PLATFORM_INTERFACE void COM_TimestampedLog( char const *fmt, ... ) FMTFUNCTION( 1, 2 );

#endif // MESSAGES_BASE_H
