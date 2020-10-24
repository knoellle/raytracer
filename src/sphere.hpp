#pragma once

#include <memory>
#include <utility>

#include "entity.hpp"

class Sphere : public Entity
{
  public:
    Sphere(Vec3 center, double radius, std::shared_ptr<Material> material)
        : center(center)
        , radius(radius)
        , material(std::move(material))
    {
        Sphere::update();
    }
    void update() override;
    bool hit(const Ray& r, const double t_min, const double t_max, HitData& data) const override;
    Vec3 center;
    double radius;
    std::shared_ptr<Material> material;
};
