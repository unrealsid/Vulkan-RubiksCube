#version 460

#extension GL_EXT_scalar_block_layout: require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_nonuniform_qualifier : require

//Fragment shader

layout(location = 0) in vec3 inColor; 
layout(location = 1) in vec2 inUV;
layout(location = 2) flat in uint inMaterialIndex;
layout(location = 3) flat in uint inTexIndex;

layout(location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform sampler2D textures[];

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

    vec4 texColor = texture(textures[nonuniformEXT(inTexIndex)], inUV);
    vec4 finalColor = texColor * diffuseColor;

    //finalColor.rgb += emissiveColor.rgb;

    // Set alpha from material if needed, or you can use the texture's alpha
    finalColor.a = 1.0;

    outColor = finalColor;
}