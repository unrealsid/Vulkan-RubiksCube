#version 460

#extension GL_EXT_scalar_block_layout: require
#extension GL_EXT_buffer_reference : require

layout(location = 0) in vec3 inColor; 
layout(location = 0) out vec4 outColor;
layout(location = 1) flat in uint inMaterialIndex;

layout(buffer_reference, scalar) buffer UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 projection;
};

// Material buffer containing all material parameters
struct Material
{
    vec4 diffuse;
    vec4 specular;
    vec4 shininess;
    vec4 emissive;

    vec4 alpha;
};

layout(buffer_reference, std430, buffer_reference_align = 16) readonly buffer MaterialBuffer
{
    Material materialParams[];
};

layout (push_constant) uniform PushConstants
{
    UniformBufferObject sceneDataReference;
    MaterialBuffer materialsDataReference;
} pushConstants;

void main() 
{
    Material materialRef = pushConstants.materialsDataReference.materialParams[inMaterialIndex];

    // Now access the material properties
    vec4 diffuseColor = materialRef.diffuse;
    vec4 emissiveColor = materialRef.emissive;
    float shininess = materialRef.shininess.x;
    float alpha = materialRef.alpha.x;
    
    outColor = vec4(diffuseColor.rgb, 1.0);
}