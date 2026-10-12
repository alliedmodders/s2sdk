cmake_minimum_required(VERSION 3.16)
project(s2sdk LANGUAGES C CXX)

# Builds the SDK's libraries from source, the way the AMBuilders do:
# interfaces, mathlib and libprotobuf, into lib/ of the build folder laid out
# like the repository's lib/.
#
#   cmake -S . -B build
#   cmake --build build
#
# It describes nothing for projects that build against the SDK; those get the
# include paths, defines and libraries from hl2sdk-manifests.
#
# S2SDK_TARGET_PREFIX names the targets (s2sdk by default), so a project can
# build the libraries of several games, each under its own prefix:
#
#   set(S2SDK_TARGET_PREFIX s2sdk_dota)
#   add_subdirectory(path/to/s2sdk-dota)
#
# With MSVC the libraries use the static runtime (/MT), like the prebuilt ones.

if(NOT DEFINED S2SDK_TARGET_PREFIX)
	set(S2SDK_TARGET_PREFIX s2sdk)
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
	message(FATAL_ERROR "s2sdk only supports 64-bit targets")
endif()

set(S2SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# AMBuildScript's settings for the SDK's own code
add_library(${S2SDK_TARGET_PREFIX}_build_settings INTERFACE)
target_compile_features(${S2SDK_TARGET_PREFIX}_build_settings INTERFACE cxx_std_20)
target_include_directories(${S2SDK_TARGET_PREFIX}_build_settings INTERFACE
	"${S2SDK_DIR}/public"
	"${S2SDK_DIR}/public/tier0"
	"${S2SDK_DIR}/public/tier1"
)
target_compile_definitions(${S2SDK_TARGET_PREFIX}_build_settings INTERFACE
	X64BITS PLATFORM_64BITS _LIB
	$<$<CONFIG:Debug>:DEBUG> $<$<CONFIG:Debug>:_DEBUG>
	$<$<NOT:$<CONFIG:Debug>>:NDEBUG>
)

if(MSVC)
	target_compile_definitions(${S2SDK_TARGET_PREFIX}_build_settings INTERFACE
		COMPILER_MSVC COMPILER_MSVC64
		_WINDOWS WINDOWS WIN32 WIN64 _CRT_SECURE_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE
		# The game is built without iterator debugging, which changes the layout of STL
		# types, like the strings in protobuf messages that plugins share with the game
		_ITERATOR_DEBUG_LEVEL=0
	)
else()
	target_compile_definitions(${S2SDK_TARGET_PREFIX}_build_settings INTERFACE
		COMPILER_GCC _LINUX LINUX POSIX GNUC
		_finite=finite
		stricmp=strcasecmp _stricmp=strcasecmp
		strnicmp=strncasecmp _strnicmp=strncasecmp
		strcmpi=strcasecmp
		_vsnprintf=vsnprintf
		_alloca=alloca
	)
	target_compile_options(${S2SDK_TARGET_PREFIX}_build_settings INTERFACE
		-Wall -Werror -Wno-overloaded-virtual -Wno-non-virtual-dtor -Wno-delete-non-virtual-dtor
		-Wno-invalid-offsetof -Wno-unknown-pragmas -Wno-unused -Wno-narrowing -Wno-register
		-march=nocona -mmmx -msse -msse2
	)
endif()

add_library(${S2SDK_TARGET_PREFIX}_interfaces STATIC
	interfaces/interfaces.cpp
)

# The same sources, in the same order, as mathlib/AMBuilder
add_library(${S2SDK_TARGET_PREFIX}_mathlib STATIC
	mathlib/almostequal.cpp
	mathlib/anorms.cpp
	mathlib/bumpvects.cpp
	mathlib/color_conversion.cpp
	mathlib/halton.cpp
	mathlib/icekey.cpp
	mathlib/imagequant.cpp
	mathlib/lightdesc.cpp
	mathlib/mathlib_base.cpp
	mathlib/polyhedron.cpp
	mathlib/powsse.cpp
	mathlib/quantize.cpp
	mathlib/randsse.cpp
	mathlib/simdvectormatrix.cpp
	mathlib/sparse_convolution_noise.cpp
	mathlib/spherical.cpp
	mathlib/sse.cpp
	mathlib/sseconst.cpp
	mathlib/ssenoise.cpp
	mathlib/vmatrix.cpp
)
target_include_directories(${S2SDK_TARGET_PREFIX}_mathlib PRIVATE "${S2SDK_DIR}/public/mathlib")

# libprotobuf: the sources its own CMake build compiles, like its AMBuilder
set(_protobuf_dir "${S2SDK_DIR}/thirdparty/protobuf-3.21.8")
set(_protobuf_sources)
foreach(_list libprotobuf-lite libprotobuf)
	file(READ "${_protobuf_dir}/cmake/${_list}.cmake" _content)
	string(REPLACE "-" "_" _var "${_list}_files")
	string(REGEX MATCH "set\\(${_var}[^)]*\\)" _files "${_content}")
	string(REGEX MATCHALL "\\$\\{protobuf_SOURCE_DIR\\}/[^ \t\r\n)]+\\.cc" _cc "${_files}")
	string(REPLACE "\${protobuf_SOURCE_DIR}" "${_protobuf_dir}" _cc "${_cc}")
	list(APPEND _protobuf_sources ${_cc})
endforeach()
add_library(${S2SDK_TARGET_PREFIX}_libprotobuf STATIC ${_protobuf_sources})
target_include_directories(${S2SDK_TARGET_PREFIX}_libprotobuf PRIVATE "${_protobuf_dir}/src")
# Third-party code, so its warnings are left to upstream
if(MSVC)
	target_compile_options(${S2SDK_TARGET_PREFIX}_libprotobuf PRIVATE /w /EHsc /utf-8 /bigobj)
else()
	target_compile_options(${S2SDK_TARGET_PREFIX}_libprotobuf PRIVATE -w)
	# The engine's and the prebuilt library's std::string ABI
	target_compile_definitions(${S2SDK_TARGET_PREFIX}_libprotobuf PRIVATE _GLIBCXX_USE_CXX11_ABI=0)
endif()

# Under the names and in the folders of the repository's lib/
if(WIN32)
	set(_lib_folder "${CMAKE_CURRENT_BINARY_DIR}/lib/public/win64")
else()
	set(_lib_folder "${CMAKE_CURRENT_BINARY_DIR}/lib/linux64")
endif()
foreach(lib interfaces mathlib libprotobuf)
	set(target ${S2SDK_TARGET_PREFIX}_${lib})
	target_link_libraries(${target} PRIVATE ${S2SDK_TARGET_PREFIX}_build_settings)
	set_target_properties(${target} PROPERTIES
		OUTPUT_NAME ${lib}
		PREFIX ""
		POSITION_INDEPENDENT_CODE ON
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
		# A generator expression, so multi-config generators add no per-config folder
		ARCHIVE_OUTPUT_DIRECTORY "$<1:${_lib_folder}>"
	)
endforeach()
