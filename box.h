#ifndef Box_H
#define Box_H

#include "hittable_list.h"
#include "arealight.h"


class box : public hittable {
public:
	box() {}
	box(const vec3& p0, const vec3& p1, shared_ptr<material> ptr);

	virtual bool hit(const ray& r, double t0, double t1, hit_record& rec) const;

	virtual bool bounding_box(double t0, double t1, aabb& output_box) const {
		output_box = aabb(box_min, box_max);
		return true;
	}

public:
	vec3 box_min;
	vec3 box_max;
	hittable_list sides;
};

box::box(const vec3& p0, const vec3& p1, shared_ptr<material> ptr) {
	box_min = p0;
	box_max = p1;

	sides.add(make_shared<xy_rect>(p0.x(), p1.x(), p0.y(), p1.y(), p1.z(), ptr));
	sides.add(make_shared<flip_face>(make_shared<xy_rect>(p0.x(), p1.x(), p0.y(), p1.y(), p0.z(), ptr)));
		
	sides.add(make_shared<xz_rect>(p0.x(), p1.x(), p0.z(), p1.z(), p1.y(), ptr));
	sides.add(make_shared<flip_face>(make_shared<xz_rect>(p0.x(), p1.x(), p0.z(), p1.z(), p0.y(), ptr)));	

	sides.add(make_shared<yz_rect>(p0.y(), p1.y(), p0.z(), p1.z(), p1.x(), ptr));
	sides.add(make_shared<flip_face>(make_shared<yz_rect>(p0.y(), p1.y(), p0.z(), p1.z(), p0.x(), ptr)));
		
}

bool box::hit(const ray& r, double t0, double t1, hit_record& rec) const {
	return sides.hit(r, t0, t1, rec);
}


//移动类
class translate : public hittable {
public:
	translate(shared_ptr<hittable> p, const vec3& displacement)
		: ptr(p), offset(displacement) {}

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const;
	virtual bool bounding_box(double t0, double t1, aabb& output_box) const;

public:
	shared_ptr<hittable> ptr;
	vec3 offset;
};

bool translate::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
	ray moved_r(r.origin() - offset, r.direction(), r.time());
	if (!ptr->hit(moved_r, t_min, t_max, rec))
		return false;

	rec.p += offset;
	rec.set_face_normal(moved_r, rec.normal);

	return true;
}

bool translate::bounding_box(double t0, double t1, aabb& output_box) const {
	if (!ptr->bounding_box(t0, t1, output_box))
		return false;

	output_box = aabb(
		output_box.min() + offset,
		output_box.max() + offset);

	return true;
}

//旋转类
class rotate_y : public hittable {
public:
	rotate_y(shared_ptr<hittable> p, double angle);

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const;
	virtual bool bounding_box(double t0, double t1, aabb& output_box) const {
		output_box = bbox;
		return hasbox;
	}

public:
	shared_ptr<hittable> ptr;
	double sin_theta;
	double cos_theta;
	bool hasbox;
	aabb bbox;
};

rotate_y::rotate_y(shared_ptr<hittable> p, double angle) : ptr(p) {
	auto radians = degrees_to_radians(angle);
	sin_theta = sin(radians);
	cos_theta = cos(radians);
	hasbox = ptr->bounding_box(0, 1, bbox);

	vec3 min(infinity, infinity, infinity);
	vec3 max(-infinity, -infinity, -infinity);

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			for (int k = 0; k < 2; k++) {
				auto x = i * bbox.max().x() + (1 - i) * bbox.min().x();
				auto y = j * bbox.max().y() + (1 - j) * bbox.min().y();
				auto z = k * bbox.max().z() + (1 - k) * bbox.min().z();

				auto newx = cos_theta * x + sin_theta * z;
				auto newz = -sin_theta * x + cos_theta * z;

				vec3 tester(newx, y, newz);

				for (int c = 0; c < 3; c++) {
					min[c] = ffmin(min[c], tester[c]);
					max[c] = ffmax(max[c], tester[c]);
				}
			}
		}
	}

	bbox = aabb(min, max);
}

bool rotate_y::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
	vec3 origin = r.origin();
	vec3 direction = r.direction();
	//对光线旋转
	origin[0] = cos_theta * r.origin()[0] - sin_theta * r.origin()[2];
	origin[2] = sin_theta * r.origin()[0] + cos_theta * r.origin()[2];
	direction[0] = cos_theta * r.direction()[0] - sin_theta * r.direction()[2];
	direction[2] = sin_theta * r.direction()[0] + cos_theta * r.direction()[2];

	ray rotated_r(origin, direction, r.time());

	if (!ptr->hit(rotated_r, t_min, t_max, rec))
		return false;

	vec3 p = rec.p;
	vec3 normal = rec.normal;

	//旋转后对法向量做修改
	p[0] = cos_theta * rec.p[0] + sin_theta * rec.p[2];
	p[2] = -sin_theta * rec.p[0] + cos_theta * rec.p[2];
	normal[0] = cos_theta * rec.normal[0] + sin_theta * rec.normal[2];
	normal[2] = -sin_theta * rec.normal[0] + cos_theta * rec.normal[2];

	rec.p = p;
	rec.set_face_normal(rotated_r, normal);

	return true;
}

#endif // !Box_H
