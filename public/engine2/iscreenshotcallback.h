#ifndef ISCREENSHOTCALLBACK_H
#define ISCREENSHOTCALLBACK_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"

//-----------------------------------------------------------------------------
// Purpose: Receives the images requested with ISource2Engine::RequestScreenshot().
// The context is the value the request was queued with.
// Returning false from Screenshot, ScreenshotJPEG and ScreenshotPNG lets the engine free the buffer after the call.
//-----------------------------------------------------------------------------
abstract_class IScreenshotCallback
{
public:
	virtual bool Screenshot( int nWidth, int nHeight, int nImageFormat, uint8 *pPixels, int nSize, int64 nContext ) = 0;
	virtual bool ScreenshotJPEG( int nWidth, int nHeight, void *pData, int nSize, void *pContext ) = 0;
	virtual bool ScreenshotPNG( int nWidth, int nHeight, void *pData, int nSize, void *pContext ) = 0;
	// Returning false lets the engine go on to make the screenshot from these pixels
	virtual bool Screenshot_GPUFrame( uint32 nWidth, uint32 nHeight, int nImageFormat, const uint8 *pPixels, size_t nPitch, int64 nContext, uint32 ) = 0;
	virtual bool IsDoneWithData( void *pContext ) = 0;
};

#endif // ISCREENSHOTCALLBACK_H
