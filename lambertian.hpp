#pragma once

#include "ray.hpp"
#include "material.hpp"
#include "random.hpp"

class Lambertian: public Material
{
    public:
        Lambertian(const Vec3 &albedo) :
            albedo(albedo)
        {}
        virtual bool scatter(const Ray& r, const HitData &data, Vec3 &attenuation, Ray &scattered) const;

        Vec3 albedo;
};

bool Lambertian::scatter(const Ray& r, const HitData &data, Vec3 &attenuation, Ray &scattered) const
{
    Vec3 target = data.hit_point + data.normal + random_unit_sphere();
    scattered = Ray(data.hit_point, target - data.hit_point);
    attenuation = albedo;
    return true;
}
