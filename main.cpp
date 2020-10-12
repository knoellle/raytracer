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

    scene.update();

    return scene;
}

template<typename T>
inline T lerp(double f, T a, T b)
{
    return (1.0f - f) * a + f * b;
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
    Camera camera(Vec3(0,0,1), Vec3(0), 50, static_cast<double>(width) / height);
    // camera.up = Vec3(0,-1,0);

    std::chrono::steady_clock::time_point last_update = std::chrono::steady_clock::now();

    // std::vector<std::tuple<unsigned char, unsigned char, unsigned char>> image(width * height);
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
        if (f < 0.5)
        {
            f = f * 2.0f;
            camera.transform = lerp(f, Vec3(0,0,1), Vec3(-1,0,0)) * 3;
        }
        else
        {
            f = f * 2.0f - 1.0f;
            camera.transform = lerp(f, Vec3(-1,0,0), Vec3(0,0,-1)) * 3;
        }
        camera.update();
        
        // Render image
        int current = 0;
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
                const double u = float(i + distribution(random_generator)) / float(width);
                const double v = float(j + distribution(random_generator)) / float(height);
                const Ray r = camera.getRay(u, v);
                c += color(r, s, 0, data);
                t += data.t;
            }
            t /= samples;
            c /= samples;
            // double duration = (std::chrono::steady_clock::now() - pixel_start_time).count() / 5000.0f / samples;
            const auto duration = std::chrono::steady_clock::now() - pixel_start_time;
            c = gamma_correct(c, 2.0f);
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
            Pixel pixel;
            pixel.color = c;
            pixel.depth = t;
            pixel.time = duration;
            return pixel;
        });

        const double duration = (double)((std::chrono::steady_clock::now() - start_time).count() / 1000000000.0f);
        std::cout << "\n" << "Total time: " << duration << "s\n";

        auto filepath = std::ostringstream();
        filepath << "data/" << std::setfill('0') << std::setw(3) << step << ".png";

        image.write_color_image(filepath.str());
        // image.write_time_image(filepath.str());

        std::cout << "Writing " << filepath.str() << "\n";
    }
}
