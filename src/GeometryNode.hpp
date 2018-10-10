#pragma once

#include "SceneNode.hpp"
#include "Primitive.hpp"
#include "Material.hpp"

class GeometryNode : public SceneNode {
public:
	GeometryNode( const std::string & name, Primitive *prim,
		Material *mat = nullptr );

	void setMaterial( Material *material );

	// Checks intersection of geometry node
	virtual bool intersect(Ray& ray, Intersection& i) const;

	Material *m_material;
	Primitive *m_primitive;
};
