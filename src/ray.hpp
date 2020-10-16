#pragma once

#include "vec3.hpp"

class Ray
{
    public:
        Ray() {}
        Ray(const Vec3& origin, const Vec3& direction) { _origin = origin; _direction = direction; }

        Vec3 origin() const { return _origin; }
        Vec3 direction() const { return _direction; }
        Vec3 point_at(const double t) const { return _origin + _direction * t; }

    private:
        Vec3 _origin;
        Vec3 _direction;

};
