#include <algorithm>
#include <chrono>
#include <execution>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <iomanip>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "vec3.hpp"
#include "ray.hpp"
#include "sphere.hpp"
#include "scene.hpp"
#include "camera.hpp"
#include "random.hpp"
#include "material.hpp"
#include "metal.hpp"
#include "lambertian.hpp"
#include "kdtree-scene.hpp"
#include "box.hpp"

Vec3 color(const Ray &r, const KDTreeScene &scene, const int depth, const int step)
{
    HitData data;
    data.debugCounter = 0;
    data.debugStep = step;
    if (scene.hit(r, 0.001, 1000, data))
    {
        // return data.normal;
        // return Vec3(1-data.t/5);
        Ray scattered;
        Vec3 attenuation;
        if (data.debug)
        {
            return data.debugColor * 1.0f / data.debugCounter;
        }
        if (data.material->scatter(r, data, attenuation, scattered))
        {
            if (depth < 50) 
            {
                return attenuation * color(scattered, scene, depth + 1, step);
            }
            else
            {
                return attenuation;
            }
        }
        else
        {
            return Vec3(0, 0, 0);
        }
    }
    if (data.debug)
    {
        return data.debugColor * 1.0f / data.debugCounter;
    }
    Vec3 unit_direction = r.direction().normalized();
    double t = 0.5 * (unit_direction.y() + 1.0f);
    return (1.0f - t) * Vec3(1.0, 1.0, 1.0) + t * Vec3(0.5, 0.7, 1.0);
}

Vec3 gamma_correct(const Vec3 color, const double gamma)
{
    return Vec3(pow(color[0], 1 / gamma),
                pow(color[1], 1 / gamma),
                pow(color[2], 1 / gamma));
}

void spawn_sphere(Scene &scene, const Vec3 &position, const float radius, const std::shared_ptr<Material> &material)
{
    std::shared_ptr<Sphere> sphere = std::make_shared<Sphere>(position, radius, material);
    scene.entities.emplace_back(std::move(sphere));
}

void spawn_box(Scene &scene, const Vec3 &position, const Vec3 &dimensions, const std::shared_ptr<Material> &material)
{
    std::shared_ptr<Box> box = std::make_shared<Box>(position, dimensions, material);
    scene.entities.emplace_back(std::move(box));
}

KDTreeScene make_test_scene()
{
    std::shared_ptr<Metal> steel = std::make_shared<Metal>(Vec3(0.8, 0.83, 0.8));
    std::shared_ptr<Metal> iron = std::make_shared<Metal>(Vec3(0.1, 0.1, 0.8));
    std::shared_ptr<Lambertian> felt = std::make_shared<Lambertian>(Vec3(0.8, 0.83, 0.8));
    std::shared_ptr<Lambertian> red_felt = std::make_shared<Lambertian>(Vec3(0.8, 0.2, 0.2));

    KDTreeScene scene;

    spawn_sphere(scene, Vec3(0, -100.5, 0), 100, felt);

    spawn_sphere(scene, Vec3(0.5, 0.5, 0), 0.25, iron);

    spawn_sphere(scene, Vec3(-1, 0, 0), 0.5, steel);
    spawn_sphere(scene, Vec3(0, 0, 0), 0.25, red_felt);
    spawn_sphere(scene, Vec3(1, 0, 0), 0.5, steel);
    for (int i = 0; i < 25; ++i)
    {
        Vec3 p = Vec3(i / 5, i % 5, 0) / 5 - Vec3(0.4);
        p[2] = 0.25;
        spawn_sphere(scene, p, 0.09, steel);
    }
    spawn_box(scene, Vec3(0, 0, -1), Vec3(1, 1, 1), iron);
    spawn_box(scene, Vec3(0, 1, -3), Vec3(1, 1, 1), red_felt);
    spawn_box(scene, Vec3(0, 2, -1), Vec3(1, 1, 1), iron);

    // spawn_sphere(scene, Vec3(-0.5, 0, 0), 0.5, steel);
    // spawn_sphere(scene, Vec3(0, 0, 0), 0.25, red_felt);
    // spawn_sphere(scene, Vec3(0.5, 0, 0), 0.5, steel);

    scene.build_tree();

    return scene;
}

inline Vec3 lerp(double f, Vec3 v1, Vec3 v2)
{
    return (1.0f-f) * v1 + f * v2;
}

