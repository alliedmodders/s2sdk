#include "tier0/dbg.h"
#include "mathlib/vector.h"
#include "mathlib/mathlib.h"
#include "interfaces/interfaces.h"
#include "networkbasetypes.pb.h"

#ifdef _WIN32
#define TEST_EXPORT extern "C" __declspec(dllexport)
#else
#define TEST_EXPORT extern "C" __attribute__((visibility("default")))
#endif

TEST_EXPORT float CMakeTest( CreateInterfaceFn factory )
{
	Vector v( 3.0f, 4.0f, 0.0f );
	float length = VectorNormalize( v );

	ConnectInterfaces( &factory, 1 );
	Msg( "length %f\n", length );

	CMsgVector msg;
	msg.set_x( v.x );
	length += (float)msg.ByteSizeLong();

	return length;
}
