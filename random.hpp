#pragma once

#include <random>

#include "vec3.hpp"

std::random_device random_device;
std::uniform_real_distribution<double> distribution(0.0f, 1.0f);
std::mt19937 random_generator(random_device());

double random_unit()
{
    return distribution(random_generator);
}

Vec3 random_unit_cube()
{
    return Vec3(distribution(random_generator), distribution(random_generator), distribution(random_generator));
}

Vec3 random_unit_sphere()
{
    Vec3 p;
    do {
        p = 2.0*Vec3(distribution(random_generator), distribution(random_generator), distribution(random_generator)) - Vec3(1,1,1);
    } while (p.squared_length() >= 1.0f);
    return p;
}