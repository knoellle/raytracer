#pragma once

#include "vec3.hpp"

class Ray
{
    public:
        Ray() = default;
        Ray(const Vec3& origin, const Vec3& direction) { origin_ = origin; direction_ = direction; }

        Vec3 origin() const { return origin_; }
        Vec3 direction() const { return direction_; }
        Vec3 pointAt(const double t) const { return origin_ + direction_ * t; }

    private:
        Vec3 origin_;
        Vec3 direction_;

};
