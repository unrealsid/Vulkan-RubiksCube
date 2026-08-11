include(FetchContent)

#1. Include Vulkan
# shaderc_combined provides runtime GLSL -> SPIR-V compilation
find_package(Vulkan REQUIRED COMPONENTS shaderc_combined)

#2. Include VK Bootstrap
find_package(vk-bootstrap QUIET)
if(vk-bootstrap_FOUND)
    message(STATUS "Using vk-bootstrap via find_package")
endif()

if(NOT vk-bootstrap_FOUND)
    FetchContent_Declare(
            vk_bootstrap
            GIT_REPOSITORY "https://github.com/charles-lunarg/vk-bootstrap"
            # Must track the installed Vulkan SDK's VK_HEADER_VERSION: the generated
            # dispatch table references entry points that only exist in matching headers.
            GIT_TAG        v1.4.341
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
    )
    message(STATUS "Using vk-bootstrap via FetchContent")
    FetchContent_MakeAvailable(vk_bootstrap)
endif()

#3. Include SDL3
# The Vulkan SDK ships a prebuilt SDL3 (headers, import library, DLL and a CMake
# package config), so look there first before falling back to a source build.
find_package(SDL3 CONFIG QUIET
        HINTS
            "$ENV{VULKAN_SDK}"
            "$ENV{VK_SDK_PATH}"
)
if(SDL3_FOUND)
    message(STATUS "Using SDL3 ${SDL3_VERSION} via find_package (${SDL3_DIR})")
endif()
if(NOT SDL3_FOUND)
    set(SDL_SHARED ON CACHE BOOL "" FORCE)
    set(SDL_STATIC OFF CACHE BOOL "" FORCE)
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
    set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
            SDL3
            GIT_REPOSITORY "https://github.com/libsdl-org/SDL.git"
            GIT_TAG        "release-3.4.14"
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
    )
    message(STATUS "Using SDL3 via FetchContent")
    FetchContent_MakeAvailable(SDL3)
endif()

#4. include STBI Image Library
#Using a custom version of stb since CMake is not defined for it in the official repo
find_package(STB QUIET)
if(STB_FOUND)
    message(STATUS "Using STB via find_package")
endif()
if(NOT STB_FOUND)
    FetchContent_Declare(
            STB
            GIT_REPOSITORY "https://github.com/unrealsid/stb-cmake.git"
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
    )
    message(STATUS "Using STB via FetchContent")
    FetchContent_MakeAvailable(STB)
endif()

#5. Include Tiny Obj Loader
find_package(Obj_Loader QUIET)
if(Obj_Loader_FOUND)
    message(STATUS "Using TinyOBJ Loader via find_package")
endif()
if(NOT Obj_Loader_FOUND)
    FetchContent_Declare(
            Obj_Loader
            GIT_REPOSITORY "https://github.com/tinyobjloader/tinyobjloader.git"
            GIT_TAG     "release"
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
    )
    message(STATUS "Using Tiny Obj Loader via FetchContent")
    FetchContent_MakeAvailable(Obj_Loader)
endif()

# MSVC 19.51 rejects the constexpr SIMD paths in the fast_float copy vendored into
# tinyobjloader's release branch. Fall back to its strtod-based parser until upstream
# picks up a newer fast_float.
if(MSVC AND TARGET tinyobjloader)
    target_compile_definitions(tinyobjloader PUBLIC TINYOBJLOADER_DISABLE_FAST_FLOAT)
endif()

#Adding ext folder
# Define EXT_DIR for libraries kept in the external folder
set(EXT_DIR ${CMAKE_SOURCE_DIR}/ext)

# 6. Add ImGui
set(IMGUI_PROJECT_NAME imgui)
set(IMGUI_DIR ${EXT_DIR}/imgui-master)

file(GLOB IMGUI_SOURCES
    "${IMGUI_DIR}/*.h"
    "${IMGUI_DIR}/*.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp"
)

message(STATUS "Current source files in imgui: ${IMGUI_SOURCES}")

add_library(${IMGUI_PROJECT_NAME} STATIC ${IMGUI_SOURCES})

# Add ImGui include paths (both main and backends)
target_include_directories(${IMGUI_PROJECT_NAME} PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(${IMGUI_PROJECT_NAME} PUBLIC Vulkan::Vulkan)

