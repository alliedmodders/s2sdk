#ifndef TIER0_STDLIB_H
#define TIER0_STDLIB_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"

// Wrappers of the C runtime math functions.
// The d suffix takes and returns double and the f suffix float. Angles are in radians.

// Trigonometric functions
PLATFORM_INTERFACE double V_acosd( double x );
PLATFORM_INTERFACE float V_acosf( float x );
PLATFORM_INTERFACE double V_asind( double x );
PLATFORM_INTERFACE float V_asinf( float x );
PLATFORM_INTERFACE double V_atand( double x );
PLATFORM_INTERFACE float V_atanf( float x );
PLATFORM_INTERFACE double V_atan2d( double y, double x );
PLATFORM_INTERFACE float V_atan2f( float y, float x );
PLATFORM_INTERFACE double V_cosd( double x );
PLATFORM_INTERFACE float V_cosf( float x );
PLATFORM_INTERFACE double V_sind( double x );
PLATFORM_INTERFACE float V_sinf( float x );
PLATFORM_INTERFACE void V_sincosd( double x, double *pSin, double *pCos );
PLATFORM_INTERFACE void V_sincosf( float x, float *pSin, float *pCos );
PLATFORM_INTERFACE double V_tand( double x );
PLATFORM_INTERFACE float V_tanf( float x );

// Hyperbolic functions
PLATFORM_INTERFACE double V_coshd( double x );
PLATFORM_INTERFACE float V_coshf( float x );
PLATFORM_INTERFACE double V_sinhd( double x );
PLATFORM_INTERFACE float V_sinhf( float x );
PLATFORM_INTERFACE double V_tanhd( double x );
PLATFORM_INTERFACE float V_tanhf( float x );

// Exponential and logarithmic functions
PLATFORM_INTERFACE double V_expd( double x );
PLATFORM_INTERFACE float V_expf( float x );
PLATFORM_INTERFACE double V_exp2d( double x );
PLATFORM_INTERFACE float V_exp2f( float x );
PLATFORM_INTERFACE double V_logd( double x );
PLATFORM_INTERFACE float V_logf( float x );
PLATFORM_INTERFACE double V_log2d( double x );
PLATFORM_INTERFACE float V_log2f( float x );
// Returns floor(log2(x)), and 0 for 0
PLATFORM_INTERFACE uint32 V_log2_uint( uint32 x );
PLATFORM_INTERFACE double V_log10d( double x );
PLATFORM_INTERFACE float V_log10f( float x );

// Power functions
PLATFORM_INTERFACE double V_powd( double x, double y );
PLATFORM_INTERFACE float V_powf( float x, float y );
PLATFORM_INTERFACE double V_tier0_sqrtd( double x );
PLATFORM_INTERFACE float V_tier0_sqrtf( float x );
PLATFORM_INTERFACE double V_hypotd( double x, double y );
PLATFORM_INTERFACE float V_hypotf( float x, float y );

// Rounding and remainder functions
PLATFORM_INTERFACE double V_tier0_ceild( double x );
PLATFORM_INTERFACE float V_tier0_ceilf( float x );
PLATFORM_INTERFACE double V_tier0_floord( double x );
PLATFORM_INTERFACE float V_tier0_floorf( float x );
PLATFORM_INTERFACE double V_roundd( double x );
PLATFORM_INTERFACE float V_roundf( float x );
PLATFORM_INTERFACE double V_fmodd( double x, double y );
PLATFORM_INTERFACE float V_fmodf( float x, float y );
PLATFORM_INTERFACE double V_remainderd( double x, double y );
PLATFORM_INTERFACE float V_remainderf( float x, float y );

// Floating-point manipulation functions
PLATFORM_INTERFACE double V_modfd( double x, double *pIntPart );
PLATFORM_INTERFACE float V_modff( float x, float *pIntPart );
PLATFORM_INTERFACE double V_frexpd( double x, int *pExponent );
PLATFORM_INTERFACE float V_frexpf( float x, int *pExponent );
PLATFORM_INTERFACE double V_ldexpd( double x, int nExponent );
PLATFORM_INTERFACE float V_ldexpf( float x, int nExponent );
PLATFORM_INTERFACE double V_tier0_fabsd( double x );
PLATFORM_INTERFACE float V_tier0_fabsf( float x );

// Classification functions
// Returns the FP_* value of the platform's C runtime, which differ between Windows and Linux
PLATFORM_INTERFACE int V_fpclassifyd( double x );
PLATFORM_INTERFACE int V_isfinited( double x );
PLATFORM_INTERFACE int V_isinfd( double x );
PLATFORM_INTERFACE int V_isnand( double x );

#endif // TIER0_STDLIB_H
