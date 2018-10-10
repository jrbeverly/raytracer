#pragma once

#include <vector>
#include <iosfwd>
#include <string>

#include <glm/glm.hpp>

#include "Primitive.hpp"

struct Triangle
{
	size_t v1;
	size_t v2;
	size_t v3;

	Triangle( size_t pv1, size_t pv2, size_t pv3 )
		: v1( pv1 )
		, v2( pv2 )
		, v3( pv3 )
	{}


	size_t get(int index) {
			switch(index) {
					case 0: return v1;
					case 1: return v2;
					case 2: return v3;
					default: return -1;
			}
	}
};

// A polygonal mesh.
class Mesh : public Primitive {
public:
  Mesh( const std::string& fname );

	virtual bool intersect(const Ray& ray, Intersection& intersection) const;
	NonhierBox* getBounding(const std::vector<glm::vec3>& vertices);

private:
	std::vector<glm::vec3> m_vertices;
	std::vector<Triangle> m_faces;
	NonhierBox* m_bounding;

    friend std::ostream& operator<<(std::ostream& out, const Mesh& mesh);
};
