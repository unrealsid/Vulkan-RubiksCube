#pragma once
#include "../structs/GPU_Buffer.h"

struct EngineContext;

namespace core
{
    //Stores a single buffer that contains all object world matrices
    class TransformManager
    {
    public:
        TransformManager(EngineContext& engine_context);
        
        //Allocates a global transform buffer
        void allocate_transform_buffer();

        [[nodiscard]] GPU_Buffer get_transforms_buffer() const { return transforms_buffer; }

    private:
        EngineContext& engine_context;

        GPU_Buffer transforms_buffer;
    };
}
