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

void AABB::expand(const AABB &box)
{
    for (int i = 0; i<3; i++)
    {
        low[i] = std::min(low[i], box.low[i]);
        high[i] = std::max(high[i], box.high[i]);
    }
}

bool AABB::intersect(const Ray &r) const
{
    double tmin, tmax, tymin, tymax, tzmin, tzmax;
    int r_sign[3];
    Vec3 r_invdir = 1.0f / r.direction();

    for (int i = 0; i < 3; ++i)
    {
        r_sign[i] = r_invdir[i] < 0 ? 1 : 0;
    }

    Vec3 bounds[2] = {low,high};

    tmin = (bounds[r_sign[0]].x() - r.origin().x()) * r_invdir.x();
    tmax = (bounds[1 - r_sign[0]].x() - r.origin().x()) * r_invdir.x();
    tymin = (bounds[r_sign[1]].y() - r.origin().y()) * r_invdir.y();
    tymax = (bounds[1 - r_sign[1]].y() - r.origin().y()) * r_invdir.y();

    if ((tmin > tymax) || (tymin > tmax))
        return false;

    if (tymin > tmin)
        tmin = tymin;
    if (tymax < tmax)
        tmax = tymax;

    tzmin = (bounds[r_sign[2]].z() - r.origin().z()) * r_invdir.z();
    tzmax = (bounds[1 - r_sign[2]].z() - r.origin().z()) * r_invdir.z();

    if ((tmin > tzmax) || (tzmin > tmax))
        return false;

    if (tzmin > tmin)
        tmin = tzmin;
    if (tzmax < tmax)
        tmax = tzmax;

    double t = tmin;

    if (t < 0)
    {
        t = tmax;
        if (t < 0)
            return false;
    }
    return true;
    // const Vec3 inverse_direction = 1.0f / r.direction();
    // double t0 = (low[0] - r.origin()[0] * inverse_direction[0]);
    // double t1 = (high[0] - r.origin()[0] * inverse_direction[0]);

    // double tmin = std::min(t0, t1);
    // double tmax = std::max(t0, t1);

    // for (int i = 1; i < 3; ++i)
    // {
    //     t0 = (low[i] - r.origin()[i]) * inverse_direction[i];
    //     t1 = (high[i] - r.origin()[i]) * inverse_direction[i];

    //     tmin = std::max(tmin, std::min(t0, t1));
    //     tmax = std::min(tmax, std::max(t0, t1));
    // }

    // return tmax > std::max(tmin, 0.0);
}