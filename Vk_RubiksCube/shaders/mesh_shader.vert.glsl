#version 460
#extension GL_EXT_debug_printf : enable

// Input vertex attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(std140, set = 0, binding = 0) uniform UniformBufferObject 
{
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

// Output to fragment shader
layout(location = 0) out vec3 outColor;

void main()
{
   gl_Position = ubo.projection * ubo.view * ubo.model * vec4(inPosition, 1.0);

    debugPrintfEXT("Value of inPosition: %v3", inPosition);

    // Simple constant color output
    outColor = vec3(1.0, 1.0, 1.0);
}