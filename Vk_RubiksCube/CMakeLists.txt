
find_package(Vulkan REQUIRED COMPONENTS shaderc_combined)
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

target_link_libraries(${ENGINE_PROJECT_NAME} PUBLIC Vulkan::Vulkan
                                                    Vulkan::shaderc_combined
                                                    vk-bootstrap::vk-bootstrap
                                                    SDL3::SDL3
                                                    STB-image::STB-image
                                                    imgui
                                                    tinyobjloader)

# Shaders are compiled to SPIR-V at runtime, so ship the GLSL sources (and their
# include/ headers) next to the executable rather than baking an absolute path in.
add_custom_command(TARGET ${ENGINE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
                "$<TARGET_FILE_DIR:${ENGINE_PROJECT_NAME}>/shaders"
)

# SDL3 is linked as a shared library, so its DLL has to sit next to the
# executable for the build tree to be runnable in place.
add_custom_command(TARGET ${ENGINE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_RUNTIME_DLLS:${ENGINE_PROJECT_NAME}>
                $<TARGET_FILE_DIR:${ENGINE_PROJECT_NAME}>
        COMMAND_EXPAND_LISTS
)
