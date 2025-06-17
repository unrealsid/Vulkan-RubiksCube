//Fragment Shader
#version 460

#extension GL_EXT_debug_printf : enable
#extension GL_EXT_scalar_block_layout: require
#extension GL_EXT_buffer_reference : require

layout(location = 0) flat in uint ID;

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

layout(push_constant) uniform PushConstants
{
    SceneDataBuffer scene_data_buffer_addr;
    ModelBuffer model_transform_addr;
    ObjectID_Buffer object_id_addr;
} push_constants;

layout(location = 0) out float outColor;

void main() 
{
    ObjectID_Buffer object_id_buffer = push_constants.object_id_addr;
    object_id_buffer.object_id = ID;

    //only needed for debugging to draw to color attachment
    outColor = object_id_buffer.object_id ;
}