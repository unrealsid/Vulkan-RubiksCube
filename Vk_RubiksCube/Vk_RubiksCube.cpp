
#include "Vk_RubiksCube.h"
#include "core/Engine.h"

int main()
{
    core::Engine& engine = core::Engine::get_instance();
    engine.init();
    engine.run();
    engine.cleanup();
    
    return 0;
}
