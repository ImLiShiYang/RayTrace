#ifndef Material_H
#define Material_H

#include "hittable.h"
#include "texture.h"

class material {
public:
    virtual bool scatter(
        const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered
    ) const = 0;

    virtual vec3 emitted(double u, double v, const vec3& p) const {
        return vec3(0, 0, 0);
    }

};

//漫反射材质
class lambertian : public material {
public:
    lambertian(shared_ptr<texture> a) : albedo(a) {}

    virtual bool scatter(
        const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const 
    {
        vec3 scatter_direction = rec.normal + random_unit_vector();
        scattered = ray(rec.p, scatter_direction, r_in.time());
        attenuation = albedo->value(rec.u, rec.v, rec.p);
        return true;
    }

public:
    //反照率
    shared_ptr<texture> albedo;
};

//金属材质
class metal : public material {
public:
    metal(const vec3& a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

    virtual bool scatter(
        const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered
    ) const {
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

    virtual bool scatter(
        const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const
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

    virtual bool scatter(
        const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const
    {
    
        return false;
    }

    virtual vec3 emitted(double u, double v, const vec3& p) const {
        return emit->value(u, v, p);
    }

public:
    shared_ptr<texture> emit;
};

#endif // !Material



