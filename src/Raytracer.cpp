#include <glm/ext.hpp>

#include <unistd.h>

#include "A4.hpp"
#include "MathHelper.hpp"
#include "PhongMaterial.hpp"

#define THREAD_RENDER_INIT 0
#define THREAD_RENDER_DONE 2

// Defines the area of the image that the thread will be rendering
struct ThreadRenderMap {

		// The image we are rendering to
		Image& img;

		// The width and height of the image
		int width, height;

		// The position where we start rendering
		int index;
		int num_thread;

		SceneNode* root;

		glm::mat4 inv_proj;
		const glm::vec3& eye;

		const glm::vec3& ambient;
		const std::list<Light *> & lights;

		int* progress;
		int* status;

		ThreadRenderMap(
			Image& m_img,
			int nthread,
			int ind,
			int w, int h,
			SceneNode* node,
			glm::mat4 mat,
			const glm::vec3 & e,
			const glm::vec3 & a,
			const std::list<Light *> & ls,
			int* prog, int* stat)
				: img(m_img),
				num_thread(nthread), index(ind),
				width(w), height(h),
				root(node), inv_proj(mat),
				eye(e), ambient(a),
				lights(ls), progress(prog), status(stat) { }
};

glm::vec3 a4_lighting(Ray& ray, Intersection intersection, const Light* light) {
		// Get details from intersection
		glm::vec3 point = intersection.point;
		glm::vec3 normal = intersection.normal;
		const PhongMaterial *material = dynamic_cast<const PhongMaterial*>(intersection.material);

		// We need to setup a couple of variables for the lights
		// For this we need to figure out how far from the surface point
		// the light soure is, as well as normalize direction vector
		glm::vec3 light_direction = light->position - point;
		double distance = glm::length(light_direction);
		light_direction = glm::normalize(light_direction);

		// Compute the attentuation coefficient for the light
		const double* falloff = light->falloff;
		double attentuation_coeff = falloff[0] + falloff[1] * distance + falloff[2] * (distance * distance);
		attentuation_coeff = 1.0 / attentuation_coeff;

		// Adjust the light colour based on this value
		glm::vec3 lcolour = attentuation_coeff * light->colour;

		// When the light hits the object we need to be able to compute the reflection
		// vector when the light impacts and bounces off the objects (reflects)
		glm::vec3 viewer = -light_direction;
		glm::vec3 reflected = viewer - 2 * glm::dot(viewer, normal) * normal;
		reflected = glm::normalize(reflected);

		// Factor in the eye value
		glm::vec3 camera_eye = glm::normalize(ray.origin - point);

		// To determine the brightness, we look at the two vectors
		// Specifically the normal of the surface and where the light is pointing
		double diffuse_brightness = std::max(0.0f, glm::dot(normal, light_direction));
		glm::vec3 diffuse = diffuse_brightness * material->diffuse() * lcolour;

		// Compute all the specular components
		double specular_brightness = 0;
		if (diffuse_brightness > 0) {
				double base = std::max(0.0f, glm::dot(camera_eye, reflected));
				specular_brightness = pow(base, material->shininess());
		} else {
				specular_brightness = 0.0;
		}
		glm::vec3 specular = specular_brightness * material->specular() * lcolour;

		glm::vec3 lighting = diffuse + specular;
		return lighting;
}

glm::vec3 trace_ray(Ray& ray, SceneNode* root, glm::vec3 & background, const glm::vec3 & ambient, const std::list<Light *> & lights, int recurse_level) {
    // Assume that the colour is the background
    glm::vec3 colour = background;
    Intersection inter;

		double epsilon = std::numeric_limits<double>::epsilon();
		double shift_epsilon = 0.01;

    bool intersected = root->intersect(ray, inter);
    if (intersected) {

        // We need to calculate the hit point
        // Note: We move the above just a little way from the object
        glm::vec3 hit = inter.point + shift_epsilon * inter.normal;

				// Get the material that we intersected with
        const PhongMaterial* material = dynamic_cast<const PhongMaterial*>(inter.material);
        colour = ambient * material->diffuse();

				// We have two iterate through the light sources with two things in mind
				// First: Can the intersection see a light source? (Shadows)
				// Second Lighting of the object
        for (Light* light : lights) {
						// Create the shadow ray (from point of hit to the light position)
						glm::vec3 lightIncident = glm::normalize(light->position - hit);
						Ray shadow_ray(hit, lightIncident);
						Intersection s_inter;

						if (root->intersect(shadow_ray, s_inter)) {
								double dist1 = glm::length(s_inter.point - shadow_ray.origin);
								double dist2 = glm::length(light->position - shadow_ray.origin);

								// Make sure that the intersection occurs before the light source
								// If this intersection does not, then there is a problem
								if (fabs(dist2 - dist1) > epsilon) {
										continue;
								}
						}

						// We now add the lighting compone for it
						colour = colour + a4_lighting(ray, inter, light);
        }

				// Added reflection
				glm::vec3 reflected_colour(0.0);
				if (recurse_level > 0) {
						Ray reflected_ray(hit, ray.direction - glm::dot(2 * ray.direction, inter.normal) * inter.normal);
						reflected_colour = trace_ray(reflected_ray, root, reflected_colour, ambient, lights, --recurse_level);
				}

				colour = colour + (1.0 / lights.size()) * reflected_colour * material->specular();
    }

		return colour;
}

double a4_math_clamp(double value, double min, double max) {
		return (value < min) ? min : (value > max) ? max : value;
}

