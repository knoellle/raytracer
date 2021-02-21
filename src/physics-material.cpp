#include "physics-material.hpp"

bool PhysicsMaterial::scatter(const Ray& r, const HitData &data, Vec3 &attenuation, Ray &scattered) const
{
    Vec3 targetDirection = (data.normal + random_unit_sphere() * roughness).reflect(r.direction().normalized());
    scattered = Ray(data.hit_point, targetDirection);
    attenuation = reflective;
    return true;
}