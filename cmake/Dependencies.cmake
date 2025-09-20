include(FetchContent)

#1. Include Vulkan
find_package(Vulkan REQUIRED)

#2. Include VK Bootstrap
find_package(vk-bootstrap QUIET)
if(vk-bootstrap_FOUND)
    message(STATUS "Using vk-bootstrap via find_package")
endif()

if(NOT vk-bootstrap_FOUND)
    FetchContent_Declare(
            vk_bootstrap
            GIT_REPOSITORY "https://github.com/charles-lunarg/vk-bootstrap"
            GIT_TAG        v1.4.321
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
    )
    message(STATUS "Using vk-bootstrap via FetchContent")
    FetchContent_MakeAvailable(vk_bootstrap)
endif()

#3. Include GLFW
find_package(GLFW CONFIG QUIET)
if(GLFW_FOUND)
    message(STATUS "Using GLFW via find_package")
endif()
if(NOT GLFW_FOUND)
    FetchContent_Declare(
            GLFW
            GIT_REPOSITORY "https://github.com/glfw/glfw.git"
            GIT_TAG        "master"
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
    )
    message(STATUS "Using GLFW via FetchContent")
    FetchContent_MakeAvailable(GLFW)
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

#Adding ext folder
# Define EXT_DIR for libraries kept in the external folder
set(EXT_DIR ${CMAKE_SOURCE_DIR}/ext)

# 6. Add ImGui
set(IMG_UI_PROJECT_NAME imgui)
set(IMGUI_DIR ${EXT_DIR}/imgui-master)

file(GLOB IMGUI_SOURCES
    "${IMGUI_DIR}/*.h"
    "${IMGUI_DIR}/*.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp"
)

message(STATUS "Current source files in imgui: ${IMGUI_SOURCES}")

add_library(${IMG_UI_PROJECT_NAME} STATIC ${IMGUI_SOURCES})

# Add ImGui include paths (both main and backends)
target_include_directories(${IMG_UI_PROJECT_NAME} PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)