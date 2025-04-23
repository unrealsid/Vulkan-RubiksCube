#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

// Add uniform for transformation matrices (Model, View, Projection)
// layout(binding = 0) uniform UniformBufferObject{
//     mat4 model;
//     mat4 view;
//     mat4 proj;
// } ubo;

layout(location = 0) out vec3 outColor; 

void main() 
{
    gl_Position = vec4(inPosition, 1.0); 
    outColor = vec3(1.0, 1.0, 1.0);
}