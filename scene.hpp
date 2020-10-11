#pragma once

#include <memory>
#include <vector>
#include "entity.hpp"

class Scene: public Entity
{
    public:
        Scene() {};
        virtual bool hit(const Ray& r, const double t_min, const double t_max, HitData &data) const;
        virtual void update();
        std::vector<std::shared_ptr<Entity>> entities;
};

bool Scene::hit(const Ray& r, const double t_min, const double t_max, HitData &data) const
{
    HitData temp_data;
    double closest_hit = t_max + 1;
    for (const auto &e : entities)
    {
        if (e->hit(r, t_min, closest_hit, temp_data))
        {
            closest_hit = temp_data.t;
            data = temp_data;
        }
    }
    return closest_hit < t_max;
}

void Scene::update()
{
    for (auto &e : entities)
    {
        e->update();
    }
}