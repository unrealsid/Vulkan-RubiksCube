//Fragment Shader
#version 460

#extension GL_EXT_debug_printf : enable
#extension GL_EXT_scalar_block_layout: require
#extension GL_EXT_buffer_reference : require

layout(early_fragment_tests) in;
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

float encode_id(uint object_id) 
{
    return (float(object_id) + 0.5) / 56.0;
}

layout(location = 0) out vec4 outColor;

void main() 
{
    ObjectID_Buffer object_id_buffer = push_constants.object_id_addr;
    uint id = object_id_buffer.object_id;

    float encoded_color = encode_id(id);

    outColor = vec4(encoded_color, 0.0, 0.0, 1.0); 
}