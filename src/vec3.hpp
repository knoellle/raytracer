#pragma once

#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>

class Vec3
{
    public:
        explicit Vec3(double e0) { e[0] = e0; e[1] = e0; e[2] = e0; }
        Vec3(double e0, double e1, double e2) { e[0] = e0; e[1] = e1; e[2] = e2; }
        Vec3() { e[0] = 0; e[1] = 0; e[2] = 0; }
        double x() const { return e[0]; }
        double y() const { return e[1]; }
        double z() const { return e[2]; }
        double r() const { return e[0]; }
        double g() const { return e[1]; }
        double b() const { return e[2]; }

        inline const Vec3& operator+() const { return *this; }
        inline Vec3 operator-() const { return {-e[0], -e[1], -e[2]}; }
        inline double operator[](int i) const { return e[i]; }
        inline double& operator[](int i) { return e[i]; }

        inline Vec3& operator+=(const Vec3 &v2);
        inline Vec3& operator-=(const Vec3 &v2);
        inline Vec3& operator*=(const Vec3 &v2);
        inline Vec3& operator/=(const Vec3 &v2);
        inline Vec3& operator*=(const double &f);
        inline Vec3& operator/=(const double &f);

        inline double dot(const Vec3 &v2) const;
        inline Vec3 cross(const Vec3 &v2) const;
        inline Vec3 reflect(const Vec3 &v) const;

        double length() const
        {
            return sqrt(squaredLength());
        }
        double squaredLength() const
        {
            return e[0]*e[0] + e[1]*e[1] + e[2]*e[2];
        }
        void normalize();
        Vec3 normalized() const;

        std::array<double, 3> e = {};
};

inline Vec3 operator+(const Vec3 &v1, const Vec3 &v2);
inline Vec3 operator-(const Vec3 &v1, const Vec3 &v2);
inline Vec3 operator*(const Vec3 &v1, const Vec3 &v2);
inline Vec3 operator/(const Vec3 &v1, const Vec3 &v2);
inline Vec3 operator*(double f, Vec3 v);
inline Vec3 operator*(Vec3 v, double f);
inline Vec3 operator/(Vec3 v, double f);
inline Vec3 operator/(double f, Vec3 v);

inline std::istream& operator>>(std::istream &is, Vec3 &v)
{
    is >> v.e[0] >> v.e[1] >> v.e[2];
    return is;
}

inline std::ostream& operator<<(std::ostream &os, const Vec3 &v)
{
    os << v.e[0] << " " << v.e[1] << " " << v.e[2];
    return os;
}


inline Vec3& Vec3::operator+=(const Vec3 &v)
{
    e[0] += v[0];
    e[1] += v[1];
    e[2] += v[2];
    return *this;
}

inline Vec3& Vec3::operator-=(const Vec3 &v)
{
    e[0] -= v[0];
    e[1] -= v[1];
    e[2] -= v[2];
    return *this;
}

inline Vec3& Vec3::operator*=(const Vec3 &v)
{
    e[0] *= v[0];
    e[1] *= v[1];
    e[2] *= v[2];
    return *this;
}

inline Vec3& Vec3::operator/=(const Vec3 &v)
{
    e[0] /= v[0];
    e[1] /= v[1];
    e[2] /= v[2];
    return *this;
}

inline Vec3& Vec3::operator*=(const double &f)
{
    e[0] *= f;
    e[1] *= f;
    e[2] *= f;
    return *this;
}

inline Vec3& Vec3::operator/=(const double &f)
{
    e[0] /= f;
    e[1] /= f;
    e[2] /= f;
    return *this;
}

inline Vec3 operator+(const Vec3 &v1, const Vec3 &v2)
{
    return {v1[0] + v2[0], v1[1] + v2[1], v1[2] + v2[2]};
}

inline Vec3 operator-(const Vec3 &v1, const Vec3 &v2)
{
    return {v1[0] - v2[0], v1[1] - v2[1], v1[2] - v2[2]};
}

inline Vec3 operator*(const Vec3 &v1, const Vec3 &v2)
{
    return {v1[0] * v2[0], v1[1] * v2[1], v1[2] * v2[2]};
}

inline Vec3 operator/(const Vec3 &v1, const Vec3 &v2)
{
    return {v1[0] / v2[0], v1[1] / v2[1], v1[2] / v2[2]};
}

inline Vec3 operator*(const double f, const Vec3 v)
{
    return {v[0] * f, v[1] * f, v[2] * f};
}

inline Vec3 operator*(const Vec3 v, const double f)
{
    return {v[0] * f, v[1] * f, v[2] * f};
}

inline Vec3 operator/(const Vec3 v, const double f)
{
    return {v[0] / f, v[1] / f, v[2] / f};
}

inline Vec3 operator/(const double f, const Vec3 v)
{
    return {f / v[0], f / v[1], f / v[2]};
}

inline double Vec3::dot(const Vec3 &v2) const
{
    return e[0] * v2[0] + e[1] * v2[1] + e[2] * v2[2];
}

inline Vec3 Vec3::cross(const Vec3 &v2) const
{
    return {(e[1] * v2[2] - e[2] * v2[1]),
           -(e[0] * v2[2] - e[2] * v2[0]),
            (e[0] * v2[1] - e[1] * v2[0])};
}

inline Vec3 Vec3::reflect(const Vec3 &v) const
{
    return v - 2.0f * dot(v) * (*this);
}

inline void Vec3::normalize()
{
    double d = length();
    e[0] /= d;
    e[1] /= d;
    e[2] /= d;
}

inline Vec3 Vec3::normalized() const
{
    double d = length();
    return {e[0] / d, e[1] / d, e[2] / d};
}