int main()
{
    const int metasteps = 2;
    const int substeps = 10;
    const int steps = metasteps * substeps;
    const int samples = 1;
    const double resolution_factor = 1;
    const int width = 1920 * resolution_factor;
    const int height = 1080 * resolution_factor;

    auto s = make_test_scene();
    // Sphere &ball = dynamic_cast<Sphere &>(*s.entities[1]);
    Camera camera;

    std::chrono::steady_clock::time_point last_update = std::chrono::steady_clock::now();

    std::vector<std::tuple<unsigned char, unsigned char, unsigned char>> image(width * height);

    std::mutex counter_mutex;
    std::vector<int> indices(width * height);
    std::iota(indices.begin(), indices.end(), 0);
    for (int step = 0; step < steps; step++)
    {
        int metastep = step / substeps;
        int substep = step % substeps;
        std::cout << "Step: " << step << " Metastep: " << metastep << " Substep: " << substep << "\n";
        double f = metasteps < 2 ? 0.0f : (double) (metastep) / (metasteps - 1);
        if (f < 0.5)
        {
            f = f * 2.0f;
            camera.origin = lerp(f, Vec3(0,0,1), Vec3(-1,0,0)) * 3;
            camera.horizontal = lerp(f, Vec3(1,0,0), Vec3(0,0,1)) * 2; // 2 0 0  0 0 -2
            camera.bottom_left = lerp(f, Vec3(0,0,-1), Vec3(1,0,0)) - camera.horizontal / 2 - camera.vertical / 2;
        }
        else
        {
            f = f * 2.0f - 1.0f;
            camera.origin = lerp(f, Vec3(-1,0,0), Vec3(0,0,-1)) * 3;
            camera.horizontal = lerp(f, Vec3(0,0,1), Vec3(-1,0,0)) * 2; // 2 0 0  0 0 -2
            camera.bottom_left = lerp(f, Vec3(1,0,0), Vec3(0,0,1)) - camera.horizontal / 2 - camera.vertical / 2;
        }
        
        // Render image
        float radius = 0.45;
        int current = 0;
        std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
        std::transform(std::execution::par_unseq, indices.begin(), indices.end(), image.begin(), [&](int index) {
            const int j = height - index / width;
            const int i = index % width;
            Vec3 c{0, 0, 0};
            for (int sample = 0; sample < samples; sample++)
            {
                const double u = float(i + distribution(random_generator)) / float(width);
                const double v = float(j + distribution(random_generator)) / float(height);
                const Ray r = camera.getRay(u, v);
                c += color(r, s, 0, substep);
            }
            c /= samples;
            c = gamma_correct(c, 2.0f) * 255.99;
            {
                std::lock_guard lock{counter_mutex};
                ++current;
                if ((std::chrono::steady_clock::now() - last_update).count() / 1000000000.0f > .1f)
                {
                    const int64_t total = width * height;
                    const double progress = (double)current / total;
                    const double pixel_per_second = (double)current / ((std::chrono::steady_clock::now() - start_time).count() / 1000000000.0f);
                    const double time_remaining = (total - current) / pixel_per_second;
                    std::cout << "\r" << std::fixed << std::setprecision(2);
                    std::cout << (double)progress * 100.0f << "% "
                              << "pps: " << pixel_per_second << " ETA: " << time_remaining << "s";
                    std::cout << std::flush;
                    last_update = std::chrono::steady_clock::now();
                }
            }
            return std::make_tuple(c[2], c[1], c[0]);
        });

        std::cout << "\n";

        // Write image
        // std::vector<unsigned char> image_8bit;
        // for (int j = height - 1; j >= 0; j--)
        // {
        //     for (int i = 0; i < width; i++)
        //     {
        //         const Vec3 color = gamma_correct(image[i + j * width], 2.0f);
        //         const int ir = int(255.99 * color[0]);
        //         const int ig = int(255.99 * color[1]);
        //         const int ib = int(255.99 * color[2]);
        //         image_8bit.emplace_back(ir);
        //         image_8bit.emplace_back(ig);
        //         image_8bit.emplace_back(ib);
        //     }
        // }
        auto filepath = std::ostringstream();
        filepath << "data/" << std::setfill('0') << std::setw(3) << step << ".png";
        std::cout << "Writing " << filepath.str() << "\n";
        stbi_write_png(filepath.str().c_str(), width, height, 3, image.data(), width * 3);
    }
}