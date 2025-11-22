message(STATUS "Adding tabulate")

include(FetchContent)

FetchContent_Declare(
        tabulate
        GIT_REPOSITORY https://github.com/p-ranav/tabulate.git
        GIT_TAG        v1.5
)

FetchContent_MakeAvailable(tabulate)

# ------------------------------------------------------------------------------ #
# Interface Library
# ------------------------------------------------------------------------------ #
add_library(libTabulate INTERFACE)

# Tabulate is header-only, so we just need to add the include directory
# The FetchContent will make the source available, we need to point to the include directory
target_include_directories(libTabulate INTERFACE ${tabulate_SOURCE_DIR}/include)

message(STATUS "Tabulate interface library 'libTabulate' created.")

