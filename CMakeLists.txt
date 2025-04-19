# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

# Allow users to specify Vulkan SDK path via CMake UI
# Set a cache variable for the Vulkan SDK path, defaulting to an empty string
set(VULKAN_SDK_PATH "" CACHE PATH "Path to the Vulkan SDK installation directory")

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Vulkan_RubiksCube") # Project name remains Vulkan_RubiksCube

# Require at least C++17 or later
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Convert paths to CMake-safe paths
# Update paths to use the correct source folder name 'Vk_RubiksCube'
file(TO_CMAKE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Vk_RubiksCube/resources/" RESOURCE_PATH) # Updated path
file(TO_CMAKE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Vk_RubiksCube/shaders/" SHADER_PATH)   # Updated path

# Store them as cache variables to allow editing via CMake GUI or Visual Studio
set(RESOURCE_PATH "${RESOURCE_PATH}" CACHE STRING "Path to the resources directory")
set(SHADER_PATH "${SHADER_PATH}" CACHE STRING "Path to the shaders directory")

# Configure a header file with this path
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"  # Template file
    "${CMAKE_CURRENT_SOURCE_DIR}/Vk_RubiksCube/Config.h"  # Output file (Updated path)
)

# Define the source directory using the correct folder name 'Vk_RubiksCube'
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Vk_RubiksCube) # Updated source directory

# Automatically get all .cpp and .h files from the src directory
file(GLOB_RECURSE SRC_FILES
    "${SRC_DIR}/*.cpp"
    "${SRC_DIR}/*.h"
	"${SRC_DIR}/*.glsl"
	"${SRC_DIR}/*.png"
)

# ------------------------------------------------------
# 1. Build the main executable FIRST
# ------------------------------------------------------
add_executable(Vulkan_RubiksCube ${SRC_FILES}) # Executable name is based on project name

# Include the generated file in the project
include_directories(${CMAKE_BINARY_DIR})


# Define EXT_DIR for libraries kept in the external folder
set(EXT_DIR ${CMAKE_SOURCE_DIR}/ext)

# ------------------------------------------------------
# 2. Include and link GLFW (from ext folder)
# ------------------------------------------------------
target_include_directories(Vulkan_RubiksCube PUBLIC ${EXT_DIR}/glfw-3.4.bin.WIN64/include) # From ext

# Choose the correct GLFW library based on compiler (from ext folder)
if (MSVC)
    target_link_libraries(Vulkan_RubiksCube ${EXT_DIR}/glfw-3.4.bin.WIN64/lib-vc2022/glfw3.lib) # From ext
elseif (MINGW)
    target_link_libraries(Vulkan_RubiksCube ${EXT_DIR}/glfw-3.4.bin.WIN64/lib-mingw-w64/libglfw3.a) # From ext
endif()

# ------------------------------------------------------
# 3. Add GLM (assuming it's in Vulkan SDK include or system paths)
# ------------------------------------------------------
# Since find_package doesn't work and it's not in ext, assuming it's in a standard include path or VULKAN_SDK_PATH/glm
# If GLM is part of the Vulkan SDK includes, it will be covered by including VULKAN_SDK_PATH/Include.
# If it's elsewhere, you might need to add another include_directories line here.
# For now, we won't explicitly add a GLM include unless it's separate from the main SDK include.
# If GLM is a header-only library (which it is), you only need to include its path.
# Assuming GLM headers are in VULKAN_SDK_PATH/Include/glm or a system path.
# A common location might be ${VULKAN_SDK_PATH}/glm if it's distributed separately but alongside.
# Let's add a specific check/include for GLM if a path is provided, otherwise assume it's findable.
# Alternatively, if GLM is *always* in a standard system location, no specific include is needed here.
# Given the user stated GLM is in the Vulkan directory, let's add an include relative to VULKAN_SDK_PATH.
if (NOT "${VULKAN_SDK_PATH}" STREQUAL "")
    target_include_directories(Vulkan_RubiksCube PUBLIC "${VULKAN_SDK_PATH}/glm") # Assuming glm is directly under SDK
else()
    message(WARNING "VULKAN_SDK_PATH not set. Assuming GLM is in a default include path.")
    # If GLM is not found here, you might need to add its specific path manually or via another variable.
