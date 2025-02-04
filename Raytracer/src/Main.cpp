#include "Ray.h"
#include "Vec3f.h"
#include "Constants.h"
#include "Scene.h"
#include "Camera.h"
#include <fstream>
#include <sstream>
#include <algorithm>

Vec3f random_in_unit_sphere()
{
	// Function that generates a random vector within a unit length sphere tangent to a hit point
	while (true)
	{
		Vec3f p = Vec3f::random(-1.0f , 1.0f);
		if (p.squared_length() >= 1)
			continue;
		return p;
	}
}

Vec3f random_unit_vector()
{
	return unit_vector(random_in_unit_sphere());
}

void write_color(std::ostringstream &oss , const Vec3f &color)
{
	// Reescales the value acording to the number of samples per pixel and display it
	// It also uses a "gamma2" correction
	float r = color.r();
	float g = color.g();
	float b = color.b();

	float scale = 1.0f / SAMPLES;
	r = std::sqrt(scale * r);
	g = std::sqrt(scale * g);
	b = std::sqrt(scale * b);

	// Writes the scaled value to [0,255] color component.
	oss << static_cast<int>(256 * std::clamp(r , 0.0f , 0.999f)) << ' '
		<< static_cast<int>(256 * std::clamp(g , 0.0f , 0.999f)) << ' '
		<< static_cast<int>(256 * std::clamp(b , 0.0f , 0.999f)) << '\n';
}

Vec3f get_color(const Ray &ray , const SceneObject &world , int depth)
{
	// Returns the RGB vector for a given ray 
	// Shaded based on a matte surface using the "Lambertian distribution" if it hits a sphere, 
	// blended background if not
	HitRecord record;

	if (depth <= 0) // If we've exceeded the ray bounce limit, no more light is gathered
		return Vec3f(0.0f , 0.0f , 0.0f);
	else if (world.hit(ray , 0.001f , infinity , record))
	{
		Vec3f target = record.point + record.normal + random_unit_vector();
		return 0.5f * get_color(Ray(record.point , target - record.point) , world , depth - 1);
	}
	else
	{
		Vec3f unitVector = unit_vector(ray.direction());
		float t = 0.5f * (unitVector.y() + 1.0f);
		return (1.0f - t) * Vec3f(1.0f , 1.0f , 1.0f) + t * Vec3f(0.5f , 0.7f , 1.0f);
	}
}

int main()
{
	std::srand(std::time(nullptr)); //Seeding std::rand()

	std::ofstream outputFile{"output.ppm"};
	std::ostringstream os{};

	ObjectList world;
	world.add(std::make_shared<Sphere>(Vec3f(0.0f , 0.0f , -1.0f) , 0.5f));
	world.add(std::make_shared<Sphere>(Vec3f(0.0f , -100.5f , -1.0f) , 100.0f));

	Camera cam;

	os << "P3\n" << CANVAS_WIDTH << " " << CANVAS_HEIGHT << "\n255\n";

	for (int j{CANVAS_HEIGHT - 1} ; j >= 0 ; j--)
	{
		std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;

		for (int i{} ; i < CANVAS_WIDTH ; i++)
		{
			Vec3f pixelColor(0.0f , 0.0f , 0.0f);

			for (int k{} ; k < SAMPLES ; k++)
			{
				float u = (i + random_float()) / (CANVAS_WIDTH - 1.0f);
				float v = (j + random_float()) / (CANVAS_HEIGHT - 1.0f);
				Ray r = cam.get_ray(u , v);
				pixelColor += get_color(r , world , MAX_DEPTH);
			}
			write_color(os , pixelColor);
		}
	}

	std::cerr << "\nDone.\n";

	if (outputFile)
		outputFile << os.str();
	else
		std::cout << "Could not open output file..." << std::endl;

	outputFile.close();

	return 0;
}