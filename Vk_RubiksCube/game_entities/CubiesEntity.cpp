#include "CubiesEntity.h"

void CubiesEntity::initialize_transform() const
{
    DrawableEntity::initialize_transform();
    //transform->set_rotation(glm::vec3(90.0f));
    //transform->set_scale(glm::vec3(0.5f));
}

void CubiesEntity::update(double delta_time)
{
    //std::cout << *transform <<std::endl;
    
    DrawableEntity::update(delta_time);
}
