#pragma once

#include "ray.hpp"
#include "entity.hpp"

class Camera: public Entity
{
    public:
        Camera(const Vec3 &position, const Vec3 &look_at, const double vFOV, const double aspect_ratio):
            // bottom_left(-2.0, -1.0, -1.0),
            // horizontal(2.0, 0.0, 0.0),
            // vertical(0.0, 1.0, 0.0),
            look_at(look_at),
            up(Vec3(0,1,0)),
            vFOV(vFOV),
            aspect_ratio(aspect_ratio)
        {
            transform = position;
            Camera::update();
        }

        Ray getRay(const double u, const double v) const;

        virtual void update() override;
        virtual bool hit(const Ray &r, const double t_min, const double t_max, HitData &data) const override;

        Vec3 bottom_left;
        Vec3 horizontal;
        Vec3 vertical;
        Vec3 look_at;
        Vec3 up;
        double vFOV;
        double aspect_ratio;
};

Ray Camera::getRay(const double u, const double v) const
{
    return Ray(transform, (bottom_left + u * horizontal + v * vertical).normalized());
}

void Camera::update()
{
    double height = tan(vFOV * M_PI / 180 / 2) * 2;
    double width = height * aspect_ratio;
    Vec3 direction = (look_at - transform).normalized();
    horizontal = (direction.cross(up)).normalized();
    vertical = horizontal.cross(direction);

    horizontal = horizontal * width;
    vertical = vertical * height;

    bottom_left = direction - (vertical + horizontal) / 2.0f;
}

bool Camera::hit(const Ray &r, const double t_min, const double t_max, HitData &data) const
{
    return false;
}