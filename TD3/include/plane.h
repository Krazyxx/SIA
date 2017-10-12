#ifndef PLANE_H
#define PLANE_H

#include "shape.h"
#include "mesh.h"

/**
 * @brief Infinite plane whose normal is facing +Z
 */
class Plane : public Shape
{
public:
    Plane();
    Plane(const PropertyList &propList);
    virtual ~Plane();

    virtual void drawGeometry() const;

    virtual void attachShader(nanogui::GLShader* shader) {
        Shape::attachShader(shader);
        m_pMesh->attachShader(shader);
    }

    virtual const Eigen::AlignedBox3f& AABB() const;

    virtual bool intersect(const Ray& ray, Hit& hit) const;

    /// Return a human-readable summary
    std::string toString() const {
        return tfm::format("Plane[\n"
                           "  material = %s,\n]",
                           m_material ? indent(m_material->toString()) : std::string("null"));
    }

protected:
    Mesh* m_pMesh;
};

#endif // PLANE_H
