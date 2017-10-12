#ifndef QUAD_H
#define QUAD_H

#include "shape.h"
#include "mesh.h"

/**
 * @brief Infinite Quad whose normal is facing +Z
 */
class Quad : public Shape
{
public:
    Quad(const PropertyList &propList);
    virtual ~Quad();

    virtual void drawGeometry() const;

    virtual void attachShader(nanogui::GLShader* shader) {
        Shape::attachShader(shader);
        m_pMesh->attachShader(shader);
    }

    Vector2f size() const { return m_size; }

    virtual const Eigen::AlignedBox3f& AABB() const;

    virtual bool intersect(const Ray& ray, Hit& hit) const;

    /// Return a human-readable summary
    std::string toString() const {
        return tfm::format("Quad[\n"
                           "  size = %s,\n"
                           "  material = %s\n]",
                           m_size.toString(),
                           m_material ? indent(m_material->toString()) : std::string("null"));
    }

protected:
    Mesh* m_pMesh;
    Vector2f m_size;
};

#endif // QUAD_H
