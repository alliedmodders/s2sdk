//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: setjmp/longjmp based cooperative multitasking system
//
//=============================================================================

#ifndef COROUTINE_H
#define COROUTINE_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"

// function prototype
typedef void (*CoroutineFunc_t)( void * );

// handle to a coroutine
typedef int32 HCoroutine;

// creates a new coroutine
// no coroutine code is executed until Coroutine_Continue() is called
PLATFORM_INTERFACE HCoroutine Coroutine_Create( CoroutineFunc_t pFunc, void *pvParam );

// continues the specified coroutine
// returns true if the coroutine is still running, false otherwise
PLATFORM_INTERFACE bool Coroutine_Continue( HCoroutine hCoroutine, const char *pchName = nullptr );

// cancels a currently running coroutine
PLATFORM_INTERFACE void Coroutine_Cancel( HCoroutine hCoroutine );

// called from the coroutine to return control to the main thread
PLATFORM_INTERFACE void Coroutine_YieldToMain();

// returns true if the code is currently running inside of a coroutine
PLATFORM_INTERFACE bool Coroutine_IsActive();

#endif // COROUTINE_H
