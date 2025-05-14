#version 460
#extension GL_EXT_debug_printf : enable
#extension GL_EXT_scalar_block_layout: require
#extension GL_EXT_buffer_reference : require

// Input vertex attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(buffer_reference, scalar) buffer UniformBufferObject 
{
    mat4 model;
    mat4 view;
    mat4 projection;
};

layout (push_constant) uniform PushConstants
{
    UniformBufferObject sceneDataReference;
} pushConstants;

// Output to fragment shader
layout(location = 0) out vec3 outColor;

void main()
{
    UniformBufferObject sceneData = pushConstants.sceneDataReference;
    
    gl_Position = sceneData.projection * sceneData.view * sceneData.model * vec4(inPosition, 1.0);

    debugPrintfEXT("Value of inPosition: %v3", inPosition);

    // Simple constant color output
    outColor = vec3(1.0, 1.0, 1.0);
}