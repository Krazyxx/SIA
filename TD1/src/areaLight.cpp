#include "areaLight.h"
#include "material.h"
#include "glPrimitives.h"

AreaLight::AreaLight(const PropertyList &propList)
    : Light(propList.getColor("intensity", Color3f(1.f)))
{
    std::string texturePath = propList.getString("texture","");
    if(texturePath.size()>0){
        filesystem::path filepath = getFileResolver()->resolve(texturePath);
        loadTexture(filepath.str());
    }
}

void AreaLight::addChild(Object *obj) {
    switch (obj->getClassType()) {
        case EShape:
            if (m_shape)
                throw RTException("AreaLight: tried to register multiple shape instances!");
            m_shape = static_cast<Quad *>(obj);
            m_shape->setEmissive(true);
            break;
        default:
            throw RTException("AreaLight::addChild(<%s>) is not supported!",
                                classTypeName(obj->getClassType()));
    }
}

Color3f AreaLight::intensity(const Point3f& x) const
{
    Vector3f dir = x-position();
    float d2 = dir.squaredNorm();
    return std::max(0.f,dir.normalized().dot(direction())) * m_intensity / d2;
}

Color3f AreaLight::intensity(const Point3f &x, const Point3f &y) const {
    Vector3f dir = (x-y);
    float d2 = dir.squaredNorm();
    Color3f intensity = m_intensity / d2;
    if(m_texture) {
        // TODO
    }
    return std::max(0.f,dir.normalized().dot(direction())) * intensity;
}

void AreaLight::loadTexture(const std::string &filename) {
    m_texture = new Bitmap(filename);
}

void AreaLight::draw()
{
}

std::string AreaLight::toString() const {
    return tfm::format(
                "AreaLight[\n"
                "  intensity = %s\n"
                "  frame = %s\n"
                "  size = %f\n"
                "]", m_intensity.toString(),
                indent(m_shape->transformation().toString(),10),
                size().toString());
}

REGISTER_CLASS(AreaLight, "areaLight")