endif()


# ------------------------------------------------------
# 4. Add ImGuis
# ------------------------------------------------------

set(IMGUI_DIR ${EXT_DIR}/imgui-master)

file(GLOB IMGUI_SOURCES
    "${IMGUI_DIR}/*.h"
    "${IMGUI_DIR}/*.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp"
)

message(STATUS "Current source files in imgui: ${IMGUI_SOURCES}")

add_library(imgui STATIC ${IMGUI_SOURCES})

# Add ImGui include paths (both main and backends)
target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# Vulkan SDK includes
if (NOT "${VULKAN_SDK_PATH}" STREQUAL "")
    target_include_directories(imgui PUBLIC
        "${VULKAN_SDK_PATH}/Include"
        "${VULKAN_SDK_PATH}/Include/vulkan"
    )
endif()

# Add to main target if you use ImGui from main
target_include_directories(Vulkan_RubiksCube PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(Vulkan_RubiksCube imgui)

# ------------------------------------------------------
# 5. Add stbi (keeping in ext folder as requested)
# ------------------------------------------------------
target_include_directories(Vulkan_RubiksCube PUBLIC ${CMAKE_SOURCE_DIR}/ext/stb-master)

# ------------------------------------------------------
#5 Add vk bootstrapper
# ------------------------------------------------------
set(VK_BOOTSTRAP ${EXT_DIR}/vk-bootstrap)

# *** IMPORTANT: Find all .cpp files in the vk_bootstrap directory and add them to source files ***
# Corrected: Use VK_BOOTSTRAP to glob within the vk-bootstrap directory
file(GLOB VK_BOOTSTRAP_SRC_FILES
    "${VK_BOOTSTRAP}/*.cpp"
    "${VK_BOOTSTRAP}/*.h"
)

# Add the found source files directly to the executable target
if(VK_BOOTSTRAP_SRC_FILES) # Check if any files were found
    #target_sources(Vulkan_RubiksCube PUBLIC ${VK_BOOTSTRAP_SRC_FILES})
	add_library(vkBootstrapper STATIC ${VK_BOOTSTRAP_SRC_FILES})

    if (NOT "${VULKAN_SDK_PATH}" STREQUAL "")
    target_include_directories(vkBootstrapper PUBLIC
        "${VULKAN_SDK_PATH}/Include"
        "${VULKAN_SDK_PATH}/Include/vulkan"
    )
endif()
	
	# Add the directory containing vk_bootstrap headers to the include paths
	target_include_directories(Vulkan_RubiksCube PRIVATE ${VK_BOOTSTRAP})
	
	target_link_libraries(Vulkan_RubiksCube vkBootstrapper)
endif()

message(STATUS "Current source files: ${SRC_FILES}")

# ------------------------------------------------------
# 6. Add Vulkan and related libraries (vk_hpp, vma) from Vulkan SDK
# ------------------------------------------------------
if (NOT "${VULKAN_SDK_PATH}" STREQUAL "")
    message(STATUS "Using Vulkan SDK from: ${VULKAN_SDK_PATH}")

    # Add include directories
    target_include_directories(Vulkan_RubiksCube PUBLIC
        "${VULKAN_SDK_PATH}/Include" # Main Vulkan headers
        "${VULKAN_SDK_PATH}/Include/vma" # vma headers (assuming this structure)
        # If vk_hpp is in a specific subdir like Include/vk_hpp, add it here:
        "${VULKAN_SDK_PATH}/Include/vulkan"
    )

    # Add library directories and link libraries
    # Assuming Windows library structure with .lib files
    set(VULKAN_SDK_LIB_DIR "${VULKAN_SDK_PATH}/Lib") # Adjust if your SDK has a different lib folder name

    target_link_libraries(Vulkan_RubiksCube
        # Vulkan main library (for Vulkan-1.dll)
        "${VULKAN_SDK_LIB_DIR}/vulkan-1.lib"
    )

    # Note: vk_hpp is typically a header-only library, so no linking is needed for it.

else()
    message(FATAL_ERROR "VULKAN_SDK_PATH is not set. Please set it in the CMake configuration to point to your Vulkan SDK installation.")
endif()