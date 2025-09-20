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

set(CMAKE_GENERATOR_PLATFORM "x64" CACHE STRING "Target platform architecture")
set(WIN32 1)
set(MSVC 1)

# Find vswhere using modern block-based path conversion
block(SCOPE_FOR VARIABLES)
  cmake_path(
    CONVERT "$ENV{ProgramFiles\(x86\)}/Microsoft Visual Studio/Installer"
    TO_CMAKE_PATH_LIST _vswhere_dir
    NORMALIZE)
  find_program(_vswhere NAMES vswhere HINTS ${_vswhere_dir} DOC "Visual Studio Locator" REQUIRED)
  set(VSWHERE_EXECUTABLE "${_vswhere}" PARENT_SCOPE)
endblock()

# Get latest VS installation path
execute_process(
    COMMAND "${VSWHERE_EXECUTABLE}" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    OUTPUT_VARIABLE VS_INSTALL_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(NOT VS_INSTALL_DIR)
    message(FATAL_ERROR "Could not detect a valid Visual Studio installation.")
endif()

# Get latest VC toolchain path
file(GLOB VC_TOOL_DIR "${VS_INSTALL_DIR}/VC/Tools/MSVC/*")
list(SORT VC_TOOL_DIR ORDER DESCENDING)
list(GET VC_TOOL_DIR 0 VC_TOOLS_PATH)
if(NOT EXISTS "${VC_TOOLS_PATH}")
    message(FATAL_ERROR "No VC Tools directory found under ${VS_INSTALL_DIR}/VC/Tools/MSVC/")
endif()

# Locate cl.exe
set(CL_PATH "${VC_TOOLS_PATH}/bin/Hostx64/x64/cl.exe")
if(NOT EXISTS "${CL_PATH}")
    message(FATAL_ERROR "Could not find cl.exe at expected path: ${CL_PATH}")
endif()

cmake_path(GET VC_TOOLS_PATH FILENAME VC_TOOLS_VERSION)
message(STATUS "Visual Studio: ${VS_INSTALL_DIR}")
message(STATUS "VC Tools Version: ${VC_TOOLS_VERSION}")
message(STATUS "Using cl.exe: ${CL_PATH}")

# Set compilers
set(CMAKE_C_COMPILER "${CL_PATH}" CACHE STRING "Path to cl.exe for C")
set(CMAKE_CXX_COMPILER "${CL_PATH}" CACHE STRING "Path to cl.exe for C++")


message(CHECK_START "Searching for Windows SDK Root Directory")
cmake_host_system_information(RESULT MSVS_WINDOWS_SDK_ROOT_PATH QUERY
        WINDOWS_REGISTRY "HKLM/SOFTWARE/Microsoft/Windows Kits/Installed Roots"
        VALUE "KitsRoot10"
        VIEW HOST
        ERROR_VARIABLE error
)
if (error)
    message(CHECK_FAIL "not found : ${error}")
else()
    cmake_path(CONVERT "${MSVS_WINDOWS_SDK_ROOT_PATH}"
            TO_CMAKE_PATH_LIST MSVS_WINDOWS_SDK_ROOT_PATH
            NORMALIZE)
    message(CHECK_PASS "found : ${MSVS_WINDOWS_SDK_ROOT_PATH}")
endif()

# Enable compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Enable generation of compile_commands.json")
