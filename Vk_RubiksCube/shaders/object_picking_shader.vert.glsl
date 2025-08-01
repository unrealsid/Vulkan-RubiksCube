#version 460

#extension GL_EXT_debug_printf : enable
#extension GL_EXT_scalar_block_layout: require
#extension GL_EXT_buffer_reference : require

layout(location = 0) in vec3 inPosition;

layout(location = 0) out uint ID;

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

void main() 
{
    SceneDataBuffer sceneData = push_constants.scene_data_buffer_addr;
    ModelBuffer model_addr = push_constants.model_transform_addr;
    ObjectID_Buffer object_id_addr = push_constants.object_id_addr;

    gl_Position = sceneData.projection * sceneData.view * model_addr.model_transform * vec4(inPosition, 1.0);
    
    ID = object_id_addr.object_id;
}
