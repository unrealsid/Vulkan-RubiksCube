
#extension GL_EXT_debug_printf : enable
#extension GL_EXT_scalar_block_layout: require
#extension GL_EXT_buffer_reference : require

layout(buffer_reference, scalar) buffer SceneDataBuffer
{
    mat4 view;
    mat4 projection;
};

layout(buffer_reference, scalar) buffer ModelBuffer
{
    mat4 model_transform;       
};

layout(buffer_reference, scalar) buffer ObjectID_Buffer
{
    uint object_id;
};

layout(buffer_reference, scalar) buffer FaceNormalBuffer
{
    vec4 face_normal;
};

layout(push_constant) uniform PushConstants
{
    SceneDataBuffer scene_data_buffer_addr;
    ModelBuffer model_transform_addr;
    ObjectID_Buffer object_id_addr;
    FaceNormalBuffer face_normal_addr;
} push_constants;