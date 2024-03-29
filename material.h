#ifndef Material_H
#define Material_H

#include "hittable.h"
#include "texture.h"

class material {
public:
    virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& albedo, ray& scattered, double& pdf) const
    {
        return false;
    }

    virtual vec3 emitted(const ray& r_in, const hit_record& rec, double u, double v, const vec3& p) const {
        return vec3(0, 0, 0);
    }

    virtual double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const
    {
        return 0;
    }

};

//漫反射材质
class lambertian : public material {
public:
    lambertian(shared_ptr<texture> a) : albedo(a) {}

    virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& alb, ray& scattered, double& pdf) const override
    {
        //反射方向
        /*vec3 scatter_direction = rec.normal + random_unit_vector();

        if (scatter_direction.near_zero())
            scatter_direction = rec.normal;

        scattered = ray(rec.p, unit_vector(scatter_direction), r_in.time());
        alb = albedo->value(rec.u, rec.v, rec.p);
        pdf = dot(rec.normal, scattered.direction()) / pi;
        return true;*/

        auto direction = random_in_hemisphere(rec.normal);
        scattered = ray(rec.p, unit_vector(direction), r_in.time());
        alb = albedo->value(rec.u, rec.v, rec.p);
        pdf = 0.5 / pi;
        return true;
    }

    double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const override
    {
        //朗伯曲面的散射pdf函数s(direction)正比于cos()
        auto cosine = dot(rec.normal, unit_vector(scattered.direction()));
        //cosine值是散射光线和表面法线的夹角，所以大于90°无效
        return cosine < 0 ? 0 : cosine / pi;
    }

public:
    //反照率
    shared_ptr<texture> albedo;
};

//金属材质
class metal : public material {
public:
    metal(const vec3& a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

    virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const
    {
        vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
        //当金属有粗糙度时，让反射方向加一点偏差
        scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere());
        attenuation = albedo;
        return (dot(scattered.direction(), rec.normal) > 0);
    }

public:
    //反照率
    vec3 albedo;
    //粗糙度
    double fuzz;
};

//折射材质
class dielectric : public material {
public:
    dielectric(double ri) : ref_idx(ri) {}

    virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const
    {
        attenuation = vec3(1.0, 1.0, 1.0);
        //空气与其他介质的比值
        double etai_over_etat = (rec.front_face) ? (1.0 / ref_idx) : (ref_idx);

        vec3 unit_direction = unit_vector(r_in.direction());

        double cos_theta = ffmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = sqrt(1.0 - cos_theta * cos_theta);
        //如果从其他介质到空气，则发生反射，不发生折射
        if (etai_over_etat * sin_theta > 1.0) {
            vec3 reflected = reflect(unit_direction, rec.normal);
            scattered = ray(rec.p, reflected);
            return true;
        }

        //现实世界中的玻璃, 发生折射的概率会随着入射角而改变
        double reflect_prob = schlick(cos_theta, etai_over_etat);
        if (random_double() < reflect_prob)
        {
            vec3 reflected = reflect(unit_direction, rec.normal);
            scattered = ray(rec.p, reflected);
            return true;
        }

        //这里只返回折射，忽略反射
        vec3 refracted = refract(unit_direction, rec.normal, etai_over_etat);
        scattered = ray(rec.p, refracted);
        return true;
    }
public:
    double ref_idx;
};

//自身发光材质
class diffuse_light : public material {
public:
    diffuse_light(shared_ptr<texture> a) : emit(a) {}

    virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const
    {
        return false;
    }

    virtual vec3 emitted(const ray& r_in, const hit_record& rec, double u, double v, const vec3& p) const override
    {    
        //入射光线与光源法向量朝向不同，则返回光源颜色
        if (rec.front_face)
            return emit->value(u, v, p);
        else
            return vec3(0, 0, 0);
    }

public:
    shared_ptr<texture> emit;
};

#endif // !Material



