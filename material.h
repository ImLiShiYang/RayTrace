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

    virtual vec3 emitted(double u, double v, const vec3& p) const {
        return vec3(0, 0, 0);
    }

    virtual double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const
    {
        return 0;
    }

};

//���������
class lambertian : public material {
public:
    lambertian(shared_ptr<texture> a) : albedo(a) {}

    virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& alb, ray& scattered, double& pdf) const override
    {
        //���䷽��
        vec3 scatter_direction = rec.normal + random_unit_vector();

        if (scatter_direction.near_zero())
            scatter_direction = rec.normal;

        scattered = ray(rec.p, unit_vector(scatter_direction), r_in.time());
        alb = albedo->value(rec.u, rec.v, rec.p);
        pdf = dot(rec.normal, scattered.direction()) / pi;
        return true;
    }

    double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const override
    {
        auto cosine = dot(rec.normal, unit_vector(scattered.direction()));
        return cosine < 0 ? 0 : cosine / pi;
    }

public:
    //������
    shared_ptr<texture> albedo;
};

//��������
class metal : public material {
public:
    metal(const vec3& a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

    virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const
    {
        vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
        //�������дֲڶ�ʱ���÷��䷽���һ��ƫ��
        scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere());
        attenuation = albedo;
        return (dot(scattered.direction(), rec.normal) > 0);
    }

public:
    //������
    vec3 albedo;
    //�ֲڶ�
    double fuzz;
};

//�������
class dielectric : public material {
public:
    dielectric(double ri) : ref_idx(ri) {}

    virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const
    {
        attenuation = vec3(1.0, 1.0, 1.0);
        //�������������ʵı�ֵ
        double etai_over_etat = (rec.front_face) ? (1.0 / ref_idx) : (ref_idx);

        vec3 unit_direction = unit_vector(r_in.direction());

        double cos_theta = ffmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = sqrt(1.0 - cos_theta * cos_theta);
        //������������ʵ��������������䣬����������
        if (etai_over_etat * sin_theta > 1.0) {
            vec3 reflected = reflect(unit_direction, rec.normal);
            scattered = ray(rec.p, reflected);
            return true;
        }

        //��ʵ�����еĲ���, ��������ĸ��ʻ���������Ƕ��ı�
        double reflect_prob = schlick(cos_theta, etai_over_etat);
        if (random_double() < reflect_prob)
        {
            vec3 reflected = reflect(unit_direction, rec.normal);
            scattered = ray(rec.p, reflected);
            return true;
        }

        //����ֻ�������䣬���Է���
        vec3 refracted = refract(unit_direction, rec.normal, etai_over_etat);
        scattered = ray(rec.p, refracted);
        return true;
    }
public:
    double ref_idx;
};

//���������
class diffuse_light : public material {
public:
    diffuse_light(shared_ptr<texture> a) : emit(a) {}

    virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const
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



