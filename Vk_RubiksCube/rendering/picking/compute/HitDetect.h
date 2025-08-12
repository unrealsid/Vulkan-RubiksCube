#pragma once

#include <string>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>

#include "../../../structs/Compute.h"
#include "../../../structs/EngineContext.h"
#include "../../../structs/GPU_Buffer.h"
#include "../../../structs/Ray.h"

namespace core
{
    class Renderer;
}

namespace material
{
    class Material;
}

namespace core
{
    class Engine;
}

namespace vulkan
{
    class DeviceManager;
    class SwapchainManager;
}

namespace rendering::compute
{
    class HitDetect
    {
    public:
        HitDetect(EngineContext& engine_context);

        //Wait for the first submit to get done, then we can wait on the fence
        bool first_submit_done;

        [[nodiscard]] GPU_Buffer get_readback_compute_buffer() const
        {
            return readback_compute_buffer;
        }

        [[nodiscard]] VkCommandBuffer get_command_buffer() const
        {
            return compute.command_buffer;
        }

        [[nodiscard]] VkFence get_fence() const
        {
            return fence;
        }

        //Init compute resources
        void init_compute();
        
        void build_compute_command_buffer() const;

    private:
        EngineContext& engine_context;

        //Manages compute related parameters
        Compute compute;

        vulkan::DeviceManager* device_manager;
        vulkan::SwapchainManager* swapchain_manager;
        core::Renderer* renderer;
        window::WindowManager* window_manager;

        //Stores result of the gpu compute shader
        GPU_Buffer readback_compute_buffer;

        //Number of triangles for the cubies
        uint32_t total_triangle_count = 0;

        //Stores triangles
        GPU_Buffer triangles_buffer;

        //Stores world matrix data
        GPU_Buffer world_transform_buffer;

        //The buffer that tracks the ray we are sending 
        GPU_Buffer ray_buffer;

        //Cache for speeding up certain operations. Driver used mostly. I dont need to worry too much about it
        VkPipelineCache pipeline_cache;

        VkFence fence;

        //Maps ray data to shared memory
        void create_ray_data() const;

        void create_readback_buffer();

        void create_ray_buffer();

        void allocate_command_buffer();

        void create_compute_fence();

        static Ray create_picking_ray(glm::vec2 mouse_screen_pos, glm::vec2 screen_size, 
                                      glm::mat4 view_matrix, glm::mat4 proj_matrix);

        void create_compute_pipeline(const std::string& shader_path);

        //inits computer shader buffers
        void load_objects_in_compute_buffer();
    };
}