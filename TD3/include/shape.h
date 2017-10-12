#ifndef SHAPE_H
#define SHAPE_H

#include "ray.h"
#include "common.h"
#include "material.h"
#include "object.h"
#include "light.h"

#include <nanogui/glutil.h>

/** represents a shape (geometry and material)
 */
class Shape : public Object
{
public:
    Shape() : m_transformation (Eigen::Affine3f::Identity().matrix()) {}

    Shape(const PropertyList&) : m_transformation(Eigen::Affine3f::Identity().matrix()) {}

    /** Draw the shape using OpenGL. */
    virtual void draw() const;

    /** Draw the geometry of the shape.
      * It must be implemented in the derived class. */
    virtual void drawGeometry() const {
        throw RTException("Shape::drawGeometry must be implemented in the derived class"); }

    /** Search the nearest intersection between the ray and the shape.
      * It must be implemented in the derived class. */
    virtual bool intersect(const Ray& ray, Hit& hit) const {
        throw RTException("Shape::intersect must be implemented in the derived class"); }

    /** Return the axis-aligned bounding box of the geometry.
      * It must be implemented in the derived class. */
    virtual const Eigen::AlignedBox3f& AABB() const {
        throw RTException("Shape::AABB must be implemented in the derived class"); }

    virtual void attachShader(nanogui::GLShader* shader) { m_shader = shader; }

    virtual const Material* material() const { return m_material; }
    virtual void setMaterial(const Material* mat) { m_material = mat; }

    virtual void setTransformation(const Eigen::Matrix4f& mat) { m_transformation = mat; }
    virtual const Transform& transformation() const { return m_transformation; }

    virtual bool isEmissive() const { return m_emissive; }
    virtual void setEmissive(bool b) { m_emissive = b; }
    virtual const Light* light() const { return m_light; }

    /// Register a child object (e.g. a material) with the shape
    virtual void addChild(Object *child);
    /// Register a parent object (e.g. a light) with the shape
    virtual void setParent(Object *parent);

    /// \brief Return the type of object provided by this instance
    EClassType getClassType() const { return EShape; }

    virtual std::string toString() const {
        throw RTException("Shape::toString must be implemented in the derived class"); }

protected:
    nanogui::GLShader* m_shader = nullptr;
    const Material* m_material = nullptr;
    const Light* m_light = nullptr;
    Transform m_transformation;
    bool m_emissive = false;
};

#endif
