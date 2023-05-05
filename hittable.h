#ifndef HITTABLE_H
#define HITTABLE_H

#include "ray.h"

class material;

struct hit_record {
    //相交的点
    vec3 p;
    //相交点的法向量
    vec3 normal;
    //材质类
    shared_ptr<material> mat_ptr;
    //经过多长时间
    double t;

    bool front_face;

	inline void set_face_normal(const ray& r, const vec3& outward_normal) {
		front_face = dot(r.direction(), outward_normal) < 0;
		normal = front_face ? outward_normal : -outward_normal;
	}
};

class hittable {
public:
    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const = 0;
};

#endif