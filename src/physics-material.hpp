#pragma once

#include "random.hpp"
#include "material.hpp"

class PhysicsMaterial: public Material
{
    public:
        PhysicsMaterial(const Vec3 &diffuse, const Vec3 &reflective, const double roughness):
            ambient(diffuse * 0.1f),
            diffuse(diffuse),
            reflective(reflective),
            emissive(0.0f),
            roughness(roughness)
        {}
        virtual bool scatter(const Ray& r, const HitData &data, Vec3 &attenuation, Ray &scattered) const override;
        // virtual bool refract(const Ray& r, const HitData &data) const override;

        Vec3 ambient;
        Vec3 diffuse;
        Vec3 reflective;
        Vec3 emissive;
        double roughness;
};

bool PhysicsMaterial::scatter(const Ray& r, const HitData &data, Vec3 &attenuation, Ray &scattered) const
{
    Vec3 target_direction = reflect(r.direction().normalized(), data.normal + random_unit_sphere() * roughness);
    scattered = Ray(data.hit_point, target_direction);
    attenuation = reflective;
    return true;
}
