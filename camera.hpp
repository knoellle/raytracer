#pragma once

#include "ray.hpp"

class Camera
{
    public:
        Camera():
            bottom_left(-2.0, -1.0, -1.0),
            horizontal(2.0, 0.0, 0.0),
            vertical(0.0, 1.0, 0.0),
            origin(0.0, 0.0, 1.0)
        {
            bottom_left = Vec3(0,0,-1) - horizontal / 2 - vertical / 2;
        }

        Ray getRay(const double u, const double v) const;
        Vec3 bottom_left;
        Vec3 horizontal;
        Vec3 vertical;
        Vec3 origin;
};

Ray Camera::getRay(const double u, const double v) const
{
    return Ray(origin, (bottom_left + u * horizontal + v * vertical).normalized());
}