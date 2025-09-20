
find_package(Vulkan REQUIRED)
set(ENGINE_PROJECT_NAME "Vulkan_RubiksCube")

# Automatically get all .cpp and .h files from the src directory
#TODO: Change to pass individual file names
file(GLOB_RECURSE SRC_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.glsl"
)

add_executable(${ENGINE_PROJECT_NAME} ${SRC_FILES})

target_include_directories(
        ${ENGINE_PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

if(NOT TARGET glfw)
  message(FATAL_ERROR "Och nuu, GLFW is not a target")

    get_property(allTargets DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)

    message(STATUS "Targets in this directory: ${allTargets}")
endif()

target_link_libraries(${ENGINE_PROJECT_NAME} PUBLIC Vulkan::Vulkan
                                                    vk-bootstrap::vk-bootstrap
                                                    glfw
                                                    STB-image::STB-image
                                                    imgui
                                                    tinyobjloader)
