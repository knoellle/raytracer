#include <algorithm>
#include <chrono>
#include <execution>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <iomanip>

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
#include "image.hpp"

Vec3 color(const Ray &r, const KDTreeScene &scene, const int depth, HitData &data)
{
    data.debugCounter = 0;
    if (scene.hit(r, 0.001, 1000, data))
    {
        Ray scattered;
        Vec3 attenuation;
        if (data.debug)
        {
            return data.debugColor * (1.0f - 1.0f / data.debugCounter);
        }
        if (data.material->scatter(r, data, attenuation, scattered))
        {
            if (depth < 5)
            {
                HitData data;
                data.debugCounter = 0;
                return attenuation * color(scattered, scene, depth + 1, data);
            }
            else
            {
                return attenuation;
            }
        }
        return Vec3(0);
    }
    if (data.debug)
    {
        return data.debugColor * (1.0f - 1.0f / data.debugCounter);
    }
    Vec3 unit_direction = r.direction().normalized();
    double t = 0.5 * (unit_direction.y() + 1.0f);
    return (1.0f - t) * Vec3(1.0, 1.0, 1.0) + t * Vec3(0.5, 0.7, 1.0);
}

auto spawn_sphere(Scene &scene, const Vec3 &position, const float radius, const std::shared_ptr<Material> &material)
{
    std::shared_ptr<Sphere> sphere = std::make_shared<Sphere>(position, radius, material);
    scene.entities.emplace_back(sphere);
    return sphere;
}

auto spawn_box(Scene &scene, const Vec3 &position, const Vec3 &dimensions, const std::shared_ptr<Material> &material)
{
    std::shared_ptr<Box> box = std::make_shared<Box>(position, dimensions, material);
    scene.entities.emplace_back(box);
    return box;
}

KDTreeScene make_test_scene()
{
    auto steel = std::make_shared<PhysicsMaterial>(
        Vec3(0.2, 0.2, 0.2),
        Vec3(0.8, 0.8, 0.8),
        0.02
    );
    auto iron = std::make_shared<PhysicsMaterial>(
        Vec3(0.2, 0.2, 0.2),
        Vec3(0.4, 0.4, 0.4),
        0.1
    );
    auto felt = std::make_shared<PhysicsMaterial>(
        Vec3(0.8, 0.83, 0.8),
        Vec3(0.0),
        0.0
    );
    auto red_felt = std::make_shared<PhysicsMaterial>(
        Vec3(0.8, 0.2, 0.2),
        Vec3(0.0),
        0.0
    );
    auto thing = std::make_shared<PhysicsMaterial>(
        Vec3(0.5,0.2,0.2),
        Vec3(0.2,0.5,0.2),
        0.01
    );
    thing->emissive = Vec3(0.5);

    KDTreeScene scene;

    spawn_sphere(scene, Vec3(0, -100.5, 0), 100, steel);

    spawn_sphere(scene, Vec3(0.5, 0.5, 0), 0.25, iron);

    spawn_sphere(scene, Vec3(-1, 0, 0), 0.5, steel);
    spawn_sphere(scene, Vec3(0, 0, 0), 0.25, thing)->emissive = true;
    spawn_sphere(scene, Vec3(1, 0, 0), 0.5, steel);
    int w = 5;
    int h = 5;
    for (int i = 0; i < w * h; ++i)
    {
        Vec3 p = Vec3(i / w, i % w, 0) / w - Vec3(0.4);
        p[2] = 0.5f;
        spawn_sphere(scene, p, 0.09, steel);
    }
    spawn_box(scene, Vec3(0, 0, -1), Vec3(1, 1, 1), iron);
    spawn_box(scene, Vec3(0, 1, -3), Vec3(1, 1, 1), red_felt);
    spawn_box(scene, Vec3(0, 2, -1), Vec3(1, 1, 1), iron);

    // spawn_sphere(scene, Vec3(-0.5, 0, 0), 0.5, steel);
    // spawn_sphere(scene, Vec3(0, 0, 0), 0.25, red_felt);
    // spawn_sphere(scene, Vec3(0.5, 0, 0), 0.5, steel);

    scene.update();

    return scene;
}

