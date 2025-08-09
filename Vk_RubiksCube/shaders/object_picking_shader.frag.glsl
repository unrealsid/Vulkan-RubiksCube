#version 460

#include "include/object_picking_shader_common.glsl"

layout(early_fragment_tests) in;

layout(location = 0) flat in uint ID;
layout(location = 1) flat in vec3 normal;

layout(location = 0) out vec4 outColor;

float encode_id(uint object_id) 
{
    return (float(object_id) + 0.5) / 56.0;
}

void main() 
{
    ObjectID_Buffer object_id_buffer = push_constants.object_id_addr;
    uint id = object_id_buffer.object_id;

    FaceNormalBuffer face_normal_buffer = push_constants.face_normal_addr;
    face_normal_buffer.face_normal = vec4(normal, 0.0);

    float encoded_color = encode_id(id);
    float depth = gl_FragCoord.z;

    outColor = vec4(encoded_color, 0.0, depth, 1.0);
}