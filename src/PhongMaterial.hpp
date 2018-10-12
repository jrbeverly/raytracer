#pragma once

#include <glm/glm.hpp>

#include "Material.hpp"

class PhongMaterial : public Material
{
public:
  PhongMaterial(const glm::vec3 &kd, const glm::vec3 &ks, double shininess);
  virtual ~PhongMaterial();

  const glm::vec3 diffuse() const { return m_kd; }
  const glm::vec3 specular() const { return m_ks; }
  double shininess() const { return m_shininess; }

private:
  glm::vec3 m_kd;
  glm::vec3 m_ks;

  double m_shininess;
};
