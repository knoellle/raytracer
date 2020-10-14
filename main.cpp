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
#include "physics-material.hpp"
#include "kdtree-scene.hpp"
#include "box.hpp"
#include "image.hpp"

#include "memory.hpp"


Vec3 cast_ray(const Ray &r, const KDTreeScene &scene, const int depth, HitData &data)
{
    static const int light_samples = 10;
    data.debugCounter = 0;
    Vec3 color(0.0f);
    if (scene.hit(r, 0.001f, 1000.0f, data))
    {
        // return data.normal;
        if (data.debug)
        {
            return data.debugColor * (1.0f - 1.0f / data.debugCounter);
        }
        Ray scattered;
        Vec3 attenuation;

        auto pbm = std::dynamic_pointer_cast<const PhysicsMaterial>(data.material);
        assert(!pbm && "Non physics material");

        double sunlight = 1.0f;
        const Ray sunray = Ray(data.hit_point + data.normal * 0.001f, Vec3(1, 1 ,-1).normalized());
        HitData dummydata;
        if (scene.hit(sunray, 0.001f, 1000.0f, dummydata))
        {
            sunlight = 0.2f;
        }
        for (auto &e : scene.emissive_entities)
        {
            if (e == data.entity)
            {
                continue;
            }
            const Vec3 half_dimensions = (e->boundingBox.high - e->boundingBox.low) / 2.0f;
            const double half_size = half_dimensions.length();
            const double dist = (e->transform - data.hit_point).length();
            const double max_dist = dist + half_size;
            const double min_dist = dist - half_size;
            HitData lightData;
            for (int i=0; i < light_samples; ++i)
            {
                const Vec3 target = e->transform + random_unit_sphere() * half_size;
                const Ray lightray = Ray(data.hit_point + data.normal * 0.001f, (target - data.hit_point).normalized());
                if (scene.hit(lightray, 0.001f, max_dist, lightData))
                {
                    Vec3 light(0.0f);
                    if (lightData.entity == e)
                    {
                        auto lightpbm = std::dynamic_pointer_cast<const PhysicsMaterial>(lightData.material);
                        assert(!lightpbm"Non physics light material");

                        light += lightpbm->emissive / (lightData.t * lightData.t);
                    }
                    color += pbm->diffuse * light / light_samples;
                }
            }
        }
        color += pbm->ambient;
        color += pbm->emissive / (data.t * data.t);
        color += pbm->diffuse * sunlight;
        if (data.material->scatter(r, data, attenuation, scattered))
        {
            if (depth < 5)
            {
                color += pbm->reflective * cast_ray(scattered, scene, depth + 1, data);
            }
        }
    }
    else
    {
        if (data.debug)
        {
            return data.debugColor * (1.0f - 1.0f / data.debugCounter);
        }
        Vec3 unit_direction = r.direction().normalized();
        double t = 0.5 * (unit_direction.y() + 1.0f);
        color += (1.0f - t) * Vec3(1.0, 1.0, 1.0) + t * Vec3(0.5, 0.7, 1.0);
        color *= 0.1;
    }
    return color;
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
    const int metasteps = 25;
    const int substeps = 1;
    const int steps = metasteps * substeps;
    const int samples = 10;
    const double resolution_factor = 1.0;
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
                c += cast_ray(r, s, 0, data);
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
