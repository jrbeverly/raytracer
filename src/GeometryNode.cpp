#include "GeometryNode.hpp"

//---------------------------------------------------------------------------------------
GeometryNode::GeometryNode(
	const std::string &name, Primitive *prim, Material *mat)
	: SceneNode(name), m_material(mat), m_primitive(prim)
{
	m_nodeType = NodeType::GeometryNode;
}

void GeometryNode::setMaterial(Material *mat)
{
	// Obviously, there's a potential memory leak here.  A good solution
	// would be to use some kind of reference counting, as in the
	// C++ shared_ptr.  But I'm going to punt on that problem here.
	// Why?  Two reasons:
	// (a) In practice we expect the scene to be constructed exactly
	//     once.  There's no reason to believe that materials will be
	//     repeatedly overwritten in a GeometryNode.
	// (b) A ray tracer is a program in which you compute once, and
	//     throw away all your data.  A memory leak won't build up and
	//     crash the program.

	m_material = mat;
}

//---------------------------------------------------------------------------------------
bool GeometryNode::intersect(Ray &ray, Intersection &i) const
{
	// For this node we need to quickly convert it from the
	// world coordinate system to the model coordinate system
	// As such we need a new ray
	Ray model_ray = ray.transform(get_inverse());

	// Perform the insection on the primitive
	Intersection intersection;
	bool intersects = m_primitive->intersect(model_ray, intersection);

	// If intersecting, set the values
	if (intersects)
	{
		// Perform the transform on the intersection
		intersection.transform(get_transform());
		i.point = intersection.point;
		i.normal = intersection.normal;
		i.material = m_material;

		return true;
	}

	// Open the scene node intersection
	return SceneNode::intersect(ray, i);
}
