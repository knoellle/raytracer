#pragma once

#include <memory>
#include <utility>

#include "entity.hpp"

class Sphere : public Entity
{
public:
  Sphere(Vec3 center, double radius, std::shared_ptr<Material> material)
    : center(center)
    , radius(radius)
    , material(std::move(material))
  {
    Sphere::update();
  }
  void update() override;
  bool hit(const Ray& r, const double t_min, const double t_max, HitData& data) const override;
  Vec3 center;
  double radius;
  std::shared_ptr<Material> material;
};

void Sphere::update()
{
  boundingBox.low = center - Vec3(radius);
  boundingBox.high = center + Vec3(radius);
  transform = center;
}

bool Sphere::hit(const Ray& r, const double t_min, const double t_max, HitData& data) const
{
  const Vec3 oc = r.origin() - center;
  const double a = dot(r.direction(), r.direction());
  const double b = dot(oc, r.direction());
  const double c = dot(oc, oc) - radius * radius;
  const double discriminator = b * b - a * c;
  if (discriminator <= 0.0f)
  {
    return false;
  }
  const double intersectionLength = sqrt(discriminator) / a;
  const double bdiva = b / a;
  double t = (-bdiva - intersectionLength);
  if (t >= t_max || t <= t_min)
  {
    t = (-bdiva + intersectionLength);
    if (t >= t_max || t <= t_min)
    {
      return false;
    }
  }
  data.t = t;
  data.hit_point = r.pointAt(t);
  data.normal = (data.hit_point - center) / radius;
  data.material = material;
  data.entity = shared_from_this();
  return true;
}
