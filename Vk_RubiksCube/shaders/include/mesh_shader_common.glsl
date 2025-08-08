
#extension GL_EXT_debug_printf : enable
#extension GL_EXT_scalar_block_layout: require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_nonuniform_qualifier : require

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