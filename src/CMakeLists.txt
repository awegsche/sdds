# CMakeList.txt : CMake project for sddslib, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

find_package(Boost COMPONENTS regex REQUIRED)
find_package(spdlog CONFIG REQUIRED)

# --------------------------------------------------------------------------------------------------
# --- test executable ------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------
add_executable (sdds_test
	"main.cpp"
	"sddslib.cpp"
	"sddslib.h"
	"endian.h"
	"endian_win.h"
	"endian_gnu.h"
)

target_include_directories(sdds_test PRIVATE ${Boost_INCLUDE_DIR})
set_property(TARGET sdds_test PROPERTY CXX_STANDARD 17)

target_link_libraries(sdds_test PRIVATE
	spdlog::spdlog
	debug
	${Boost_REGEX_LIBRARY_DEBUG}
	optimized
	${Boost_REGEX_LIBRARY_RELEASE}
)
target_compile_definitions(sdds_test PRIVATE -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG)

# --------------------------------------------------------------------------------------------------
# --- library --------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------
add_library (sdds "sddslib.cpp" "sddslib.h" "endian.h" "endian_win.h" "endian_gnu.h")

target_include_directories(sdds PRIVATE ${Boost_INCLUDE_DIR})
set_property(TARGET sdds PROPERTY CXX_STANDARD 17)
target_link_libraries(sdds PRIVATE
	spdlog::spdlog
	debug
	${Boost_REGEX_LIBRARY_DEBUG}
	optimized
	${Boost_REGEX_LIBRARY_RELEASE}
)

target_link_options(sdds PRIVATE -static-libgcc -static-libstdc++)
