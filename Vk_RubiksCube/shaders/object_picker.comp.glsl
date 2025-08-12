#version 460

#include "/include/hit_detection_common.glsl"

layout (local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

bool ray_triangle_intersect(Ray ray, vec3 v0, vec3 v1, vec3 v2, out float t, out vec3 normal) 
{
    const float EPSILON = 1e-8;

    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;
    normal = normalize(cross(edge1, edge2));

    vec3 h = cross(ray.direction, edge2);
    float a = dot(edge1, h);

    if (a > -EPSILON && a < EPSILON) 
    {
        return false;
    }

    float f = 1.0 / a;
    vec3 s = ray.origin - v0;
    float u = f * dot(s, h);

    if (u < 0.0 || u > 1.0) 
    {
        return false;
    }

    vec3 q = cross(s, edge1);
    float v = f * dot(ray.direction, q);

    if (v < 0.0 || u + v > 1.0) 
    {
        return false;
    }

    t = f * dot(edge2, q);

    return (t > ray.t_min && t < ray.t_max);
}

void main() 
{
    uint thread_idx = gl_GlobalInvocationID.x;
    
    //We have exactly 1260 triangles atm
    if(thread_idx > 1260) 
        return;
    
    //Scene Data
    SceneDataBuffer scene_data_addr = push_constants.scene_data_buffer_addr;
    
    //Triangle Data
    MeshTriangleBuffer triangle_data_addr = push_constants.mesh_triangle_data_addr;
    TriangleInfo triangle_info = triangle_data_addr.mesh_triangle_data[thread_idx];

    //World Transform Data
    WorldTransformBuffer world_transform_addr = push_constants.world_transform_addr;
    mat4 world_matrix = world_transform_addr.world_transform[uint(triangle_info.world_transform_id)];

    // Transform triangle vertices to world space
    vec3 world_v0 = (world_matrix * vec4(triangle_info.vertex1.xyz, 1.0)).xyz;
    vec3 world_v1 = (world_matrix * vec4(triangle_info.vertex2.xyz, 1.0)).xyz;
    vec3 world_v2 = (world_matrix * vec4(triangle_info.vertex3.xyz, 1.0)).xyz;

    RayDataBuffer ray_data_addr = push_constants.ray_data_addr;
    Ray pick_ray = ray_data_addr.ray_data;

    float t;
    vec3 normal;
    bool hit = ray_triangle_intersect(pick_ray, world_v0, world_v1, world_v2, t, normal);

    if (hit) 
    {
       vec3 hit_point = pick_ray.origin + pick_ray.direction * t;

        OutputInfoBuffer output_info_addr = push_constants.output_info_addr;
        output_info_addr.output_info = vec4(vec3(triangle_info.normal), float(triangle_info.object_id));

        // Alternative: store triangle ID and distance
        // output_info_addr.output_info[thread_idx] = vec4(float(thread_idx), t, 0.0, 1.0);
    }
}
