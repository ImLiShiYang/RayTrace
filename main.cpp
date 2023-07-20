#include <iostream>
#include<fstream>

#include "ray.h"
#include "rtweekend.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"
#include "bvh.h"
#include "image_texture.h"
#include "arealight.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "box.h"
#include "tgaimage.h"

using namespace std;


vec3 ray_color(const ray& r, const hittable& world,int depth) 
{
    if (depth <= 0)
        return vec3(1, 0, 0);

	hit_record rec;
    //判断光线是否击中物体
	if (world.hit(r, 0.001, infinity, rec)) {
        ray scattered;
        vec3 attenuation;

        if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
            return attenuation * ray_color(scattered, world, depth - 1);
        return vec3(0, 0, 0);
	}

    vec3 unit_direction = unit_vector(r.direction());
    //(x+1)*0.5是为了把(-1,1)转换成(0,1)
    auto p = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - p) * vec3(1.0, 1.0, 1.0) + p * vec3(0.5, 0.7, 1.0);
}

vec3 ray_color(const ray& r, const vec3& background, const hittable& world, int depth) {
    hit_record rec;

    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
        return vec3(1, 0, 0);

    // 判断光线是否击中物体，如果没有则直接返回黑色
    if (!world.hit(r, 0.001, infinity, rec))
        return background;

    ray scattered;
    vec3 attenuation;
    vec3 emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);
    if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered))
        return emitted;

    return emitted + attenuation * ray_color(scattered, background, world, depth - 1);
}

hittable_list random_scene() {

    hittable_list world;

    //作为地板的大球
    auto checker = make_shared<checker_texture>(
        make_shared<constant_texture>(vec3(0.2, 0.3, 0.1)),
        make_shared<constant_texture>(vec3(0.9, 0.9, 0.9))
    );

    world.add(make_shared<sphere>(vec3(0, -1000, 0), 1000, make_shared<lambertian>(checker)));
        

    int i = 1;
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            vec3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());
            if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = vec3::random() * vec3::random();
                    auto albedo_ptr = make_shared<lambertian>(make_shared<constant_texture>(albedo));
                    world.add(make_shared<moving_sphere>(
                        center, center + vec3(0, random_double(0, .5), 0), 0.0, 1.0, 0.2, albedo_ptr));
                        
                }
                else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = vec3::random(.5, 1);
                    auto fuzz = random_double(0, .5);
                    world.add(
                        make_shared<sphere>(center, 0.2, make_shared<metal>(albedo, fuzz)));
                }
                else {
                    // glass
                    world.add(make_shared<sphere>(center, 0.2, make_shared<dielectric>(1.5)));
                }
            }
        }
    }

    world.add(make_shared<sphere>(vec3(0, 1, 0), 1.0, make_shared<dielectric>(1.5)));

    auto ptr2 = make_shared<lambertian>(make_shared<constant_texture>(vec3(0.4, 0.2, 0.1)));
    world.add(
        make_shared<sphere>(vec3(-4, 1, 0), 1.0, ptr2));

    world.add(
        make_shared<sphere>(vec3(4, 1, 0), 1.0, make_shared<metal>(vec3(0.7, 0.6, 0.5), 0.0)));

    return static_cast<hittable_list>(make_shared<bvh_node>(world, 0, 1));
    //return world;
}

hittable_list earth() {
    int nx, ny, nn;
    unsigned char* texture_data = stbi_load("earthmap.jpg", &nx, &ny, &nn, 0);
    if (texture_data == nullptr)
        cout << "Cannot load texture" << endl;

    auto earth_surface =
        make_shared<lambertian>(make_shared<image_texture>(texture_data, nx, ny));
    auto globe = make_shared<sphere>(vec3(0, 0, 0), 2, earth_surface);

    hittable_list world;
    world.add(globe);
    return hittable_list(world);
}

hittable_list simple_light() {
    hittable_list world;

    int nx, ny, nn;
    unsigned char* texture_data = stbi_load("earthmap.jpg", &nx, &ny, &nn, 0);
    if (texture_data == nullptr)
        cout << "Cannot load texture" << endl;

    auto earth_surface =
        make_shared<lambertian>(make_shared<image_texture>(texture_data, nx, ny));
    auto globe = make_shared<sphere>(vec3(0, 1, 0), 1, earth_surface);
    world.add(globe);

    //作为地板的大球
    auto checker = make_shared<checker_texture>(
        make_shared<constant_texture>(vec3(0.2, 0.3, 0.1)),
        make_shared<constant_texture>(vec3(0.9, 0.9, 0.9))
    );
    world.add(make_shared<sphere>(vec3(0, -1000, 0), 1000, make_shared<lambertian>(checker)));

    //面光源材质
    auto difflight = make_shared<diffuse_light>(make_shared<constant_texture>(vec3(4, 4, 4)));
    //球形光源
    world.add(make_shared<sphere>(vec3(0, 3, 0), 1, difflight));
    //面光源
    world.add(make_shared<xy_rect>(3, 5, 1, 3, -2, difflight));

    

    return world;
}

hittable_list cornell_box() {
    hittable_list objects;

    auto red = make_shared<lambertian>(make_shared<constant_texture>(vec3(0.65, 0.05, 0.05)));
    auto white = make_shared<lambertian>(make_shared<constant_texture>(vec3(0.73, 0.73, 0.73)));
    auto green = make_shared<lambertian>(make_shared<constant_texture>(vec3(0.12, 0.45, 0.15)));
    auto light = make_shared<diffuse_light>(make_shared<constant_texture>(vec3(15, 15, 15)));

    objects.add(make_shared<flip_face>(make_shared<yz_rect>(0, 555, 0, 555, 555, green)));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<xz_rect>(213, 343, 227, 332, 554, light));
    objects.add(make_shared<flip_face>(make_shared<xz_rect>(0, 555, 0, 555, 555, white)));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<flip_face>(make_shared<xy_rect>(0, 555, 0, 555, 555, white)));
    
    objects.add(make_shared<box>(vec3(130, 0, 65), vec3(295, 165, 230), white));
    objects.add(make_shared<box>(vec3(265, 0, 295), vec3(430, 330, 460), white));

    return objects;
}

int main() {
    const int image_width = 800;
    const int image_height = 600;
    const int samples_per_pixel = 100;
    const int max_depth = 50;
    const auto aspect_ratio = double(image_width) / image_height;

    const vec3 background(0, 0, 0);

    ofstream fout("MyImage.ppm"); //文件输出流对象
    streambuf* pOld = cout.rdbuf(fout.rdbuf());

    vec3 eye_pos(278, 278, -800);
    vec3 lookat(278, 278, 0);
    vec3 vup(0, 1, 0);
    auto dist_to_focus = 10.0;
    auto aperture = 0.0;
    auto vfov = 40.0;

    camera cam(eye_pos, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);

	hittable_list world = cornell_box();

    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
    TGAImage image(image_width, image_height, TGAImage::RGB);

    for (int j = image_height - 1; j >= 0; --j) {
        std::cerr << "\r剩余进度: " << j << ' ' << std::flush;
        for (int i = 0; i < image_width; ++i) {
            vec3 color(0, 0, 0);
            for (int s = 0; s < samples_per_pixel; ++s) {
                auto u = (i + random_double()) / image_width;
                auto v = (j + random_double()) / image_height;
                ray r = cam.get_ray(u, v);
                //color += ray_color(r, world, max_depth);
                color += ray_color(r, background, world, max_depth);
            }
            color.write_color(i, j, image, samples_per_pixel);
        }
    }
    image.write_tga_file("Image.tga");
    std::cerr << "\nDone.\n";
}