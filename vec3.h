#ifndef VEC3
#define VEC3

#include <iostream>
#include "rtweekend.h"
#include "tgaimage.h"

class vec3 {
public:
    vec3() : e{ 0,0,0 } {}
    vec3(double e1) : e{ e1,e1,e1 } {}
    vec3(double e0, double e1, double e2) : e{ e0, e1, e2 } {}

    double x() const { return e[0]; }
    double y() const { return e[1]; }
    double z() const { return e[2]; }

    vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
    double operator[](int i) const { return e[i]; }
    double& operator[](int i) { return e[i]; }

    vec3& operator+=(const vec3& v) {
        e[0] += v.e[0];
        e[1] += v.e[1];
        e[2] += v.e[2];
        return *this;
    }

    vec3& operator*=(const double t) {
        e[0] *= t;
        e[1] *= t;
        e[2] *= t;
        return *this;
    }

    vec3& operator/=(const double t) {
        return *this *= 1 / t;
    }

    bool near_zero()
    {
        if (std::abs(e[0]) < 0.000001 && std::abs(e[1]) < 0.000001 && std::abs(e[2]) < 0.000001)
            return true;
        return false;
    }

    inline static vec3 random() {
        return vec3(random_double(), random_double(), random_double());
    }

    inline static vec3 random(double min, double max) {
        return vec3(random_double(min, max), random_double(min, max), random_double(min, max));
    }

    double length() const {
        return sqrt(length_squared());
    }

    double length_squared() const {
        return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
    }

    void write_color(int i,int j,TGAImage& image, int samples_per_pixel) {
        // Divide the color total by the number of samples.
        auto scale = 1.0 / samples_per_pixel;
        auto r = sqrt(scale * e[0]);
        auto g = sqrt(scale * e[1]);
        auto b = sqrt(scale * e[2]);

        // Write the translated [0,255] value of each color component.
        //out << static_cast<int>(256 * clamp(r, 0.0, 0.999)) << ' '
        //    << static_cast<int>(256 * clamp(g, 0.0, 0.999)) << ' '
        //    << static_cast<int>(256 * clamp(b, 0.0, 0.999)) << '\n';
        image.set(i, j, TGAColor(256 * clamp(r, 0.0, 0.999), 256 * clamp(g, 0.0, 0.999), 256 * clamp(b, 0.0, 0.999)));
    }

public:
    double e[3];
};

inline std::ostream& operator<<(std::ostream& out, const vec3& v) {
    return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

inline vec3 operator+(const vec3& u, const vec3& v) {
    return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
}

inline vec3 operator-(const vec3& u, const vec3& v) {
    return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}

inline vec3 operator*(const vec3& u, const vec3& v) {
    return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}

inline vec3 operator*(double t, const vec3& v) {
    return vec3(t * v.e[0], t * v.e[1], t * v.e[2]);
}

inline vec3 operator*(const vec3& v, double t) {
    return t * v;
}

inline vec3 operator/(vec3 v, double t) {
    return (1 / t) * v;
}

inline double dot(const vec3& u, const vec3& v) {
    return u.e[0] * v.e[0]
        + u.e[1] * v.e[1]
        + u.e[2] * v.e[2];
}

inline vec3 cross(const vec3& u, const vec3& v) {
    return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
        u.e[2] * v.e[0] - u.e[0] * v.e[2],
        u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

inline vec3 unit_vector(vec3 v) {
    return v / v.length();
}

//在单位球内的随机点组成的单位方向向量
inline vec3 random_unit_vector() {
    auto a = random_double(0, 2 * pi);
    auto z = random_double(-1, 1);
    auto r = sqrt(1 - z * z);
    return vec3(r * cos(a), r * sin(a), z);
}

//在- 1到 + 1的单位立方体中选取一个随机点
inline vec3 random_in_unit_sphere() {
    while (true) {
        vec3 p = vec3::random(-1, 1);
        if (p.length_squared() >= 1)
            continue;
        return p;
    }
}

//在半球内的随机单位方向向量
inline vec3 random_in_hemisphere(const vec3& normal) {
    vec3 in_unit_sphere = random_in_unit_sphere();
    if (dot(in_unit_sphere, normal) > 0.0) // In the same hemisphere as the normal
        return in_unit_sphere;
    else
        return -in_unit_sphere;
}

inline vec3 reflect(const vec3& v, const vec3& n) {
    return v - 2 * dot(v, n) * n;
}

//根据折射公式计算
inline vec3 refract(const vec3& incidentLight, const vec3& Normal, double etai_over_etat) {
    auto cos_theta = dot(-incidentLight, Normal);
    vec3 r_out_parallel = etai_over_etat * (incidentLight + cos_theta * Normal);
    vec3 r_out_perp = -sqrt(1.0 - r_out_parallel.length_squared()) * Normal;
    return r_out_parallel + r_out_perp;
}

//发生折射的概率会随着入射角而改变
inline double schlick(double cosine, double ref_idx) {
    auto r0 = (1 - ref_idx) / (1 + ref_idx);
    r0 = r0 * r0;
    return r0 + (1 - r0) * pow((1 - cosine), 5);
}

//从单位圆返回点
inline vec3 random_in_unit_disk() {
    while (true) {
        auto p = vec3(random_double(-1, 1), random_double(-1, 1), 0);
        if (p.length_squared() >= 1) continue;
        return p;
    }
}

inline int random_int(int min,int max)
{
    return random_double(min, max);
}


#endif // !VEC3
