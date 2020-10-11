#pragma once

#include "ray.hpp"
#include "material.hpp"
#include "random.hpp"

class Metal: public Material
{
    public:
        Metal(const Vec3 &albedo) :
            albedo(albedo)
        {}
        virtual bool scatter(const Ray& r, const HitData &data, Vec3 &attenuation, Ray &scattered) const;

        Vec3 albedo;
};

bool Metal::scatter(const Ray& r, const HitData &data, Vec3 &attenuation, Ray &scattered) const
{
    Vec3 target_direction = reflect(r.direction().normalized(), data.normal);
    scattered = Ray(data.hit_point, target_direction);
    attenuation = albedo;
    return true;
}