void* RenderThread_Run(void* thread_args) {
		ThreadRenderMap renderMap = *static_cast<ThreadRenderMap*>(thread_args);

		// The initial y position is equal to the index of the thread
		int init_y = renderMap.index;
		int advance = renderMap.num_thread;

		// Dimensions of the image
		int width = renderMap.width;
		int height = renderMap.height;

		// The number of pixels this thread is likely to render
		int total_pixels = ceil((width * height) / renderMap.num_thread);
		int count = 0;

		glm::mat4 inv_proj = renderMap.inv_proj;
		glm::vec3 eye = renderMap.eye;

		// We now apply a pixel by pixel basis for rendering
		for (int y = init_y; y < height; y += advance) {
				for (int x = 0; x < width; x++) {

						// We need to get pixel onto projection plane
						glm::vec4 pixel(x, y, 0.0, 1.0);
						glm::vec4 pixel_world = inv_proj * pixel;

						// Take the world pixel and convert it into a ray direction
						glm::vec3 pworld = glm::vec3(pixel_world);
						glm::vec3 rayDir = glm::normalize(pworld - eye);

						// Create a ray
						Ray ray(eye, rayDir);

						// Create a three colour gradient background
						glm::vec3 bg_colour(1 - ((double)x / width), 1 - ((double)y / height), 0.0);

						// Launch the ray into the scene and determine the returned colour
						glm::vec3 colour(0.0, 0.0, 0.0);
						colour = trace_ray(ray, renderMap.root, bg_colour, renderMap.ambient, renderMap.lights, 1);

						// set RGB values in the image
						renderMap.img(x, y, 0) = a4_math_clamp(colour.r, 0.0, 1.0);
						renderMap.img(x, y, 1) = a4_math_clamp(colour.g, 0.0, 1.0);
						renderMap.img(x, y, 2) = a4_math_clamp(colour.b, 0.0, 1.0);

						// Increment the number of pixels we have handled
						count++;
						*renderMap.progress = (int) ((count * 100) / total_pixels);
				}
		}

		*renderMap.progress = 100;
		*renderMap.status = THREAD_RENDER_DONE;
}

void A4_Render(
		// What to render
		SceneNode * root,

		// Image to write to, set to a given width and height
		Image & image,

		// Viewing parameters
		const glm::vec3 & eye,
		const glm::vec3 & view,
		const glm::vec3 & up,
		double fovy,

		// Lighting parameters
		const glm::vec3 & ambient,
		const std::list<Light *> & lights
) {

	const int PROGRAM_FAILURE = 4;

  // Printing the details of the render system
  std::cout << "Calling A4_Render(\n" <<
		  "\t" << *root <<
          "\t" << "Image(width:" << image.width() << ", height:" << image.height() << ")\n"
          "\t" << "eye:  " << glm::to_string(eye) << std::endl <<
		  "\t" << "view: " << glm::to_string(view) << std::endl <<
		  "\t" << "up:   " << glm::to_string(up) << std::endl <<
		  "\t" << "fovy: " << fovy << std::endl <<
          "\t" << "ambient: " << glm::to_string(ambient) << std::endl <<
		  "\t" << "lights{" << std::endl;

	for(const Light * light : lights) {
		std::cout << "\t\t" <<  *light << std::endl;
	}
	std::cout << "\t}" << std::endl;
	std:: cout <<")" << std::endl;

	// Printing ends

	// Get the project matrix inverted
	double dist = glm::length(view);
	glm::mat4 unproj = a4_get_proj_inverse(image.width(), image.height(), fovy, dist, eye, view, up);

	size_t h = image.height();
	size_t w = image.width();

	// In order to speed up the process we are going to use multi-threading
	// This is fairly easily in this case, as we are only really being read-only
	// on the data

	const int num_threads = 8;

	int thread_progress[num_threads];
	int thread_status[num_threads];
	ThreadRenderMap* renderMap[num_threads];

	// We let each thread iterate over the entire x path then increment
	// forward based on the y.  The result is that an entire 'row' is being
	// handled by the threading manager
	for (int i = 0; i < num_threads; i++) {
			thread_progress[i] = 0;
			thread_status[i] = THREAD_RENDER_INIT;

			// Setup the details of the rendering platform
			ThreadRenderMap* map = new ThreadRenderMap(
					image,
					num_threads,
					i,
					w, h,
					root, unproj,
					eye, ambient,
					lights,
					&thread_progress[i],
					&thread_status[i]);

			renderMap[i] = map;
	}

	std:: cout << "Rendering with " << num_threads << " threads." << std::endl;

	pthread_t threads[num_threads];
	int ret = 0;
	for(int i = 0; i < num_threads; i++) {
			ret = pthread_create(&threads[i], NULL, RenderThread_Run, renderMap[i]);
			if (ret) {
					std::cerr << "Application had to abort:  pthread_Create failed with error code: " << ret << std::endl;
					exit(PROGRAM_FAILURE);
			}
	}

	std:: cout << "Starting rendering process" << std::endl;

	const int update_interval = 100 * 1000; // 100 milliseconds in microseconds

	bool is_processing = true;
	while (is_processing) {
			// Is the program done?
			bool is_done = true;
			for (int i = 0; i < num_threads; i++) {
					is_done = is_done && thread_status[i];
			}

			// output the progress
			int overall_progress = 0;
			for (int i = 0; i < num_threads; i++) {
					overall_progress += thread_progress[i];
			}
			overall_progress = overall_progress / num_threads;

			std:: cout << "Progress: " << overall_progress << "% \r" << std::flush;

			if (is_done) {
					is_processing = false;
			}

			// Wait a bit before printing
			usleep(update_interval);
	}

	std:: cout << "Rendering process complete" << std::endl;

	// Wait on all threads to finish
	for(int i = 0; i < num_threads; i++) {
			pthread_join(threads[i], NULL);
			delete renderMap[i];
	}

	std:: cout << "Scene rendered" << std::endl;
}
