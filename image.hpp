#pragma once

#include <chrono>
#include <algorithm>
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "vec3.hpp"

struct Pixel
{
    Vec3 color;
    double depth;
    std::chrono::steady_clock::duration time;
};

bool pixelTimeComp(const Pixel &a, const Pixel &b) { return a.time < b.time; }

class Image
{
    public:
        std::vector<Pixel> pixels;

        void set_dimensions(const int new_width, const int new_height);
        int width() const;
        int height() const;

        bool write_color_image(const std::string filepath) const;
        bool write_time_image(const std::string filepath, const double outlier_percentage = 5) const;
    private:
        int _width;
        int _height;
};

void Image::set_dimensions(const int new_width, const int new_height)
{
    _width = new_width;
    _height = new_height;
    pixels.resize(_width * _height);
}

bool Image::write_color_image(const std::string filepath) const
{
    std::vector<std::tuple<unsigned char, unsigned char, unsigned char>> pixels_8bit;
    pixels_8bit.resize(_width * _height);
    std::transform(std::execution::par_unseq, pixels.begin(), pixels.end(), pixels_8bit.begin(), [&](const Pixel &pixel)
    {
        Vec3 c = pixel.color * 255.99f;
        return std::make_tuple(c[2], c[1], c[0]);
    });
    return stbi_write_png(filepath.c_str(), _width, _height, 3, pixels_8bit.data(), _width * 3);
}

bool Image::write_time_image(const std::string filepath, const double outlier_percentage) const
{
    const int num_pixels = _width * _height;
    std::vector<std::chrono::steady_clock::duration> times;
    times.resize(_width * _height);
    std::transform(pixels.begin(), pixels.end(), times.begin(),
    [](const Pixel &pixel) {
        return pixel.time;
    });
    // calculate range

    // const auto minmax = std::minmax_element(pixels.begin(), pixels.end());
    // const auto base = minmax.first->time;
    // const auto range = minmax.second->time - base;


    auto low = times.begin() + static_cast<int>(num_pixels * outlier_percentage / 100.0f);
    std::nth_element(std::execution::par_unseq, times.begin(), low, times.end());
    auto high = times.begin() + static_cast<int>(num_pixels - num_pixels * outlier_percentage / 100.0f - 1);
    std::nth_element(std::execution::par_unseq, times.begin(), high, times.end());
    
    std::chrono::steady_clock::duration base = *low;
    std::chrono::steady_clock::duration range = *high - base;

    std::cout << base.count() << " " << range.count() << "\n";
    std::vector<std::tuple<unsigned char, unsigned char, unsigned char>> pixels_8bit;
    pixels_8bit.resize(_width * _height);
    std::transform(std::execution::par_unseq, pixels.begin(), pixels.end(), pixels_8bit.begin(), [&](const Pixel &pixel)
    {
        // Vec3 c = pixel.duration * 255.99f;
        const double duration = static_cast<double>(pixel.time.count() - base.count()) / range.count();
        Vec3 c = Vec3(std::clamp<double>(duration * 255.99, 0.0f, 255.0f));
        return std::make_tuple(c[2], c[1], c[0]);
    });
    return stbi_write_png(filepath.c_str(), _width, _height, 3, pixels_8bit.data(), _width * 3);
}
