
#ifndef SPHERE_H
#define SPHERE_H

#include "shape.h"
#include "mesh.h"

#include <Eigen/Core>
#include <map>

/** Represents a sphere
 */
class Sphere : public Shape
{
public:

    Sphere(float radius);
    Sphere(const PropertyList &propList);
    virtual ~Sphere();

    virtual void drawGeometry() const;

    virtual bool intersect(const Ray& ray, Hit& hit) const;

    virtual const Eigen::AlignedBox3f& AABB() const;

    virtual void attachShader(nanogui::GLShader* shader) {
        Shape::attachShader(shader);
        m_pMesh->attachShader(shader);
    }

    float radius() const { return m_radius; }

    /// Return a human-readable summary
    std::string toString() const {
        return tfm::format(
            "Sphere[\n"
            "  frame = %s,\n"
            "  radius = %f\n"
            "  material = %s,\n]"
            "]", indent(m_transformation.toString(),10),
                 m_radius,
                 m_material ? indent(m_material->toString()) : std::string("null"));
    }

protected:
    static void subdivide(int i1, int i2, int i3, std::vector<Point3f> &sphere_points,
                          std::map< std::pair<int,int>, int> &dico, std::vector<int> &indices, const unsigned int depth);
    void init();

    float m_radius;
    Mesh* m_pMesh;
};

#endif
