cmake_minimum_required(VERSION 3.16)
project(s2sdk LANGUAGES C CXX)

# Consumers link s2sdk (or s2sdk::s2sdk): the SDK's include paths, the defines
# its headers expect, C++17, and tier0, interfaces, mathlib and protobuf.
#
#   add_subdirectory(path/to/s2sdk)
#   target_link_libraries(my_plugin PRIVATE s2sdk::s2sdk)
#
# interfaces and mathlib are the prebuilt ones in lib/ unless S2SDK_BUILD_LIBS
# is on, which builds them from interfaces/ and mathlib/ like the AMBuilders.
# S2SDK_PROTOC is the protoc for the host, and s2sdk_protobufs() generates and
# compiles the SDK's protos a target uses.
#
# With MSVC the libraries use the static runtime (/MT), so the consumer has to
# as well: set CMAKE_MSVC_RUNTIME_LIBRARY to MultiThreaded before adding s2sdk.

option(S2SDK_BUILD_LIBS "Build interfaces and mathlib from source instead of linking the prebuilt ones in lib/" OFF)

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
	message(FATAL_ERROR "s2sdk only supports 64-bit targets")
endif()

if(MSVC AND NOT CMAKE_MSVC_RUNTIME_LIBRARY MATCHES "^MultiThreaded(\\$<\\$<CONFIG:Debug>:Debug>)?$")
	message(WARNING "s2sdk's libraries use the static runtime (/MT); set CMAKE_MSVC_RUNTIME_LIBRARY to MultiThreaded")
endif()

set(S2SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "s2sdk root")

if(CMAKE_HOST_WIN32)
	set(S2SDK_PROTOC "${S2SDK_DIR}/devtools/bin/protoc.exe" CACHE INTERNAL "protoc")
else()
	set(S2SDK_PROTOC "${S2SDK_DIR}/devtools/bin/linux/protoc" CACHE INTERNAL "protoc")
endif()

if(WIN32)
	set(S2SDK_LIB_DIR "${S2SDK_DIR}/lib/public/win64")
	set(S2SDK_TIER0 "${S2SDK_LIB_DIR}/tier0.lib")
	set(S2SDK_INTERFACES "${S2SDK_LIB_DIR}/interfaces.lib")
	set(S2SDK_MATHLIB "${S2SDK_LIB_DIR}/mathlib.lib")
	set(S2SDK_PROTOBUF "${S2SDK_LIB_DIR}/2015/libprotobuf.lib")
	set(S2SDK_STEAM_API "${S2SDK_LIB_DIR}/steam_api64.lib")
else()
	set(S2SDK_LIB_DIR "${S2SDK_DIR}/lib/linux64")
	set(S2SDK_TIER0 "${S2SDK_LIB_DIR}/libtier0.so")
	set(S2SDK_INTERFACES "${S2SDK_LIB_DIR}/interfaces.a")
	set(S2SDK_MATHLIB "${S2SDK_LIB_DIR}/mathlib.a")
	set(S2SDK_PROTOBUF "${S2SDK_LIB_DIR}/release/libprotobuf.a")
	set(S2SDK_STEAM_API "${S2SDK_LIB_DIR}/libsteam_api.so")
endif()

# Includes, defines and language level, without any library
add_library(s2sdk_headers INTERFACE)
add_library(s2sdk::headers ALIAS s2sdk_headers)

target_compile_features(s2sdk_headers INTERFACE cxx_std_17)

target_include_directories(s2sdk_headers INTERFACE
	"${S2SDK_DIR}/thirdparty/protobuf-3.21.8/src"
	"${S2SDK_DIR}/public"
	"${S2SDK_DIR}/public/engine"
	"${S2SDK_DIR}/public/mathlib"
	"${S2SDK_DIR}/public/tier0"
	"${S2SDK_DIR}/public/tier1"
	"${S2SDK_DIR}/public/entity2"
	"${S2SDK_DIR}/public/game/server"
	"${S2SDK_DIR}/game/shared"
	"${S2SDK_DIR}/game/server"
	"${S2SDK_DIR}/common"
)

target_compile_definitions(s2sdk_headers INTERFACE X64BITS PLATFORM_64BITS)

if(MSVC)
	target_compile_definitions(s2sdk_headers INTERFACE COMPILER_MSVC COMPILER_MSVC64 WIN32 WIN64)
else()
	target_compile_definitions(s2sdk_headers INTERFACE
		COMPILER_GCC _LINUX LINUX POSIX GNUC
		# libprotobuf.a and tier0 use the old std::string ABI
		_GLIBCXX_USE_CXX11_ABI=0
		_finite=finite
		stricmp=strcasecmp _stricmp=strcasecmp
		strnicmp=strncasecmp _strnicmp=strncasecmp
		strcmpi=strcasecmp
		_vsnprintf=vsnprintf
		_alloca=alloca
	)
endif()

