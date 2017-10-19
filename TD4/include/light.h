#ifndef LIGHT_H
#define LIGHT_H

#include "object.h"
#include "bitmap.h"

#include <nanogui/glutil.h>

class Light : public Object
{
public:   
    Light(const Color3f& a_intensity) : m_intensity(a_intensity) {}

    /** \returns the direction (unit vector) from x to the light,
    * optionally, returns the distance to the light in *dist
    */
    virtual Vector3f direction(const Point3f& x, float* dist = 0) const = 0;

    /** \returns the intensity emitted at x */
    virtual Color3f intensity(const Point3f& x) const = 0;

    /** Draw the light using OpenGL.
      * It must be implemented in the derived class. */
    virtual void draw() { throw RTException("Light::drawGeometry must be implemented in the derived class"); }
    virtual void attachShader(nanogui::GLShader* shader) { m_shader = shader; }

    EClassType getClassType() const { return ELight; }

protected:
    nanogui::GLShader* m_shader = nullptr;

    Color3f m_intensity;
};

typedef std::vector<Light*> LightList;

#endif // LIGHT_H
