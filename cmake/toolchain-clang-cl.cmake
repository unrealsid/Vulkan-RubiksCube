if(NOT (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows))
    return()
endif()

if(NOT CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE)
    if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL ARM64)
        set(CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE arm64)
    else()
        set(CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE x64)
    endif()
endif()

if(NOT CMAKE_SYSTEM_PROCESSOR)
    set(CMAKE_SYSTEM_PROCESSOR ${CMAKE_HOST_SYSTEM_PROCESSOR})
endif()

if(NOT (CMAKE_SYSTEM_PROCESSOR STREQUAL ${CMAKE_HOST_SYSTEM_PROCESSOR}))
    set(CMAKE_SYSTEM_NAME Windows)
endif()

# Locate required tools
find_program(CLANG_CL_EXE clang-cl REQUIRED)
find_program(LLD_LINK_EXE lld-link REQUIRED)

set(CMAKE_C_COMPILER "${CLANG_CL_EXE}" CACHE STRING "Path to clang-cl C compiler")
set(CMAKE_CXX_COMPILER "${CLANG_CL_EXE}" CACHE STRING "Path to clang-cl C++ compiler")
set(CMAKE_LINKER "${LLD_LINK_EXE}" CACHE FILEPATH "Path to lld-link linker")

set(CMAKE_GENERATOR_PLATFORM "x64" CACHE STRING "Target architecture")

# Detect and log Windows SDK root (optional)
cmake_host_system_information(RESULT PATH_MSVS_WINDOWS_SDK_ROOT QUERY
  WINDOWS_REGISTRY "HKLM/SOFTWARE/Microsoft/Windows Kits/Installed Roots"
  VALUE "KitsRoot10"
  VIEW BOTH)
if (PATH_MSVS_WINDOWS_SDK_ROOT)
  cmake_path(CONVERT "${PATH_MSVS_WINDOWS_SDK_ROOT}" TO_CMAKE_PATH_LIST PATH_MSVS_WINDOWS_SDK_ROOT NORMALIZE)
  message(STATUS "Windows SDK Path: ${PATH_MSVS_WINDOWS_SDK_ROOT}")
else()
  message(WARNING "Could not determine Windows SDK path.")
endif()

# Export compile_commands.json for tooling
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Enable compilation database")