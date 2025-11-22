message(STATUS "Adding Boost")

include(FetchContent)

set(BOOST_INCLUDE_LIBRARIES
        core
        heap
        dynamic_bitset
)

set(BOOST_ENABLE_CMAKE ON CACHE BOOL "" FORCE)
set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(
        Boost
        URL https://github.com/boostorg/boost/releases/download/boost-1.88.0/boost-1.88.0-cmake.7z
        USES_TERMINAL_DOWNLOAD TRUE
        GIT_PROGRESS TRUE
        DOWNLOAD_NO_EXTRACT FALSE
)

FetchContent_MakeAvailable(Boost)

# ---------------------------------------------------------------------- #
#  Interface library
# ---------------------------------------------------------------------- #

add_library(libBoost INTERFACE)

target_link_libraries(
        libBoost
        INTERFACE
        Boost::core
        Boost::heap
)

# Add Boost include directory for header-only libraries like dynamic_bitset
# Try multiple ways to get the Boost include directory
if(TARGET Boost::dynamic_bitset)
    target_link_libraries(libBoost INTERFACE Boost::dynamic_bitset)
elseif(DEFINED Boost_SOURCE_DIR)
    target_include_directories(libBoost INTERFACE "${Boost_SOURCE_DIR}")
elseif(TARGET Boost::boost)
    target_link_libraries(libBoost INTERFACE Boost::boost)
else()
    # Fallback: try to find Boost_SOURCE_DIR from the populated content
    get_target_property(BOOST_SOURCE_DIR Boost SOURCE_DIR)
    if(BOOST_SOURCE_DIR)
        target_include_directories(libBoost INTERFACE "${BOOST_SOURCE_DIR}")
    endif()
endif()

message(STATUS "Boost interface library 'libBoost' created.")

