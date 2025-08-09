#version 460

#include "include/object_picking_shader_common.glsl"

//Vertex Attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

//Frag shader outputs
layout(location = 0) out uint ID;
layout(location = 1) out vec3 normal;

void main() 
{
    SceneDataBuffer sceneData = push_constants.scene_data_buffer_addr;
    ModelBuffer model_addr = push_constants.model_transform_addr;
    ObjectID_Buffer object_id_addr = push_constants.object_id_addr;

    gl_Position = sceneData.projection * sceneData.view * model_addr.model_transform * vec4(inPosition, 1.0);
    
    ID = object_id_addr.object_id;
    normal = inNormal;
}
