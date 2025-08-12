#extension GL_EXT_debug_printf : enable
#extension GL_EXT_scalar_block_layout: require
#extension GL_EXT_buffer_reference : require
#extension GL_ARB_gpu_shader_int64 : require

layout(buffer_reference, scalar) buffer SceneDataBuffer
{
    mat4 view;
    mat4 projection;
};

//Info for a single triangle
struct TriangleInfo
{
    vec4 vertex1;
    vec4 vertex2;
    vec4 vertex3;
    vec4 normal;

    uint64_t world_transform_id;
    uint64_t object_id;
};

struct Ray
{
    vec3 origin;
    float padding0;
    
    vec3 direction;
    float padding1;
    
    float t_min;
    vec3 padding3;
    
    float t_max;
    vec3 padding4;
};

//Info about triangles -> local space data
layout (buffer_reference, std430, buffer_reference_align = 16) readonly buffer MeshTriangleBuffer
{
    TriangleInfo mesh_triangle_data[];
};

//World space transform
layout (buffer_reference, std430, buffer_reference_align = 16) readonly buffer WorldTransformBuffer
{
    mat4 world_transform[];
};

layout(buffer_reference, scalar) buffer OutputInfoBuffer
{
    //First 3 elements represent the face normal and last one the encoded object ID
    vec4 output_info;
};

layout(buffer_reference, scalar) buffer RayDataBuffer
{
    Ray ray_data;
};

layout(push_constant) uniform PushConstants
{
    SceneDataBuffer scene_data_buffer_addr;
    WorldTransformBuffer world_transform_addr;
    MeshTriangleBuffer mesh_triangle_data_addr;
    RayDataBuffer ray_data_addr;
    OutputInfoBuffer output_info_addr;
} push_constants;