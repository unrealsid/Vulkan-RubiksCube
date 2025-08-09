#include "PointerEntity.h"

void PointerEntity::update(double delta_time)
{
    DrawableEntity::update(delta_time);
}

void PointerEntity::initialize_transform() const
{
    DrawableEntity::initialize_transform();
    get_transform()->set_scale(glm::vec3(0.1f));
}
