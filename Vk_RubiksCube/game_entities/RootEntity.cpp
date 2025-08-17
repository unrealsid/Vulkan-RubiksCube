#include "RootEntity.h"

void RootEntity::update(double delta_time)
{
   DrawableEntity::update(delta_time);
}

void RootEntity::initialize_transform() const
{
    DrawableEntity::initialize_transform();
}
