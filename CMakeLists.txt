# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required(
    VERSION 4.0.0
    FATAL_ERROR
)

project ("Vulkan_RubiksCube"
        LANGUAGES CXX
        VERSION 0.0.1)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Require at least C++17 or later
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/Dependencies.cmake)
include(GNUInstallDirs)

# Convert paths to CMake-safe paths
# Update paths to use the correct source folder name 'Vk_RubiksCube'
file(TO_CMAKE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Vk_RubiksCube/resources/" RESOURCE_PATH)
file(TO_CMAKE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Vk_RubiksCube/shaders/" SHADER_PATH)

# Store them as cache variables to allow editing via CMake GUI or Visual Studio
set(RESOURCE_PATH "${RESOURCE_PATH}" CACHE STRING "Path to the resources directory")
set(SHADER_PATH "${SHADER_PATH}" CACHE STRING "Path to the shaders directory")

# Configure a header file with this path
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"  # Template file
    "${CMAKE_CURRENT_SOURCE_DIR}/Vk_RubiksCube/Config.h"  # Output file
)

add_subdirectory(Vk_RubiksCube)