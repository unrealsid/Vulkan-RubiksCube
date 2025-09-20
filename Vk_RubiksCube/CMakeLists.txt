
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

target_link_libraries(${ENGINE_PROJECT_NAME} PRIVATE Vulkan::Vulkan
                                                    vk-bootstrap::vk-bootstrap
                                                    GLFW
                                                    STB-image::STB-image
                                                    imgui
                                                    tinyobjloader)
