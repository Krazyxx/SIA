#ifndef SIRE_AREALIGHT_H
#define SIRE_AREALIGHT_H

#include "light.h"
#include "quad.h"

class AreaLight : public Light
{
public:
    AreaLight(const PropertyList &propList);

    /// Average intensity (considering the area light as a point light)
    virtual Color3f intensity(const Point3f& x) const;

    /// Intensity from point \param x at the world space position \param y on the area light
    Color3f intensity(const Point3f& x, const Point3f& y) const;

    void loadTexture(const std::string& filename);

    /// \returns the position of the camera
    inline const Point3f position() const { return m_shape->transformation().getMatrix().col(3).head<3>(); }
    /// \returns the light direction, i.e., the -z axis of the frame
    Vector3f direction() const { return m_shape->transformation().getMatrix().col(2).head<3>(); }
    Vector3f direction(const Point3f& x, float* dist = 0) const { return direction(); }
    /// \returns the first tangent axis of the light plane
    Vector3f uVec() const { return m_shape->transformation().getMatrix().col(1).head<3>(); }
    /// \returns the second tangent axis of the light plane
    Vector3f vVec() const { return m_shape->transformation().getMatrix().col(0).head<3>(); }
    //------------------------------------------------------------

    Vector2f size() const { return m_shape->size(); }

    void draw();

    Shape* shape() { return m_shape; }

    /// Return a human-readable summary
    std::string toString() const;

    virtual void addChild(Object *child);

protected:
    Quad* m_shape = nullptr;
    Bitmap* m_texture = nullptr;
};

#endif // AREALIGHT_H

