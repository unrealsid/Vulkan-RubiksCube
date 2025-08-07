#version 460
#extension GL_EXT_debug_printf : enable
#extension GL_EXT_scalar_block_layout: require
#extension GL_EXT_buffer_reference : require

// Input vertex attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in uint inMaterialIndex;
layout(location = 4) in uint inTextureIndex;

layout(buffer_reference, scalar) buffer SceneDataBuffer 
{
    mat4 view;
    mat4 projection;
};

layout(buffer_reference, scalar) buffer ModelBuffer
{
    mat4 model_transform;
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
    SceneDataBuffer sceneDataReference;
    MaterialBuffer materialsDataReference;
    ModelBuffer model_transform_addr;
} pushConstants;

// Output to fragment shader
layout(location = 0) out vec3 outColor;
layout(location = 1) out vec2 outUV;
layout(location = 2) flat out uint outMaterialIndex;
layout(location = 3) flat out uint outTexIndex;

void main()
{
    SceneDataBuffer sceneData = pushConstants.sceneDataReference;
    ModelBuffer model_addr = pushConstants.model_transform_addr;
    
    gl_Position = sceneData.projection * sceneData.view * model_addr.model_transform * vec4(inPosition, 1.0);

    debugPrintfEXT("Value of inPosition: %v3", inPosition);

    // Simple constant color output
    outColor = vec3(1.0, 1.0, 1.0);
    
    outUV = inTexCoord;
    outMaterialIndex = inMaterialIndex;
    outTexIndex = inTextureIndex;
}