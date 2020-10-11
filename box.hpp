#pragma once

#include <memory>

#include "entity.hpp"

class Box : public Entity
{
public:
    Box(Vec3 center, Vec3 dimensions, std::shared_ptr<Material> material) : center(center),
                                                                            dimensions(dimensions),
                                                                            material(material)
    {
        Box::update();
    }
    virtual void update() override;
    virtual bool hit(const Ray &r, const double t_min, const double t_max, HitData &data) const override;
    Vec3 center;
    Vec3 dimensions;
    std::shared_ptr<Material> material;
};

void Box::update()
{
    boundingBox.low = center - dimensions / 2.0;
    boundingBox.high = center + dimensions / 2.0;
    transform = center;
}

bool Box::hit(const Ray &r, const double t_min, const double t_max, HitData &data) const
{
    double tmin, tmax, tymin, tymax, tzmin, tzmax;
    int r_sign[3];
    Vec3 r_invdir = 1.0f / r.direction();

    for (int i = 0; i < 3; ++i)
    {
        r_sign[i] = r_invdir[i] < 0 ? 1 : 0;
    }

    Vec3 bounds[2] = {boundingBox.low, boundingBox.high};

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
    if (t > t_max || t < t_min)
        return false;
    
    data.t = t;
    data.hit_point = r.point_at(t);
    const Vec3 direction = data.hit_point - transform;
    const Vec3 normal = (abs(direction[0]) > abs(direction[1]) && abs(direction[0]) > abs(direction[2])) ?
        Vec3(direction[0],0,0) : (abs(direction[1]) > abs(direction[2]) ? Vec3(0,direction[1],0) : Vec3(0,0,direction[2]));
    // std::cout << direction << " " << normal << std::endl;
    data.normal = normal.normalized();
    data.material = material;
    return true;
}
