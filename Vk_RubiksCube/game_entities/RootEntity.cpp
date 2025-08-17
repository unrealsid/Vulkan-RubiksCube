#include "RootEntity.h"

float i = 0.0;

void RootEntity::update(double delta_time)
{
    //Update data on cpu and write to shared cpu/gpu memory
    i += 0.05f * delta_time;
    
    DrawableEntity::update(delta_time);
}

void RootEntity::initialize_transform() const
{
    DrawableEntity::initialize_transform();
}