if(S2SDK_BUILD_LIBS)
	add_library(s2sdk_interfaces STATIC
		interfaces/interfaces.cpp
	)

	add_library(s2sdk_mathlib STATIC
		mathlib/anorms.cpp
		mathlib/bumpvects.cpp
		mathlib/color_conversion.cpp
		mathlib/halton.cpp
		mathlib/IceKey.cpp
		mathlib/imagequant.cpp
		mathlib/lightdesc.cpp
		mathlib/mathlib_base.cpp
		mathlib/polyhedron.cpp
		mathlib/powsse.cpp
		mathlib/quantize.cpp
		mathlib/randsse.cpp
		mathlib/simdvectormatrix.cpp
		mathlib/sparse_convolution_noise.cpp
		mathlib/sse.cpp
		mathlib/sseconst.cpp
		mathlib/ssenoise.cpp
		mathlib/vmatrix.cpp
	)

	# The same settings as AMBuildScript
	foreach(lib s2sdk_interfaces s2sdk_mathlib)
		target_link_libraries(${lib} PRIVATE s2sdk_headers)
		target_compile_definitions(${lib} PRIVATE _LIB)
		set_target_properties(${lib} PROPERTIES
			POSITION_INDEPENDENT_CODE ON
			PREFIX ""
			MSVC_RUNTIME_LIBRARY MultiThreaded
		)
		if(MSVC)
			target_compile_definitions(${lib} PRIVATE _WINDOWS WINDOWS _CRT_SECURE_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE)
		else()
			target_compile_definitions(${lib} PRIVATE NO_MALLOC_OVERRIDE)
			target_compile_options(${lib} PRIVATE
				-Wall -Wno-overloaded-virtual -Wno-non-virtual-dtor -Wno-delete-non-virtual-dtor
				-Wno-invalid-offsetof -Wno-unknown-pragmas -Wno-unused -Wno-narrowing -Wno-register
				-fno-strict-aliasing -march=nocona -mmmx -msse -msse2
			)
		endif()
	endforeach()

	set_target_properties(s2sdk_interfaces PROPERTIES OUTPUT_NAME interfaces)
	set_target_properties(s2sdk_mathlib PROPERTIES OUTPUT_NAME mathlib)
	target_include_directories(s2sdk_mathlib PRIVATE "${S2SDK_DIR}/public/mathlib")

	set(S2SDK_INTERFACES s2sdk_interfaces)
	set(S2SDK_MATHLIB s2sdk_mathlib)
endif()

add_library(s2sdk INTERFACE)
add_library(s2sdk::s2sdk ALIAS s2sdk)

target_link_libraries(s2sdk INTERFACE
	s2sdk_headers
	${S2SDK_MATHLIB}
	${S2SDK_INTERFACES}
	"${S2SDK_TIER0}"
	"${S2SDK_PROTOBUF}"
)

add_library(s2sdk_steam_api INTERFACE)
add_library(s2sdk::steam_api ALIAS s2sdk_steam_api)
target_link_libraries(s2sdk_steam_api INTERFACE "${S2SDK_STEAM_API}")

# Generates the given protos into a target and compiles them with it, like
# AMBuild's Protoc tool with the protoc hl2sdk-manifests sets up. The protos are
# relative to the SDK or absolute, and a proto's imports have to be listed too.
# Imports are looked up in each proto's folder, then the SDK's include paths.
# The generated headers' folder is public, for a static library of protobufs.
#
#   s2sdk_protobufs(my_plugin common/netmessages.proto common/networkbasetypes.proto)
function(s2sdk_protobufs target)
	set(out "${CMAKE_CURRENT_BINARY_DIR}/${target}_protobufs")
	file(MAKE_DIRECTORY "${out}")

	set(protos)
	set(dirs)
	foreach(proto ${ARGN})
		if(NOT IS_ABSOLUTE "${proto}")
			set(proto "${S2SDK_DIR}/${proto}")
		endif()
		get_filename_component(dir "${proto}" DIRECTORY)
		list(APPEND protos "${proto}")
		list(APPEND dirs "${dir}")
	endforeach()
	get_target_property(includes s2sdk_headers INTERFACE_INCLUDE_DIRECTORIES)
	list(APPEND dirs ${includes})

	# One protoc run per proto with its own folder first, like the Protoc tool: a
	# proto in a subfolder of another one's folder would otherwise be seen under
	# two names when one imports it.
	set(sources)
	foreach(proto ${protos})
		get_filename_component(dir "${proto}" DIRECTORY)
		get_filename_component(name "${proto}" NAME_WE)
		set(proto_paths "${dir}" ${dirs})
		list(REMOVE_DUPLICATES proto_paths)
		list(TRANSFORM proto_paths PREPEND "--proto_path=")
		add_custom_command(
			OUTPUT "${out}/${name}.pb.cc" "${out}/${name}.pb.h"
			COMMAND "${S2SDK_PROTOC}" ${proto_paths} "--cpp_out=${out}" "${proto}"
			DEPENDS ${protos}
			COMMENT "Generating ${name}.pb.cc"
			VERBATIM
		)
		list(APPEND sources "${out}/${name}.pb.cc")
	endforeach()

	target_sources(${target} PRIVATE ${sources})
	target_include_directories(${target} PUBLIC "${out}")
endfunction()
