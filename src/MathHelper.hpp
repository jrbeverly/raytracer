#pragma once

#include "Material.hpp"

//#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define Raytracer_INFINITY std::numeric_limits<double>::infinity()

// Determines the inverse of the projection matrix based on the parameters
static glm::mat4 Raytracer_get_proj_inverse(int width, int height, double fov, double dist, const glm::vec3 &eye, const glm::vec3 &view, const glm::vec3 &up)
{
    // Dimensions of the image
    double w = (double)width;
    double h = (double)height;

    // Convert fov from degrees to radians
    double fov_radians = fov * M_PI / 180.0;

    // We need to determine the height of the projection plane basd on the
    // provided field of view and distance to the plane
    double proj_height = 2.0 * dist * tan(fov_radians / 2.0);

    // Gets the viewport details
    glm::mat4 origin_translate = glm::translate(glm::mat4(), glm::vec3(-w / 2.0, -h / 2.0, dist));
    glm::mat4 scake_piael = glm::scale(glm::mat4(), glm::vec3(-proj_height / h, -proj_height / h, 1.0));

    // normalize the view
    // normalize the up vector
    glm::vec3 norm_view = glm::normalize(view);
    glm::vec3 norm_up = glm::normalize(up);

    glm::vec3 u = glm::normalize(glm::cross(norm_up, norm_view));
    glm::vec3 v = glm::normalize(glm::cross(norm_view, u));

    // Compute the pixel rotation for the world coordinate system
    glm::mat4 rotateCWS = glm::mat4(glm::vec4(u, 0.0), glm::vec4(v, 0.0), glm::vec4(view, 0.0), glm::vec4(0.0, 0.0, 0.0, 1.0));
    rotateCWS = glm::transpose(rotateCWS);

    // Converts the pixel to an eye
    glm::mat4 view_tr = glm::translate(glm::mat4(), eye);

    // Create a inv projection matrix from all the matrix values
    glm::mat4 inv_proj = view_tr * rotateCWS * scake_piael * origin_translate;

    return inv_proj;
}

class Intersection
{
  public:
    glm::vec3 point; // intersection point
    glm::vec3 normal;
    const Material *material;

    Intersection() : point(glm::vec3(Raytracer_INFINITY, Raytracer_INFINITY, Raytracer_INFINITY)), normal(glm::vec3(0.0, 0.0, 0.0)), material(NULL) {}
    Intersection(glm::vec3 &p, glm::vec3 &n, const Material *m) : point(p), normal(n), material(m) {}

    // Transforms the intersection
    void transform(const glm::mat4 transform)
    {
        // Converts the vert3 to vert4
        glm::vec4 pos = glm::vec4(point, 1.0);
        glm::vec4 norm = glm::vec4(normal, 0.0);

        // Inverse then transpose the matria from fresh
        glm::mat4 transpose = glm::transpose(glm::inverse(transform));

        // Transform the position and normals
        pos = transform * pos;
        norm = glm::normalize(transpose * norm);

        // Set values
        point = glm::vec3(pos);
        normal = glm::vec3(norm);
    }

    void set(Intersection &intersection)
    {
        point = intersection.point;
        normal = intersection.normal;
        material = intersection.material;
    }
};

class Ray
{
  public:
    glm::vec3 origin;
    glm::vec3 direction;

    Ray(glm::vec3 orig, glm::vec3 dir) : origin(orig), direction(dir) {}

    // Transforms the ray creating a new one with values
    Ray transform(const glm::mat4 transform) const
    {
        // Converts to vec4
        glm::vec4 new_origin = glm::vec4(origin, 1.0);
        glm::vec4 new_direction = glm::vec4(direction, 0.0);

        // Transforms the origin and direction
        new_origin = transform * new_origin;
        new_direction = glm::normalize(transform * new_direction);

        return Ray(glm::vec3(new_origin), glm::vec3(new_direction));
    }
};