template<typename T>
inline T lerp(double f, T a, T b)
{
    return (1.0f - f) * a + f * b;
}

int main(const int argc, const char* argv[])
{
    const int metasteps = 2;
    const int substeps = 10;
    const int steps = metasteps * substeps;
    const int samples = 1;
    const double resolution_factor = 1;
    const int width = 1920 * resolution_factor;
    const int height = 1080 * resolution_factor;

    auto s = make_test_scene();
    Camera camera(Vec3(0,0,1), Vec3(0), 50, static_cast<double>(width) / height);

    std::chrono::steady_clock::time_point last_update = std::chrono::steady_clock::now();

    Image image;
    image.set_dimensions(width, height);

    std::mutex counter_mutex;
    std::vector<int> indices(width * height);
    std::iota(indices.begin(), indices.end(), 0);
    for (int step = 0; step < steps; step++)
    {
        int metastep = step / substeps;
        int substep = step % substeps;
        std::cout << "Step: " << step << " Metastep: " << metastep << " Substep: " << substep << "\n";
        double f = metasteps < 2 ? 0.0f : (double) (metastep) / (metasteps - 1);

        double angle = lerp<double>(f, 45.0f, -315.0f) * M_PI / 180.0f;
        camera.transform = Vec3(cos(angle), 0.5 + 0.5 * sin(angle), sin(angle)) * 3;
        camera.update();
        
        // Render image
        int current = 0;
        int last = 0;
        double average_pixel_per_second = 0.0f;
        static const double alpha = 0.1;
        std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
        std::transform(std::execution::par_unseq, indices.begin(), indices.end(), image.pixels.begin(), [&](int index) {
            std::chrono::steady_clock::time_point pixel_start_time = std::chrono::steady_clock::now();
            const int j = height - index / width;
            const int i = index % width;
            Vec3 c{0, 0, 0};
            double t = 0;
            for (int sample = 0; sample < samples; sample++)
            {
                HitData data;
                const double u = float(i + random_unit() * 4) / float(width);
                const double v = float(j + random_unit() * 4) / float(height);
                const Ray r = camera.getRay(u, v);
                c += color(r, s, 0, data);
                t += data.t;
            }
            t /= samples;
            c /= samples;
            const auto duration = std::chrono::steady_clock::now() - pixel_start_time;
            {
                std::lock_guard lock{counter_mutex};
                ++current;
                if ((std::chrono::steady_clock::now() - last_update).count() / 1000000000.0f > .1f)
                {
                    const int64_t total = width * height;
                    const double progress = (double)current / total;
                    // const double pixel_per_second = (double)current / ((std::chrono::steady_clock::now() - start_time).count() / 1000000000.0f);
                    const double pixel_per_second = (double) (current - last) / ((std::chrono::steady_clock::now() - last_update).count() / 1000000000.0f);
                    average_pixel_per_second = alpha * pixel_per_second + (1.0f - alpha) * average_pixel_per_second;
                    const double time_remaining = (total - current) / average_pixel_per_second;
                    std::cout << "\r" << std::fixed << std::setprecision(2);
                    std::cout << (double)progress * 100.0f << "% "
                              << "pps: " << average_pixel_per_second << " ETA: " << time_remaining << "s";
                    std::cout << std::flush;
                    last_update = std::chrono::steady_clock::now();
                    last = current;
                }
            }
            Pixel pixel;
            pixel.color = c;
            pixel.depth = t;
            pixel.time = duration;
            return pixel;
        });

        const double duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count() / 1000.0f;

        std::cout << "\n" << "Total time: " << duration << "s\n";

        auto filepath = std::ostringstream();
        filepath << "data/" << std::setfill('0') << std::setw(3) << step << ".png";

        if (argc > 1)
        {
            image.write_time_image(filepath.str());
        }
        else
        {
            image.post_process(1.0f, 2.0f);
            image.write_color_image(filepath.str());
        }


        std::cout << "Writing " << filepath.str() << "\n";
    }
}
