#include "RootEntity.h"

float i = 0.0;

void RootEntity::update(double delta_time)
{
    //Update data on cpu and write to shared cpu/gpu memory
    i += 0.05f * delta_time;
    //get_transform()->set_rotation(glm::vec3(0.0f, 90.0 * i, 0.0f));

    DrawableEntity::update(delta_time);
}

void RootEntity::initialize_transform() const
{
    DrawableEntity::initialize_transform();
}
