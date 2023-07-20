#ifndef HITTABLE_H
#define HITTABLE_H

#include "ray.h"
#include "boundingBox.h"

class material;

struct hit_record {
    //相交的点
    vec3 p;
    //相交点的法向量
    vec3 normal;
    //材质类
    shared_ptr<material> mat_ptr;
    //贴图uv
    double u, v;
    //经过多长时间
    double t;
    //判断法线朝向
    bool front_face;

	inline void set_face_normal(const ray& r, const vec3& outward_normal) {
		front_face = dot(r.direction(), outward_normal) < 0;
		normal = front_face ? outward_normal : -outward_normal;
	}
};

class hittable {
public:
    //用于计算光线是否与物体相交，并保存相交点的信息
    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const = 0;
    virtual bool bounding_box(double t0, double t1, aabb& output_box) const = 0;
};

//用于修改法线朝向的类
class flip_face : public hittable {
public:
    flip_face(shared_ptr<hittable> p) : ptr(p) {}

    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
        if (!ptr->hit(r, t_min, t_max, rec))
            return false;

        rec.front_face = !rec.front_face;
        return true;
    }

    virtual bool bounding_box(double t0, double t1, aabb& output_box) const {
        return ptr->bounding_box(t0, t1, output_box);
    }

public:
    shared_ptr<hittable> ptr;
};


inline bool box_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b, int axis) {
    aabb box_a;
    aabb box_b;

    if (!a->bounding_box(0, 0, box_a) || !b->bounding_box(0, 0, box_b))
        std::cerr << "No bounding box in bvh_node constructor.\n";

    return box_a.min().e[axis] < box_b.min().e[axis];
}


inline bool box_x_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
    return box_compare(a, b, 0);
}

inline bool box_y_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
    return box_compare(a, b, 1);
}

inline bool box_z_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
    return box_compare(a, b, 2);
}

#endif