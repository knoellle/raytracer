#pragma once

#include <memory>

#include "ray.hpp"

class Material;
class Entity;

struct HitData
{
    double t;
    Vec3 hit_point;
    Vec3 normal;
    Vec3 debugColor;
    int debugCounter;
    int debugStep;
    bool debug = false;
    std::shared_ptr<const Material> material;
    std::shared_ptr<const Entity> entity;
};

struct AABB
{
    Vec3 low;
    Vec3 high;
    void expand(const AABB &box);
    bool intersect(const Ray &r) const;
};

class Entity: public std::enable_shared_from_this<Entity>
{
    public:
        virtual void update() = 0;
        virtual bool hit(const Ray &r, const double t_min, const double t_max, HitData &data) const = 0;
        AABB boundingBox;
        Vec3 transform;
        bool emissive = false;
};
