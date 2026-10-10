# The CMake side of CheckScript: the tests' template instantiations and the
# sources plugins compile themselves, in a library linked like a plugin, and
# the test programs in run/. Built with -DS2SDK_BUILD_TESTS=ON.

file(GLOB _test_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB _plugin_sources CONFIGURE_DEPENDS "${S2SDK_DIR}/entity2/*.cpp" "${S2SDK_DIR}/tier1/*.cpp")

add_library(s2sdk_check SHARED
	${_test_sources}
	${_plugin_sources}
	"${S2SDK_DIR}/public/tier0/memoverride.cpp"
)
target_link_libraries(s2sdk_check PRIVATE s2sdk::s2sdk)
# AMBuildScript's flags, as CheckScript compiles with them: -march=nocona brings the
# 16-byte compare-exchange CTSQueue needs, -Werror fails on warnings
target_compile_definitions(s2sdk_check PRIVATE ${S2SDK_OWN_DEFINITIONS})
target_compile_options(s2sdk_check PRIVATE ${S2SDK_OWN_OPTIONS})
s2sdk_protobufs(s2sdk_check
	common/network_connection.proto
	# Messages for the tests, from a proto without imports
	common/source2_steam_stats.proto
)
# Fail on symbols the libraries don't have, which shared libraries allow by default
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	target_link_options(s2sdk_check PRIVATE -Wl,-z,defs)
endif()

# Runs KeyValues3 against tier0, so it needs the game's tier0 to start
add_executable(keyvalues3_test
	"${CMAKE_CURRENT_SOURCE_DIR}/run/keyvalues3.cpp"
	"${S2SDK_DIR}/tier1/keyvalues3.cpp"
	"${S2SDK_DIR}/public/tier0/memoverride.cpp"
)
target_link_libraries(keyvalues3_test PRIVATE s2sdk::s2sdk)
target_compile_definitions(keyvalues3_test PRIVATE ${S2SDK_OWN_DEFINITIONS})
target_compile_options(keyvalues3_test PRIVATE ${S2SDK_OWN_OPTIONS})

# Iterates the utl containers, which allocate through tier0
add_executable(utliterators_test
	"${CMAKE_CURRENT_SOURCE_DIR}/run/utliterators.cpp"
	"${S2SDK_DIR}/public/tier0/memoverride.cpp"
)
target_link_libraries(utliterators_test PRIVATE s2sdk::s2sdk)
target_compile_definitions(utliterators_test PRIVATE ${S2SDK_OWN_DEFINITIONS})
target_compile_options(utliterators_test PRIVATE ${S2SDK_OWN_OPTIONS})
