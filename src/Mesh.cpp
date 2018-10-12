#include <iostream>
#include <fstream>

#include <glm/ext.hpp>

// #include "cs488-framework/ObjFileDecoder.hpp"
#include "Mesh.hpp"

Mesh::Mesh(const std::string &fname)
	: m_vertices(), m_faces(), m_bounding(NULL)
{
	std::string code;
	double vx, vy, vz;
	size_t s1, s2, s3;

	std::ifstream ifs(fname.c_str());
	while (ifs >> code)
	{
		if (code == "v")
		{
			ifs >> vx >> vy >> vz;

			m_vertices.push_back(glm::vec3(vx, vy, vz));
		}
		else if (code == "f")
		{
			ifs >> s1 >> s2 >> s3;
			m_faces.push_back(Triangle(s1 - 1, s2 - 1, s3 - 1));
		}
	}

	m_bounding = getBounding(m_vertices);
}

// Gets a bounding box
NonhierBox *Mesh::getBounding(const std::vector<glm::vec3> &vertices)
{
	if (m_vertices.size() == 0)
	{
		return new NonhierBox(glm::vec3(0.0), 0.0);
	}

	// Gets the maximum possible x, y, z values for vertices
	glm::vec3 min = m_vertices[0], max = m_vertices[0];
	for (glm::vec3 vertex : vertices)
	{
		min.x = std::min(min.x, vertex.x);
		max.x = std::min(max.x, vertex.x);

		min.y = std::min(min.y, vertex.y);
		max.y = std::min(max.y, vertex.y);

		min.z = std::min(min.z, vertex.z);
		max.z = std::min(max.z, vertex.z);
	}

	// Algorithm to quickly construct a bounding sphere
	double size = std::max(max.x - min.x, max.y - min.y);
	size = std::max((float)size, max.z - min.z);

	// Bounding box
	NonhierBox *box = new NonhierBox(min, size);
	return box;
}

// Intersection for the mesh
bool Mesh::intersect(const Ray &ray, Intersection &intersection) const
{
	bool intersects = false;

	// Checks the bounding box for quick mesh
	Intersection binter;
	bool bintersects = m_bounding->intersect(ray, binter);

	if (!bintersects)
	{
		return false;
	}

	double epsilon = std::numeric_limits<double>::epsilon();

	// Attempted design but failed causing a mesh of non-sense
	// I mean the cow wasn't even cow-like it was.... deformed..
	double prev_impact = std::numeric_limits<double>::infinity();

	// Iterate through the faces
	for (Triangle face : m_faces)
	{
		// Get the vertices of the face
		glm::vec3 v0 = m_vertices[face.get(0)];
		glm::vec3 v1 = m_vertices[face.get(1)];
		glm::vec3 v2 = m_vertices[face.get(2)];

		// Get the normal
		glm::vec3 normal = glm::cross(v1 - v0, v2 - v0);
		normal = glm::normalize(normal);

		// Compute the denominator and determine if the ray
		// intersects the polygon face (if 0 then no.)
		double denom = glm::dot(normal, ray.direction);
		if (fabs(denom) < epsilon)
		{
			continue;
		}

		// Get the gamma shift we will use for advancing the ray
		double impact = glm::dot(v0 - ray.origin, normal) / denom;
		if (impact < 0 || prev_impact < impact)
		{
			continue;
		}

		// The intersection point
		glm::vec3 ipoint = ray.origin + impact * ray.direction;

		// Determine if the value is within the model or not (visible to us)
		bool outside = false;
		for (int i = 0; i < 4; i++)
		{
			int next = (i == 3) ? 0 : i + 1;

			glm::vec3 p0 = m_vertices[face.get(i)];
			glm::vec3 p1 = m_vertices[face.get(next)];

			glm::vec3 cp = glm::cross(p1 - p0, ipoint - p0);
			double result = glm::dot(cp, normal);

			if (result < epsilon)
			{
				outside = true;
				break;
			}
		}

		// Skip if the value is outside
		if (outside)
		{
			continue;
		}

		// Set values
		intersects = true;
		prev_impact = impact;

		intersection.point = ipoint;
		intersection.normal = normal;
	}

	return intersects;
}

std::ostream &operator<<(std::ostream &out, const Mesh &mesh)
{
	out << "mesh {";
	/*

  for( size_t idx = 0; idx < mesh.m_verts.size(); ++idx ) {
  	const MeshVertex& v = mesh.m_verts[idx];
  	out << glm::to_string( v.m_position );
	if( mesh.m_have_norm ) {
  	  out << " / " << glm::to_string( v.m_normal );
	}
	if( mesh.m_have_uv ) {
  	  out << " / " << glm::to_string( v.m_uv );
	}
  }

*/
	out << "}";
	return out;
}
