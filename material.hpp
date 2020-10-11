#pragma once

#include "entity.hpp"
#include "ray.hpp"

class Material
{
    public:
        virtual bool scatter(const Ray& r, const HitData &data, Vec3 &attenuation, Ray &scattered) const = 0;
};
