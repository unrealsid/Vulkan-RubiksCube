#include "Vk_RubiksCube.h"
#include <SDL3/SDL_main.h>
#include "core/Engine.h"

int main(int argc, char* argv[])
{
    core::Engine& engine = core::Engine::get_instance();
    engine.init();
    engine.run();
    engine.cleanup();

    return 0;
}
