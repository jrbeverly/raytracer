#include "Primitive.hpp"
#include <iostream>

Primitive::~Primitive()
{
}

Sphere::~Sphere()
{
}

bool Sphere::intersect(const Ray &ray, Intersection &intersection) const
{
    NonhierSphere sphere(glm::vec3(0.0, 0.0, 0.0), 1.0);
    return sphere.intersect(ray, intersection);
}

Cube::~Cube()
{
}

bool Cube::intersect(const Ray &ray, Intersection &intersection) const
{
    NonhierBox box(glm::vec3(0.0, 0.0, 0.0), 1.0);
    return box.intersect(ray, intersection);
}

NonhierSphere::~NonhierSphere()
{
}

bool NonhierSphere::intersect(const Ray &ray, Intersection &intersection) const
{
    // Have to compute the intersection of a sphere

    double roots[2];
    glm::vec3 position = ray.origin - m_pos;

    // Get the values to do polyroots
    // We can determine this by Line/Sphere intersection test (see wikipedia)
    double a = glm::dot(ray.direction, ray.direction);
    double b = glm::dot(2 * ray.direction, position);
    double c = glm::dot(position, position) - m_radius * m_radius;

    // Get the number of roots
    size_t num_roots = quadraticRoots(a, b, c, roots);

    // We now have the roots and can do work on them so
    // The three cases we have are: no roots, 1 root, 2 roots
    // Therefore
    // No roots = no intersection
    // 1 root = hits on the edge
    // 2 root = we hit the sphere (then leave the sphere)
    // So we care about '2 root', we take the entry point and done

    if (num_roots > 0)
    {
        double min = std::min<double>(roots[0], roots[1]);
        double impact = 0;

        if (num_roots == 1)
        {
            impact = roots[0];
        }
        else
        {
            impact = (min < 0) ? std::max<double>(roots[0], roots[1]) : min;
        }

        // if negative it is behind camera (not in view)
        // so there is no intersection
        if (impact < 0)
        {
            return false;
        }

        intersection.point = ray.origin + impact * ray.direction;
        intersection.normal = glm::normalize(intersection.point - m_pos);

        return true;
    }

    return false;
}

NonhierBox::~NonhierBox()
{
}

bool NonhierBox::intersect(const Ray &ray, Intersection &intersection) const
{
    // Falling back on using the design that was used by the mesh checker
    // Consider the box as if it was a 3d polygon, then perform the check
    // based on this.

    // List all the points of the box
    glm::vec3 vertices[8] = {
        glm::vec3(m_pos.x, m_pos.y, m_pos.z),                           //0
        glm::vec3(m_pos.x + m_size, m_pos.y, m_pos.z),                  //1
        glm::vec3(m_pos.x, m_pos.y + m_size, m_pos.z),                  //2
        glm::vec3(m_pos.x + m_size, m_pos.y + m_size, m_pos.z),         //3
        glm::vec3(m_pos.x, m_pos.y, m_pos.z + m_size),                  //4
        glm::vec3(m_pos.x + m_size, m_pos.y, m_pos.z + m_size),         //5
        glm::vec3(m_pos.x, m_pos.y + m_size, m_pos.z + m_size),         //6
        glm::vec3(m_pos.x + m_size, m_pos.y + m_size, m_pos.z + m_size) //7
    };

    // List all the faces by vertex positions
    glm::vec3 face_points[6][4] = {
        {vertices[0], vertices[1], vertices[3], vertices[2]},
        {vertices[2], vertices[3], vertices[7], vertices[6]},
        {vertices[1], vertices[5], vertices[7], vertices[3]},
        {vertices[0], vertices[2], vertices[6], vertices[4]},
        {vertices[4], vertices[6], vertices[7], vertices[5]},
        {vertices[0], vertices[4], vertices[5], vertices[1]},
    };

    // Compute the normals of all faces
    glm::vec3 face_normals[6];
    for (int i = 0; i < 6; i++)
    {
        // Get point vectors
        glm::vec3 dist1 = face_points[i][2] - face_points[i][0];
        glm::vec3 dist2 = face_points[i][1] - face_points[i][0];

        // Cross product for normal
        glm::vec3 cp = glm::cross(dist1, dist2);
        face_normals[i] = glm::normalize(cp);
    }

    double prev_impact = std::numeric_limits<double>::infinity();
    double epsilon = std::numeric_limits<double>::epsilon();
    bool intersects = false;

    // for each of the faces we do a polygon check
    for (int f = 0; f < 6; f++)
    {
        double num = glm::dot(face_normals[f], ray.direction);
        if (fabs(num) < epsilon)
        {
            continue;
        }

        glm::vec3 face_point = face_points[f][0];
        double impact = glm::dot(face_point - ray.origin, face_normals[f]) / num;

        // If negative we cannot see
        if (impact < 0)
        {
            continue;
        }

        // If it is not closer then previous, then ignore
        if (prev_impact < impact)
        {
            continue;
        }

        glm::vec3 point = ray.origin + impact * ray.direction;

        // We know have to compare the new point of impact (intersection)
        // that we go against the normals of the actual cube
        bool outside = false;
        for (int i = 0; i < 4; i++)
        {
            int next = (i == 3) ? 0 : i + 1;

            glm::vec3 cp = glm::cross(face_points[f][i] - face_points[f][next], point - face_points[f][next]);
            double result = glm::dot(cp, face_normals[f]);

            if (result < 0)
            {
                outside = true;
                break;
            }
        }

        // Skip if the value is not good
        if (outside)
        {
            continue;
        }

        prev_impact = impact;
        intersects = true;

        intersection.point = point;
        intersection.normal = face_normals[f];
    }

    return intersects;
}
