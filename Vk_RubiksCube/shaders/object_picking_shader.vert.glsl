#version 460

#include "include/object_picking_shader_common.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 0) out uint ID;

void main() 
{
    SceneDataBuffer sceneData = push_constants.scene_data_buffer_addr;
    ModelBuffer model_addr = push_constants.model_transform_addr;
    ObjectID_Buffer object_id_addr = push_constants.object_id_addr;

    gl_Position = sceneData.projection * sceneData.view * model_addr.model_transform * vec4(inPosition, 1.0);
    
    ID = object_id_addr.object_id;
}
