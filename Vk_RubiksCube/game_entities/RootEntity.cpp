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
    transform->set_position(glm::vec3(0.0, 0.0, -5.0f));
    //transform->set_rotation(glm::vec3(90.0f));
    transform->set_scale(glm::vec3(0.5f));
}
