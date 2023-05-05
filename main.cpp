#include <iostream>
#include<fstream>

#include "ray.h"
#include "rtweekend.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "vec3.h"

using namespace std;

double hit_sphere(const vec3& center, double radius, const ray& r) {
    vec3 oc = r.origin() - center;
    auto a = dot(r.direction(), r.direction());
    auto b = 2.0 * dot(oc, r.direction());
    auto c = dot(oc, oc) - radius * radius;
    auto discriminant = b * b - 4 * a * c;
    if (discriminant < 0) {
        return -1.0;
    }
    else {
        return (-b - sqrt(discriminant)) / (2.0 * a);
    }
}

vec3 ray_color(const ray& r, const hittable& world,int depth) 
{
    if (depth <= 0)
        return vec3(1, 0, 0);

	hit_record rec;
	if (world.hit(r, 0.001, infinity, rec)) {
        vec3 reflectPoint = rec.p + rec.normal + random_in_unit_sphere();
        return 0.5 * ray_color(ray(rec.p, reflectPoint), world,depth-1);
	}

    vec3 unit_direction = unit_vector(r.direction());
    //(x+1)*0.5是为了把(-1,1)转换成(0,1)
    auto p = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - p) * vec3(1.0, 1.0, 1.0) + p * vec3(0.5, 0.7, 1.0);
}

int main() {
    const int image_width = 800;
    const int image_height = 400;
    const int samples_per_pixel = 100;
    const int max_depth = 50;

    ofstream fout("MyImage.ppm"); //文件输出流对象
    streambuf* pOld = cout.rdbuf(fout.rdbuf());

    camera cam;

	hittable_list world;
	world.add(make_shared<sphere>(vec3(0, 0, -1), 0.5));
	world.add(make_shared<sphere>(vec3(0, -100.5, -1), 100));

    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    for (int j = image_height - 1; j >= 0; --j) {
        std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
        for (int i = 0; i < image_width; ++i) {
            vec3 color(0, 0, 0);
            for (int s = 0; s < samples_per_pixel; ++s) {
                auto u = (i + random_double()) / image_width;
                auto v = (j + random_double()) / image_height;
                ray r = cam.get_ray(u, v);
                color += ray_color(r, world, max_depth);
            }
            color.write_color(std::cout, samples_per_pixel);
        }
    }

    std::cerr << "\nDone.\n";
}