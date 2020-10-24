#pragma once

#include <memory>
#include <utility>

#include "entity.hpp"

class Box : public Entity
{
  public:
    Box(Vec3 center, Vec3 dimensions, std::shared_ptr<Material> material)
        : center(center)
        , dimensions(dimensions)
        , material(std::move(material))
    {
        Box::update();
    }
    void update() override;
    bool hit(const Ray& r, const double t_min, const double t_max,
                     HitData& data) const override;
    Vec3 center;
    Vec3 dimensions;
    std::shared_ptr<Material> material;
};
